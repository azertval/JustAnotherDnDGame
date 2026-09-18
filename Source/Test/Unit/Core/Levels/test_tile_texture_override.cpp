// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

/**
 * @file test_tile_texture_override.cpp
 * @brief Tests unitaires de la texture assignée par instance (`EX-EDIT-043`).
 */

#include <algorithm>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "Core/Levels/GridPosition.h"
#include "Core/Levels/Level.h"
#include "Core/Levels/LevelDraft.h"
#include "Core/Levels/LevelLoader.h"
#include "Core/Levels/LevelWriter.h"
#include "Core/Levels/TileType.h"

namespace {

using core::GridPosition;
using core::LevelDraft;
using core::TileType;

// Brouillon d'une carte width x height dont le mur en (column, row) porte la texture "wall_red.png".
LevelDraft brouillonHabille(int width, int height, int column, int row) {
    const std::string json = R"({ "width": )" + std::to_string(width) + R"(, "height": )" +
                             std::to_string(height) + R"(, "tiles": [
        { "x": 0, "y": 0, "type": "entry" },
        { "x": )" + std::to_string(column) + R"(, "y": )" + std::to_string(row) +
                             R"(, "type": "wall", "texture": "wall_red.png" }
      ] })";
    const core::LevelLoadResult loaded = core::LevelLoader::loadFromString(json);
    EXPECT_TRUE(loaded.ok()) << loaded.error;
    return LevelDraft::fromLevel(*loaded.level);
}

}  // namespace

/**
 * @brief Repeindre un **autre** type de tuile sur une case habillée retire son override.
 * \castest{<b>Repeindre un autre type retire l'override.</b><br/>
 * \tcat Unitaire · Tile Texture Override<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Repeindre un autre type de tuile sur une case habillée retire son override.
 * }
 */
TEST(TileTextureOverrideTest, PeindreUnAutreTypeRetireLOverride) {
    LevelDraft draft = brouillonHabille(4, 4, 1, 1);
    ASSERT_EQ(draft.textureOverrides().size(), 1u);

    draft.paintTile(1, 1, TileType::Solid);

    EXPECT_TRUE(draft.textureOverrides().empty());
}

/**
 * @brief Repeindre le **même** type de tuile sur une case habillée conserve son override : un coup
 * de pinceau involontaire ne doit pas effacer un habillage.
 * \castest{<b>Repeindre le même type conserve l'override.</b><br/>
 * \tcat Unitaire · Tile Texture Override<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Repeindre le même type de tuile sur une case habillée conserve son override.
 * }
 */
TEST(TileTextureOverrideTest, PeindreLeMemeTypeConserveLOverride) {
    LevelDraft draft = brouillonHabille(4, 4, 1, 1);

    draft.paintTile(1, 1, TileType::Wall);

    ASSERT_EQ(draft.textureOverrides().size(), 1u);
    EXPECT_EQ(draft.textureOverrides().front().assetName, "wall_red.png");
}

/**
 * @brief paintRegion retire aussi l'override d'une case dont le type change (même funnel que
 * paintTile, appliqué cellule par cellule).
 * \castest{<b>paintRegion retire l'override d'une case dont le type change.</b><br/>
 * \tcat Unitaire · Tile Texture Override<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu paintRegion retire l'override d'une case dont le type change.
 * }
 */
TEST(TileTextureOverrideTest, PaintRegionRetireLOverrideDUneCaseDontLeTypeChange) {
    LevelDraft draft = brouillonHabille(4, 4, 1, 1);

    draft.paintRegion(1, 1,
                      {{TileType::Solid, TileType::Solid}, {TileType::Solid, TileType::Solid}});

    EXPECT_TRUE(draft.textureOverrides().empty());
}

/**
 * @brief Coller une région (`paintRegion`) ne copie **pas** les overrides des cases source : un
 * override reste attaché à sa case d'origine, `paintRegion` ne transporte que des types de tuile.
 * \castest{<b>Coller une région ne copie pas les overrides de la source.</b><br/>
 * \tcat Unitaire · Tile Texture Override<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Coller une région ne copie pas les overrides des cases source.
 * }
 */
TEST(TileTextureOverrideTest, CollerUneRegionNeCopiePasLesOverridesDeLaSource) {
    LevelDraft draft = brouillonHabille(6, 6, 1, 1);

    // "Copie" du bloc source : seuls les types de tuile voyagent, jamais les overrides (aucune
    // API ne permet de les inclure dans un bloc de paintRegion).
    const std::vector<std::vector<TileType>> block{{draft.tileMap().tile(1, 1)}};
    draft.paintRegion(3, 3, block);

    ASSERT_EQ(draft.textureOverrides().size(), 1u);
    EXPECT_EQ(draft.textureOverrides().front().position, (GridPosition{1, 1}));
}

/**
 * @brief Réduire la grille tronque silencieusement les overrides hors des nouvelles bornes, comme
 * les autres données annexes.
 * \castest{<b>Réduire la grille tronque les overrides hors bornes.</b><br/>
 * \tcat Unitaire · Tile Texture Override<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Réduire la grille tronque silencieusement les overrides hors des nouvelles bornes.
 * }
 */
TEST(TileTextureOverrideTest, ReduireLaGrilleTronqueLesOverridesHorsBornes) {
    LevelDraft draft = brouillonHabille(5, 5, 4, 4);

    draft.resize(2, 2);

    EXPECT_TRUE(draft.textureOverrides().empty());
}

/**
 * @brief wouldResizeDropContent signale la perte d'un override hors des bornes envisagées.
 * \castest{<b>wouldResizeDropContent signale la perte d'un override.</b><br/>
 * \tcat Unitaire · Tile Texture Override<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu wouldResizeDropContent signale la perte d'un override hors des bornes envisagées.
 * }
 */
TEST(TileTextureOverrideTest, WouldResizeDropContentSignaleLaPerteDUnOverride) {
    const LevelDraft draft = brouillonHabille(5, 5, 4, 4);

    EXPECT_TRUE(draft.wouldResizeDropContent(2, 2));
    EXPECT_FALSE(draft.wouldResizeDropContent(5, 5));
}

/**
 * @brief undo annule un retrait d'override (le restitue tel quel).
 * \castest{<b>undo restitue un override retiré.</b><br/>
 * \tcat Unitaire · Tile Texture Override<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu undo annule un retrait d'override (le restitue tel quel).
 * }
 */
TEST(TileTextureOverrideTest, UndoRestitueUnOverrideRetire) {
    LevelDraft draft = brouillonHabille(4, 4, 1, 1);

    draft.paintTile(1, 1, TileType::Grass);
    ASSERT_TRUE(draft.textureOverrides().empty());

    ASSERT_TRUE(draft.undo());
    ASSERT_EQ(draft.textureOverrides().size(), 1u);
    EXPECT_EQ(draft.textureOverrides().front().assetName, "wall_red.png");
}

/**
 * @brief fromLevel restitue les overrides de texture d'un niveau déjà chargé.
 * \castest{<b>fromLevel restitue les overrides de texture.</b><br/>
 * \tcat Unitaire · Tile Texture Override<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu fromLevel restitue les overrides de texture d'un niveau déjà chargé.
 * }
 */
TEST(TileTextureOverrideTest, FromLevelRestitueLesOverrides) {
    const core::LevelLoadResult loaded = core::LevelLoader::loadFromString(R"({
      "width": 4, "height": 4,
      "tiles": [
        { "x": 0, "y": 0, "type": "entry" },
        { "x": 2, "y": 2, "type": "wall", "texture": "wall_red.png" }
      ]
    })");
    ASSERT_TRUE(loaded.ok()) << loaded.error;

    const LevelDraft draft = LevelDraft::fromLevel(*loaded.level);

    ASSERT_EQ(draft.textureOverrides().size(), 1u);
    EXPECT_EQ(draft.textureOverrides().front().position, (GridPosition{2, 2}));
    EXPECT_EQ(draft.textureOverrides().front().assetName, "wall_red.png");
}

/**
 * @brief Le champ `"texture"` d'une tuile survit au round-trip JSON (écriture puis relecture).
 * \castest{<b>Le champ texture survit au round-trip JSON.</b><br/>
 * \tcat Unitaire · Tile Texture Override<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Le champ texture survit au round-trip JSON.
 * }
 */
TEST(TileTextureOverrideTest, LeChampTextureSurvitAuRoundTripJson) {
    const core::LevelLoadResult loaded = core::LevelLoader::loadFromString(R"({
      "width": 4, "height": 4,
      "tiles": [
        { "x": 0, "y": 0, "type": "entry" },
        { "x": 1, "y": 1, "type": "solid", "texture": "crate.png" },
        { "x": 2, "y": 2, "type": "wall", "texture": "wall_red.png" }
      ]
    })");
    ASSERT_TRUE(loaded.ok()) << loaded.error;

    const std::string json = core::LevelWriter::toJsonString(*loaded.level);
    const core::LevelLoadResult reloaded = core::LevelLoader::loadFromString(json);
    ASSERT_TRUE(reloaded.ok()) << reloaded.error;

    ASSERT_EQ(reloaded.level->textureOverrides().size(), 2u);
    const auto& overrides = reloaded.level->textureOverrides();
    const bool hasCrate = std::any_of(overrides.begin(), overrides.end(), [](const auto& o) {
        return o.position == core::GridPosition{1, 1} && o.assetName == "crate.png";
    });
    const bool hasWall = std::any_of(overrides.begin(), overrides.end(), [](const auto& o) {
        return o.position == core::GridPosition{2, 2} && o.assetName == "wall_red.png";
    });
    EXPECT_TRUE(hasCrate);
    EXPECT_TRUE(hasWall);
}

/**
 * @brief Un niveau sans aucune tuile portant `"texture"` n'écrit ce champ pour aucune tuile.
 * \castest{<b>Sans override, le champ texture est absent du JSON produit.</b><br/>
 * \tcat Unitaire · Tile Texture Override<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un niveau sans aucune tuile portant "texture" n'écrit ce champ pour aucune tuile.
 * }
 */
TEST(TileTextureOverrideTest, SansOverrideLeChampTextureEstAbsentDuJson) {
    const core::LevelLoadResult loaded = core::LevelLoader::loadFromString(R"({
        "width": 3, "height": 3,
        "tiles": [ {"x":0,"y":0,"type":"entry"} ] })");
    ASSERT_TRUE(loaded.ok()) << loaded.error;

    const std::string json = core::LevelWriter::toJsonString(*loaded.level);
    EXPECT_EQ(json.find("\"texture\""), std::string::npos);
}

/**
 * @brief Un niveau existant sans champ `"texture"` se charge sans erreur ni override
 * (rétrocompatibilité, `EX-LVL-005`).
 * \castest{<b>Un niveau sans champ texture se charge sans override.</b><br/>
 * \tcat Unitaire · Tile Texture Override<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un niveau existant sans champ texture se charge sans erreur ni override.
 * }
 */
TEST(TileTextureOverrideTest, NiveauSansChampTextureSeChargeSansOverride) {
    const core::LevelLoadResult loaded = core::LevelLoader::loadFromString(R"({
        "width": 3, "height": 3,
        "tiles": [
          {"x":0,"y":0,"type":"entry"},
          {"x":1,"y":1,"type":"solid"}
        ] })");
    ASSERT_TRUE(loaded.ok()) << loaded.error;
    EXPECT_TRUE(loaded.level->textureOverrides().empty());
}
