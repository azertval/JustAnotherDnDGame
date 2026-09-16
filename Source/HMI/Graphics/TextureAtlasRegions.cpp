// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "HMI/Graphics/ProceduralAtlas.h"
#include "HMI/Graphics/TextureAtlas.h"

/**
 * @file HMI/Graphics/TextureAtlasRegions.cpp
 * @brief Mapping (colonne, ligne) -> région d'atlas de `hmi::TextureAtlas` (`tile`,
 *        `playerFrameRegion`).
 *
 * Séparé de `TextureAtlas.cpp` (chargement fichier/procédural, dépendant de Qt/Direct3D 11) :
 * cette pure arithmétique de grille ne dépend que des constantes de la classe, ce qui permet de
 * la compiler et de la tester **sans GPU ni Qt** (`EX-NFR-010`), comme le reste de la logique pure
 * du projet (`Source/Test/CMakeLists.txt`).
 */

namespace hmi {

// Région (en pixels) de la tuile à une position de la grille.
core::AtlasRegion TextureAtlas::tile(int column, int row) {
    return core::AtlasRegion{
        .x = column * TILE_SIZE, .y = row * TILE_SIZE, .width = TILE_SIZE, .height = TILE_SIZE};
}

// Région (en pixels) d'une image d'animation du personnage, sous la grille de tuiles.
core::AtlasRegion TextureAtlas::playerFrameRegion(PlayerClipKind clip, int frameIndex) {
    const int flatIndex = flatPlayerFrameIndex(clip, frameIndex);
    const int column = flatIndex % PLAYER_FRAME_COLUMNS;
    const int row = flatIndex / PLAYER_FRAME_COLUMNS;
    return core::AtlasRegion{.x = column * PLAYER_FRAME_SIZE,
                             .y = (TILE_SIZE * TILES_PER_SIDE) + (row * PLAYER_FRAME_SIZE),
                             .width = PLAYER_FRAME_SIZE,
                             .height = PLAYER_FRAME_SIZE};
}

}  // namespace hmi
