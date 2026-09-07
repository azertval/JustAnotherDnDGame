// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_character_sheet_plate.cpp
 * @brief Tests de la table de la **planche gravée** de la fiche de personnage (`LOT-38`).
 *
 * La table livrée (`Assets/UI/character-sheet-plate.json`) est lue ici, et non une copie : c'est
 * elle que le jeu charge, et elle que `scripts/build_character_sheet_plate.py` a suivie pour
 * retirer le lettrage anglais du tracé. Une fixture serait une seconde source, qui divergerait au
 * premier rectangle déplacé — exactement ce que cette table existe pour éviter.
 *
 * Ce que ces tests tiennent, et que rien d'autre ne tient :
 * - un `valueId` que `hmi::characterSheetValues` ne produit pas laisse un champ au tiret **au
 *   milieu de champs remplis**, ce qui ne se voit pas à l'écran ;
 * - une `labelKey` absente des catalogues affiche la clé brute, ou du vide ;
 * - un rectangle sorti de la page efface du tracé là où personne ne regarde.
 */

#include <filesystem>
#include <fstream>
#include <map>
#include <set>
#include <sstream>
#include <string>

#include <gtest/gtest.h>

#include "HMI/Interface/CharacterSheetPlate.h"
#include "HMI/Localization/Localization.h"

namespace {

[[nodiscard]] std::string lire(const std::filesystem::path& chemin) {
    std::ifstream flux(chemin, std::ios::binary);
    EXPECT_TRUE(flux.is_open()) << "fichier absent : " << chemin.string();
    std::ostringstream tampon;
    tampon << flux.rdbuf();
    return tampon.str();
}

/// La table LIVRÉE, celle que l'écran charge et que le script d'effacement a suivie.
[[nodiscard]] const hmi::CharacterSheetPlateLayout& planche() {
    static const hmi::CharacterSheetPlateLayout table = hmi::parseCharacterSheetPlate(
        lire(std::filesystem::path(JADG_ASSETS_DIR) / "UI" / "character-sheet-plate.json"));
    return table;
}

}  // namespace

/**
 * @brief La table de la planche livree se lit, et sa page est celle de la maquette.
 * \castest{<b>La table de la planche livree se lit, et sa page est celle de la maquette.</b><br/>
 * 	cat Unitaire · Fiche de personnage<br/>
 * 	crit Critique<br/>
 * 	etapes 1. Lire Assets/UI/character-sheet-plate.json.<br/>2. Verifier la taille de page.<br/>
 * 	attendu La table est valide et sa page mesure 816 x 1056, le systeme de coordonnees de tous ses rectangles.
 * }
 */
TEST(CharacterSheetPlateTest, LaTableLivreeSeLit) {
    ASSERT_TRUE(planche().isValid());
    // La page est celle de la maquette : c'est le système de coordonnées de tous les rectangles,
    // et celui du `viewBox` de la gravure. Les deux doivent rester d'accord.
    EXPECT_DOUBLE_EQ(planche().pageWidth, 816.0);
    EXPECT_DOUBLE_EQ(planche().pageHeight, 1056.0);
}

/**
 * @brief Aucun champ de la planche n'est declare deux fois.
 * \castest{<b>Aucun champ de la planche n'est declare deux fois.</b><br/>
 * 	cat Unitaire · Fiche de personnage<br/>
 * 	crit Majeur<br/>
 * 	etapes 1. Parcourir les champs de la table.<br/>2. Verifier l'unicite des identifiants.<br/>
 * 	attendu Chaque identifiant de champ est unique.
 * }
 */
TEST(CharacterSheetPlateTest, LesIdentifiantsDeChampSontUniques) {
    std::set<std::string> vus;
    for (const hmi::PlateField& champ : planche().fields) {
        EXPECT_TRUE(vus.insert(champ.id).second) << "champ en double : " << champ.id;
    }
}

/**
 * @brief La planche porte les dix-huit competences, les six caracteristiques et les six sauvegardes.
 * \castest{<b>La planche porte les dix-huit competences, les six caracteristiques et les six sauvegardes.</b><br/>
 * 	cat Unitaire · Fiche de personnage<br/>
 * 	crit Critique<br/>
 * 	etapes 1. Compter les champs par role dans la table.<br/>
 * 	attendu 18 competences, 6 caracteristiques et 6 jets de sauvegarde.
 * }
 */
TEST(CharacterSheetPlateTest, LaPlancheCouvreLesDixHuitCompetencesEtLesSixCaracteristiques) {
    int competences = 0;
    int caracteristiques = 0;
    int sauvegardes = 0;
    for (const hmi::PlateField& champ : planche().fields) {
        competences += champ.role == hmi::PlateRole::Skill ? 1 : 0;
        caracteristiques += champ.role == hmi::PlateRole::Ability ? 1 : 0;
        sauvegardes += champ.role == hmi::PlateRole::Save ? 1 : 0;
    }
    EXPECT_EQ(competences, 18);
    EXPECT_EQ(caracteristiques, 6);
    EXPECT_EQ(sauvegardes, 6);
}

/**
 * @brief Chaque intitule de la planche existe en francais et en anglais.
 * \castest{<b>Chaque intitule de la planche existe en francais et en anglais.</b><br/>
 * 	cat Unitaire · Fiche de personnage<br/>
 * 	crit Critique<br/>
 * 	etapes 1. Lire fr.lang et en.lang.<br/>2. Verifier que chaque cle de la table y figure.<br/>
 * 	attendu Aucune cle manquante : la gravure ayant perdu son lettrage, un intitule sans cle n'afficherait rien.
 * }
 */
TEST(CharacterSheetPlateTest, ChaqueIntituleEstTraduitDansLesDeuxLangues) {
    for (const char* langue : {"fr", "en"}) {
        const std::string contenu =
            lire(std::filesystem::path(JADG_LOCALIZATION_DIR) / (std::string(langue) + ".lang"));
        ASSERT_FALSE(contenu.empty()) << "catalogue vide : " << langue;
        const auto catalogue = hmi::Localization::parseCatalog(contenu);
        for (const hmi::PlateField& champ : planche().fields) {
            if (champ.labelKey.empty()) {
                continue;
            }
            EXPECT_TRUE(catalogue.count(champ.labelKey) != 0)
                << champ.id << " porte la cle « " << champ.labelKey << " », absente de " << langue
                << ".lang : la gravure ayant perdu son lettrage, ce champ n'afficherait RIEN";
        }
    }
}

/**
 * @brief Aucun rectangle de la planche ne sort de la page.
 * \castest{<b>Aucun rectangle de la planche ne sort de la page.</b><br/>
 * 	cat Unitaire · Fiche de personnage<br/>
 * 	crit Majeur<br/>
 * 	etapes 1. Parcourir intitules, valeurs et couvertures.<br/>2. Verifier leur inclusion dans la page.<br/>
 * 	attendu Tous les rectangles tiennent dans les 816 x 1056 de la maquette.
 * }
 */
TEST(CharacterSheetPlateTest, AucunRectangleNeSortDeLaPage) {
    const auto dansLaPage = [](const hmi::PlateRect& rectangle) {
        return rectangle.isEmpty() ||
               (rectangle.x >= -1.0 && rectangle.y >= -1.0 &&
                rectangle.x + rectangle.width <= planche().pageWidth + 1.0 &&
                rectangle.y + rectangle.height <= planche().pageHeight + 1.0);
    };
    for (const hmi::PlateField& champ : planche().fields) {
        EXPECT_TRUE(dansLaPage(champ.label)) << champ.id << " : intitule hors page";
        EXPECT_TRUE(dansLaPage(champ.value)) << champ.id << " : valeur hors page";
        for (const hmi::PlateRect& couverture : champ.cover) {
            EXPECT_TRUE(dansLaPage(couverture)) << champ.id << " : couverture hors page";
        }
    }
}

/**
 * @brief Tout champ de la planche pose au moins un intitule ou une valeur.
 * \castest{<b>Tout champ de la planche pose au moins un intitule ou une valeur.</b><br/>
 * 	cat Unitaire · Fiche de personnage<br/>
 * 	crit Majeur<br/>
 * 	etapes 1. Parcourir les champs.<br/>2. Verifier qu'aucun n'est sans intitule ni valeur.<br/>
 * 	attendu Aucun champ efface de la gravure ne reste sans rien pour le remplacer.
 * }
 */
TEST(CharacterSheetPlateTest, ToutChampAffichableAUnIntituleOuUneValeur) {
    for (const hmi::PlateField& champ : planche().fields) {
        // Un champ qui ne pose ni intitule ni valeur n'apparait nulle part : il aurait ete efface
        // de la gravure sans que rien ne le remplace, ce qui laisse un blanc dans la feuille.
        EXPECT_TRUE(!champ.label.isEmpty() || !champ.value.isEmpty())
            << champ.id << " ne pose ni intitule ni valeur";
    }
}

/**
 * @brief Une table illisible rend une planche invalide, sans faire tomber l'application.
 * \castest{<b>Une table illisible rend une planche invalide, sans faire tomber l'application.</b><br/>
 * 	cat Unitaire · Fiche de personnage<br/>
 * 	crit Majeur<br/>
 * 	etapes 1. Analyser du JSON invalide, vide, puis un tableau.<br/>
 * 	attendu La planche est invalide dans les trois cas, et aucune exception n'est levee (EX-NFR-040).
 * }
 */
TEST(CharacterSheetPlateTest, UneTableIllisibleRendUnePlancheInvalidePlutotQueDeLever) {
    // Un asset abime est un cas ATTENDU (EX-NFR-040) : l'ecran montre la planche nue, ce qui se
    // voit, plutot que de faire tomber l'application.
    EXPECT_FALSE(hmi::parseCharacterSheetPlate("{ ceci n'est pas du json").isValid());
    EXPECT_FALSE(hmi::parseCharacterSheetPlate("").isValid());
    EXPECT_FALSE(hmi::parseCharacterSheetPlate("[]").isValid());
}
