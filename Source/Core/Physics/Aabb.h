// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include "Core/Math/Vector2.h"

/**
 * @file Core/Physics/Aabb.h
 * @brief Boîte englobante alignée aux axes (AABB) et fabrique associée.
 */

namespace core {

/**
 * @brief Rectangle **aligné aux axes**, décrit par ses coins `min` (haut-gauche) et `max`
 *        (bas-droite), en unités monde (origine haut-gauche, `y` vers le bas).
 *
 * Type géométrique pur, sans logique de gameplay : il sert de brique au balayage
 * (`sweepAabb`, `EX-GP-014`). La fabrique `fromTopLeftSize` évite les erreurs de conversion
 * depuis la convention « coin haut-gauche + taille » du ::core::Collider.
 */
struct Aabb {
    /// Coin haut-gauche (plus petites coordonnées).
    Vector2 min{};
    /// Coin bas-droite (plus grandes coordonnées).
    Vector2 max{};

    /// Construit une boîte à partir de son coin haut-gauche et de ses dimensions pleines.
    [[nodiscard]] static Aabb fromTopLeftSize(const Vector2& topLeft, const Vector2& size) {
        return Aabb{topLeft, topLeft + size};
    }
};

}  // namespace core
