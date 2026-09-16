// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

/**
 * @file test_tile_skin_resolution.cpp
 * @brief Tests unitaires de la résolution d'apparence par skin (LOT-42, EX-EDIT-042, EX-EDIT-025).
 */

#include <optional>

#include <gtest/gtest.h>

#include "Core/Ecs/Components/Sprite.h"
#include "Core/Levels/TileType.h"
#include "HMI/Graphics/MissingTexture.h"
#include "HMI/Graphics/RenderMode.h"
#include "HMI/Graphics/SkinCatalog.h"
#include "HMI/Graphics/TextureAtlas.h"
#include "HMI/Graphics/TileAppearance.h"
#include "HMI/Graphics/TileAutotile.h"
#include "HMI/Graphics/TileSkinTag.h"

namespace {

// Textures factices : la resolution ne compare que des identites et des dimensions, jamais des
// pixels -- c'est ce qui la rend verifiable sans GPU (EX-NFR-004).
int atlasStorage = 0;
int missingStorage = 0;
int stoneStorage = 0;
int crateStorage = 0;

constexpr int TILE = hmi::TextureAtlas::TILE_SIZE;

/// Catalogue de reference : un type a raccords, un type a image unique, un type non skinne.
hmi::SkinCatalog referenceCatalog() {
    hmi::SkinCatalog catalog;
    catalog.assign("foret", core::TileType::Solid,
                   hmi::SkinEntry{"stone.png", hmi::SkinMode::Bitmask16});
    catalog.assign("foret", core::TileType::Block,
                   hmi::SkinEntry{"crate.png", hmi::SkinMode::Single});
    return catalog;
}

/// Textures de scene : atlas, damier, et les deux skins du catalogue de reference, charges.
hmi::SceneTextures texturesWith(const hmi::SkinCatalog& catalog) {
    hmi::SceneTextures textures;
    textures.atlas = &atlasStorage;
    textures.atlasWidth = 80;
    textures.atlasHeight = 80;
    textures.missing = &missingStorage;
    textures.missingWidth = hmi::MISSING_TEXTURE_SIZE;
    textures.missingHeight = hmi::MISSING_TEXTURE_SIZE;
    // Aucun des deux types n'a de silhouette : leur variante est l'image d'origine (maskType vide).
    textures.skins = {
        hmi::SkinTexture{"stone.png", std::nullopt, &stoneStorage, TILE * 4, TILE * 4},
        hmi::SkinTexture{"crate.png", std::nullopt, &crateStorage, TILE, TILE},
    };
    textures.skinCatalog = &catalog;
    textures.skinSet = "foret";
    return textures;
}

}  // namespace

/**
 * @brief Le mode Physique ignore totalement les skins.
 * \castest{<b>Le mode Physique rend la region d'atlas meme quand un skin est assigne.</b><br/>
 * \tcat Unitaire · Resolution de skin<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Resoudre une tuile Solid skinnee, en mode Physique.<br/>
 * \tattendu La source est l'atlas et la region est celle passee en entree.
 * }
 */
TEST(TileSkinResolutionTest, ModePhysiqueIgnoreLesSkins) {
    const hmi::SkinCatalog catalog = referenceCatalog();
    const hmi::SceneTextures textures = texturesWith(catalog);
    const hmi::TileSkinTag tag{core::TileType::Solid, 0};
    const core::AtlasRegion physical = hmi::TextureAtlas::tile(0, 2);

    // Le mode Physique reste la reference de lecture des collisions : l'habillage ne doit jamais
    // s'y inviter, sous peine de perdre le seul rendu ou geometrie affichee = geometrie simulee.
    const hmi::TileAppearance appearance =
        hmi::resolveTileAppearance(hmi::RenderMode::Physique, physical, &tag, textures).value();

    EXPECT_EQ(appearance.source, hmi::AppearanceSource::Atlas);
    EXPECT_EQ(appearance.region.x, physical.x);
    EXPECT_EQ(appearance.region.y, physical.y);
}

/**
 * @brief Un skin en mode single rend l'image entiere.
 * \castest{<b>Un skin en mode image unique rend l'image entiere.</b><br/>
 * \tcat Unitaire · Resolution de skin<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Resoudre une tuile Block, assignee a un skin en mode single, en mode Texture.<br/>
 * \tattendu La source est le skin et la region couvre une case entiere a l'origine.
 * }
 */
TEST(TileSkinResolutionTest, SkinSingleRendLImageEntiere) {
    const hmi::SkinCatalog catalog = referenceCatalog();
    const hmi::SceneTextures textures = texturesWith(catalog);
    // Le voisinage ne doit avoir aucun effet en mode single : on en met un non nul pour le
    // verifier.
    const hmi::TileSkinTag tag{core::TileType::Block, hmi::NEIGHBOR_UP | hmi::NEIGHBOR_LEFT};

    const hmi::TileAppearance appearance =
        hmi::resolveTileAppearance(hmi::RenderMode::Texture, hmi::TextureAtlas::tile(0, 0), &tag,
                                   textures)
            .value();

    EXPECT_EQ(appearance.source, hmi::AppearanceSource::Skin);
    EXPECT_EQ(appearance.region.x, 0);
    EXPECT_EQ(appearance.region.y, 0);
    EXPECT_EQ(appearance.region.width, TILE);
    EXPECT_EQ(appearance.region.height, TILE);
    EXPECT_EQ(textures.textureFor(appearance), &crateStorage);
}

/**
 * @brief Un skin en mode bitmask16 choisit sa case selon le voisinage.
 * \castest{<b>Un skin a raccords choisit sa case selon le voisinage solide.</b><br/>
 * \tcat Unitaire · Resolution de skin<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Resoudre une tuile Solid entouree de solide, puis la meme isolee.<br/>
 * \tattendu Les deux rendent des regions differentes de la meme planche.
 * }
 */
TEST(TileSkinResolutionTest, SkinBitmaskChoisitSaCase) {
    const hmi::SkinCatalog catalog = referenceCatalog();
    const hmi::SceneTextures textures = texturesWith(catalog);

    const hmi::TileSkinTag inside{core::TileType::Solid, hmi::NEIGHBOR_UP | hmi::NEIGHBOR_RIGHT |
                                                             hmi::NEIGHBOR_DOWN |
                                                             hmi::NEIGHBOR_LEFT};
    const hmi::TileSkinTag alone{core::TileType::Solid, 0};

    const hmi::TileAppearance insideAppearance =
        hmi::resolveTileAppearance(hmi::RenderMode::Texture, hmi::TextureAtlas::tile(0, 0), &inside,
                                   textures)
            .value();
    const hmi::TileAppearance aloneAppearance =
        hmi::resolveTileAppearance(hmi::RenderMode::Texture, hmi::TextureAtlas::tile(0, 0), &alone,
                                   textures)
            .value();

    EXPECT_EQ(insideAppearance.source, hmi::AppearanceSource::Skin);
    EXPECT_EQ(aloneAppearance.source, hmi::AppearanceSource::Skin);
    EXPECT_EQ(textures.textureFor(insideAppearance), &stoneStorage);

    // Interieur plein = derniere case de la planche ; isolee = premiere.
    const hmi::AutotileCell fullCell = hmi::autotileRepresentativeCell();
    EXPECT_EQ(insideAppearance.region.x, fullCell.column * TILE);
    EXPECT_EQ(insideAppearance.region.y, fullCell.row * TILE);
    EXPECT_EQ(aloneAppearance.region.x, 0);
    EXPECT_EQ(aloneAppearance.region.y, 0);

    // C'est tout l'objet des raccords : un bord ne s'affiche pas comme un interieur.
    EXPECT_NE(insideAppearance.region.x, aloneAppearance.region.x);
}

/**
 * @brief Un type non skinne retombe sur le damier.
 * \castest{<b>Un type de tuile sans skin assigne retombe sur le damier.</b><br/>
 * \tcat Unitaire · Resolution de skin<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Resoudre une tuile Danger, absente du catalogue, en mode Texture.<br/>
 * \tattendu La source est le damier, couvert en entier.
 * }
 */
TEST(TileSkinResolutionTest, TypeNonSkinneRetombeSurLeDamier) {
    const hmi::SkinCatalog catalog = referenceCatalog();
    const hmi::SceneTextures textures = texturesWith(catalog);
    const hmi::TileSkinTag tag{core::TileType::Danger, 0};

    // Etat normal du programme d'habillage tant que tous les types ne sont pas habilles : le
    // damier signale ce qui reste a faire plutot que de le masquer.
    const hmi::TileAppearance appearance =
        hmi::resolveTileAppearance(hmi::RenderMode::Texture, hmi::TextureAtlas::tile(0, 0), &tag,
                                   textures)
            .value();

    EXPECT_EQ(appearance.source, hmi::AppearanceSource::MissingTexture);
    EXPECT_EQ(appearance.region.width, hmi::MISSING_TEXTURE_SIZE);
    EXPECT_EQ(textures.textureFor(appearance), &missingStorage);
}

/**
 * @brief Un skin assigne mais non charge retombe sur le damier.
 * \castest{<b>Un skin assigne dont la texture n'est pas chargee retombe sur le damier.</b><br/>
 * \tcat Unitaire · Resolution de skin<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Assigner un asset absent des textures chargees, puis resoudre en mode Texture.<br/>
 * \tattendu La source est le damier, sans acces hors bornes.
 * }
 */
TEST(TileSkinResolutionTest, SkinNonChargeRetombeSurLeDamier) {
    hmi::SkinCatalog catalog = referenceCatalog();
    catalog.assign("foret", core::TileType::Danger,
                   hmi::SkinEntry{"jamais_charge.png", hmi::SkinMode::Single});
    const hmi::SceneTextures textures = texturesWith(catalog);
    const hmi::TileSkinTag tag{core::TileType::Danger, 0};

    // Fichier absent, illisible ou refuse par le contrat d'asset : le TextureCache a deja
    // journalise l'avertissement, la resolution se contente de degrader.
    const hmi::TileAppearance appearance =
        hmi::resolveTileAppearance(hmi::RenderMode::Texture, hmi::TextureAtlas::tile(0, 0), &tag,
                                   textures)
            .value();

    EXPECT_EQ(appearance.source, hmi::AppearanceSource::MissingTexture);
}

/**
 * @brief Sans catalogue, tout retombe sur le damier.
 * \castest{<b>En l'absence de catalogue de skins, tout retombe sur le damier.</b><br/>
 * \tcat Unitaire · Resolution de skin<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Resoudre une tuile Solid avec des textures sans catalogue.<br/>
 * \tattendu La source est le damier, sans dereferencement de pointeur nul.
 * }
 */
TEST(TileSkinResolutionTest, SansCatalogueToutRetombeSurLeDamier) {
    const hmi::SkinCatalog catalog = referenceCatalog();
    hmi::SceneTextures textures = texturesWith(catalog);
    textures.skinCatalog = nullptr;
    const hmi::TileSkinTag tag{core::TileType::Solid, 0};

    const hmi::TileAppearance appearance =
        hmi::resolveTileAppearance(hmi::RenderMode::Texture, hmi::TextureAtlas::tile(0, 0), &tag,
                                   textures)
            .value();

    EXPECT_EQ(appearance.source, hmi::AppearanceSource::MissingTexture);
}

/**
 * @brief Un index de skin invalide ne lit jamais hors des bornes.
 * \castest{<b>Un index de skin invalide retombe sur le damier sans lecture hors bornes.</b><br/>
 * \tcat Unitaire · Resolution de skin<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Interroger les textures avec une apparence portant un index hors bornes.<br/>
 * \tattendu La texture et les dimensions rendues sont celles du damier.
 * }
 */
TEST(TileSkinResolutionTest, IndexDeSkinInvalideBorne) {
    const hmi::SkinCatalog catalog = referenceCatalog();
    const hmi::SceneTextures textures = texturesWith(catalog);

    const hmi::TileAppearance broken{hmi::AppearanceSource::Skin, core::AtlasRegion{0, 0, 16, 16},
                                     99};

    EXPECT_EQ(textures.textureFor(broken), &missingStorage);
    EXPECT_EQ(textures.widthFor(broken), hmi::MISSING_TEXTURE_SIZE);
    EXPECT_EQ(textures.heightFor(broken), hmi::MISSING_TEXTURE_SIZE);
}
