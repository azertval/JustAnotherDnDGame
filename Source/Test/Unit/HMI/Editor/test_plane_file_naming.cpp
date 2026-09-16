// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

/**
 * @file test_plane_file_naming.cpp
 * @brief Tests unitaires du nommage et des dimensions des fichiers de plan (`EX-EDIT-047`,
 *        `EX-DEC-041`, LOT-69 TACHE-08).
 */

#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "HMI/Editor/PlaneFileNaming.h"

/**
 * @brief Les dimensions du PNG d'un plan découlent de la taille du niveau et de la densité.
 * \castest{<b>Les dimensions d'un plan decoulent de la taille et de la densite.</b><br/>
 * \tcat Unitaire · Nommage des plans<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Deriver les dimensions d'un niveau 50x26 aux trois densites admises.<br/>
 * \tattendu Chaque densite donne la taille du niveau multipliee par elle.
 * }
 */
TEST(PlaneFileNamingTest, DimensionsDecoulentDeLaTailleEtDeLaDensite) {
    EXPECT_EQ(hmi::planePixelSize(50, 26, 16), (hmi::PlanePixelSize{800, 416}));
    EXPECT_EQ(hmi::planePixelSize(50, 26, 8), (hmi::PlanePixelSize{400, 208}));
    EXPECT_EQ(hmi::planePixelSize(50, 26, 4), (hmi::PlanePixelSize{200, 104}));
}

/**
 * @brief Une densité hors format ou une taille dégénérée ne produit aucune dimension : mieux vaut
 * refuser que créer un fichier d'une taille que rien ne saura relire.
 * \castest{<b>Une densite ou une taille invalide ne produit aucune dimension.</b><br/>
 * \tcat Unitaire · Nommage des plans<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Deriver les dimensions avec une densite hors format, puis avec une largeur
 * nulle.<br/>
 * \tattendu Les deux renvoient des dimensions nulles.
 * }
 */
TEST(PlaneFileNamingTest, DensiteOuTailleInvalideRefusee) {
    EXPECT_EQ(hmi::planePixelSize(50, 26, 6), (hmi::PlanePixelSize{0, 0}));
    EXPECT_EQ(hmi::planePixelSize(0, 26, 16), (hmi::PlanePixelSize{0, 0}));
    EXPECT_EQ(hmi::planePixelSize(50, -1, 16), (hmi::PlanePixelSize{0, 0}));
}

/**
 * @brief Le nom dérive de celui du niveau, et un **suffixe numérique croissant** le rend unique.
 * \castest{<b>Le nom d'un plan est unique par suffixe numerique croissant.</b><br/>
 * \tcat Unitaire · Nommage des plans<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Demander un nom pour un dossier vide.<br/>2. Le redemander avec ce nom deja pris,
 * puis avec les deux premiers pris.<br/>
 * \tattendu foret.png, puis foret-2.png, puis foret-3.png.
 * }
 */
TEST(PlaneFileNamingTest, NomUniqueParSuffixeCroissant) {
    EXPECT_EQ(hmi::uniquePlaneFileName("foret", {}), "foret.png");
    EXPECT_EQ(hmi::uniquePlaneFileName("foret", {"foret.png"}), "foret-2.png");
    EXPECT_EQ(hmi::uniquePlaneFileName("foret", {"foret.png", "foret-2.png"}), "foret-3.png");
    // Un trou dans la suite est comble, plutot que d'incrementer indefiniment.
    EXPECT_EQ(hmi::uniquePlaneFileName("foret", {"foret.png", "foret-3.png"}), "foret-2.png");
}

/**
 * @brief Les caractères refusés sont **retirés**, l'espace devient un tiret, et les tirets de bord
 * disparaissent — un nom qui perd ses accents reste lisible.
 * \castest{<b>Les caracteres refuses sont retires du nom de plan.</b><br/>
 * \tcat Unitaire · Nommage des plans<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Assainir des noms contenant espaces, accents, ponctuation et tirets de bord.<br/>
 * \tattendu Seuls lettres, chiffres, tiret et tiret bas subsistent, sans tiret aux extremites.
 * }
 */
TEST(PlaneFileNamingTest, CaracteresRefusesRetires) {
    EXPECT_EQ(hmi::sanitizePlaneBaseName("Foret Sombre"), "Foret-Sombre");
    EXPECT_EQ(hmi::sanitizePlaneBaseName("niveau/../secret"), "niveausecret");
    EXPECT_EQ(hmi::sanitizePlaneBaseName("--bord--"), "bord");
    EXPECT_EQ(hmi::sanitizePlaneBaseName("ciel_2"), "ciel_2");
}

/**
 * @brief Un nom qui ne laisse **aucun** caractère utilisable ne produit pas de fichier plutôt
 * qu'un fichier au nom vide.
 * \castest{<b>Un nom sans caractere utilisable ne produit aucun fichier.</b><br/>
 * \tcat Unitaire · Nommage des plans<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Demander un nom pour un niveau nomme uniquement de ponctuation.<br/>
 * \tattendu Le nom renvoye est vide.
 * }
 */
TEST(PlaneFileNamingTest, NomSansCaractereUtilisableRefuse) {
    EXPECT_TRUE(hmi::uniquePlaneFileName("///", {}).empty());
    EXPECT_TRUE(hmi::uniquePlaneFileName("", {}).empty());
}
