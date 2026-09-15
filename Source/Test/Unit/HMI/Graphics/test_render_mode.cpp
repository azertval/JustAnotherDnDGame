// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

/**
 * @file test_render_mode.cpp
 * @brief Tests unitaires de la bascule Physique/Texture (LOT-41, EX-REN-046).
 */

#include <gtest/gtest.h>

#include "Core/Ecs/Components/Sprite.h"
#include "Core/Ecs/Components/Transform.h"
#include "Core/Ecs/World.h"
#include "HMI/Graphics/ComposedScene.h"
#include "HMI/Graphics/MissingTexture.h"
#include "HMI/Graphics/QuadRecorder.h"
#include "HMI/Graphics/RenderMode.h"
#include "HMI/Graphics/TextureAtlas.h"
#include "HMI/Graphics/TileAppearance.h"
#include "HMI/Input/EditorKeyBindings.h"
#include "HMI/Input/GameKeyBindings.h"

namespace {

// Textures factices : la composition ne fait que comparer des identites (cf. test_quad_recorder).
int atlasStorage = 0;
int missingStorage = 0;

/// Textures de reference des tests : atlas 80x80, damier a la taille d'une case.
hmi::SceneTextures testTextures() {
    hmi::SceneTextures textures;
    textures.atlas = &atlasStorage;
    textures.atlasWidth = 80;
    textures.atlasHeight = 80;
    textures.missing = &missingStorage;
    textures.missingWidth = hmi::MISSING_TEXTURE_SIZE;
    textures.missingHeight = hmi::MISSING_TEXTURE_SIZE;
    return textures;
}

/// Ajoute une entite tuile (Transform + Sprite) au monde, a une region d'atlas donnee.
void addTile(core::World& world, float x, float y, const core::AtlasRegion& region) {
    const core::Entity entity = world.createEntity();
    world.addComponent(entity,
                       core::Transform{core::Vector2{x, y}, core::Vector2{1.0f, 1.0f}, 0.0f});
    core::Sprite sprite;
    sprite.region = region;
    world.addComponent(entity, sprite);
}

/// Compose la scene de reference (trois tuiles + un personnage) dans un mode donne.
void composeReference(hmi::ComposedScene& scene, core::World& world, hmi::RenderMode mode) {
    scene.clear();
    hmi::composeWorldSprites(scene, world, mode, testTextures(), 0.0f);
    scene.sort();
}

/// Peuple la scene de reference : trois tuiles et un personnage sur son propre calque.
void buildReferenceWorld(core::World& world) {
    addTile(world, 0.0f, 0.0f, core::AtlasRegion{0, 0, 16, 16});
    addTile(world, 1.0f, 0.0f, core::AtlasRegion{16, 0, 16, 16});
    addTile(world, 2.0f, 0.0f, core::AtlasRegion{32, 16, 16, 16});

    const core::Entity player = world.createEntity();
    world.addComponent(player,
                       core::Transform{core::Vector2{1.0f, 5.0f}, core::Vector2{0.4f, 0.8f}, 0.0f});
    core::Sprite sprite;
    sprite.region = core::AtlasRegion{0, 64, 16, 16};
    world.addComponent(player, sprite);
    world.addComponent(player, hmi::RenderLayerTag{hmi::RenderLayer::Player});
}

}  // namespace

/**
 * @brief Le nom persisté fait l'aller-retour sans perte, pour les deux modes.
 * \castest{<b>Le nom persiste du mode fait l'aller-retour sans perte.</b><br/>
 * \tcat Unitaire · Render Mode<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Convertir chaque mode en nom, puis le nom en mode.<br/>
 * \tattendu Le mode d'origine est retrouve.
 * }
 */
TEST(RenderModeTest, AllerRetourDuNomPersiste) {
    EXPECT_EQ(hmi::renderModeFromName(hmi::renderModeName(hmi::RenderMode::Physique)),
              hmi::RenderMode::Physique);
    EXPECT_EQ(hmi::renderModeFromName(hmi::renderModeName(hmi::RenderMode::Texture)),
              hmi::RenderMode::Texture);
}

/**
 * @brief Une préférence absente, vide ou corrompue retombe sur `Texture`, le défaut unique de
 *        toutes les configurations de build.
 * \castest{<b>Une preference absente ou invalide retombe sur le mode Texture.</b><br/>
 * \tcat Unitaire · Render Mode<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Convertir une chaine vide, une chaine inconnue et une chaine tronquee.<br/>
 * \tattendu Les trois donnent le mode par defaut, qui est Texture.
 * }
 */
TEST(RenderModeTest, PreferenceInvalideRetombeSurLeDefaut) {
    EXPECT_EQ(hmi::DEFAULT_RENDER_MODE, hmi::RenderMode::Texture);
    EXPECT_EQ(hmi::renderModeFromName(""), hmi::DEFAULT_RENDER_MODE);
    EXPECT_EQ(hmi::renderModeFromName("plein-ecran"), hmi::DEFAULT_RENDER_MODE);
    EXPECT_EQ(hmi::renderModeFromName("phys"), hmi::DEFAULT_RENDER_MODE);
}

/**
 * @brief La lecture d'un nom persisté est insensible à la casse : une préférence éditée à la main
 *        ne doit pas réinitialiser silencieusement le mode.
 * \castest{<b>La lecture du nom persiste est insensible a la casse.</b><br/>
 * \tcat Unitaire · Render Mode<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Convertir « PHYSIQUE » et « Texture ».<br/>
 * \tattendu Les modes correspondants sont retrouves.
 * }
 */
TEST(RenderModeTest, LectureInsensibleALaCasse) {
    EXPECT_EQ(hmi::renderModeFromName("PHYSIQUE"), hmi::RenderMode::Physique);
    EXPECT_EQ(hmi::renderModeFromName("Texture"), hmi::RenderMode::Texture);
}

/**
 * @brief En mode Physique, le résolveur rend **exactement** la région d'atlas attendue pour chaque
 *        type de tuile : le mode de référence est inchangé.
 * \castest{<b>En mode Physique, le resolveur rend la region d'atlas de chaque type de
 * tuile.</b><br/>
 * \tcat Unitaire · Render Mode<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Pour chaque type de tuile, resoudre l'apparence en mode Physique.<br/>
 * \tattendu La region est celle de l'atlas et la source est l'atlas.
 * }
 */
TEST(RenderModeTest, ModePhysiqueRendLaRegionDAtlas) {
    // Balaye toute la grille de tuiles de l'atlas : c'est l'ensemble des regions que
    // `hmi::regionForTile` peut rendre, quel que soit le `core::TileType` demande.
    for (int row = 0; row < hmi::TextureAtlas::TILES_PER_SIDE; ++row) {
        for (int column = 0; column < hmi::TextureAtlas::TILES_PER_SIDE; ++column) {
            const core::AtlasRegion expected = hmi::TextureAtlas::tile(column, row);
            const hmi::TileAppearance appearance =
                hmi::resolveTileAppearance(hmi::RenderMode::Physique, expected, nullptr,
                                           testTextures())
                    .value();

            EXPECT_EQ(appearance.source, hmi::AppearanceSource::Atlas);
            EXPECT_EQ(appearance.region.x, expected.x);
            EXPECT_EQ(appearance.region.y, expected.y);
            EXPECT_EQ(appearance.region.width, expected.width);
            EXPECT_EQ(appearance.region.height, expected.height);
        }
    }
}

/**
 * @brief En mode Texture, une entité **sans marque d'habillage** retombe sur le damier de repli :
 *        c'est le cas du personnage et des aides d'édition, jamais habillés.
 * \castest{<b>En mode Texture, une entite sans marque d'habillage retombe sur le damier.</b><br/>
 * \tcat Unitaire · Render Mode<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Resoudre l'apparence en mode Texture sans marque d'habillage.<br/>
 * \tattendu La source est le damier et la region couvre le damier entier.
 * }
 */
TEST(RenderModeTest, ModeTextureRetombeSurLeDamier) {
    for (int row = 0; row < hmi::TextureAtlas::TILES_PER_SIDE; ++row) {
        for (int column = 0; column < hmi::TextureAtlas::TILES_PER_SIDE; ++column) {
            const hmi::TileAppearance appearance =
                hmi::resolveTileAppearance(hmi::RenderMode::Texture,
                                           hmi::TextureAtlas::tile(column, row), nullptr,
                                           testTextures())
                    .value();

            EXPECT_EQ(appearance.source, hmi::AppearanceSource::MissingTexture);
            EXPECT_EQ(appearance.region.x, 0);
            EXPECT_EQ(appearance.region.y, 0);
            EXPECT_EQ(appearance.region.width, hmi::MISSING_TEXTURE_SIZE);
            EXPECT_EQ(appearance.region.height, hmi::MISSING_TEXTURE_SIZE);
        }
    }
}

/**
 * @brief Non-régression : en mode Physique, la scène de référence produit exactement les
 *        primitives d'avant le lot — mêmes positions, mêmes régions, mêmes calques, même ordre.
 * \castest{<b>Non-regression : le mode Physique produit les primitives d'avant le lot.</b><br/>
 * \tcat Unitaire · Render Mode<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Composer la scene de reference en mode Physique.<br/>2. Inspecter la liste
 * capturee.<br/>
 * \tattendu Les tuiles precedent le personnage, sur la texture d'atlas, avec les regions
 * d'origine.
 * }
 */
TEST(RenderModeTest, ModePhysiqueNonRegression) {
    core::World world;
    buildReferenceWorld(world);

    hmi::ComposedScene scene;
    composeReference(scene, world, hmi::RenderMode::Physique);

    hmi::QuadRecorder recorder;
    recorder.record(scene);

    ASSERT_EQ(recorder.size(), 4u);
    EXPECT_TRUE(recorder.isLayerOrderRespected()) << recorder.describe();
    EXPECT_EQ(recorder.countOnLayer(hmi::RenderLayer::Tile), 3);
    EXPECT_EQ(recorder.countOnLayer(hmi::RenderLayer::Player), 1);
    EXPECT_EQ(recorder.countWithTexture(testTextures().atlas), 4);
    EXPECT_EQ(recorder.statistics().batches, 1);

    // Regions d'origine : la deuxieme tuile echantillonne bien la colonne 1 de l'atlas 80x80.
    EXPECT_FLOAT_EQ(recorder.quads()[1].sprite.u0, 16.0f / 80.0f);
    EXPECT_FLOAT_EQ(recorder.quads()[1].sprite.u1, 32.0f / 80.0f);
}

/**
 * @brief En mode Texture, toutes les primitives sont liées au damier, mais **la géométrie est
 *        identique** au mode Physique : la bascule compare l'habillage au physique, pas deux
 *        tailles de tuiles.
 * \castest{<b>Le mode Texture lie le damier sans changer la geometrie composee.</b><br/>
 * \tcat Unitaire · Render Mode<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Composer la scene de reference dans les deux modes.<br/>2. Comparer textures,
 * positions et tailles.<br/>
 * \tattendu Les textures different, les positions et tailles sont identiques.
 * }
 */
TEST(RenderModeTest, ModeTextureConserveLaGeometrie) {
    core::World world;
    buildReferenceWorld(world);

    hmi::ComposedScene scene;
    composeReference(scene, world, hmi::RenderMode::Physique);
    const std::vector<hmi::ComposedQuad> physique = scene.quads();

    composeReference(scene, world, hmi::RenderMode::Texture);
    const std::vector<hmi::ComposedQuad> texture = scene.quads();

    ASSERT_EQ(physique.size(), texture.size());
    for (std::size_t i = 0; i < physique.size(); ++i) {
        EXPECT_EQ(texture[i].texture, testTextures().missing);
        EXPECT_EQ(texture[i].layer, physique[i].layer);
        EXPECT_FLOAT_EQ(texture[i].sprite.x, physique[i].sprite.x);
        EXPECT_FLOAT_EQ(texture[i].sprite.y, physique[i].sprite.y);
        EXPECT_FLOAT_EQ(texture[i].sprite.width, physique[i].sprite.width);
        EXPECT_FLOAT_EQ(texture[i].sprite.height, physique[i].sprite.height);
    }
}

/**
 * @brief La bascule est sans effet rémanent : `Physique` → `Texture` → `Physique` restitue
 *        exactement la première liste.
 * \castest{<b>La bascule Physique -> Texture -> Physique est sans effet remanent.</b><br/>
 * \tcat Unitaire · Render Mode<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Composer en Physique, puis en Texture, puis de nouveau en Physique.<br/>2.
 * Comparer la premiere et la troisieme liste.<br/>
 * \tattendu Les deux listes Physique sont identiques.
 * }
 */
TEST(RenderModeTest, BasculeSansEffetRemanent) {
    core::World world;
    buildReferenceWorld(world);

    hmi::ComposedScene scene;
    composeReference(scene, world, hmi::RenderMode::Physique);
    const std::vector<hmi::ComposedQuad> before = scene.quads();

    composeReference(scene, world, hmi::RenderMode::Texture);
    composeReference(scene, world, hmi::RenderMode::Physique);
    const std::vector<hmi::ComposedQuad> after = scene.quads();

    ASSERT_EQ(before.size(), after.size());
    for (std::size_t i = 0; i < before.size(); ++i) {
        EXPECT_EQ(after[i].texture, before[i].texture);
        EXPECT_EQ(after[i].layer, before[i].layer);
        EXPECT_EQ(after[i].sortOrder, before[i].sortOrder);
        EXPECT_FLOAT_EQ(after[i].sprite.x, before[i].sprite.x);
        EXPECT_FLOAT_EQ(after[i].sprite.u0, before[i].sprite.u0);
        EXPECT_FLOAT_EQ(after[i].sprite.v1, before[i].sprite.v1);
    }
}

/**
 * @brief `F8` n'apparaît dans **aucune** table de remappage : ni parmi les touches par défaut du
 *        jeu, ni parmi celles de l'éditeur.
 *
 * La garantie est structurelle : `hmi::qtKeyToHmiKey` (couche Qt) ne traduit pas `Qt::Key_F8`, la
 * touche ne peut donc jamais atteindre `hmi::InputState` ni être capturée par l'écran de
 * remappage (`EX-CTRL-012`). Ce test verrouille l'autre moitié — qu'aucune valeur par défaut ne
 * l'utilise déjà.
 * \castest{<b>F8 n'apparait dans aucune table de remappage.</b><br/>
 * \tcat Unitaire · Render Mode<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Parcourir les touches par defaut de toutes les actions de jeu et d'edition.<br/>
 * \tattendu Aucune n'est le code virtuel de F8.
 * }
 */
TEST(RenderModeTest, F8HorsDesTablesDeRemappage) {
    constexpr int VIRTUAL_KEY_F8 = 0x77;  // VK_F8 (Win32), code que porterait une touche liee

    for (int action = 0; action < hmi::GAME_ACTION_COUNT; ++action) {
        const hmi::Key key = hmi::GameKeyBindings::defaultKey(static_cast<hmi::GameAction>(action));
        EXPECT_NE(static_cast<int>(key), VIRTUAL_KEY_F8);
    }
    for (int action = 0; action < hmi::EDITOR_ACTION_COUNT; ++action) {
        const hmi::Key key =
            hmi::EditorKeyBindings::defaultKey(static_cast<hmi::EditorAction>(action));
        EXPECT_NE(static_cast<int>(key), VIRTUAL_KEY_F8);
    }
}
