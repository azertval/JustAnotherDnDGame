// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include "Core/Ecs/ISystem.h"

/**
 * @file Core/Ecs/Systems/MovementSystem.h
 * @brief Système intégrant la position des entités à partir de leur vitesse.
 */

namespace core {

/**
 * @brief Fait avancer chaque entité mobile selon sa vitesse, au pas de temps fixe.
 *
 * Pour chaque entité possédant à la fois un ::core::Transform et une
 * ::core::Velocity, applique `position += velocity * fixedDelta`. Sert de système
 * de démonstration de bout en bout de l'ECS et d'exemple de référence pour les
 * futurs systèmes de gameplay : toute la logique vit ici, les composants restent
 * des données pures (`EX-ARCH-011`).
 */
class MovementSystem : public ISystem {
public:
    /**
     * @brief Intègre la position des entités mobiles pour un pas de simulation.
     * @param world      Monde dont les entités mobiles sont mises à jour.
     * @param fixedDelta Durée du pas de simulation, en secondes.
     */
    void update(World& world, float fixedDelta) override;
};

}  // namespace core
