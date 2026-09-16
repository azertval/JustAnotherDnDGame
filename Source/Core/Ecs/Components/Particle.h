// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include "Core/Math/Vector2.h"

/**
 * @file Core/Ecs/Components/Particle.h
 * @brief Composant de particule visuelle, simulée au pas fixe (`LOT-53`, `EX-REN-008`).
 */

namespace core {

/**
 * @brief Effet visuel à l'origine d'une particule -- tag opaque pour `Core`, exploité par le
 *        rendu (`HMI`) pour choisir calque, teinte et texture (`LOT-53` TACHE-03), sans que
 *        `Core` n'ait à connaître ces notions de présentation (`EX-NFR-011`).
 */
enum class ParticleKind {
    /// Traînée continue émise pendant un dash (`LOT-53` TACHE-02).
    DashTrail,
    /// Bouffée de poussière à l'atterrissage.
    LandingDust,
    /// Éclatement à la mort du personnage.
    Death,
};

/**
 * @brief Particule visuelle pure : position, vitesse, durée de vie -- aucun effet de gameplay
 *        (`EX-ARCH-012`).
 *
 * Simulée au pas fixe par `core::ParticleSystem` : intégration simple (position += vitesse ×
 * pas), sans collision, jamais lue par un système de jeu. `life` décompte vers zéro ; la
 * particule disparaît (destruction de l'entité) au pas où elle l'atteint. `maxLife` est
 * conservé pour le fondu de rendu (`hmi::ParticleRenderer`, `LOT-53` TACHE-03) : le rapport
 * `life / maxLife` donne l'opacité restante, sans que le rendu n'ait à connaître la durée de vie
 * d'origine par un autre moyen.
 */
struct Particle {
    /// Position courante, en unités monde.
    Vector2 position;
    /// Vitesse courante, en unités monde par seconde.
    Vector2 velocity;
    /// Temps restant avant disparition, en secondes.
    float life = 0.0f;
    /// Durée de vie totale à l'émission, en secondes (> 0) ; référence du fondu de rendu.
    float maxLife = 0.0f;
    /// Effet à l'origine de cette particule (présentation, cf. ci-dessus).
    ParticleKind kind = ParticleKind::LandingDust;
};

}  // namespace core
