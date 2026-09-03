// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "Core/Ecs/Entity.h"
#include "Core/Physics/PlayerInput.h"
#include "Core/Physics/TopDownConfig.h"

/**
 * @file Core/Ecs/Systems/TopDownMovementSystem.h
 * @brief Déplacement libre en 8 directions sur une carte en vue de dessus (`LOT-06`).
 */

namespace core {

class World;
class TileMap;

/**
 * @brief Avance un acteur d'un pas fixe : intention → vitesse → balayage → position.
 *
 * Referme la parenthèse ouverte par le `LOT-01`, où la physique de plateforme a été retirée sans
 * remplaçant. **Sans gravité** : aucun axe n'est privilégié, `x` et `y` sont traités exactement de
 * la même façon — c'est toute la différence avec le contrôleur qu'il remplace.
 *
 * L'entité doit porter `Transform`, `Velocity`, `Collider` et `Actor` ; les autres sont ignorées.
 * Le déplacement passe par `core::sweepAabb`, balayage **continu** contre la grille : quelle que
 * soit la vitesse, la boîte ne traverse jamais une tuile solide, et le résidu d'un axe bloqué
 * **glisse** le long du mur (`EX-EXP-002`, `EX-EXP-003`).
 *
 * @note **La diagonale n'est pas plus rapide.** L'intention est normalisée avant d'être mise à
 *       l'échelle : sans cela, aller en diagonale donnerait `√2 ≈ 1,41` fois la vitesse cardinale
 *       — le défaut le plus courant du genre, et le plus visible en jeu (`EX-EXP-001`).
 *
 * Fonction **pure** au sens du déterminisme (`EX-NFR-002`) : mêmes entrées, mêmes positions, sans
 * aléatoire ni horloge. Testable **sans fenêtre ni GPU** (`EX-ARCH-001`).
 *
 * @param world      Monde ECS contenant l'acteur.
 * @param actor      Entité à déplacer.
 * @param input      Intention de ce pas (`moveX`/`moveY`, chacun dans [-1, 1]).
 * @param collision  Grille de collision de la carte (`Level::tileMap`, `EX-LVL-016`).
 * @param config     Réglages de vitesse, d'accélération et de friction.
 * @param fixedDelta Durée du pas fixe, en secondes.
 */
void updateTopDownMovement(World& world, Entity actor, const PlayerInput& input,
                           const TileMap& collision, const TopDownConfig& config, float fixedDelta);

}  // namespace core
