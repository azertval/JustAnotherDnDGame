// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_level_writer.cpp
 * @brief Tests unitaires de la sérialisation de niveau (round-trip avec LevelLoader, EX-EDIT-011).
 */

#include <filesystem>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "Core/Levels/GridPosition.h"
#include "Core/Levels/LevelLoader.h"
#include "Core/Levels/LevelWriter.h"
#include "Core/Levels/TileType.h"

namespace {

constexpr const char* LEVEL_WITH_MECHANISM = R"({
  "name": "Tutoriel",
  "width": 4,
  "height": 3,
  "tiles": [
    { "x": 0, "y": 0, "type": "solid" },
    { "x": 1, "y": 1, "type": "entry" },
    { "x": 3, "y": 2, "type": "exit" },
    { "x": 0, "y": 1, "type": "danger" },
    { "x": 2, "y": 0, "type": "switch", "id": "s1" },
    { "x": 3, "y": 0, "type": "door", "opensWith": "s1" }
  ]
})";

}  // namespace


/**
 * @brief Une plaque de pression survit au round-trip (sérialisation puis rechargement),
 * `TileType` et liaison préservés (`EX-GP-025`).
 * \castest{<b>Une plaque de pression survit au round-trip.</b><br/>
 * \tcat Unitaire · Level Writer<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Une plaque de pression survit au round-trip : `TileType` et liaison préservés.
 * }
 */
TEST(LevelWriterTest, PlaqueDePressionSurvitAuRoundTrip) {
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
    const core::LevelLoadResult loaded = core::LevelLoader::loadFromString(LEVEL);
    ASSERT_TRUE(loaded.ok()) << loaded.error;

    const std::string json = core::LevelWriter::toJsonString(*loaded.level);
    const core::LevelLoadResult reloaded = core::LevelLoader::loadFromString(json);
    ASSERT_TRUE(reloaded.ok()) << reloaded.error;

    EXPECT_EQ(reloaded.level->tileMap().tile(2, 0), core::TileType::PressurePlate);
    ASSERT_EQ(reloaded.level->mechanisms().size(), 1u);
    EXPECT_EQ(reloaded.level->mechanisms().front().switchPosition, (core::GridPosition{2, 0}));
    EXPECT_EQ(reloaded.level->mechanisms().front().doorPosition, (core::GridPosition{3, 0}));
}

/**
 * @brief Une clé et une porte verrouillée survivent au round-trip (sérialisation puis
 *        rechargement), `TileType` et liaison préservés (`EX-GP-023`).
 * \castest{<b>Une clé et une porte verrouillée survivent au round-trip.</b><br/>
 * \tcat Unitaire · Level Writer<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu La clé et la porte verrouillée survivent au round-trip : `TileType` et liaison
 * préservés.
 * }
 */
TEST(LevelWriterTest, CleEtPorteVerrouilleeSurvitAuRoundTrip) {
    constexpr const char* LEVEL = R"({
      "name": "Cle",
      "width": 4,
      "height": 3,
      "tiles": [
        { "x": 1, "y": 1, "type": "entry" },
        { "x": 3, "y": 2, "type": "exit" },
        { "x": 2, "y": 0, "type": "key", "id": "k1" },
        { "x": 3, "y": 0, "type": "lockedDoor", "opensWith": "k1" }
      ]
    })";
    const core::LevelLoadResult loaded = core::LevelLoader::loadFromString(LEVEL);
    ASSERT_TRUE(loaded.ok()) << loaded.error;

    const std::string json = core::LevelWriter::toJsonString(*loaded.level);
    const core::LevelLoadResult reloaded = core::LevelLoader::loadFromString(json);
    ASSERT_TRUE(reloaded.ok()) << reloaded.error;

    EXPECT_EQ(reloaded.level->tileMap().tile(2, 0), core::TileType::Key);
    EXPECT_EQ(reloaded.level->tileMap().tile(3, 0), core::TileType::LockedDoor);
    ASSERT_EQ(reloaded.level->mechanisms().size(), 1u);
    EXPECT_EQ(reloaded.level->mechanisms().front().switchPosition, (core::GridPosition{2, 0}));
    EXPECT_EQ(reloaded.level->mechanisms().front().doorPosition, (core::GridPosition{3, 0}));
}

/**
 * @brief Un bloc poussable survit au round-trip (sérialisation puis rechargement), `TileType`
 * préservé (`EX-GP-022`).
 * \castest{<b>Un bloc poussable survit au round-trip.</b><br/>
 * \tcat Unitaire · Level Writer<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un bloc poussable survit au round-trip : `TileType` préservé.
 * }
 */
TEST(LevelWriterTest, BlocPoussableSurvitAuRoundTrip) {
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
    const core::LevelLoadResult loaded = core::LevelLoader::loadFromString(LEVEL);
    ASSERT_TRUE(loaded.ok()) << loaded.error;

    const std::string json = core::LevelWriter::toJsonString(*loaded.level);
    const core::LevelLoadResult reloaded = core::LevelLoader::loadFromString(json);
    ASSERT_TRUE(reloaded.ok()) << reloaded.error;

    EXPECT_EQ(reloaded.level->tileMap().tile(2, 0), core::TileType::Block);
    EXPECT_TRUE(reloaded.level->mechanisms().empty());
}


/**
 * @brief Le fond et le jeu de skins survivent au round-trip (sérialisation puis rechargement,
 * `EX-REN-044`, `EX-EDIT-024`).
 * \castest{<b>Le fond et le jeu de skins survivent au round-trip.</b><br/>
 * \tcat Unitaire · Level Writer<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Le fond et le jeu de skins survivent au round-trip.
 * }
 */
TEST(LevelWriterTest, FondEtJeuDeSkinsSurviventAuRoundTrip) {
    constexpr const char* LEVEL = R"({
      "background": "forest.png",
      "skinSet": "foret",
      "width": 3, "height": 3,
      "tiles": [ {"x":0,"y":0,"type":"entry"}, {"x":2,"y":2,"type":"exit"} ] })";
    const core::LevelLoadResult loaded = core::LevelLoader::loadFromString(LEVEL);
    ASSERT_TRUE(loaded.ok()) << loaded.error;

    const std::string json = core::LevelWriter::toJsonString(*loaded.level);
    const core::LevelLoadResult reloaded = core::LevelLoader::loadFromString(json);
    ASSERT_TRUE(reloaded.ok()) << reloaded.error;

    ASSERT_TRUE(reloaded.level->background().has_value());
    EXPECT_EQ(*reloaded.level->background(), "forest.png");
    ASSERT_TRUE(reloaded.level->skinSet().has_value());
    EXPECT_EQ(*reloaded.level->skinSet(), "foret");
}

/**
 * @brief Un niveau sans fond ni jeu de skins n'écrit ni l'une ni l'autre clé dans le JSON produit
 * (`EX-REN-044`, `EX-EDIT-024`) ; le champ `"version"`, lui, est toujours écrit.
 * \castest{<b>Sans fond ni jeu de skins, les cles sont absentes du JSON produit.</b><br/>
 * \tcat Unitaire · Level Writer<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Sans fond ni jeu de skins, les cles sont absentes du JSON produit.
 * }
 */
TEST(LevelWriterTest, SansFondNiJeuDeSkinsLesClesSontOmisesDuJson) {
    const core::LevelLoadResult loaded = core::LevelLoader::loadFromString(R"({
        "width": 3, "height": 3,
        "tiles": [ {"x":0,"y":0,"type":"entry"}, {"x":2,"y":2,"type":"exit"} ] })");
    ASSERT_TRUE(loaded.ok()) << loaded.error;

    const std::string json = core::LevelWriter::toJsonString(*loaded.level);
    EXPECT_EQ(json.find("\"background\""), std::string::npos);
    EXPECT_EQ(json.find("\"skinSet\""), std::string::npos);
    EXPECT_NE(json.find("\"version\""), std::string::npos);
}

/**
 * @brief Un interrupteur non relié à une porte est tout de même sérialisé, avec un identifiant.
 * \castest{<b>Un interrupteur non relié à une porte est tout de même sérialisé, avec un
 * identifiant.</b><br/>
 * \tcat Unitaire · Level Writer<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un interrupteur non relié à une porte est tout de même sérialisé, avec un
 * identifiant.
 * }
 */
TEST(LevelWriterTest, InterrupteurNonRelieRegenereUnIdentifiant) {
    const core::LevelLoadResult loaded = core::LevelLoader::loadFromString(R"({
        "width": 3, "height": 3,
        "tiles": [ {"x":0,"y":0,"type":"entry"}, {"x":2,"y":2,"type":"exit"},
                   {"x":1,"y":1,"type":"switch","id":"quelconque"} ] })");
    ASSERT_TRUE(loaded.ok()) << loaded.error;

    const std::string json = core::LevelWriter::toJsonString(*loaded.level);
    const core::LevelLoadResult reloaded = core::LevelLoader::loadFromString(json);
    ASSERT_TRUE(reloaded.ok()) << reloaded.error;
    EXPECT_EQ(reloaded.level->tileMap().tile(1, 1), core::TileType::Switch);
    EXPECT_TRUE(reloaded.level->mechanisms().empty());
}


/**
 * @brief saveToFile écrit un fichier qui se recharge à l'identique (round-trip disque).
 * \castest{<b>saveToFile écrit un fichier qui se recharge à l'identique (round-trip
 * disque).</b><br/>
 * \tcat Unitaire · Level Writer<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu saveToFile écrit un fichier qui se recharge à l'identique (round-trip disque).
 * }
 */
TEST(LevelWriterTest, SaveToFileEcritUnFichierRechargeable) {
    const core::LevelLoadResult loaded = core::LevelLoader::loadFromString(LEVEL_WITH_MECHANISM);
    ASSERT_TRUE(loaded.ok()) << loaded.error;

    const std::filesystem::path path =
        std::filesystem::temp_directory_path() / "jadg_test_save_to_file.json";
    ASSERT_TRUE(core::LevelWriter::saveToFile(*loaded.level, path));

    const core::LevelLoadResult reloaded = core::LevelLoader::loadFromFile(path);
    std::filesystem::remove(path);

    ASSERT_TRUE(reloaded.ok()) << reloaded.error;
    EXPECT_EQ(reloaded.level->name(), loaded.level->name());
    EXPECT_EQ(reloaded.level->entry(), loaded.level->entry());
}

/**
 * @brief saveToFile vers un dossier inexistant échoue proprement (récupérable, EX-NFR-040).
 * \castest{<b>saveToFile vers un dossier inexistant échoue proprement (récupérable,
 * EX-NFR-040).</b><br/>
 * \tcat Unitaire · Level Writer<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu saveToFile vers un dossier inexistant échoue proprement (récupérable, EX-NFR-040).
 * }
 */
TEST(LevelWriterTest, SaveToFileVersDossierInexistantEchoueProprement) {
    const core::LevelLoadResult loaded = core::LevelLoader::loadFromString(LEVEL_WITH_MECHANISM);
    ASSERT_TRUE(loaded.ok()) << loaded.error;

    const std::filesystem::path path = "chemin/inexistant/pas_la/niveau.json";
    EXPECT_FALSE(core::LevelWriter::saveToFile(*loaded.level, path));
}


/**
 * @brief Chacun des trois modes de cadrage, avec une taille de salle personnalisée pour le mode
 * *par salle*, survit à un aller-retour JSON à l'identique (`EX-LVL-006`).
 * \castest{<b>Chacun des trois modes de cadrage survit au round-trip.</b><br/>
 * \tcat Unitaire · Level Writer<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Charger un niveau déclarant chacun des trois modes de cadrage.<br/>2. Sérialiser
 * puis recharger.<br/>
 * \tattendu Le cadrage résolu du niveau rechargé est identique à celui du niveau d'origine,
 * y compris la taille de salle personnalisée du mode *par salle*.
 * }
 */
TEST(LevelWriterTest, ChacunDesTroisModesDeCadrageSurvitAuRoundTrip) {
    constexpr const char* WHOLE_LEVEL = R"({
      "width": 30, "height": 20,
      "cameraFraming": { "mode": "wholeLevel" },
      "tiles": [
        { "x": 1, "y": 1, "type": "entry" },
        { "x": 3, "y": 2, "type": "exit" }
      ]
    })";
    constexpr const char* PER_ROOM = R"({
      "width": 30, "height": 20,
      "cameraFraming": { "mode": "perRoom", "roomWidthTiles": 10, "roomHeightTiles": 8 },
      "tiles": [
        { "x": 1, "y": 1, "type": "entry" },
        { "x": 3, "y": 2, "type": "exit" }
      ]
    })";
    // La taille de vue du mode suivi (EX-REN-017, memes champs que la taille de salle du mode par
    // salle) doit elle aussi survivre au round-trip.
    constexpr const char* FOLLOW = R"({
      "width": 10, "height": 8,
      "cameraFraming": { "mode": "follow", "roomWidthTiles": 8, "roomHeightTiles": 6 },
      "tiles": [
        { "x": 1, "y": 1, "type": "entry" },
        { "x": 3, "y": 2, "type": "exit" }
      ]
    })";

    for (const char* levelJson : {WHOLE_LEVEL, PER_ROOM, FOLLOW}) {
        const core::LevelLoadResult loaded = core::LevelLoader::loadFromString(levelJson);
        ASSERT_TRUE(loaded.ok()) << loaded.error;

        const std::string json = core::LevelWriter::toJsonString(*loaded.level);
        const core::LevelLoadResult reloaded = core::LevelLoader::loadFromString(json);
        ASSERT_TRUE(reloaded.ok()) << reloaded.error;

        EXPECT_EQ(reloaded.level->cameraFraming().mode, loaded.level->cameraFraming().mode);
        EXPECT_EQ(reloaded.level->cameraFraming().roomWidthTiles,
                  loaded.level->cameraFraming().roomWidthTiles);
        EXPECT_EQ(reloaded.level->cameraFraming().roomHeightTiles,
                  loaded.level->cameraFraming().roomHeightTiles);
    }
}

/**
 * @brief Des zones de caméra dessinées à la main survivent à un aller-retour JSON, dans le même
 * ordre (`EX-LVL-007`).
 * \castest{<b>Les zones de caméra dessinées à la main survivent au round-trip.</b><br/>
 * \tcat Unitaire · Level Writer<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Charger un niveau déclarant deux zones de caméra.<br/>2. Sérialiser puis
 * recharger.<br/>
 * \tattendu Les zones rechargées sont identiques, dans le même ordre.
 * }
 */
TEST(LevelWriterTest, ZonesDeCameraSurviventAuRoundTrip) {
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
    const core::LevelLoadResult loaded = core::LevelLoader::loadFromString(LEVEL);
    ASSERT_TRUE(loaded.ok()) << loaded.error;

    const std::string json = core::LevelWriter::toJsonString(*loaded.level);
    const core::LevelLoadResult reloaded = core::LevelLoader::loadFromString(json);
    ASSERT_TRUE(reloaded.ok()) << reloaded.error;

    EXPECT_EQ(reloaded.level->cameraFraming().zones, loaded.level->cameraFraming().zones);
}

/**
 * @brief Un niveau dont le cadrage résolu coïncide avec la règle de repli est réécrit **sans** le
 * champ `"cameraFraming"` : aucune régression de round-trip pour les niveaux jamais retouchés sur
 * ce point (`EX-LVL-006`).
 * \castest{<b>Un cadrage identique au repli reste omis du JSON écrit.</b><br/>
 * \tcat Unitaire · Level Writer<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Charger un petit niveau sans champ `cameraFraming` (repli : niveau entier).<br/>
 * 2. Sérialiser.<br/>
 * \tattendu Le JSON produit ne contient pas le champ `"cameraFraming"`.
 * }
 */
TEST(LevelWriterTest, CadrageIdentiqueAuRepliResteOmisDuJson) {
    constexpr const char* LEVEL = R"({
      "width": 4, "height": 3,
      "tiles": [
        { "x": 1, "y": 1, "type": "entry" },
        { "x": 3, "y": 2, "type": "exit" }
      ]
    })";
    const core::LevelLoadResult loaded = core::LevelLoader::loadFromString(LEVEL);
    ASSERT_TRUE(loaded.ok()) << loaded.error;

    const std::string json = core::LevelWriter::toJsonString(*loaded.level);
    EXPECT_EQ(json.find("cameraFraming"), std::string::npos);
}

namespace {

// Un niveau 6x6 dont l'unique plateforme mobile porte les champs de route donnes.
std::string levelWithPlatform(const std::string& platformFields) {
    return R"({
      "name": "N", "width": 6, "height": 6,
      "tiles": [
        { "x": 0, "y": 0, "type": "entry" },
        { "x": 5, "y": 5, "type": "exit" },
        { "x": 1, "y": 1, "type": "movingPlatform")" +
           platformFields + R"( }
      ]
    })";
}

}  // namespace


