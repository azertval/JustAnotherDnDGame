// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

/**
 * @file test_thumbnail_geometry.cpp
 * @brief Tests unitaires du dimensionnement des vignettes à l'échelle d'affichage
 *        (`EX-IHM-053`).
 */

#include <gtest/gtest.h>

#include "Editor/Logic/ThumbnailGeometry.h"

/**
 * @brief Pour une taille logique et un facteur d'échelle donnés, la taille en pixels réels
 *        attendue est produite, aux facteurs usuels 1, 1,25, 1,5 et 2.
 * \castest{<b>Le dimensionnement produit la taille en pixels reels attendue.</b><br/>
 * \tcat Unitaire · Vignettes<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Calculer la taille en pixels pour une taille logique de 48 aux facteurs 1, 1.25,
 * 1.5 et 2.<br/>
 * \tattendu Les tailles produites sont 48, 60, 72 et 96.
 * }
 */
TEST(ThumbnailGeometryTest, DimensionnementAuxFacteursUsuels) {
    EXPECT_EQ(hmi::thumbnailPixelSize(48, 1.0), 48);
    EXPECT_EQ(hmi::thumbnailPixelSize(48, 1.25), 60);
    EXPECT_EQ(hmi::thumbnailPixelSize(48, 1.5), 72);
    EXPECT_EQ(hmi::thumbnailPixelSize(48, 2.0), 96);
}

/**
 * @brief Un facteur non entier ne produit jamais de dimension nulle, même pour la plus petite
 *        taille d'icône employée.
 * \castest{<b>Un facteur non entier ne produit jamais de dimension nulle.</b><br/>
 * \tcat Unitaire · Vignettes<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Calculer la taille en pixels pour la plus petite icone (16) a 1.25 et a un facteur
 * tres faible (0.1).<br/>
 * \tattendu Le resultat est toujours strictement positif.
 * }
 */
TEST(ThumbnailGeometryTest, FacteurNonEntierNeProduitJamaisZero) {
    EXPECT_GT(hmi::thumbnailPixelSize(16, 1.25), 0);
    EXPECT_GT(hmi::thumbnailPixelSize(1, 0.1), 0);
    EXPECT_GT(hmi::thumbnailPixelSize(0, 2.0), 0);
}

/**
 * @brief La fonction est pure : deux appels avec les mêmes entrées produisent la même sortie.
 * \castest{<b>Le dimensionnement est une fonction pure.</b><br/>
 * \tcat Unitaire · Vignettes<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Appeler deux fois avec les memes entrees.<br/>
 * \tattendu Les deux resultats sont identiques.
 * }
 */
TEST(ThumbnailGeometryTest, FonctionPure) {
    EXPECT_EQ(hmi::thumbnailPixelSize(32, 1.5), hmi::thumbnailPixelSize(32, 1.5));
}
