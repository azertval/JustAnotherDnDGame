// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

/**
 * @file test_texture_atlas.cpp
 * @brief Tests unitaires du mapping de régions de `hmi::TextureAtlas` (non-régression).
 *
 * `tile` est de la pure arithmétique de grille (aucun état d'instance, `static`) : testable sans
 * GPU (`EX-NFR-010`).
 */

#include <gtest/gtest.h>

#include "HMI/Graphics/TextureAtlas.h"

/**
 * @brief `tile(colonne, ligne)` renvoie un rectangle de 16×16 pixels à l'origine attendue.
 * \castest{<b>tile(colonne, ligne) renvoie un rectangle de 16x16 pixels à l'origine
 * attendue.</b><br/>
 * \tcat Unitaire · Texture Atlas<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu tile(colonne, ligne) renvoie un rectangle de 16x16 pixels à l'origine attendue.
 * }
 */
TEST(TextureAtlasTest, TileRenvoieLeRectangleAttendu) {
    const core::AtlasRegion origin = hmi::TextureAtlas::tile(0, 0);
    EXPECT_EQ(origin.x, 0);
    EXPECT_EQ(origin.y, 0);
    EXPECT_EQ(origin.width, hmi::TextureAtlas::TILE_SIZE);
    EXPECT_EQ(origin.height, hmi::TextureAtlas::TILE_SIZE);

    const core::AtlasRegion secondRow = hmi::TextureAtlas::tile(2, 1);
    EXPECT_EQ(secondRow.x, 2 * hmi::TextureAtlas::TILE_SIZE);
    EXPECT_EQ(secondRow.y, 1 * hmi::TextureAtlas::TILE_SIZE);
    EXPECT_EQ(secondRow.width, hmi::TextureAtlas::TILE_SIZE);
    EXPECT_EQ(secondRow.height, hmi::TextureAtlas::TILE_SIZE);
}
