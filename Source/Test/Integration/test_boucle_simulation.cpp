// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

/**
 * @file test_boucle_simulation.cpp
 * @brief Test d'intégration inter-lots : le cadenceur à pas fixe (LOT-01) pilote
 *        la simulation ECS (LOT-03) — `FixedTimestep` → `World::update`.
 */

#include <memory>
#include <vector>

#include <gtest/gtest.h>

#include "Core/Ecs/Components/Transform.h"
#include "Core/Ecs/Components/Velocity.h"
#include "Core/Ecs/Systems/MovementSystem.h"
#include "Core/Ecs/World.h"
#include "Core/Math/Vector2.h"
#include "Core/Time/FixedTimestep.h"

namespace {
constexpr float STEP = 1.0f / 60.0f;
constexpr float TOLERANCE = 1e-4f;

/// Prépare un monde avec une entité mobile et le système de mouvement enregistré.
core::Entity buildMovingWorld(core::World& world, const core::Vector2& velocity) {
    const core::Entity mover = world.createEntity();
    world.addComponent(mover, core::Transform{});
    world.addComponent(mover, core::Velocity{velocity});
    world.addSystem(std::make_unique<core::MovementSystem>());
    return mover;
}

/// Rejoue une séquence de durées de frame et renvoie le nombre total de pas exécutés.
int runFrames(core::World& world, core::FixedTimestep& timestep,
              const std::vector<float>& frameDurations) {
    int totalSteps = 0;
    for (const float frame : frameDurations) {
        const int steps = timestep.advance(frame);
        for (int step = 0; step < steps; ++step) {
            world.update(STEP);
            ++totalSteps;
        }
    }
    return totalSteps;
}
}  // namespace

/**
 * @brief Le cadenceur découpe un temps réel variable en pas fixes, et la position finale correspond
 * exactement au nombre de pas réellement exécutés (aucune dérive).
 * \castest{<b>Le cadenceur découpe un temps réel variable en pas fixes, et la position finale
 * correspond exactement au nombre de pas réellement exécutés (aucune dérive).</b><br/>
 * \tcat Integration · Boucle Simulation<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Le cadenceur découpe un temps réel variable en pas fixes, et la position finale
 * correspond exactement au nombre de pas réellement exécutés (aucune dérive).
 * }
 */
TEST(BoucleSimulationIntegration, CadenceurPiloteLaSimulation) {
    core::World world;
    const core::Entity mover = buildMovingWorld(world, core::Vector2{6.0f, 0.0f});

    core::FixedTimestep timestep(STEP);
    // Trois frames d'une durée d'un pas et demi : 4,5 pas cumulés -> 4 pas exécutés,
    // le reste (0,5 pas) est conservé par le cadenceur.
    const int totalSteps = runFrames(world, timestep, {STEP * 1.5f, STEP * 1.5f, STEP * 1.5f});

    EXPECT_EQ(totalSteps, 4);
    const core::Vector2& position = world.getComponent<core::Transform>(mover).position;
    EXPECT_NEAR(position.x, 6.0f * totalSteps * STEP, TOLERANCE);
}

/**
 * @brief Déterminisme : même séquence de frames et même état initial -> même résultat.
 * \castest{<b>Déterminisme : même séquence de frames et même état initial -> même
 * résultat.</b><br/>
 * \tcat Integration · Boucle Simulation<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Déterminisme : même séquence de frames et même état initial -> même résultat.
 * }
 */
TEST(BoucleSimulationIntegration, MemeEntreeMemeResultat) {
    const std::vector<float> frames{STEP * 0.7f, STEP * 2.3f, STEP, STEP * 0.4f};
    const core::Vector2 velocity{0.0f, 5.0f};

    core::World first;
    const core::Entity firstMover = buildMovingWorld(first, velocity);
    core::FixedTimestep firstTimestep(STEP);
    runFrames(first, firstTimestep, frames);

    core::World second;
    const core::Entity secondMover = buildMovingWorld(second, velocity);
    core::FixedTimestep secondTimestep(STEP);
    runFrames(second, secondTimestep, frames);

    const core::Vector2& firstPosition = first.getComponent<core::Transform>(firstMover).position;
    const core::Vector2& secondPosition =
        second.getComponent<core::Transform>(secondMover).position;
    EXPECT_NEAR(firstPosition.x, secondPosition.x, TOLERANCE);
    EXPECT_NEAR(firstPosition.y, secondPosition.y, TOLERANCE);
}
