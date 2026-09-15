// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

/**
 * @file test_credits_catalog.cpp
 * @brief Tests unitaires de la lecture des crédits (LOT-87, T3.3).
 */

#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "HMI/Presentation/CreditsCatalog.h"

namespace {

constexpr const char* REFERENCE_JSON = R"({
  "version": 1,
  "sections": [
    {
      "id": "development",
      "column": 0,
      "title": { "fr": "Développement", "en": "Development" },
      "lines": [
        { "role": { "fr": "Programmation", "en": "Programming" }, "names": ["A", "B"] }
      ]
    },
    {
      "id": "audio",
      "column": 1,
      "title": { "fr": "Audio" },
      "lines": [ { "role": { "fr": "Bruitages" }, "names": ["C"] } ]
    }
  ]
})";

}  // namespace

/**
 * @brief Des crédits valides se lisent dans la langue demandée, noms et colonnes compris.
 * \castest{<b>Les crédits se lisent dans la langue demandée.</b><br/>
 * \tcat Unitaire · Crédits<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Lire un JSON de deux sections en anglais.<br/>
 * \tattendu Deux sections, titres et rôles anglais, noms inchangés, colonnes 0 et 1.
 * }
 */
TEST(CreditsCatalogTest, CreditsLusDansLaLangueDemandee) {
    const hmi::CreditsResult result = hmi::readCredits(REFERENCE_JSON, "en");
    ASSERT_TRUE(result.ok()) << result.error;
    ASSERT_EQ(result.sections.size(), 2U);
    EXPECT_EQ(result.sections[0].title, "Development");
    EXPECT_EQ(result.sections[0].column, 0);
    ASSERT_EQ(result.sections[0].lines.size(), 1U);
    EXPECT_EQ(result.sections[0].lines[0].role, "Programming");
    EXPECT_EQ(result.sections[0].lines[0].names, (std::vector<std::string>{"A", "B"}));
    EXPECT_EQ(result.sections[1].column, 1);
}

/**
 * @brief Un libellé sans la langue demandée retombe sur le français, langue source.
 * \castest{<b>Un libellé non traduit des crédits s'affiche en français.</b><br/>
 * \tcat Unitaire · Crédits<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Lire en anglais une section dont le titre et le rôle n'ont que le français.<br/>
 * \tattendu Le titre et le rôle sont ceux du français.
 * }
 */
TEST(CreditsCatalogTest, LibelleNonTraduitRetombeSurLeFrancais) {
    const hmi::CreditsResult result = hmi::readCredits(REFERENCE_JSON, "en");
    ASSERT_TRUE(result.ok()) << result.error;
    EXPECT_EQ(result.sections[1].title, "Audio");
    EXPECT_EQ(result.sections[1].lines[0].role, "Bruitages");
}

/**
 * @brief Un nom peut être un libellé traduit, pour une mention qui n'est pas un nom propre.
 * \castest{<b>Une mention des crédits se traduit comme un rôle.</b><br/>
 * \tcat Unitaire · Crédits<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Lire en anglais une ligne dont les noms mêlent une chaîne, un libellé traduit et un
 * libellé qui n'a que le français.<br/>
 * \tattendu La chaîne est inchangée, le libellé est anglais, le troisième retombe sur le français.
 * }
 */
TEST(CreditsCatalogTest, NomTraduitSuitLaLangue) {
    const hmi::CreditsResult result = hmi::readCredits(
        R"({"sections":[{"id":"story","title":{"fr":"Univers"},"lines":[{"role":{"fr":"R"},)"
        R"("names":["Dragori Games, Inc.",{"fr":"Fan game non officiel","en":"Unofficial fan game"},)"
        R"({"fr":"Non commercial"}]}]}]})",
        "en");
    ASSERT_TRUE(result.ok()) << result.error;
    EXPECT_EQ(
        result.sections[0].lines[0].names,
        (std::vector<std::string>{"Dragori Games, Inc.", "Unofficial fan game", "Non commercial"}));
}

/**
 * @brief Un nom ni chaîne ni libellé traduit fait échouer la lecture.
 * \castest{<b>Un nom de crédits illisible fait échouer la lecture.</b><br/>
 * \tcat Unitaire · Crédits<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Lire une ligne dont un nom est un nombre, puis une autre dont un nom est un libellé
 * vide.<br/>
 * \tattendu Les deux lectures échouent.
 * }
 */
TEST(CreditsCatalogTest, NomIllisibleRefuse) {
    EXPECT_FALSE(
        hmi::readCredits(
            R"({"sections":[{"id":"x","title":{"fr":"X"},"lines":[{"role":{"fr":"R"},"names":[3]}]}]})",
            "fr")
            .ok());
    EXPECT_FALSE(
        hmi::readCredits(
            R"({"sections":[{"id":"x","title":{"fr":"X"},"lines":[{"role":{"fr":"R"},"names":[{"en":""}]}]}]})",
            "fr")
            .ok());
}

/**
 * @brief Une ligne sans nom fait échouer toute la lecture, jamais des crédits partiels.
 * \castest{<b>Une ligne de crédits sans nom fait échouer la lecture.</b><br/>
 * \tcat Unitaire · Crédits<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Lire un JSON dont la seule ligne a une liste de noms vide.<br/>
 * \tattendu La lecture échoue, aucune section n'est rendue, l'erreur nomme la section.
 * }
 */
TEST(CreditsCatalogTest, LigneSansNomFaitEchouerLaLecture) {
    const hmi::CreditsResult result = hmi::readCredits(
        R"({"sections":[{"id":"x","title":{"fr":"X"},"lines":[{"role":{"fr":"R"},"names":[]}]}]})",
        "fr");
    EXPECT_FALSE(result.ok());
    EXPECT_TRUE(result.sections.empty());
    EXPECT_NE(result.error.find("x"), std::string::npos);
}

/**
 * @brief Une colonne hors de 0 et 1 est refusée.
 * \castest{<b>Une section de crédits hors des deux colonnes est refusée.</b><br/>
 * \tcat Unitaire · Crédits<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Lire une section de colonne 2.<br/>
 * \tattendu La lecture échoue.
 * }
 */
TEST(CreditsCatalogTest, ColonneHorsBornesRefusee) {
    const hmi::CreditsResult result = hmi::readCredits(
        R"({"sections":[{"id":"x","column":2,"title":{"fr":"X"},"lines":[{"role":{"fr":"R"},"names":["N"]}]}]})",
        "fr");
    EXPECT_FALSE(result.ok());
}

/**
 * @brief Le fichier livré se lit dans les deux langues, et remplit les deux colonnes.
 * \castest{<b>Le fichier de crédits livré se lit en français et en anglais.</b><br/>
 * \tcat Unitaire · Crédits<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Lire `Source/Elements/Credits/credits.json` en français, puis en anglais.<br/>
 * \tattendu Les deux lectures aboutissent, avec au moins une section dans chaque colonne.
 * }
 */
TEST(CreditsCatalogTest, FichierLivreSeLitDansLesDeuxLangues) {
    std::ifstream file(JADG_CREDITS_FILE, std::ios::binary);
    ASSERT_TRUE(file) << JADG_CREDITS_FILE;
    std::stringstream buffer;
    buffer << file.rdbuf();
    for (const char* language : {"fr", "en"}) {
        const hmi::CreditsResult result = hmi::readCredits(buffer.str(), language);
        ASSERT_TRUE(result.ok()) << language << " : " << result.error;
        bool left = false;
        bool right = false;
        for (const hmi::CreditSection& section : result.sections) {
            left = left || section.column == 0;
            right = right || section.column == 1;
        }
        EXPECT_TRUE(left && right) << language;
    }
}
