// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

/**
 * @file test_texture_resolution.cpp
 * @brief Tests unitaires de la priorité de résolution surcharge > skin > damier (LOT-45,
 * `EX-EDIT-043`), de l'affichage isolé sans repli (`LOT-51`, `EX-EDIT-044`) et de
 * `hmi::textureOverrideAt`.
 */

#include <optional>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "Core/Ecs/Components/Sprite.h"
#include "Core/Levels/GridPosition.h"
#include "Core/Levels/Level.h"
#include "Core/Levels/TileType.h"
#include "HMI/Graphics/MissingTexture.h"
#include "HMI/Graphics/RenderMode.h"
#include "HMI/Graphics/SkinCatalog.h"
#include "HMI/Graphics/TextureAtlas.h"
#include "HMI/Graphics/TileAppearance.h"
#include "HMI/Graphics/TileSkinTag.h"
#include "HMI/Graphics/TileVisuals.h"

namespace {

// Textures factices : la resolution ne compare que des identites et des dimensions (EX-NFR-004).
int atlasStorage = 0;
int missingStorage = 0;
int stoneStorage = 0;
int doorRedStorage = 0;

constexpr int TILE = hmi::TextureAtlas::TILE_SIZE;

hmi::SkinCatalog referenceCatalog() {
    hmi::SkinCatalog catalog;
    catalog.assign("foret", core::TileType::Solid,
                   hmi::SkinEntry{"stone.png", hmi::SkinMode::Single});
    return catalog;
}

hmi::SceneTextures texturesWith(const hmi::SkinCatalog& catalog) {
    hmi::SceneTextures textures;
    textures.atlas = &atlasStorage;
    textures.atlasWidth = 80;
    textures.atlasHeight = 80;
    textures.missing = &missingStorage;
    textures.missingWidth = hmi::MISSING_TEXTURE_SIZE;
    textures.missingHeight = hmi::MISSING_TEXTURE_SIZE;
    textures.skins = {hmi::SkinTexture{"stone.png", std::nullopt, &stoneStorage, TILE, TILE}};
    textures.objects = {
        hmi::SkinTexture{"door_red.png", std::nullopt, &doorRedStorage, TILE, TILE}};
    textures.skinCatalog = &catalog;
    textures.skinSet = "foret";
    return textures;
}

}  // namespace

/**
 * @brief Une surcharge chargée est prioritaire sur le skin du type, même si le type a un skin
 * assigné différent.
 * \castest{<b>Une surcharge chargée est prioritaire sur le skin du type.</b><br/>
 * \tcat Unitaire · Résolution de texture<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Résoudre une tuile Solid (skinnée) portant une surcharge chargée, en mode
 * Texture.<br/> \tattendu La source est la surcharge, pas le skin du type.
 * }
 */
TEST(TextureResolutionTest, SurchargeChargeePrioritaireSurLeSkin) {
    const hmi::SkinCatalog catalog = referenceCatalog();
    const hmi::SceneTextures textures = texturesWith(catalog);
    const hmi::TileSkinTag tag{core::TileType::Solid, 0, std::string{"door_red.png"}};

    const hmi::TileAppearance appearance =
        hmi::resolveTileAppearance(hmi::RenderMode::Texture, hmi::TextureAtlas::tile(0, 0), &tag,
                                   textures)
            .value();

    EXPECT_EQ(appearance.source, hmi::AppearanceSource::Override);
    EXPECT_EQ(appearance.region.width, TILE);
    EXPECT_EQ(appearance.region.height, TILE);
    EXPECT_EQ(textures.textureFor(appearance), &doorRedStorage);
}

/**
 * @brief Une surcharge dont l'asset n'est pas chargé retombe directement sur le damier, jamais
 * silencieusement sur le skin du type.
 * \castest{<b>Une surcharge introuvable retombe sur le damier, pas sur le skin.</b><br/>
 * \tcat Unitaire · Résolution de texture<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Résoudre une tuile Solid skinnée portant une surcharge dont l'asset n'est pas
 * chargé.<br/>
 * \tattendu La source est le damier.
 * }
 */
TEST(TextureResolutionTest, SurchargeIntrouvableRetombeSurLeDamierPasSurLeSkin) {
    const hmi::SkinCatalog catalog = referenceCatalog();
    const hmi::SceneTextures textures = texturesWith(catalog);
    const hmi::TileSkinTag tag{core::TileType::Solid, 0, std::string{"asset_jamais_charge.png"}};

    const hmi::TileAppearance appearance =
        hmi::resolveTileAppearance(hmi::RenderMode::Texture, hmi::TextureAtlas::tile(0, 0), &tag,
                                   textures)
            .value();

    EXPECT_EQ(appearance.source, hmi::AppearanceSource::MissingTexture);
    EXPECT_EQ(textures.textureFor(appearance), &missingStorage);
}

/**
 * @brief Sans surcharge, la résolution retombe sur le comportement du skin (LOT-42), inchangé.
 * \castest{<b>Sans surcharge, le skin du type s'applique normalement.</b><br/>
 * \tcat Unitaire · Résolution de texture<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Résoudre une tuile Solid skinnée, sans surcharge.<br/>
 * \tattendu La source est le skin du type.
 * }
 */
TEST(TextureResolutionTest, SansSurchargeLeSkinSAppliqueNormalement) {
    const hmi::SkinCatalog catalog = referenceCatalog();
    const hmi::SceneTextures textures = texturesWith(catalog);
    const hmi::TileSkinTag tag{core::TileType::Solid, 0};

    const hmi::TileAppearance appearance =
        hmi::resolveTileAppearance(hmi::RenderMode::Texture, hmi::TextureAtlas::tile(0, 0), &tag,
                                   textures)
            .value();

    EXPECT_EQ(appearance.source, hmi::AppearanceSource::Skin);
    EXPECT_EQ(textures.textureFor(appearance), &stoneStorage);
}

/**
 * @brief Une surcharge sur un type sans skin assigné s'affiche quand même (elle ne dépend pas du
 * catalogue).
 * \castest{<b>Une surcharge s'affiche même sur un type sans skin.</b><br/>
 * \tcat Unitaire · Résolution de texture<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Résoudre une tuile Danger (non skinnée) portant une surcharge chargée.<br/>
 * \tattendu La source est la surcharge.
 * }
 */
TEST(TextureResolutionTest, SurchargeSAppliqueMemeSansSkinDuType) {
    const hmi::SkinCatalog catalog = referenceCatalog();
    const hmi::SceneTextures textures = texturesWith(catalog);
    const hmi::TileSkinTag tag{core::TileType::Danger, 0, std::string{"door_red.png"}};

    const hmi::TileAppearance appearance =
        hmi::resolveTileAppearance(hmi::RenderMode::Texture, hmi::TextureAtlas::tile(0, 0), &tag,
                                   textures)
            .value();

    EXPECT_EQ(appearance.source, hmi::AppearanceSource::Override);
}

/**
 * @brief Une surcharge sans catalogue de skins chargé s'affiche quand même.
 * \castest{<b>Une surcharge s'affiche même sans catalogue de skins.</b><br/>
 * \tcat Unitaire · Résolution de texture<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Résoudre une tuile portant une surcharge, catalogue absent.<br/>
 * \tattendu La source est la surcharge, sans déréférencement de pointeur nul.
 * }
 */
TEST(TextureResolutionTest, SurchargeSAppliqueMemeSansCatalogue) {
    const hmi::SkinCatalog catalog = referenceCatalog();
    hmi::SceneTextures textures = texturesWith(catalog);
    textures.skinCatalog = nullptr;
    const hmi::TileSkinTag tag{core::TileType::Solid, 0, std::string{"door_red.png"}};

    const hmi::TileAppearance appearance =
        hmi::resolveTileAppearance(hmi::RenderMode::Texture, hmi::TextureAtlas::tile(0, 0), &tag,
                                   textures)
            .value();

    EXPECT_EQ(appearance.source, hmi::AppearanceSource::Override);
    EXPECT_EQ(textures.textureFor(appearance), &doorRedStorage);
}

/**
 * @brief Le mode Physique ignore totalement les surcharges, comme les skins.
 * \castest{<b>Le mode Physique ignore les surcharges.</b><br/>
 * \tcat Unitaire · Résolution de texture<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Résoudre une tuile portant une surcharge, en mode Physique.<br/>
 * \tattendu La source est l'atlas, la région physique passée en entrée.
 * }
 */
TEST(TextureResolutionTest, ModePhysiqueIgnoreLesSurcharges) {
    const hmi::SkinCatalog catalog = referenceCatalog();
    const hmi::SceneTextures textures = texturesWith(catalog);
    const hmi::TileSkinTag tag{core::TileType::Solid, 0, std::string{"door_red.png"}};
    const core::AtlasRegion physical = hmi::TextureAtlas::tile(0, 2);

    const hmi::TileAppearance appearance =
        hmi::resolveTileAppearance(hmi::RenderMode::Physique, physical, &tag, textures).value();

    EXPECT_EQ(appearance.source, hmi::AppearanceSource::Atlas);
    EXPECT_EQ(appearance.region.x, physical.x);
}

/**
 * @brief Un index d'objet invalide ne lit jamais hors des bornes.
 * \castest{<b>Un index d'objet invalide retombe sur le damier sans lecture hors bornes.</b><br/>
 * \tcat Unitaire · Résolution de texture<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Interroger les textures avec une apparence Override portant un index hors
 * bornes.<br/> \tattendu La texture et les dimensions rendues sont celles du damier.
 * }
 */
TEST(TextureResolutionTest, IndexDObjetInvalideBorne) {
    const hmi::SkinCatalog catalog = referenceCatalog();
    const hmi::SceneTextures textures = texturesWith(catalog);

    const hmi::TileAppearance broken{hmi::AppearanceSource::Override,
                                     core::AtlasRegion{0, 0, TILE, TILE}, 42};

    EXPECT_EQ(textures.textureFor(broken), &missingStorage);
    EXPECT_EQ(textures.widthFor(broken), hmi::MISSING_TEXTURE_SIZE);
    EXPECT_EQ(textures.heightFor(broken), hmi::MISSING_TEXTURE_SIZE);
}

/**
 * @brief Affichage isolé « objets interactifs seuls » (`LOT-51`, `EX-EDIT-044`) : une surcharge
 * chargée s'affiche normalement, l'axe skin n'intervient pas.
 * \castest{<b>Objets interactifs seuls : une surcharge chargee s'affiche normalement.</b><br/>
 * \tcat Unitaire · Résolution de texture<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Résoudre une tuile Solid skinnée portant une surcharge chargée, axe skin masqué.<br/>
 * \tattendu La source est la surcharge, comme en mode composé.
 * }
 */
TEST(TextureResolutionTest, ObjetsSeulsSurchargeChargeeSAffiche) {
    const hmi::SkinCatalog catalog = referenceCatalog();
    const hmi::SceneTextures textures = texturesWith(catalog);
    const hmi::TileSkinTag tag{core::TileType::Solid, 0, std::string{"door_red.png"}};

    const std::optional<hmi::TileAppearance> appearance =
        hmi::resolveTileAppearance(hmi::RenderMode::Texture, hmi::TextureAtlas::tile(0, 0), &tag,
                                   textures, /*skinVisible=*/false, /*overrideVisible=*/true);

    ASSERT_TRUE(appearance.has_value());
    EXPECT_EQ(appearance->source, hmi::AppearanceSource::Override);
}

/**
 * @brief Affichage isolé « objets interactifs seuls » : une case sans surcharge, même si son type
 * a un skin, n'affiche rien — ni skin, ni damier.
 * \castest{<b>Objets interactifs seuls : une case sans surcharge n'affiche rien.</b><br/>
 * \tcat Unitaire · Résolution de texture<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Résoudre une tuile Solid skinnée sans surcharge, axe skin masqué.<br/>
 * \tattendu Aucune apparence n'est renvoyée (pas de repli sur le skin ni le damier).
 * }
 */
TEST(TextureResolutionTest, ObjetsSeulsSansSurchargeNAfficheRien) {
    const hmi::SkinCatalog catalog = referenceCatalog();
    const hmi::SceneTextures textures = texturesWith(catalog);
    const hmi::TileSkinTag tag{core::TileType::Solid, 0};

    const std::optional<hmi::TileAppearance> appearance =
        hmi::resolveTileAppearance(hmi::RenderMode::Texture, hmi::TextureAtlas::tile(0, 0), &tag,
                                   textures, /*skinVisible=*/false, /*overrideVisible=*/true);

    EXPECT_FALSE(appearance.has_value());
}

/**
 * @brief Affichage isolé « objets interactifs seuls » : une case sans surcharge ni skin n'affiche
 * pas non plus le damier.
 * \castest{<b>Objets interactifs seuls : une case sans surcharge ni skin n'affiche pas le
 * damier.</b><br/>
 * \tcat Unitaire · Résolution de texture<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Résoudre une tuile Danger (non skinnée) sans surcharge, axe skin masqué.<br/>
 * \tattendu Aucune apparence n'est renvoyée.
 * }
 */
TEST(TextureResolutionTest, ObjetsSeulsSansSurchargeNiSkinNAfficheRien) {
    const hmi::SkinCatalog catalog = referenceCatalog();
    const hmi::SceneTextures textures = texturesWith(catalog);
    const hmi::TileSkinTag tag{core::TileType::Danger, 0};

    const std::optional<hmi::TileAppearance> appearance =
        hmi::resolveTileAppearance(hmi::RenderMode::Texture, hmi::TextureAtlas::tile(0, 0), &tag,
                                   textures, /*skinVisible=*/false, /*overrideVisible=*/true);

    EXPECT_FALSE(appearance.has_value());
}

/**
 * @brief Affichage isolé « skin des tuiles seul » : un type skinné, sans surcharge, s'affiche
 * normalement.
 * \castest{<b>Skin des tuiles seul : un type skinne s'affiche normalement.</b><br/>
 * \tcat Unitaire · Résolution de texture<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Résoudre une tuile Solid skinnée sans surcharge, axe surcharge masqué.<br/>
 * \tattendu La source est le skin du type, comme en mode composé.
 * }
 */
TEST(TextureResolutionTest, SkinSeulTypeSkinneSAfficheNormalement) {
    const hmi::SkinCatalog catalog = referenceCatalog();
    const hmi::SceneTextures textures = texturesWith(catalog);
    const hmi::TileSkinTag tag{core::TileType::Solid, 0};

    const std::optional<hmi::TileAppearance> appearance =
        hmi::resolveTileAppearance(hmi::RenderMode::Texture, hmi::TextureAtlas::tile(0, 0), &tag,
                                   textures, /*skinVisible=*/true, /*overrideVisible=*/false);

    ASSERT_TRUE(appearance.has_value());
    EXPECT_EQ(appearance->source, hmi::AppearanceSource::Skin);
}

/**
 * @brief Affichage isolé « skin des tuiles seul » : un type non skinné n'affiche rien — c'est le
 * diagnostic recherché (quels types manquent encore un skin), pas le damier habituel.
 * \castest{<b>Skin des tuiles seul : un type non skinne n'affiche rien.</b><br/>
 * \tcat Unitaire · Résolution de texture<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Résoudre une tuile Danger (non skinnée) sans surcharge, axe surcharge masqué.<br/>
 * \tattendu Aucune apparence n'est renvoyée (pas de damier).
 * }
 */
TEST(TextureResolutionTest, SkinSeulTypeNonSkinneNAfficheRien) {
    const hmi::SkinCatalog catalog = referenceCatalog();
    const hmi::SceneTextures textures = texturesWith(catalog);
    const hmi::TileSkinTag tag{core::TileType::Danger, 0};

    const std::optional<hmi::TileAppearance> appearance =
        hmi::resolveTileAppearance(hmi::RenderMode::Texture, hmi::TextureAtlas::tile(0, 0), &tag,
                                   textures, /*skinVisible=*/true, /*overrideVisible=*/false);

    EXPECT_FALSE(appearance.has_value());
}

/**
 * @brief Affichage isolé « skin des tuiles seul » : une case portant une surcharge n'affiche rien
 * non plus — l'axe surcharge est masqué, l'isolement ne retombe jamais sur un autre axe.
 * \castest{<b>Skin des tuiles seul : une case avec surcharge n'affiche rien.</b><br/>
 * \tcat Unitaire · Résolution de texture<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Résoudre une tuile Solid portant une surcharge chargée, axe surcharge masqué.<br/>
 * \tattendu Aucune apparence n'est renvoyée.
 * }
 */
TEST(TextureResolutionTest, SkinSeulCaseAvecSurchargeNAfficheRien) {
    const hmi::SkinCatalog catalog = referenceCatalog();
    const hmi::SceneTextures textures = texturesWith(catalog);
    const hmi::TileSkinTag tag{core::TileType::Solid, 0, std::string{"door_red.png"}};

    const std::optional<hmi::TileAppearance> appearance =
        hmi::resolveTileAppearance(hmi::RenderMode::Texture, hmi::TextureAtlas::tile(0, 0), &tag,
                                   textures, /*skinVisible=*/true, /*overrideVisible=*/false);

    EXPECT_FALSE(appearance.has_value());
}

/**
 * @brief Les deux axes masqués n'affichent jamais rien, quelle que soit la tuile — équivalent à un
 * calque « Tile » entièrement masqué (`LOT-51` TACHE-01).
 * \castest{<b>Les deux axes masques n'affichent jamais rien.</b><br/>
 * \tcat Unitaire · Résolution de texture<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Résoudre une tuile Solid skinnée avec surcharge, les deux axes masqués.<br/>
 * \tattendu Aucune apparence n'est renvoyée.
 * }
 */
TEST(TextureResolutionTest, DeuxAxesMasquesNAfficheJamaisRien) {
    const hmi::SkinCatalog catalog = referenceCatalog();
    const hmi::SceneTextures textures = texturesWith(catalog);
    const hmi::TileSkinTag tag{core::TileType::Solid, 0, std::string{"door_red.png"}};

    const std::optional<hmi::TileAppearance> appearance =
        hmi::resolveTileAppearance(hmi::RenderMode::Texture, hmi::TextureAtlas::tile(0, 0), &tag,
                                   textures, /*skinVisible=*/false, /*overrideVisible=*/false);

    EXPECT_FALSE(appearance.has_value());
}

/**
 * @brief Le résolveur en mode composé (les deux axes visibles, le défaut) garde exactement le
 * comportement de `LOT-45` : une surcharge introuvable retombe sur le damier même quand elle est
 * la seule chose composée pour cette case.
 * \castest{<b>Mode compose : une surcharge introuvable retombe toujours sur le damier.</b><br/>
 * \tcat Unitaire · Résolution de texture<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Résoudre une tuile Solid skinnée portant une surcharge introuvable, les deux axes
 * visibles (valeurs par défaut).<br/>
 * \tattendu La source est le damier, comme avant `LOT-51`.
 * }
 */
TEST(TextureResolutionTest, ModeComposeSurchargeIntrouvableRetombeSurLeDamier) {
    const hmi::SkinCatalog catalog = referenceCatalog();
    const hmi::SceneTextures textures = texturesWith(catalog);
    const hmi::TileSkinTag tag{core::TileType::Solid, 0, std::string{"asset_jamais_charge.png"}};

    const std::optional<hmi::TileAppearance> appearance = hmi::resolveTileAppearance(
        hmi::RenderMode::Texture, hmi::TextureAtlas::tile(0, 0), &tag, textures);

    ASSERT_TRUE(appearance.has_value());
    EXPECT_EQ(appearance->source, hmi::AppearanceSource::MissingTexture);
}

/**
 * @brief textureOverrideAt trouve la surcharge d'une case précise, et distingue les positions
 * voisines.
 * \castest{<b>textureOverrideAt trouve la surcharge d'une case precise.</b><br/>
 * \tcat Unitaire · Résolution de texture<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Chercher une position avec et sans surcharge dans une liste de surcharges.<br/>
 * \tattendu Le nom d'asset est trouvé pour la position exacte, absent ailleurs.
 * }
 */
TEST(TextureResolutionTest, TextureOverrideAtTrouveLaSurchargeDeLaBonneCase) {
    const std::vector<core::TileTextureOverride> overrides{
        core::TileTextureOverride{core::GridPosition{2, 3}, "door_red.png"},
        core::TileTextureOverride{core::GridPosition{5, 1}, "crate.png"},
    };

    const std::optional<std::string> found =
        hmi::textureOverrideAt(overrides, core::GridPosition{2, 3});
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(*found, "door_red.png");

    EXPECT_FALSE(hmi::textureOverrideAt(overrides, core::GridPosition{0, 0}).has_value());
}
