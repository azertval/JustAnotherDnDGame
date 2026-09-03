// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <vector>

#include "Core/Ecs/Components/Particle.h"
#include "Core/Ecs/Entity.h"
#include "Core/Math/Vector2.h"

/**
 * @file Core/Ecs/Systems/ParticleSystem.h
 * @brief Émetteur et simulation de particules, au pas fixe, déterministe, budget borné
 *        (`LOT-53` TACHE-01, `EX-REN-008`).
 */

namespace core {

class World;

/// Graine par défaut de `core::ParticleSystem` -- fixe et documentée, **jamais** l'horloge
/// système (`EX-NFR-002`) : deux sessions démarrées avec cette graine et soumises à la même
/// séquence d'entrées produisent exactement les mêmes particules.
inline constexpr std::uint64_t DEFAULT_PARTICLE_SEED = 0x504152544943454CULL;

/// Nombre maximal de particules simultanément vivantes (`EX-NFR-005`) : au-delà, `spawn()` recycle
/// les plus anciennes plutôt que d'allouer sans borne.
inline constexpr int MAX_PARTICLES = 256;

/// Vitesse d'impact (`LOT-53` TACHE-02) en-dessous de laquelle un atterrissage ne produit
/// **aucune** poussière -- sans ce seuil, chaque petit pas en émettrait.
inline constexpr float LANDING_MIN_IMPACT_SPEED = 4.0f;
/// Vitesse d'impact au-delà de laquelle l'intensité de la poussière plafonne (`LANDING_DUST_MAX_
/// COUNT`) : une chute extrême ne produit pas un nuage toujours plus dense.
inline constexpr float LANDING_MAX_IMPACT_SPEED = 14.0f;
/// Nombre de particules de poussière au seuil `LANDING_MIN_IMPACT_SPEED` (intensité minimale).
inline constexpr int LANDING_DUST_MIN_COUNT = 3;
/// Nombre de particules de poussière au plafond `LANDING_MAX_IMPACT_SPEED` (intensité maximale).
inline constexpr int LANDING_DUST_MAX_COUNT = 10;

/**
 * @brief Données décrivant un effet de particules : combien, à quelle vitesse, pendant combien de
 *        temps, avec quelle dispersion angulaire -- en constantes nommées côté appelant, jamais
 *        de nombres magiques dispersés dans le code (`LOT-53` TACHE-01).
 */
struct ParticleEffect {
    /// Nombre de particules émises par appel à `ParticleSystem::spawn`.
    int count = 0;
    /// Vitesse initiale minimale, en unités monde par seconde.
    float speedMin = 0.0f;
    /// Vitesse initiale maximale (>= `speedMin`).
    float speedMax = 0.0f;
    /// Durée de vie minimale, en secondes.
    float lifeMin = 0.0f;
    /// Durée de vie maximale (>= `lifeMin`).
    float lifeMax = 0.0f;
    /// Demi-angle de dispersion autour de la direction de base, en radians (`π` = cercle
    /// complet, la direction de base devient alors sans effet).
    float spreadRadians = 0.0f;
};

/**
 * @brief Émet et simule des particules visuelles au pas fixe, strictement déterministe.
 *
 * Les particules sont des entités du `core::World` portant un `core::Particle` (pas un tableau
 * interne) : le sparse set de l'ECS n'offre aucun ordre d'itération stable après un retrait
 * (swap-and-pop, `core::ComponentPool`), c'est pourquoi le système tient sa **propre** liste
 * d'ordre d'émission (`_order`), seule source de vérité pour l'intégration et le recyclage.
 *
 * **Aléa maîtrisé** (`EX-NFR-002`) : chaque particule tire ses valeurs d'un `core::
 * DeterministicRandom` reseedé pour elle seule, à partir de la graine de base, du numéro de pas
 * courant et de l'identifiant de son entité (`core::deriveSeed`) -- jamais l'horloge système,
 * jamais un générateur par défaut. Deux exécutions de la même séquence d'appels produisent donc
 * exactement les mêmes particules.
 *
 * **Budget borné** (`EX-NFR-005`) : au-delà de `MAX_PARTICLES` vivantes, `spawn()` recycle la plus
 * **ancienne** (front de `_order`) plutôt que d'allouer sans limite -- décision déterministe,
 * jamais dépendante d'un ordre d'itération instable.
 *
 * **Aucun effet de gameplay** (`EX-ARCH-012`) : les particules ne collisionnent pas, ne sont
 * jamais lues par un système de jeu, et n'affectent aucune entité `core::Actor`.
 */
class ParticleSystem {
public:
    /// @param seed Graine explicite (par défaut `DEFAULT_PARTICLE_SEED`), jamais l'horloge.
    explicit ParticleSystem(std::uint64_t seed = DEFAULT_PARTICLE_SEED) noexcept;

    /**
     * @brief Avance toutes les particules vivantes d'un pas fixe.
     *
     * Intègre la position (`position += vitesse × fixedDelta`), décompte `life`, et détruit
     * l'entité des particules dont la durée de vie atteint zéro **ce** pas -- ni avant, ni après.
     * @param world      Monde ECS portant les entités-particules.
     * @param fixedDelta Durée du pas, en secondes.
     */
    void update(World& world, float fixedDelta);

    /**
     * @brief Émet les particules d'un effet, à partir d'une origine et d'une direction de base.
     *
     * Chaque particule tire sa vitesse (norme dans `[speedMin, speedMax]`, angle dans
     * `[-spreadRadians, +spreadRadians]` autour de la direction de base) et sa durée de vie dans
     * `[lifeMin, lifeMax]`, d'un générateur reseedé pour elle (voir en-tête de la classe) : le
     * résultat ne dépend que de (graine de base, pas courant, identifiant d'entité), jamais de
     * l'ordre ou du nombre d'appels à `spawn` au sein du même pas.
     * @param world     Monde ECS recevant les entités-particules.
     * @param effect    Effet à émettre (constantes nommées côté appelant).
     * @param origin    Position d'émission, en unités monde.
     * @param direction Direction de base (n'a pas besoin d'être normalisée) ; sans effet si
     *                  `effect.spreadRadians >= π` (dispersion en cercle complet).
     * @param kind      Effet visuel porté par chaque particule émise (présentation, `HMI`).
     */
    void spawn(World& world, const ParticleEffect& effect, Vector2 origin, Vector2 direction,
               ParticleKind kind);

    /**
     * @brief Traînée de dash (émission **continue**, pas ponctuelle, `LOT-53` TACHE-02).
     *
     * **Sans appelant depuis le `LOT-06`** : le dash était une capacité du personnage de
     * plateforme, retirée avec lui. Conservée telle quelle plutôt que supprimée — une capacité de
     * ruée reviendra très probablement au RPG (`LOT-25`), et cet émetteur n'a rien de spécifique
     * au genre.
     * @param world    Monde ECS recevant les particules.
     * @param position Position du personnage, en unités monde.
     * @param facing   Sens horizontal du mouvement (-1/+1) : la traînée part de l'arrière du
     *                 mouvement (opposé à l'orientation).
     */
    void emitDashTrail(World& world, Vector2 position, float facing);

    /**
     * @brief Bouffée de poussière à l'atterrissage, d'intensité fonction de la vitesse d'impact.
     *
     * Sans effet en-dessous de `LANDING_MIN_IMPACT_SPEED` (un petit pas ne doit produire aucune
     * poussière) ; au-delà de `LANDING_MAX_IMPACT_SPEED`, l'effet plafonne à `LANDING_DUST_MAX_
     * COUNT` particules -- une chute longue produit ainsi plus de poussière qu'un petit saut,
     * sans devenir illisible pour une chute extrême.
     * @param world       Monde ECS recevant les particules.
     * @param position    Position du personnage à l'atterrissage, en unités monde.
     * @param impactSpeed Vitesse verticale (positive, vers le bas) au moment de l'impact.
     */
    void emitLanding(World& world, Vector2 position, float impactSpeed);

    /// Éclatement à la mort du personnage : rafale en cercle complet.
    /// @param world    Monde ECS recevant les particules.
    /// @param position Position du personnage au moment de l'échec, en unités monde.
    void emitDeath(World& world, Vector2 position);

    /// Détruit toutes les particules vivantes (rechargement de niveau, `LOT-53` TACHE-02).
    void clear(World& world);

    /// @return Le nombre de particules actuellement vivantes.
    [[nodiscard]] int aliveCount() const noexcept {
        return static_cast<int>(_order.size());
    }

private:
    /// Réserve l'emplacement d'une nouvelle particule : recycle la plus **ancienne** (front de
    /// `_order`) si le budget (`MAX_PARTICLES`) est atteint, sinon crée une entité neuve.
    [[nodiscard]] Entity reserveSlot(World& world);

    std::uint64_t _baseSeed;
    /// Numéro de pas courant, incrémenté par `update()` -- entre dans la graine de chaque
    /// particule émise (`core::deriveSeed`), jamais l'horloge (`EX-NFR-002`).
    std::uint64_t _stepIndex = 0;
    /// Entités-particules vivantes, dans leur ordre d'émission (la plus ancienne en tête) :
    /// source de vérité de l'intégration et du recyclage, indépendante du sparse set de l'ECS.
    std::vector<Entity> _order;
};

}  // namespace core
