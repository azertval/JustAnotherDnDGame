// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_identity_scale.cpp
 * @brief Tests unitaires du facteur d'agrandissement des écrans du jeu (`LOT-66`, `EX-IHM-070`).
 */

#include <gtest/gtest.h>

#include "HMI/Interface/IdentityScale.h"

/**
 * @brief Le facteur est toujours un entier d'au moins 1 et n'excède jamais le plafond, pour
 *        n'importe quelle hauteur — y compris absurde (nulle, négative, gigantesque). Un facteur
 *        nul ferait disparaître l'interface ; un facteur non borné remplirait l'écran de trois
 *        entrées de menu.
 * \castest{<b>Le facteur reste entier, superieur a 1 et borne par le plafond.</b><br/>
 * \tcat Unitaire · Echelle de l identite<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Demander le facteur pour des hauteurs allant du negatif au tres grand.<br/>
 * \tattendu Chaque resultat est compris entre 1 et IDENTITY_MAX_SCALE inclus.
 * }
 */
TEST(IdentityScaleTest, FacteurToujoursBorneEntreUnEtLePlafond) {
    constexpr int HEIGHTS[] = {-1080, -1, 0, 1, 200, 359, 360, 719, 720, 1079, 1080, 2160, 100000};
    for (const int height : HEIGHTS) {
        const int scale = hmi::identityScaleFor(height);
        EXPECT_GE(scale, 1) << "hauteur " << height;
        EXPECT_LE(scale, hmi::IDENTITY_MAX_SCALE) << "hauteur " << height;
    }
}

/**
 * @brief Le facteur ne décroît jamais quand la fenêtre grandit. C'est ce qui rend le
 *        redimensionnement prévisible : agrandir la fenêtre ne peut pas rapetisser l'interface.
 * \castest{<b>Le facteur croit de facon monotone avec la hauteur de fenetre.</b><br/>
 * \tcat Unitaire · Echelle de l identite<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Parcourir les hauteurs de 0 a 2400 par pas de 1.<br/>2. Comparer chaque facteur au
 * precedent.<br/>
 * \tattendu Le facteur ne diminue jamais.
 * }
 */
TEST(IdentityScaleTest, FacteurMonotoneEnHauteur) {
    int previous = hmi::identityScaleFor(0);
    for (int height = 1; height <= 2400; ++height) {
        const int scale = hmi::identityScaleFor(height);
        EXPECT_GE(scale, previous) << "le facteur a diminue a la hauteur " << height;
        previous = scale;
    }
}

/**
 * @brief Une hauteur juste sous un seuil garde le facteur **inférieur** : la maquette doit tenir
 *        ENTIÈREMENT dans la fenêtre. Arrondir plutôt que tronquer ferait passer une fenêtre de
 *        700 px à l'échelle 2, pour laquelle il manque 20 px — la dernière entrée du menu
 *        passerait sous le bord.
 * \castest{<b>Une hauteur juste insuffisante ne declenche pas le facteur superieur.</b><br/>
 * \tcat Unitaire · Echelle de l identite<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Demander le facteur juste avant et juste apres chaque multiple de la hauteur de
 * base.<br/>
 * \tattendu Le facteur ne monte qu'a partir du multiple exact, jamais avant.
 * }
 */
TEST(IdentityScaleTest, LeSeuilEstAtteintParDefautJamaisParArrondi) {
    for (int factor = 1; factor <= hmi::IDENTITY_MAX_SCALE; ++factor) {
        const int threshold = factor * hmi::IDENTITY_BASE_HEIGHT;
        EXPECT_EQ(hmi::identityScaleFor(threshold), factor);
        EXPECT_EQ(hmi::identityScaleFor(threshold - 1), factor - 1 < 1 ? 1 : factor - 1);
    }
}

/**
 * @brief Les trois hauteurs de fenêtre courantes tombent sur les facteurs annoncés dans les
 *        maquettes : 720p à ×2, 1080p à ×3, et 1440p reste à ×3 (plafond). Ce test fige la
 *        promesse faite au level designer, pas seulement la formule.
 * \castest{<b>Les hauteurs d'ecran courantes donnent les facteurs annonces.</b><br/>
 * \tcat Unitaire · Echelle de l identite<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Demander le facteur pour 360, 720, 1080 et 1440 pixels logiques.<br/>
 * \tattendu Respectivement 1, 2, 3 et 3.
 * }
 */
TEST(IdentityScaleTest, HauteursCourantesDonnentLesFacteursAnnonces) {
    EXPECT_EQ(hmi::identityScaleFor(360), 1);
    EXPECT_EQ(hmi::identityScaleFor(720), 2);
    EXPECT_EQ(hmi::identityScaleFor(1080), 3);
    EXPECT_EQ(hmi::identityScaleFor(1440), 3);
}

/**
 * @brief Le facteur borné n'excède jamais celui qu'admet la zone d'affichage disponible. C'est la
 *        fermeture du cliquet : le facteur grossit les grandeurs d'habillage, qui grossissent la
 *        taille minimale des écrans, qui grossit la fenêtre — laquelle relancerait le calcul un
 *        cran plus haut si la seule hauteur de fenêtre décidait.
 * \castest{<b>Le facteur borne n'excede jamais celui de la zone disponible.</b><br/>
 * \tcat Unitaire · Echelle de l identite<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Demander le facteur borne pour des couples (hauteur de fenetre, zone
 * disponible).<br/>2. Comparer au facteur de la seule zone disponible.<br/>
 * \tattendu Le facteur borne ne depasse jamais celui de la zone disponible.
 * }
 */
TEST(IdentityScaleTest, FacteurBorneParLaZoneDisponible) {
    constexpr int WINDOW_HEIGHTS[] = {200, 360, 720, 1080, 1560, 2400};
    constexpr int AVAILABLE_HEIGHTS[] = {400, 720, 1009, 1080, 1440};
    for (const int available : AVAILABLE_HEIGHTS) {
        for (const int window : WINDOW_HEIGHTS) {
            const int bounded = hmi::identityScaleForDisplay(window, available);
            EXPECT_LE(bounded, hmi::identityScaleFor(available))
                << "fenetre " << window << ", zone disponible " << available;
            EXPECT_GE(bounded, 1) << "fenetre " << window << ", zone disponible " << available;
        }
    }
}

/**
 * @brief Une fenêtre que Qt a été contraint d'agrandir au-delà de l'écran ne gagne aucun facteur
 *        pour autant. C'est exactement le cas signalé : sur un écran de 1009 pixels utiles, une
 *        fenêtre poussée à 1560 réclamait le facteur 3, dont les grandeurs la poussaient plus loin
 *        encore.
 * \castest{<b>Une fenetre plus haute que l'ecran ne gagne pas de facteur.</b><br/>
 * \tcat Unitaire · Echelle de l identite<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Zone disponible de 1009 pixels (cas rapporte).<br/>2. Demander le facteur pour des
 * hauteurs de fenetre croissantes, jusqu'au-dela de l'ecran.<br/>
 * \tattendu Le facteur plafonne a celui de la zone disponible et n'augmente plus.
 * }
 */
TEST(IdentityScaleTest, UneFenetreDebordanteNeGagnePasDeFacteur) {
    constexpr int AVAILABLE = 1009;  // zone utile rapportee par l'utilisateur (ecran « R271 »).
    const int ceiling = hmi::identityScaleFor(AVAILABLE);
    EXPECT_EQ(ceiling, 2) << "1009 px n'admettent pas la maquette a l'echelle 3 (1080 px requis)";
    for (const int window : {1009, 1160, 1476, 1497, 1560, 3000}) {
        EXPECT_EQ(hmi::identityScaleForDisplay(window, AVAILABLE), ceiling)
            << "la fenetre de " << window << " px a gagne un facteur que l'ecran n'admet pas";
    }
}

/**
 * @brief Une zone disponible inconnue laisse la seule hauteur de fenêtre décider. Le cas se produit
 *        avant que la fenêtre ne soit rattachée à un écran : refuser de choisir y vaudrait mieux
 *        qu'un facteur arbitraire, mais l'interface doit tout de même s'afficher.
 * \castest{<b>Une zone disponible inconnue laisse la fenetre decider.</b><br/>
 * \tcat Unitaire · Echelle de l identite<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Demander le facteur borne avec une zone disponible nulle puis negative.<br/>
 * \tattendu Le resultat est celui de `identityScaleFor` sur la seule hauteur de fenetre.
 * }
 */
TEST(IdentityScaleTest, ZoneDisponibleInconnueLaisseLaFenetreDecider) {
    for (const int window : {0, 360, 720, 1080, 2400}) {
        EXPECT_EQ(hmi::identityScaleForDisplay(window, 0), hmi::identityScaleFor(window));
        EXPECT_EQ(hmi::identityScaleForDisplay(window, -1), hmi::identityScaleFor(window));
    }
}

/**
 * @brief Une fenêtre plus petite que l'écran garde son propre facteur : la borne ne doit pas
 *        *imposer* le facteur de l'écran, seulement l'empêcher d'être dépassé. Sans quoi une petite
 *        fenêtre sur un grand écran rendrait une maquette plus grande qu'elle.
 * \castest{<b>Une petite fenetre garde son facteur sur un grand ecran.</b><br/>
 * \tcat Unitaire · Echelle de l identite<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Zone disponible de 1440 pixels.<br/>2. Demander le facteur pour des fenetres de 360
 * et 720 pixels.<br/>
 * \tattendu Respectivement 1 et 2, jamais le facteur 3 que l'ecran admettrait.
 * }
 */
TEST(IdentityScaleTest, UnePetiteFenetreGardeSonFacteur) {
    EXPECT_EQ(hmi::identityScaleForDisplay(360, 1440), 1);
    EXPECT_EQ(hmi::identityScaleForDisplay(720, 1440), 2);
    EXPECT_EQ(hmi::identityScaleForDisplay(1080, 1440), 3);
}
