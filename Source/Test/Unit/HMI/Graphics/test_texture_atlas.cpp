// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

/**
 * @file test_texture_atlas.cpp
 * @brief Tests unitaires du mapping de régions de `hmi::TextureAtlas` (LOT-39, non-régression).
 *
 * `tile`/`playerFrameRegion` sont de la pure arithmétique de grille (aucun état d'instance,
 * `static`) : testables sans device Direct3D, indépendamment de l'origine (fichier ou
 * procédurale) de l'atlas (`EX-NFR-010`).
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

/**
 * @brief `playerFrameRegion` place les images du personnage sous la grille de tuiles, dans
 *        l'ordre Idle, Run, Jump — non-régression du placement historique.
 * \castest{<b>playerFrameRegion place les images du personnage sous la grille de tuiles, dans
 * l'ordre Idle, Run, Jump.</b><br/>
 * \tcat Unitaire · Texture Atlas<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu playerFrameRegion place les images du personnage sous la grille de tuiles, dans
 * l'ordre Idle, Run, Jump.
 * }
 */
TEST(TextureAtlasTest, PlayerFrameRegionSousLaGrilleDeTuiles) {
    const int gridSide = hmi::TextureAtlas::TILE_SIZE * hmi::TextureAtlas::TILES_PER_SIDE;

    // Premiere image (Idle, index 0) : premiere case de la grille de personnage, juste sous la
    // grille de tuiles.
    const core::AtlasRegion idleFirst =
        hmi::TextureAtlas::playerFrameRegion(hmi::PlayerClipKind::Idle, 0);
    EXPECT_EQ(idleFirst.x, 0);
    EXPECT_EQ(idleFirst.y, gridSide);
    EXPECT_EQ(idleFirst.width, hmi::TextureAtlas::PLAYER_FRAME_SIZE);
    EXPECT_EQ(idleFirst.height, hmi::TextureAtlas::PLAYER_FRAME_SIZE);

    // Run (index 0) suit immediatement les PLAYER_IDLE_FRAME_COUNT images Idle (meme ordre a plat
    // que hmi::PlayerClipKind, hmi::flatPlayerFrameIndex).
    const core::AtlasRegion runFirst =
        hmi::TextureAtlas::playerFrameRegion(hmi::PlayerClipKind::Run, 0);
    const int runFlatIndex = hmi::PLAYER_IDLE_FRAME_COUNT;
    EXPECT_EQ(runFirst.x, (runFlatIndex % hmi::TextureAtlas::PLAYER_FRAME_COLUMNS) *
                              hmi::TextureAtlas::PLAYER_FRAME_SIZE);
    EXPECT_EQ(runFirst.y, gridSide + (runFlatIndex / hmi::TextureAtlas::PLAYER_FRAME_COLUMNS) *
                                         hmi::TextureAtlas::PLAYER_FRAME_SIZE);

    // Jump (pose unique) suit les images Idle et Run.
    const core::AtlasRegion jump =
        hmi::TextureAtlas::playerFrameRegion(hmi::PlayerClipKind::Jump, 0);
    const int jumpFlatIndex = hmi::PLAYER_IDLE_FRAME_COUNT + hmi::PLAYER_RUN_FRAME_COUNT;
    EXPECT_EQ(jump.x, (jumpFlatIndex % hmi::TextureAtlas::PLAYER_FRAME_COLUMNS) *
                          hmi::TextureAtlas::PLAYER_FRAME_SIZE);
    EXPECT_EQ(jump.y, gridSide + (jumpFlatIndex / hmi::TextureAtlas::PLAYER_FRAME_COLUMNS) *
                                     hmi::TextureAtlas::PLAYER_FRAME_SIZE);
}
