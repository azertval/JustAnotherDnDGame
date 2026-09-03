// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_animated_tiles.cpp
 * @brief Tests unitaires des tuiles animées (`LOT-46` TACHE-05) : horloge partagée par asset,
 *        mise en phase, exclusion des combinaisons non supportées. Sans GPU.
 */

#include <memory>
#include <optional>

#include <gtest/gtest.h>

#include "Core/Ecs/AnimationClip.h"
#include "Core/Ecs/Components/Animation.h"
#include "Core/Ecs/Systems/AnimationSystem.h"
#include "Core/Levels/TileType.h"
#include "HMI/Graphics/AnimationCatalog.h"
#include "HMI/Graphics/RenderMode.h"
#include "HMI/Graphics/SkinCatalog.h"
#include "HMI/Graphics/TextureAtlas.h"
#include "HMI/Graphics/TileAppearance.h"
#include "HMI/Graphics/TileSkinTag.h"

namespace {

constexpr int TILE = hmi::TextureAtlas::TILE_SIZE;
constexpr float STEP = 1.0f / 60.0f;

// Jeu de clips d'un asset anime simple (eau) : un seul clip boucle de 4 images, 0,1 s chacune.
std::shared_ptr<const core::ClipSet> waterClips() {
    auto clips = std::make_shared<core::ClipSet>();
    core::AnimationClip wave;
    wave.name = "wave";
    wave.frames = {0, 1, 2, 3};
    wave.frameDuration = 0.1f;
    wave.endMode = core::ClipEndMode::Loop;
    clips->addClip(wave);
    return clips;
}

core::Animation freshWaterAnimation() {
    core::Animation animation;
    animation.clips = waterClips();
    return animation;
}

hmi::AnimationDescription waterDescription() {
    hmi::AnimationDescription description;
    description.frameWidth = TILE;
    description.frameHeight = TILE;
    description.clips = *waterClips();
    return description;
}

int storageA = 0;
int storageB = 0;

}  // namespace

/**
 * @brief Un skin animé change de région au fil des pas, en restant sur le même rang de la
 * spritesheet : c'est la preuve que l'horloge d'animation atteint bien la tuile, et que le
 * découpage parcourt la planche horizontalement comme le veut la convention d'asset.
 * \castest{<b>Un skin animé produit des régions différentes au fil des pas, sur un même
 * rang.</b><br/>
 * \tcat Unitaire · Tuiles animées<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(AnimatedTilesTest, SkinAnimeProduitDesRegionsDifferentesAuFilDesPas) {
    core::Animation animation = freshWaterAnimation();
    const hmi::AnimationDescription description = waterDescription();

    const core::AtlasRegion first =
        hmi::AnimationCatalog::currentFrameRegion(description, animation);
    for (int i = 0; i < 6; ++i) {  // 6 pas = 0,1 s : une image complete a la cadence du clip.
        core::advanceAnimation(animation, STEP);
    }
    const core::AtlasRegion second =
        hmi::AnimationCatalog::currentFrameRegion(description, animation);

    EXPECT_NE(first.x, second.x);
    EXPECT_EQ(first.y, second.y);  // spritesheet a un seul rang.
}

/**
 * @brief Un skin **non animé** rend la même région à chaque appel : sans image courante
 * renseignée, la résolution retombe sur l'image entière. La grande majorité des skins n'est pas
 * animée, et ne doit rien payer au mécanisme d'animation.
 * \castest{<b>Un skin non animé produit une région constante d'un appel à l'autre.</b><br/>
 * \tcat Unitaire · Tuiles animées<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(AnimatedTilesTest, SkinNonAnimeProduitUneRegionConstante) {
    // Sans animatedFrame renseigne (asset non anime, TACHE-03) : resolveTileAppearance retombe
    // sur l'image entiere, quel que soit le nombre d'appels.
    hmi::SkinCatalog catalog;
    catalog.assign("defaut", core::TileType::Solid,
                   hmi::SkinEntry{"stone.png", hmi::SkinMode::Single});

    hmi::SceneTextures textures;
    textures.atlas = &storageA;
    textures.atlasWidth = TILE * 5;
    textures.atlasHeight = TILE * 5;
    textures.skins = {
        hmi::SkinTexture{"stone.png", std::nullopt, &storageB, TILE, TILE, std::nullopt}};
    textures.skinCatalog = &catalog;
    textures.skinSet = "defaut";

    const hmi::TileSkinTag tag{core::TileType::Solid, 0};
    const hmi::TileAppearance first =
        hmi::resolveTileAppearance(hmi::RenderMode::Texture, core::AtlasRegion{}, &tag, textures)
            .value();
    const hmi::TileAppearance second =
        hmi::resolveTileAppearance(hmi::RenderMode::Texture, core::AtlasRegion{}, &tag, textures)
            .value();

    EXPECT_EQ(first.region.x, 0);
    EXPECT_EQ(first.region.x, second.region.x);
    EXPECT_EQ(first.region.width, TILE);
}

/**
 * @brief Quand une image courante est renseignée, la résolution l'utilise telle quelle plutôt que
 * la planche entière : c'est le point de jonction entre l'horloge d'animation, partagée par asset,
 * et le rendu d'une tuile donnée.
 * \castest{<b>Un skin animé utilise la région de l'image courante, pas la planche entière.</b><br/>
 * \tcat Unitaire · Tuiles animées<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(AnimatedTilesTest, SkinAnimeUtiliseLaRegionCouranteDeAnimatedFrame) {
    hmi::SkinCatalog catalog;
    catalog.assign("defaut", core::TileType::Solid,
                   hmi::SkinEntry{"water.png", hmi::SkinMode::Single});

    const core::AtlasRegion currentFrame{TILE * 2, 0, TILE, TILE};  // 3e image de la spritesheet.
    hmi::SceneTextures textures;
    textures.atlas = &storageA;
    textures.atlasWidth = TILE * 5;
    textures.atlasHeight = TILE * 5;
    textures.skins = {
        hmi::SkinTexture{"water.png", std::nullopt, &storageB, TILE * 4, TILE, currentFrame}};
    textures.skinCatalog = &catalog;
    textures.skinSet = "defaut";

    const hmi::TileSkinTag tag{core::TileType::Solid, 0};
    const hmi::TileAppearance appearance =
        hmi::resolveTileAppearance(hmi::RenderMode::Texture, core::AtlasRegion{}, &tag, textures)
            .value();

    EXPECT_EQ(appearance.region.x, currentFrame.x);
    EXPECT_EQ(appearance.region.width, TILE);
}

/**
 * @brief Deux tuiles du même asset restent en phase à chaque pas, image par image : c'est la
 * garantie de déterminisme sur laquelle repose l'horloge unique par asset. Deux cases d'eau
 * voisines déphasées se verraient immédiatement, et trahiraient une progression par instance.
 * \castest{<b>Deux tuiles du même asset affichent la même image à chaque pas.</b><br/>
 * \tcat Unitaire · Tuiles animées<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(AnimatedTilesTest, MiseEnPhaseDeuxTuilesDuMemeTypeAffichentLaMemeImageAuMemePas) {
    // Deux "tuiles" independantes, meme jeu de clips partage : si chacune progressait pour son
    // propre compte (au lieu d'une horloge unique par asset), elles resteraient identiques tant
    // que la sequence de pas est identique -- ce test verifie precisement cette garantie de
    // determinisme, condition necessaire a la mise en phase reelle (horloge unique, LOT-46
    // TACHE-05).
    core::Animation tileOne = freshWaterAnimation();
    core::Animation tileTwo = freshWaterAnimation();
    const hmi::AnimationDescription description = waterDescription();

    for (int step = 0; step < 37; ++step) {
        core::advanceAnimation(tileOne, STEP);
        core::advanceAnimation(tileTwo, STEP);
        const core::AtlasRegion regionOne =
            hmi::AnimationCatalog::currentFrameRegion(description, tileOne);
        const core::AtlasRegion regionTwo =
            hmi::AnimationCatalog::currentFrameRegion(description, tileTwo);
        ASSERT_EQ(regionOne.x, regionTwo.x) << "pas " << step;
        ASSERT_EQ(tileOne.frameIndex, tileTwo.frameIndex) << "pas " << step;
    }
}

/**
 * @brief Le mode à raccords automatiques exclut l'animation : la planche y sert déjà à décrire les
 * seize voisinages possibles, ses cases ne peuvent pas servir en même temps d'images successives.
 * \castest{<b>Le mode à raccords automatiques exclut l'animation.</b><br/>
 * \tcat Unitaire · Tuiles animées<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(AnimatedTilesTest, Bitmask16ExclutLAnimation) {
    EXPECT_TRUE(hmi::animationExcludedForTile(hmi::SkinMode::Bitmask16, core::TileType::Solid));
}


/**
 * @brief Une tuile pleine en mode image simple, elle, **peut** être animée : c'est le cas
 * nominal (eau, lave, bloc scintillant). Le pendant positif des deux exclusions ci-dessus, sans
 * lequel elles pourraient être trop larges sans que rien ne le signale.
 * \castest{<b>Une tuile pleine en mode image simple n'exclut pas l'animation.</b><br/>
 * \tcat Unitaire · Tuiles animées<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * }
 */
TEST(AnimatedTilesTest, SingleSansSilhouetteNExclutPasLAnimation) {
    EXPECT_FALSE(hmi::animationExcludedForTile(hmi::SkinMode::Single, core::TileType::Solid));
    EXPECT_FALSE(hmi::animationExcludedForTile(hmi::SkinMode::Single, core::TileType::Block));
}
