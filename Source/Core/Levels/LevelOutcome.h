// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <vector>

#include "Core/Physics/Aabb.h"

/**
 * @file Core/Levels/LevelOutcome.h
 * @brief Issue d'un niveau (en cours / gagné / perdu) évaluée depuis la position du personnage.
 */

namespace core {

class Level;

/// Issue courante d'un niveau, déduite de la position du personnage.
enum class LevelOutcome {
    Playing,  ///< Partie en cours.
    Won,      ///< Le personnage a atteint la sortie (`EX-GP-030`).
    Lost      ///< Le personnage a touché un danger ou est sorti par le bas (`EX-GP-031`).
};

/**
 * @brief Détermine l'issue du niveau à partir de la **boîte** du personnage.
 *
 * Fonction **pure** d'observation (ne modifie rien) : elle classe l'état courant, sans déclencher
 * de transition (le retour au menu ou le redémarrage relèvent de l'intégration). Priorité
 * **déterministe** en cas de recouvrement simultané : l'**échec l'emporte sur le succès**.
 *
 * Les dangers **statiques** (`Danger`, `DangerUp`/`DangerDown`/`DangerLeft`/`DangerRight`,
 * `EX-GP-050`) sont résolus directement depuis la grille du niveau (via `core::dangerHitbox`) : ce
 * module (`Core/Levels`) n'a pas de dépendance vers `Core/Gameplay`, donc pas de connaissance des
 * contrôleurs qui font vivre les dangers à **état** (mobile, commuté, temporisé — `EX-GP-051`/
 * `052`/`053`). Leurs boîtes **actuellement mortelles**, calculées par l'appelant
 * (`core::DangerController`, `core::MechanismController`) qui possède déjà cet état, sont passées
 * via @p extraDangerBoxes plutôt que dupliquées ici.
 *
 * @param playerBox       Boîte englobante du personnage, en unités monde.
 * @param level           Niveau courant (sortie, tuiles de danger statique, limites de la grille).
 * @param extraDangerBoxes Boîtes mortelles supplémentaires, hors grille statique (dangers mobile/
 *                         commuté/temporisé actuellement actifs) ; vide par défaut (niveaux sans
 *                         danger à état, ou appelants qui ignorent ces variantes).
 * @return `Lost` si contact avec un danger (statique ou @p extraDangerBoxes) ou sortie par le bas ;
 *         sinon `Won` si la sortie est recouverte ; sinon `Playing`.
 */
[[nodiscard]] LevelOutcome evaluateOutcome(const Aabb& playerBox, const Level& level,
                                           const std::vector<Aabb>& extraDangerBoxes = {});

}  // namespace core
