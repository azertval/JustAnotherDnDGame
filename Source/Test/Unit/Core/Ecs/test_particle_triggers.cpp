// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

/**
 * @file test_particle_triggers.cpp
 * @brief Tests unitaires des déclencheurs de particules du personnage (`LOT-53` TACHE-02,
 *        `EX-REN-008`).
 */

#include <gtest/gtest.h>

#include "Core/Ecs/Components/Particle.h"
#include "Core/Ecs/Systems/ParticleSystem.h"
#include "Core/Ecs/World.h"
#include "Core/Math/Vector2.h"

/**
 * @brief En-dessous du seuil `LANDING_MIN_IMPACT_SPEED`, un atterrissage ne produit **aucune**
 * particule -- sinon chaque petit pas émettrait de la poussière.
 * \castest{<b>Atterrissage sous le seuil : aucun effet.</b><br/>
 * \tcat Unitaire · Déclencheurs de particules<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Appeler `emitLanding` avec une vitesse d'impact strictement inférieure au
 * seuil.<br/>
 * \tattendu `aliveCount() == 0`.
 * }
 */
TEST(ParticleTriggersTest, AtterrissageSousLeSeuilAucunEffet) {
    core::World world;
    core::ParticleSystem system;

    system.emitLanding(world, core::Vector2{0.0f, 0.0f}, core::LANDING_MIN_IMPACT_SPEED - 0.5f);

    EXPECT_EQ(system.aliveCount(), 0);
}

/**
 * @brief Au seuil minimal, l'atterrissage produit l'intensité minimale ; au-delà du plafond,
 * l'intensité maximale (jamais davantage) -- l'intensité croît strictement entre les deux.
 * \castest{<b>Intensité d'atterrissage proportionnelle à la vitesse d'impact.</b><br/>
 * \tcat Unitaire · Déclencheurs de particules<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Émettre au seuil minimal.<br/>2. Émettre à mi-intervalle (mondes séparés).<br/>
 * 3. Émettre au-delà du plafond.<br/>
 * \tattendu Nombre de particules croissant strictement (min < mi-intervalle < max) ; jamais plus
 * que `LANDING_DUST_MAX_COUNT` même bien au-delà du plafond.
 * }
 */
TEST(ParticleTriggersTest, IntensiteAtterrissageProportionnelleALaVitesseDImpact) {
    core::World worldMin;
    core::ParticleSystem systemMin;
    systemMin.emitLanding(worldMin, core::Vector2{}, core::LANDING_MIN_IMPACT_SPEED);
    EXPECT_EQ(systemMin.aliveCount(), core::LANDING_DUST_MIN_COUNT);

    core::World worldMid;
    core::ParticleSystem systemMid;
    const float midSpeed = (core::LANDING_MIN_IMPACT_SPEED + core::LANDING_MAX_IMPACT_SPEED) * 0.5f;
    systemMid.emitLanding(worldMid, core::Vector2{}, midSpeed);

    core::World worldMax;
    core::ParticleSystem systemMax;
    systemMax.emitLanding(worldMax, core::Vector2{}, core::LANDING_MAX_IMPACT_SPEED);
    EXPECT_EQ(systemMax.aliveCount(), core::LANDING_DUST_MAX_COUNT);

    EXPECT_GT(systemMid.aliveCount(), systemMin.aliveCount());
    EXPECT_LT(systemMid.aliveCount(), systemMax.aliveCount());

    // Bien au-dela du plafond : jamais davantage que l'intensite maximale.
    core::World worldExtreme;
    core::ParticleSystem systemExtreme;
    systemExtreme.emitLanding(worldExtreme, core::Vector2{},
                              core::LANDING_MAX_IMPACT_SPEED * 10.0f);
    EXPECT_EQ(systemExtreme.aliveCount(), core::LANDING_DUST_MAX_COUNT);
}

/**
 * @brief `emitDashTrail` est une émission **continue** : appelée à chaque pas simulé pendant le
 * dash, elle ajoute des particules à chaque appel plutôt que de n'émettre qu'une fois.
 * \castest{<b>Traînée de dash : émission continue, un appel par pas.</b><br/>
 * \tcat Unitaire · Déclencheurs de particules<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Appeler `emitDashTrail` puis `update` quatre fois de suite (simulant quatre pas de
 * dash).<br/>
 * \tattendu Le nombre de particules vivantes croît à chaque appel (émission répétée), jamais figé
 * après le premier appel.
 * }
 */
TEST(ParticleTriggersTest, TraineeDeDashEmissionContinueUnAppelParPas) {
    core::World world;
    core::ParticleSystem system;
    constexpr float FIXED_DELTA = 1.0f / 60.0f;

    int previousCount = 0;
    for (int step = 0; step < 4; ++step) {
        system.emitDashTrail(world, core::Vector2{static_cast<float>(step), 0.0f}, 1.0f);
        EXPECT_GT(system.aliveCount(), previousCount)
            << "le pas " << step << " doit ajouter des particules";
        previousCount = system.aliveCount();
        system.update(world, FIXED_DELTA);
    }
}

/**
 * @brief `emitDeath` produit une rafale ponctuelle de particules.
 * \castest{<b>Mort : rafale ponctuelle de particules.</b><br/>
 * \tcat Unitaire · Déclencheurs de particules<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Appeler `emitDeath` une fois.<br/>
 * \tattendu Plusieurs particules vivantes, toutes de type `ParticleKind::Death`.
 * }
 */
TEST(ParticleTriggersTest, MortProduitUneRafaleDeParticulesDeType) {
    core::World world;
    core::ParticleSystem system;

    system.emitDeath(world, core::Vector2{1.0f, 2.0f});

    EXPECT_GT(system.aliveCount(), 1);
    world.view<core::Particle>().each([](core::Entity, const core::Particle& particle) {
        EXPECT_EQ(particle.kind, core::ParticleKind::Death);
    });
}

/**
 * @brief Le rechargement de niveau vide les particules résiduelles : après `clear()`, un
 * `emitDeath` antérieur n'a plus aucune particule vivante.
 * \castest{<b>Rechargement : aucune particule résiduelle.</b><br/>
 * \tcat Unitaire · Déclencheurs de particules<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Émettre un éclatement de mort.<br/>2. Appeler `clear` (rechargement).<br/>
 * \tattendu Plus aucune particule vivante.
 * }
 */
TEST(ParticleTriggersTest, RechargementAucuneParticuleResiduelle) {
    core::World world;
    core::ParticleSystem system;
    system.emitDeath(world, core::Vector2{0.0f, 0.0f});
    ASSERT_GT(system.aliveCount(), 0);

    system.clear(world);

    EXPECT_EQ(system.aliveCount(), 0);
}
