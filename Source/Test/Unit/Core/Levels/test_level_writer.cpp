// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

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

constexpr const char* LEVEL_WITH_TERRAIN = R"({
  "name": "Tutoriel",
  "width": 4,
  "height": 3,
  "tiles": [
    { "x": 0, "y": 0, "type": "solid" },
    { "x": 1, "y": 1, "type": "entry" },
    { "x": 0, "y": 1, "type": "water" },
    { "x": 2, "y": 0, "type": "wall" },
    { "x": 3, "y": 0, "type": "bridge" }
  ]
})";

}  // namespace

/**
 * @brief Le JSON produit porte toujours le champ `"version"`, et aucune des clés que le format ne
 * lit plus (fond, jeu de skins, cadrage).
 * \castest{<b>Le JSON produit porte la version et aucune clé retirée du format.</b><br/>
 * \tcat Unitaire · Level Writer<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Le JSON produit porte la version et aucune clé retirée du format.
 * }
 */
TEST(LevelWriterTest, LeJsonProduitPorteLaVersionEtAucuneCleRetiree) {
    const core::LevelLoadResult loaded = core::LevelLoader::loadFromString(R"({
        "width": 3, "height": 3,
        "tiles": [ {"x":0,"y":0,"type":"entry"} ] })");
    ASSERT_TRUE(loaded.ok()) << loaded.error;

    const std::string json = core::LevelWriter::toJsonString(*loaded.level);
    EXPECT_EQ(json.find("\"background\""), std::string::npos);
    EXPECT_EQ(json.find("\"skinSet\""), std::string::npos);
    EXPECT_EQ(json.find("\"cameraFraming\""), std::string::npos);
    EXPECT_NE(json.find("\"version\""), std::string::npos);
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
    const core::LevelLoadResult loaded = core::LevelLoader::loadFromString(LEVEL_WITH_TERRAIN);
    ASSERT_TRUE(loaded.ok()) << loaded.error;

    const std::filesystem::path path =
        std::filesystem::temp_directory_path() / "jadg_test_save_to_file.json";
    ASSERT_TRUE(core::LevelWriter::saveToFile(*loaded.level, path));

    const core::LevelLoadResult reloaded = core::LevelLoader::loadFromFile(path);
    std::filesystem::remove(path);

    ASSERT_TRUE(reloaded.ok()) << reloaded.error;
    EXPECT_EQ(reloaded.level->name(), loaded.level->name());
    EXPECT_EQ(reloaded.level->entry(), loaded.level->entry());
    for (int row = 0; row < loaded.level->tileMap().height(); ++row) {
        for (int column = 0; column < loaded.level->tileMap().width(); ++column) {
            EXPECT_EQ(reloaded.level->tileMap().tile(column, row),
                      loaded.level->tileMap().tile(column, row));
        }
    }
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
    const core::LevelLoadResult loaded = core::LevelLoader::loadFromString(LEVEL_WITH_TERRAIN);
    ASSERT_TRUE(loaded.ok()) << loaded.error;

    const std::filesystem::path path = "chemin/inexistant/pas_la/niveau.json";
    EXPECT_FALSE(core::LevelWriter::saveToFile(*loaded.level, path));
}
