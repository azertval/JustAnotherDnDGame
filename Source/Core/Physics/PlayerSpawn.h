// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include "Core/Math/Vector2.h"

/**
 * @file Core/Physics/PlayerSpawn.h
 * @brief Taille du personnage jouable (humanoïde) et placement centré au spawn.
 */

namespace core {

/// Largeur du personnage, en unités monde (une tuile = 1 unité).
inline constexpr float PLAYER_WIDTH = 0.4f;
/// Hauteur du personnage, en unités monde.
inline constexpr float PLAYER_HEIGHT = 0.8f;

/// @return Les dimensions (largeur, hauteur) du personnage, pour le Collider et le rendu.
[[nodiscard]] inline Vector2 playerSize() {
    return Vector2{PLAYER_WIDTH, PLAYER_HEIGHT};
}

/**
 * @brief Coin haut-gauche du personnage **centré** dans la tuile (@p column, @p row).
 *
 * Marges symétriques : la boîte 0,4 × 0,8 est centrée dans la tuile 1×1 (repère haut-gauche,
 * `y` vers le bas). Évite de faire apparaître le personnage collé à un bord de la tuile.
 */
[[nodiscard]] inline Vector2 playerSpawnPosition(int column, int row) {
    return Vector2{static_cast<float>(column) + (1.0f - PLAYER_WIDTH) * 0.5f,
                   static_cast<float>(row) + (1.0f - PLAYER_HEIGHT) * 0.5f};
}

}  // namespace core
