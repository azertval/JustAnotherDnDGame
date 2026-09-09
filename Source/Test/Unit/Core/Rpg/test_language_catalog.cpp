// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_language_catalog.cpp
 * @brief Tests unitaires du catalogue des langues (`EX-RPG-042`).
 *
 * Le catalogue livré est lu tel quel : c'est lui que le jeu ouvre, et un test sur une copie
 * fabriquée ne dirait rien du jour où une entrée réelle perd un champ.
 */

#include <filesystem>
#include <fstream>
#include <set>
#include <string>

#include <gtest/gtest.h>

#include "Core/Rpg/CharacterOptions.h"
#include "Core/Rpg/Language.h"

namespace {

const std::filesystem::path RPG{JADG_RPG_DIR};

}  // namespace

TEST(LanguageCatalog, LeCatalogueLivreSeChargeSansErreur) {
    const core::LanguageCatalog catalogue = core::loadLanguages(RPG / "languages");
    EXPECT_TRUE(catalogue.errors.empty()) << catalogue.errors.front();
    // Seize langues : le nombre est fixé par les règles, pas par le corpus. S'il bouge, c'est
    // l'extraction qui a régressé.
    EXPECT_EQ(catalogue.languages.size(), 16U);
}

TEST(LanguageCatalog, LIdentifiantRendLeNomLisible) {
    const core::LanguageCatalog catalogue = core::loadLanguages(RPG / "languages");
    const core::LanguageDefinition* const commun = catalogue.find("common");
    ASSERT_NE(commun, nullptr);
    EXPECT_EQ(commun->name, "commun");
    EXPECT_FALSE(commun->exotic);
}

TEST(LanguageCatalog, LesLanguesExotiquesSontMarquees) {
    const core::LanguageCatalog catalogue = core::loadLanguages(RPG / "languages");
    const core::LanguageDefinition* const infernal = catalogue.find("infernal");
    ASSERT_NE(infernal, nullptr);
    // L'infernal ne se prend pas librement à la création : c'est la donnée qui le dit, et le
    // deviner reviendrait à ouvrir toutes les langues à tout le monde.
    EXPECT_TRUE(infernal->exotic);
}

TEST(LanguageCatalog, UnIdentifiantInconnuNeRendRien) {
    const core::LanguageCatalog catalogue = core::loadLanguages(RPG / "languages");
    EXPECT_EQ(catalogue.find("klingon"), nullptr);
    EXPECT_EQ(catalogue.find(""), nullptr);
}

TEST(LanguageCatalog, UnDossierAbsentEstUneErreurEtNonUnCatalogueVide) {
    const core::LanguageCatalog catalogue = core::loadLanguages(RPG / "langues-inexistantes");
    EXPECT_TRUE(catalogue.languages.empty());
    // Le silence serait pire : un catalogue vide et un dossier absent se ressemblent à l'écran,
    // et seul le second est une panne d'installation.
    EXPECT_FALSE(catalogue.errors.empty());
}

TEST(LanguageCatalog, ToutesLesLanguesDesEspecesSontAuCatalogue) {
    // C'est le contrôle qui compte vraiment : une espèce qui accorde une langue absente du
    // catalogue afficherait son identifiant brut sur la fiche, et personne ne verrait la cause.
    const core::LanguageCatalog catalogue = core::loadLanguages(RPG / "languages");
    const core::CharacterOptions options =
        core::loadCharacterOptions(RPG / "species", RPG / "backgrounds", RPG / "classes");
    ASSERT_FALSE(options.species.empty());

    std::set<std::string> manquantes;
    for (const core::Species& espece : options.species) {
        for (const std::string& langue : espece.languages) {
            if (catalogue.find(langue) == nullptr) {
                manquantes.insert(espece.id + " : " + langue);
            }
        }
    }
    EXPECT_TRUE(manquantes.empty()) << *manquantes.begin();
}
