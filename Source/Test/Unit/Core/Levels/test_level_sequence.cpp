// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_level_sequence.cpp
 * @brief Tests unitaires du chargement de séquence de niveaux (LOT-59 TACHE-04, EX-LVL-013).
 */

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <system_error>

#include <gtest/gtest.h>

#include "Core/Levels/LevelSequence.h"

namespace {

constexpr const char* VALID_SEQUENCE = R"({
  "version": 1,
  "titleKey": "sequence.demo.title",
  "levels": ["a.json", "b.json"]
})";

// Écrit @p content dans @p path (création du fichier, écrasement si déjà présent).
void writeFile(const std::filesystem::path& path, const std::string& content) {
    std::ofstream file(path, std::ios::binary);
    file << content;
}

// Fournit un dossier temporaire vierge par test (créé/supprimé automatiquement) -- même patron que
// LevelFileOps (Source/Test/Unit/HMI/Editor/test_level_file_operations.cpp).
class LevelSequenceDir : public ::testing::Test {
protected:
    std::filesystem::path dir;

    void SetUp() override {
        dir = std::filesystem::temp_directory_path() /
              ("pg_levelsequence_" + std::to_string(reinterpret_cast<std::uintptr_t>(this)));
        std::filesystem::create_directories(dir);
    }
    void TearDown() override {
        std::error_code error;
        std::filesystem::remove_all(dir, error);
    }
};

}  // namespace

/**
 * @brief Une séquence valide se charge dans l'ordre exact du fichier, avec sa clé de titre.
 * \castest{<b>Une séquence valide se charge dans l'ordre exact du fichier.</b><br/>
 * \tcat Unitaire · Level Sequence<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Charger une séquence JSON valide depuis une chaîne.<br/>2. Vérifier l'ordre des
 * niveaux et la clé de titre.<br/>
 * \tattendu La séquence chargée reflète exactement l'ordre et le contenu du fichier.
 * }
 */
TEST(LevelSequenceLoaderTest, SequenceValideSeChargeDansLOrdreExact) {
    const core::LevelSequenceLoadResult result =
        core::LevelSequenceLoader::loadFromString(VALID_SEQUENCE);
    ASSERT_TRUE(result.ok()) << result.error;
    EXPECT_EQ(result.errorCode, core::LevelSequenceError::None);
    EXPECT_EQ(result.sequence->titleKey, "sequence.demo.title");
    ASSERT_EQ(result.sequence->levels.size(), 2u);
    EXPECT_EQ(result.sequence->levels[0], "a.json");
    EXPECT_EQ(result.sequence->levels[1], "b.json");
}

/**
 * @brief Une séquence sans champ `"version"` se charge sans erreur, comme la version initiale du
 * format (`EX-LVL-005`, même rétrocompatibilité que `LevelLoader`).
 * \castest{<b>Une séquence sans champ version se charge sans erreur.</b><br/>
 * \tcat Unitaire · Level Sequence<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Charger une séquence JSON sans champ `version`.<br/>2. Vérifier le succès.<br/>
 * \tattendu Le chargement réussit sans erreur ni avertissement.
 * }
 */
TEST(LevelSequenceLoaderTest, SequenceSansVersionSeChargeSansErreur) {
    const core::LevelSequenceLoadResult result =
        core::LevelSequenceLoader::loadFromString(R"({ "levels": ["a.json"] })");
    EXPECT_TRUE(result.ok()) << result.error;
}

/**
 * @brief Une séquence dont la version dépasse celle gérée échoue proprement (`EX-LVL-005`).
 * \castest{<b>Une séquence dont la version dépasse celle gérée échoue proprement.</b><br/>
 * \tcat Unitaire · Level Sequence<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Charger une séquence dont `version` dépasse la version gérée.<br/>2. Vérifier
 * l'échec et son code.<br/>
 * \tattendu L'échec est récupérable, nommant `UnsupportedFormatVersion`.
 * }
 */
TEST(LevelSequenceLoaderTest, VersionSuperieureALaVersionGereeEchoueProprement) {
    const core::LevelSequenceLoadResult result =
        core::LevelSequenceLoader::loadFromString(R"({ "version": 999, "levels": ["a.json"] })");
    EXPECT_FALSE(result.ok());
    EXPECT_EQ(result.errorCode, core::LevelSequenceError::UnsupportedFormatVersion);
}

/**
 * @brief Un JSON malformé échoue proprement, sans exception qui remonte.
 * \castest{<b>Un JSON malformé échoue proprement.</b><br/>
 * \tcat Unitaire · Level Sequence<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Charger une chaîne qui n'est pas du JSON valide.<br/>2. Vérifier l'échec et son
 * code.<br/>
 * \tattendu L'échec est récupérable, nommant `ParseError`, sans exception.
 * }
 */
TEST(LevelSequenceLoaderTest, JsonMalformeEchoueProprement) {
    const core::LevelSequenceLoadResult result =
        core::LevelSequenceLoader::loadFromString("{ pas du json");
    EXPECT_FALSE(result.ok());
    EXPECT_FALSE(result.error.empty());
    EXPECT_EQ(result.errorCode, core::LevelSequenceError::ParseError);
}

/**
 * @brief Une liste de niveaux vide échoue proprement, nommant le problème.
 * \castest{<b>Une liste de niveaux vide échoue proprement.</b><br/>
 * \tcat Unitaire · Level Sequence<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Charger une séquence dont `levels` est une liste vide.<br/>2. Vérifier l'échec et
 * son code.<br/>
 * \tattendu L'échec est récupérable, nommant `EmptySequence`.
 * }
 */
TEST(LevelSequenceLoaderTest, ListeDeNiveauxVideEchoueProprement) {
    const core::LevelSequenceLoadResult result =
        core::LevelSequenceLoader::loadFromString(R"({ "levels": [] })");
    EXPECT_FALSE(result.ok());
    EXPECT_EQ(result.errorCode, core::LevelSequenceError::EmptySequence);
}

/**
 * @brief Charger un fichier de séquence inexistant échoue proprement (récupérable).
 * \castest{<b>Charger un fichier de séquence inexistant échoue proprement.</b><br/>
 * \tcat Unitaire · Level Sequence<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Charger un chemin de fichier inexistant.<br/>2. Vérifier l'échec et son code.<br/>
 * \tattendu L'échec est récupérable, nommant `FileNotFound`.
 * }
 */
TEST_F(LevelSequenceDir, FichierIntrouvableRejete) {
    const core::LevelSequenceLoadResult result =
        core::LevelSequenceLoader::loadFromFile(dir / "pas_la.json");
    EXPECT_FALSE(result.ok());
    EXPECT_FALSE(result.error.empty());
    EXPECT_EQ(result.errorCode, core::LevelSequenceError::FileNotFound);
}

/**
 * @brief Un niveau référencé mais absent du dossier échoue proprement, nommant le fautif -- pas un
 * chargement hors bornes différé au premier niveau joué.
 * \castest{<b>Un niveau référencé mais absent échoue proprement, nommant le fautif.</b><br/>
 * \tcat Unitaire · Level Sequence<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Écrire un fichier de séquence référençant un niveau absent du même dossier.<br/>2.
 * Charger ce fichier.<br/>3. Vérifier l'échec, son code, et que le nom du niveau fautif apparaît
 * dans le message.<br/>
 * \tattendu L'échec est récupérable, nommant `MissingLevelFile` et le fichier fautif.
 * }
 */
TEST_F(LevelSequenceDir, NiveauReferenceMaisAbsentEchoueProprementEtNommeLeFautif) {
    writeFile(dir / "sequence.json", R"({ "levels": ["absent.json"] })");

    const core::LevelSequenceLoadResult result =
        core::LevelSequenceLoader::loadFromFile(dir / "sequence.json");
    EXPECT_FALSE(result.ok());
    EXPECT_EQ(result.errorCode, core::LevelSequenceError::MissingLevelFile);
    EXPECT_NE(result.error.find("absent.json"), std::string::npos) << result.error;
}

/**
 * @brief Un fichier de séquence dont tous les niveaux référencés existent à côté de lui se charge.
 * \castest{<b>Une séquence dont tous les niveaux référencés existent se charge.</b><br/>
 * \tcat Unitaire · Level Sequence<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Écrire un fichier de séquence et les niveaux qu'il référence, dans le même
 * dossier.<br/>2. Charger le fichier de séquence.<br/>
 * \tattendu Le chargement réussit.
 * }
 */
TEST_F(LevelSequenceDir, SequenceDontTousLesNiveauxExistentSeCharge) {
    writeFile(dir / "present.json", "{}");
    writeFile(dir / "sequence.json", R"({ "levels": ["present.json"] })");

    const core::LevelSequenceLoadResult result =
        core::LevelSequenceLoader::loadFromFile(dir / "sequence.json");
    EXPECT_TRUE(result.ok()) << result.error;
}

