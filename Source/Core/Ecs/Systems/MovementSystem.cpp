// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "Core/Ecs/Systems/MovementSystem.h"

#include "Core/Ecs/Components/Transform.h"
#include "Core/Ecs/Components/Velocity.h"
#include "Core/Ecs/World.h"

namespace core {

// Intègre la position des entités mobiles pour un pas de simulation.
void MovementSystem::update(World& world, float fixedDelta) {
    // Intégration explicite (Euler) au pas fixe : la vue ne visite que les
    // entités possédant Transform ET Velocity, les autres restent immobiles.
    world.view<Transform, Velocity>().each(
        [fixedDelta](Entity, Transform& transform, Velocity& velocity) {
            transform.position += velocity.value * fixedDelta;
        });
}

}  // namespace core
