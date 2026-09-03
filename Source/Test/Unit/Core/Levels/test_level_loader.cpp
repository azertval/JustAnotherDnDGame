// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_level_loader.cpp
 * @brief Tests unitaires du chargement de niveau (format JSON, erreurs récupérables).
 */

#include <filesystem>
#include <vector>

#include <gtest/gtest.h>

#include "Core/Levels/CameraFraming.h"
#include "Core/Levels/GridPosition.h"
#include "Core/Levels/LevelLoader.h"
#include "Core/Levels/TileType.h"

namespace {

// Un niveau JSON valide et minimal (entrée, sortie, un solide, une paire interrupteur/porte).
constexpr const char* VALID_LEVEL = R"({
  "name": "Tutoriel",
  "width": 4,
  "height": 3,
  "tiles": [
    { "x": 0, "y": 0, "type": "solid" },
    { "x": 1, "y": 1, "type": "entry" },
    { "x": 3, "y": 2, "type": "exit" },
    { "x": 2, "y": 0, "type": "switch", "id": "s1" },
    { "x": 3, "y": 0, "type": "door", "opensWith": "s1" }
  ]
})";

}  // namespace

/**
 * @brief Un niveau valide est chargé avec ses dimensions, tuiles, entrée/sortie et mécanismes.
 * \castest{<b>Un niveau valide est chargé avec ses dimensions, tuiles, entrée/sortie et
 * mécanismes.</b><br/>
 * \tcat Unitaire · Level Loader<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un niveau valide est chargé avec ses dimensions, tuiles, entrée/sortie et mécanismes.
 * }
 */
TEST(LevelLoaderTest, ChargeUnNiveauValide) {
    const core::LevelLoadResult result = core::LevelLoader::loadFromString(VALID_LEVEL);
    ASSERT_TRUE(result.ok()) << result.error;
    EXPECT_EQ(result.errorCode, core::LevelValidationError::None);

    const core::Level& level = *result.level;
    EXPECT_EQ(level.name(), "Tutoriel");
    EXPECT_EQ(level.tileMap().width(), 4);
    EXPECT_EQ(level.tileMap().height(), 3);
    EXPECT_EQ(level.tileMap().tile(0, 0), core::TileType::Solid);
    EXPECT_EQ(level.tileMap().tile(1, 1), core::TileType::Entry);
    EXPECT_EQ(level.entry(), (core::GridPosition{1, 1}));
    EXPECT_EQ(level.exit(), (core::GridPosition{3, 2}));

    ASSERT_EQ(level.mechanisms().size(), 1u);
    EXPECT_EQ(level.mechanisms().front().switchPosition, (core::GridPosition{2, 0}));
    EXPECT_EQ(level.mechanisms().front().doorPosition, (core::GridPosition{3, 0}));
}

/**
 * @brief Un niveau sans champ `"version"` se charge sans erreur ni avertissement, comme la
 * version initiale du format (`EX-LVL-005`, rétrocompatibilité des niveaux antérieurs à ce
 * champ).
 * \castest{<b>Un niveau sans champ version se charge sans erreur.</b><br/>
 * \tcat Unitaire · Level Loader<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un niveau sans champ version se charge sans erreur.
 * }
 */
TEST(LevelLoaderTest, NiveauSansVersionSeChargeSansErreur) {
    const core::LevelLoadResult result = core::LevelLoader::loadFromString(VALID_LEVEL);
    ASSERT_TRUE(result.ok()) << result.error;
    EXPECT_EQ(result.errorCode, core::LevelValidationError::None);
}

/**
 * @brief Un niveau dont la version dépasse celle gérée échoue avec une erreur exploitable
 * (`EX-LVL-005`), pas une lecture au mieux.
 * \castest{<b>Un niveau dont la version depasse celle geree echoue proprement.</b><br/>
 * \tcat Unitaire · Level Loader<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un niveau dont la version depasse celle geree echoue proprement.
 * }
 */
TEST(LevelLoaderTest, VersionSuperieureALaVersionGereeEchoueProprement) {
    constexpr const char* LEVEL = R"({
      "version": 999,
      "width": 4,
      "height": 3,
      "tiles": [
        { "x": 1, "y": 1, "type": "entry" },
        { "x": 3, "y": 2, "type": "exit" }
      ]
    })";
    const core::LevelLoadResult result = core::LevelLoader::loadFromString(LEVEL);
    EXPECT_FALSE(result.ok());
    EXPECT_EQ(result.errorCode, core::LevelValidationError::UnsupportedFormatVersion);
}

/**
 * @brief Un niveau désignant un fond et un jeu de skins restitue les deux chaînes ; sans eux,
 * elles sont absentes (`EX-REN-044`, `EX-EDIT-024`).
 * \castest{<b>Un niveau designant un fond et un jeu de skins restitue les deux champs.</b><br/>
 * \tcat Unitaire · Level Loader<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un niveau designant un fond et un jeu de skins restitue les deux champs.
 * }
 */
TEST(LevelLoaderTest, ChargeLeFondEtLeJeuDeSkins) {
    constexpr const char* LEVEL = R"({
      "background": "forest.png",
      "skinSet": "foret",
      "width": 4,
      "height": 3,
      "tiles": [
        { "x": 1, "y": 1, "type": "entry" },
        { "x": 3, "y": 2, "type": "exit" }
      ]
    })";
    const core::LevelLoadResult result = core::LevelLoader::loadFromString(LEVEL);
    ASSERT_TRUE(result.ok()) << result.error;
    ASSERT_TRUE(result.level->background().has_value());
    EXPECT_EQ(*result.level->background(), "forest.png");
    ASSERT_TRUE(result.level->skinSet().has_value());
    EXPECT_EQ(*result.level->skinSet(), "foret");
}

/**
 * @brief Un niveau sans fond ni jeu de skins désignés n'en restitue aucun (état normal, pas une
 * anomalie, `EX-REN-044`).
 * \castest{<b>Un niveau sans fond ni jeu de skins n'en restitue aucun.</b><br/>
 * \tcat Unitaire · Level Loader<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un niveau sans fond ni jeu de skins n'en restitue aucun.
 * }
 */
TEST(LevelLoaderTest, SansFondNiJeuDeSkinsLesDeuxChampsSontAbsents) {
    const core::LevelLoadResult result = core::LevelLoader::loadFromString(VALID_LEVEL);
    ASSERT_TRUE(result.ok()) << result.error;
    EXPECT_FALSE(result.level->background().has_value());
    EXPECT_FALSE(result.level->skinSet().has_value());
}

/**
 * @brief Un niveau déclarant un mode de cadrage explicite le restitue tel quel, y compris une
 * taille de salle personnalisée pour le mode *par salle* (`EX-LVL-006`).
 * \castest{<b>Un mode de cadrage déclaré explicitement est restitué tel quel.</b><br/>
 * \tcat Unitaire · Level Loader<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Charger un niveau déclarant le mode *par salle* avec une taille personnalisée.<br/>
 * 2. Vérifier le cadrage résolu.<br/>
 * \tattendu Le mode et la taille de salle déclarés sont restitués sans modification.
 * }
 */
TEST(LevelLoaderTest, ModeDeCadrageDeclareExplicitementEstRestitue) {
    constexpr const char* LEVEL = R"({
      "width": 30, "height": 20,
      "cameraFraming": { "mode": "perRoom", "roomWidthTiles": 10, "roomHeightTiles": 8 },
      "tiles": [
        { "x": 1, "y": 1, "type": "entry" },
        { "x": 3, "y": 2, "type": "exit" }
      ]
    })";
    const core::LevelLoadResult result = core::LevelLoader::loadFromString(LEVEL);
    ASSERT_TRUE(result.ok()) << result.error;
    EXPECT_EQ(result.level->cameraFraming().mode, core::CameraFramingMode::PerRoom);
    ASSERT_TRUE(result.level->cameraFraming().roomWidthTiles.has_value());
    EXPECT_EQ(*result.level->cameraFraming().roomWidthTiles, 10);
    ASSERT_TRUE(result.level->cameraFraming().roomHeightTiles.has_value());
    EXPECT_EQ(*result.level->cameraFraming().roomHeightTiles, 8);
}

/**
 * @brief Un niveau déclarant des zones de caméra dessinées à la main (mode *par salle*) les
 * restitue dans l'ordre exact du fichier (`EX-LVL-007`).
 * \castest{<b>Les zones de caméra dessinées à la main sont restituées dans l'ordre du fichier.
 * </b><br/>
 * \tcat Unitaire · Level Loader<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Charger un niveau déclarant deux zones de caméra.<br/>2. Vérifier leur nombre, leur
 * ordre et leurs bornes.<br/>
 * \tattendu Les deux zones sont restituées dans l'ordre exact du fichier, avec leurs bornes
 * exactes.
 * }
 */
TEST(LevelLoaderTest, ZonesDeCameraDeclareesSontRestitueesDansLOrdre) {
    constexpr const char* LEVEL = R"({
      "width": 30, "height": 20,
      "cameraFraming": { "mode": "perRoom", "zones": [
        { "x": 0, "y": 0, "width": 24, "height": 14 },
        { "x": 24, "y": 0, "width": 6, "height": 20 }
      ] },
      "tiles": [
        { "x": 1, "y": 1, "type": "entry" },
        { "x": 3, "y": 2, "type": "exit" }
      ]
    })";
    const core::LevelLoadResult result = core::LevelLoader::loadFromString(LEVEL);
    ASSERT_TRUE(result.ok()) << result.error;
    ASSERT_EQ(result.level->cameraFraming().zones.size(), 2u);
    EXPECT_EQ(result.level->cameraFraming().zones[0],
              (core::CameraZone{.x = 0, .y = 0, .width = 24, .height = 14}));
    EXPECT_EQ(result.level->cameraFraming().zones[1],
              (core::CameraZone{.x = 24, .y = 0, .width = 6, .height = 20}));
}

/**
 * @brief Un niveau sans champ `cameraFraming` reproduit exactement la règle historique : niveau
 * entier s'il tient dans une salle de taille par défaut, par salle sinon (`EX-LVL-006`, critère
 * d'acceptation numéro un du lot).
 * \castest{<b>L'absence de cameraFraming reproduit la règle historique.</b><br/>
 * \tcat Unitaire · Level Loader<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Charger un petit niveau (tient dans une salle) sans champ `cameraFraming`.<br/>
 * 2. Charger un grand niveau (dépasse une salle) sans champ `cameraFraming`.<br/>
 * \tattendu Le petit niveau résout en mode *niveau entier* ; le grand résout en mode *par salle*,
 * avec une taille de salle par défaut (absente, pas explicitement 24×14).
 * }
 */
TEST(LevelLoaderTest, AbsenceDeCameraFramingReproduitLaRegleHistorique) {
    constexpr const char* SMALL_LEVEL = R"({
      "width": 10, "height": 8,
      "tiles": [
        { "x": 1, "y": 1, "type": "entry" },
        { "x": 3, "y": 2, "type": "exit" }
      ]
    })";
    const core::LevelLoadResult small = core::LevelLoader::loadFromString(SMALL_LEVEL);
    ASSERT_TRUE(small.ok()) << small.error;
    EXPECT_EQ(small.level->cameraFraming().mode, core::CameraFramingMode::WholeLevel);

    constexpr const char* LARGE_LEVEL = R"({
      "width": 30, "height": 20,
      "tiles": [
        { "x": 1, "y": 1, "type": "entry" },
        { "x": 3, "y": 2, "type": "exit" }
      ]
    })";
    const core::LevelLoadResult large = core::LevelLoader::loadFromString(LARGE_LEVEL);
    ASSERT_TRUE(large.ok()) << large.error;
    EXPECT_EQ(large.level->cameraFraming().mode, core::CameraFramingMode::PerRoom);
    EXPECT_FALSE(large.level->cameraFraming().roomWidthTiles.has_value());
    EXPECT_FALSE(large.level->cameraFraming().roomHeightTiles.has_value());
}

/**
 * @brief Un cadrage invalide (mode inconnu, taille de salle nulle ou supérieure au niveau,
 * paramètre étranger au mode) échoue avec une erreur nommant le champ fautif (`EX-LVL-004`).
 * \castest{<b>Un cadrage invalide échoue avec une erreur exploitable.</b><br/>
 * \tcat Unitaire · Level Loader<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Charger successivement un niveau par variante d'erreur (mode inconnu, taille
 * nulle, taille supérieure au niveau, paramètre étranger au mode).<br/>
 * \tattendu Chaque variante échoue avec `LevelValidationError::InvalidCameraFraming`.
 * }
 */
TEST(LevelLoaderTest, CadrageInvalideEchoueAvecErreurExploitable) {
    constexpr const char* UNKNOWN_MODE = R"({
      "width": 10, "height": 8,
      "cameraFraming": { "mode": "zoomInfini" },
      "tiles": [ { "x": 1, "y": 1, "type": "entry" }, { "x": 3, "y": 2, "type": "exit" } ]
    })";
    constexpr const char* ZERO_ROOM_WIDTH = R"({
      "width": 10, "height": 8,
      "cameraFraming": { "mode": "perRoom", "roomWidthTiles": 0 },
      "tiles": [ { "x": 1, "y": 1, "type": "entry" }, { "x": 3, "y": 2, "type": "exit" } ]
    })";
    constexpr const char* ROOM_WIDTH_EXCEEDS_LEVEL = R"({
      "width": 10, "height": 8,
      "cameraFraming": { "mode": "perRoom", "roomWidthTiles": 11 },
      "tiles": [ { "x": 1, "y": 1, "type": "entry" }, { "x": 3, "y": 2, "type": "exit" } ]
    })";
    constexpr const char* FOREIGN_PARAMETER = R"({
      "width": 10, "height": 8,
      "cameraFraming": { "mode": "wholeLevel", "roomWidthTiles": 5 },
      "tiles": [ { "x": 1, "y": 1, "type": "entry" }, { "x": 3, "y": 2, "type": "exit" } ]
    })";

    for (const char* levelJson :
         {UNKNOWN_MODE, ZERO_ROOM_WIDTH, ROOM_WIDTH_EXCEEDS_LEVEL, FOREIGN_PARAMETER}) {
        const core::LevelLoadResult result = core::LevelLoader::loadFromString(levelJson);
        EXPECT_FALSE(result.ok());
        EXPECT_EQ(result.errorCode, core::LevelValidationError::InvalidCameraFraming);
    }
}

/**
 * @brief Une plaque de pression se charge comme un interrupteur : même règle d'identifiant, même
 * résolution de liaison vers une porte (`EX-GP-025`).
 * \castest{<b>Une plaque de pression se charge comme un interrupteur.</b><br/>
 * \tcat Unitaire · Level Loader<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Une plaque de pression se charge comme un interrupteur : même règle d'identifiant,
 * même résolution de liaison vers une porte.
 * }
 */
TEST(LevelLoaderTest, ChargeUnePlaqueDePression) {
    constexpr const char* LEVEL = R"({
      "name": "Poids",
      "width": 4,
      "height": 3,
      "tiles": [
        { "x": 1, "y": 1, "type": "entry" },
        { "x": 3, "y": 2, "type": "exit" },
        { "x": 2, "y": 0, "type": "pressurePlate", "id": "p1" },
        { "x": 3, "y": 0, "type": "door", "opensWith": "p1" }
      ]
    })";
    const core::LevelLoadResult result = core::LevelLoader::loadFromString(LEVEL);
    ASSERT_TRUE(result.ok()) << result.error;

    const core::Level& level = *result.level;
    EXPECT_EQ(level.tileMap().tile(2, 0), core::TileType::PressurePlate);
    ASSERT_EQ(level.mechanisms().size(), 1u);
    EXPECT_EQ(level.mechanisms().front().switchPosition, (core::GridPosition{2, 0}));
    EXPECT_EQ(level.mechanisms().front().doorPosition, (core::GridPosition{3, 0}));
}

/**
 * @brief Un bloc poussable se charge comme une simple tuile, sans identifiant ni liaison
 * (`EX-GP-022`).
 * \castest{<b>Un bloc poussable se charge comme une simple tuile, sans identifiant ni
 * liaison.</b><br/>
 * \tcat Unitaire · Level Loader<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un bloc poussable se charge comme une simple tuile, sans identifiant ni liaison.
 * }
 */
TEST(LevelLoaderTest, ChargeUnBlocPoussable) {
    constexpr const char* LEVEL = R"({
      "name": "Bloc",
      "width": 4,
      "height": 3,
      "tiles": [
        { "x": 1, "y": 1, "type": "entry" },
        { "x": 3, "y": 2, "type": "exit" },
        { "x": 2, "y": 0, "type": "block" }
      ]
    })";
    const core::LevelLoadResult result = core::LevelLoader::loadFromString(LEVEL);
    ASSERT_TRUE(result.ok()) << result.error;

    const core::Level& level = *result.level;
    EXPECT_EQ(level.tileMap().tile(2, 0), core::TileType::Block);
    EXPECT_TRUE(level.mechanisms().empty());  // un bloc n'est pas un mecanisme lie
}
