// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_parchment_frame.cpp
 * @brief Tests unitaires de la géométrie de l'encadrement de parchemin (`LOT-66`, `EX-IHM-070`).
 */

#include <algorithm>
#include <optional>
#include <vector>

#include <gtest/gtest.h>

#include "HMI/Presentation/ParchmentFrame.h"

namespace {

[[nodiscard]] bool containsRole(const std::vector<hmi::ParchmentStroke>& strokes,
                                hmi::ParchmentRole role) {
    return std::any_of(strokes.begin(), strokes.end(),
                       [role](const hmi::ParchmentStroke& stroke) { return stroke.role == role; });
}

/// Role de la DERNIERE bande couvrant (@p x, @p y) : les bandes sont rendues dans l'ordre, la
/// derniere est donc celle que le joueur voit. Comparer le premier recouvrement donnerait toujours
/// le champ, qui couvre tout le widget.
[[nodiscard]] std::optional<hmi::ParchmentRole> topRoleAt(
    const std::vector<hmi::ParchmentStroke>& strokes, int x, int y) {
    std::optional<hmi::ParchmentRole> role;
    for (const hmi::ParchmentStroke& stroke : strokes) {
        if (x >= stroke.x && x < stroke.x + stroke.width && y >= stroke.y &&
            y < stroke.y + stroke.height) {
            role = stroke.role;
        }
    }
    return role;
}

}  // namespace

/**
 * @brief Aucune bande ne déborde de l'encadrement, à aucune taille ni aucune échelle — y compris
 *        quand le cadre est plus petit que ses propres bordures. Un débordement peindrait
 * par-dessus le widget voisin, défaut invisible en revue de code et criant à l'écran. Même garde
 * que `GeometrieDesIconesNonVideEtDansLeCadre` pour les icônes.
 * \castest{<b>Aucune bande de l'encadrement ne sort du rectangle du widget.</b><br/>
 * \tcat Unitaire · Encadrement de parchemin<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Produire la geometrie pour un balayage de tailles et d'echelles.<br/>2. Verifier
 * que chaque bande a des dimensions positives et reste dans [0,largeur] x [0,hauteur].<br/>
 * \tattendu Aucune bande ne deborde ni n'est vide.
 * }
 */
TEST(ParchmentFrameTest, AucuneBandeNeDeborde) {
    for (const int scale : {1, 2, 3}) {
        for (int size = 1; size <= 200; ++size) {
            const std::vector<hmi::ParchmentStroke> strokes =
                hmi::parchmentFrameStrokes(size, size, scale);
            for (const hmi::ParchmentStroke& stroke : strokes) {
                EXPECT_GT(stroke.width, 0) << "taille " << size << " echelle " << scale;
                EXPECT_GT(stroke.height, 0) << "taille " << size << " echelle " << scale;
                EXPECT_GE(stroke.x, 0);
                EXPECT_GE(stroke.y, 0);
                EXPECT_LE(stroke.x + stroke.width, size)
                    << "taille " << size << " echelle " << scale;
                EXPECT_LE(stroke.y + stroke.height, size)
                    << "taille " << size << " echelle " << scale;
            }
        }
    }
}

/**
 * @brief Un encadrement de taille utile porte les quatre rôles. Sans son filet ornemental ni son
 *        ombre, il ne resterait qu'un trait sur un aplat : une bordure ordinaire, pas l'encadrement
 *        d'une feuille de personnage.
 * \castest{<b>Un encadrement de taille utile porte les quatre roles de couleur.</b><br/>
 * \tcat Unitaire · Encadrement de parchemin<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Produire la geometrie d'un cadre de 120 x 80 a l'echelle 2.<br/>2. Chercher chacun
 * des quatre roles.<br/>
 * \tattendu Les quatre roles sont presents.
 * }
 */
TEST(ParchmentFrameTest, LesQuatreRolesSontPresents) {
    const std::vector<hmi::ParchmentStroke> strokes = hmi::parchmentFrameStrokes(120, 80, 2);
    EXPECT_TRUE(containsRole(strokes, hmi::ParchmentRole::Field));
    EXPECT_TRUE(containsRole(strokes, hmi::ParchmentRole::Edge));
    EXPECT_TRUE(containsRole(strokes, hmi::ParchmentRole::Ornament));
    EXPECT_TRUE(containsRole(strokes, hmi::ParchmentRole::Shadow));
}

/**
 * @brief Entre le trait extérieur et le filet ornemental subsiste une **réserve de parchemin**.
 *        C'est elle, et rien d'autre, qui fait lire les deux traits comme un encadrement : sans
 *        elle ils se touchent, et l'ensemble devient une bordure épaisse de deux tons — un défaut
 *        qu'aucune erreur ne signalerait, puisque toutes les bandes seraient toujours là.
 * \castest{<b>Une reserve de parchemin separe le trait exterieur du filet ornemental.</b><br/>
 * \tcat Unitaire · Encadrement de parchemin<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Produire la geometrie d'un cadre de 120 x 80 a l'echelle 2.<br/>2. Relever le role
 * visible a mi-hauteur, une unite a l'interieur du trait exterieur.<br/>
 * \tattendu Le role visible y est le champ, pas le filet.
 * }
 */
TEST(ParchmentFrameTest, UneReserveSepareLeTraitDuFilet) {
    constexpr int UNIT = 2;
    const std::vector<hmi::ParchmentStroke> strokes = hmi::parchmentFrameStrokes(120, 80, UNIT);

    EXPECT_EQ(topRoleAt(strokes, 0, 40), hmi::ParchmentRole::Edge);
    EXPECT_EQ(topRoleAt(strokes, UNIT, 40), hmi::ParchmentRole::Field) << "reserve de parchemin";
    EXPECT_EQ(topRoleAt(strokes, 2 * UNIT, 40), hmi::ParchmentRole::Ornament);
    EXPECT_EQ(topRoleAt(strokes, 3 * UNIT, 40), hmi::ParchmentRole::Shadow);
}

/**
 * @brief Les quatre angles portent un **cabochon doré**, posé par-dessus le trait. C'est l'inverse
 *        exact de l'entaille du pixel art, qui retirait de la matière aux angles ; l'oublier
 *        redonnerait au cadre des coins nus, et rien ne distinguerait plus l'encadrement d'un
 *        rectangle tracé.
 * \castest{<b>Les quatre angles du cadre portent un cabochon ornemental.</b><br/>
 * \tcat Unitaire · Encadrement de parchemin<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Produire la geometrie d'un cadre de 120 x 80 a l'echelle 2.<br/>2. Relever le role
 * visible au pixel de chacun des quatre angles.<br/>
 * \tattendu Les quatre angles portent le role ornemental.
 * }
 */
TEST(ParchmentFrameTest, LesAnglesPortentUnCabochon) {
    constexpr int WIDTH = 120;
    constexpr int HEIGHT = 80;
    const std::vector<hmi::ParchmentStroke> strokes = hmi::parchmentFrameStrokes(WIDTH, HEIGHT, 2);

    EXPECT_EQ(topRoleAt(strokes, 0, 0), hmi::ParchmentRole::Ornament);
    EXPECT_EQ(topRoleAt(strokes, WIDTH - 1, 0), hmi::ParchmentRole::Ornament);
    EXPECT_EQ(topRoleAt(strokes, 0, HEIGHT - 1), hmi::ParchmentRole::Ornament);
    EXPECT_EQ(topRoleAt(strokes, WIDTH - 1, HEIGHT - 1), hmi::ParchmentRole::Ornament);
}

/**
 * @brief L'encadrement est symétrique : la bordure gauche et la bordure droite ont la même
 *        épaisseur, de même que la haute et la basse. Une asymétrie d'un pixel est invisible en
 *        relecture et saute aux yeux une fois quatre cadres alignés à l'écran.
 * \castest{<b>Les bordures opposees de l'encadrement ont la meme epaisseur.</b><br/>
 * \tcat Unitaire · Encadrement de parchemin<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Produire la geometrie d'un cadre rectangulaire.<br/>2. Mesurer l'epaisseur des
 * quatre bandes du trait exterieur.<br/>
 * \tattendu Gauche egale droite, haut egale bas.
 * }
 */
TEST(ParchmentFrameTest, LesBorduresOpposeesSontSymetriques) {
    constexpr int WIDTH = 160;
    constexpr int HEIGHT = 96;
    const std::vector<hmi::ParchmentStroke> strokes = hmi::parchmentFrameStrokes(WIDTH, HEIGHT, 3);

    int left = 0;
    int right = 0;
    int top = 0;
    int bottom = 0;
    for (const hmi::ParchmentStroke& stroke : strokes) {
        if (stroke.role != hmi::ParchmentRole::Edge) {
            continue;
        }
        // Une bande est VERTICALE ou HORIZONTALE selon sa forme, jamais selon sa seule abscisse :
        // les bandes horizontales partent elles aussi de x = 0, et les classer par la position les
        // ferait passer pour la bordure gauche -- large de tout le cadre.
        const bool vertical = stroke.height > stroke.width;
        if (vertical) {
            if (stroke.x == 0) {
                left = stroke.width;
            } else if (stroke.x + stroke.width == WIDTH) {
                right = stroke.width;
            }
        } else {
            if (stroke.y == 0) {
                top = stroke.height;
            } else if (stroke.y + stroke.height == HEIGHT) {
                bottom = stroke.height;
            }
        }
    }
    EXPECT_GT(left, 0);
    EXPECT_EQ(left, right);
    EXPECT_GT(top, 0);
    EXPECT_EQ(top, bottom);
}

/**
 * @brief Une taille nulle ou négative ne produit rien, et une taille trop petite pour porter un
 *        encadrement produit un simple champ plutôt qu'une géométrie dégénérée.
 * \castest{<b>Les tailles degenerees produisent le vide ou un champ nu, jamais une geometrie
 * invalide.</b><br/>
 * \tcat Unitaire · Encadrement de parchemin<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Demander la geometrie pour une largeur ou une hauteur nulle, puis negative.<br/>
 * 2. Demander celle d'un cadre de 2 x 2 a l'echelle 3.<br/>
 * \tattendu Les cas nuls et negatifs sont vides ; le cadre minuscule porte une unique bande de
 * champ couvrant tout.
 * }
 */
TEST(ParchmentFrameTest, TaillesDegenereesSansGeometrieInvalide) {
    EXPECT_TRUE(hmi::parchmentFrameStrokes(0, 40, 2).empty());
    EXPECT_TRUE(hmi::parchmentFrameStrokes(40, 0, 2).empty());
    EXPECT_TRUE(hmi::parchmentFrameStrokes(-10, -10, 2).empty());

    const std::vector<hmi::ParchmentStroke> tiny = hmi::parchmentFrameStrokes(2, 2, 3);
    ASSERT_EQ(tiny.size(), 1u);
    EXPECT_EQ(tiny.front().role, hmi::ParchmentRole::Field);
    EXPECT_EQ(tiny.front().width, 2);
    EXPECT_EQ(tiny.front().height, 2);
}

/**
 * @brief Le fleuron de focus est une pointe **échancrée** tournée vers la droite, contenue dans son
 *        carré (`LOT-66`, `EX-IHM-071`). Un fleuron qui déborderait mordrait sur le texte de
 *        l'entrée ; sans échancrure, il se lirait comme le curseur de saisie d'un champ de texte.
 * \castest{<b>Le fleuron de focus est une pointe echancree contenue dans son carre.</b><br/>
 * \tcat Unitaire · Encadrement de parchemin<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Produire le fleuron pour plusieurs tailles.<br/>2. Verifier le confinement, la
 * symetrie haut/bas, l'orientation a droite et la presence de l'echancrure arriere.<br/>
 * \tattendu Quatre sommets, tous dans le carre, pointe a droite et echancrure non nulle.
 * }
 */
TEST(ParchmentFrameTest, LeFleuronEstUnePointeEchancree) {
    for (const int size : {8, 16, 24, 40}) {
        const std::vector<hmi::ParchmentPoint> points = hmi::focusFleuronPoints(size);
        ASSERT_EQ(points.size(), 4u) << "taille " << size;
        for (const hmi::ParchmentPoint& point : points) {
            EXPECT_GE(point.x, 0) << "taille " << size;
            EXPECT_GE(point.y, 0) << "taille " << size;
            EXPECT_LE(point.x, size) << "taille " << size;
            EXPECT_LE(point.y, size) << "taille " << size;
        }
        // La pointe est le sommet le plus a droite, et se trouve a mi-hauteur.
        EXPECT_EQ(points[1].x, size) << "taille " << size;
        EXPECT_EQ(points[1].y, size / 2) << "taille " << size;
        // Symetrie haut/bas des deux sommets arriere.
        EXPECT_EQ(points[0].y, 0) << "taille " << size;
        EXPECT_EQ(points[2].y, size) << "taille " << size;
        EXPECT_EQ(points[0].x, points[2].x) << "taille " << size;
        // L'echancrure : le quatrieme sommet rentre dans la figure, il ne l'aplatit pas.
        EXPECT_GT(points[3].x, points[0].x) << "taille " << size;
        EXPECT_LT(points[3].x, points[1].x) << "taille " << size;
    }
}

/**
 * @brief Un fleuron trop petit pour porter son échancrure est **vide** plutôt que déformé : mieux
 *        vaut aucune marque qu'une marque qu'on ne reconnaît pas.
 * \castest{<b>Un fleuron trop petit pour son echancrure est vide.</b><br/>
 * \tcat Unitaire · Encadrement de parchemin<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Demander le fleuron pour une taille inferieure a quatre, nulle et negative.<br/>
 * \tattendu Le resultat est vide dans les trois cas.
 * }
 */
TEST(ParchmentFrameTest, FleuronTropPetitEstVide) {
    EXPECT_TRUE(hmi::focusFleuronPoints(3).empty());
    EXPECT_TRUE(hmi::focusFleuronPoints(0).empty());
    EXPECT_TRUE(hmi::focusFleuronPoints(-8).empty());
}
