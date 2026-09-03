// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

/**
 * @file Core/Physics/TopDownConfig.h
 * @brief Réglages du déplacement en vue de dessus (données pures).
 */

namespace core {

/**
 * @brief Paramètres réglables du déplacement top-down, regroupés pour faciliter le *tuning*.
 *
 * Donnée pure (`EX-ARCH-011`) : le système de déplacement la lit, ne la modifie pas. Unités monde :
 * une tuile = 1 unité, `y` vers le bas, 1 case = 1,5 m.
 *
 * Trois champs là où `core::PhysicsConfig` en comptait vingt-cinq : sans gravité, il n'y a ni
 * hauteur de saut, ni coyote time, ni vitesse terminale, ni wall jump à régler. Ce qui reste est
 * exactement ce qui fait le ressenti d'une marche — sa vitesse, le temps qu'elle met à s'établir,
 * le temps qu'elle met à s'éteindre.
 */
struct TopDownConfig {
    /// Vitesse de marche à pleine intention, en unités/seconde (`EX-EXP-001`). À 1,5 m par case,
    /// 4 unités/s ≈ 6 m/s : le pas soutenu d'un jeu d'action, pas la vitesse réelle d'un marcheur.
    /// ⚠️ à affiner au ressenti.
    float moveSpeed = 4.0F;

    /// Accélération vers la vitesse d'intention, en unités/seconde². Avec 40, la pleine vitesse
    /// est atteinte en 0,1 s : assez court pour que le personnage réponde, assez long pour que le
    /// départ ne soit pas un saut d'image. ⚠️ à affiner.
    float acceleration = 40.0F;

    /// Décélération à intention nulle, en unités/seconde². Plus vive que l'accélération : un
    /// personnage qu'on relâche doit s'arrêter **net**, sous peine de glisser au-delà de la case
    /// visée — insupportable dans un jeu où l'on se place à la case près (`LOT-19`). ⚠️ à affiner.
    float friction = 60.0F;
};

}  // namespace core
