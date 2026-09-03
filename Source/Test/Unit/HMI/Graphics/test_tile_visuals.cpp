// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_tile_visuals.cpp
 * @brief Tests unitaires de la correspondance type de tuile -> région d'atlas procédurale
 *        (`hmi::regionForTile`), pour le repli sans asset (`EX-NFR-040`).
 */

#include <cstddef>
#include <vector>

#include <gtest/gtest.h>

#include "Core/Levels/TileType.h"
#include "HMI/Graphics/TileVisuals.h"

namespace {

// Deux regions sont "distinctes" si elles ne pointent pas exactement la meme case de la grille
// procedurale (memes coordonnees x/y) -- deux types partageant une case seraient indiscernables en
// repli procedural (LOT-63, TACHE-04).
bool sameRegion(const core::AtlasRegion& a, const core::AtlasRegion& b) {
    return a.x == b.x && a.y == b.y;
}

}  // namespace
