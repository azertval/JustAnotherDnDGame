// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include "Core/Math/Vector2.h"

/**
 * @file Core/Ecs/Components/Velocity.h
 * @brief Composant de vitesse linéaire d'une entité (données pures).
 */

namespace core {

/**
 * @brief Vitesse linéaire d'une entité, en unités monde par seconde.
 *
 * Donnée pure sans logique (`EX-ARCH-011`) : le ::core::MovementSystem la lit
 * pour faire évoluer la position du ::core::Transform associé.
 */
struct Velocity {
    /// Déplacement par seconde, en unités monde.
    Vector2 value{};
};

}  // namespace core
