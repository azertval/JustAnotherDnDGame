// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Core/Ecs/Systems/TopDownMovementSystem.h"

#include "Core/Ecs/Components/Actor.h"
#include "Core/Ecs/Components/Collider.h"
#include "Core/Ecs/Components/Transform.h"
#include "Core/Ecs/Components/Velocity.h"
#include "Core/Ecs/World.h"
#include "Core/Math/Vector2.h"
#include "Core/Physics/Aabb.h"
#include "Core/Physics/SweptCollision.h"

namespace core {

namespace {

// En deca de cette vitesse (unites/s), un acteur est considere a l'arret : la friction l'y pose
// net plutot que de le laisser deriver indefiniment vers zero. Sous le dixieme de pixel par
// seconde (16 px/unite), aucun mouvement n'est observable.
constexpr float REST_SPEED = 1e-3F;

// Intention de deplacement, NORMALISEE. C'est ici, et nulle part ailleurs, que se joue
// l'isotropie : sans cette normalisation, (1, 1) donnerait une norme de racine de 2, et la
// diagonale serait 41 % plus rapide que la marche cardinale (EX-EXP-001).
//
// Les composantes arrivent deja bornees a [-1, 1] par hmi::PlayerInputMapper ; une intention plus
// courte que 1 (manette poussee a mi-course) est conservee telle quelle -- seule une intention
// PLUS LONGUE que 1, ce que seule la diagonale produit, est ramenee sur le cercle unite.
[[nodiscard]] Vector2 desiredDirection(const PlayerInput& input) {
    const Vector2 raw{input.moveX, input.moveY};
    const float length = raw.length();
    if (length <= 1.0F) {
        return raw;
    }
    return raw * (1.0F / length);
}

// Rapproche @p velocity de @p target d'au plus @p maxDelta : acceleration quand une intention est
// donnee, friction quand elle est nulle. Le rapprochement est fait sur le VECTEUR, jamais axe par
// axe -- composante par composante, un changement de direction en diagonale accelererait un axe
// pendant que l'autre freine, et la vitesse resultante ne suivrait plus le cercle.
[[nodiscard]] Vector2 approach(const Vector2& velocity, const Vector2& target, float maxDelta) {
    const Vector2 difference = target - velocity;
    const float distance = difference.length();
    if (distance <= maxDelta || distance <= 0.0F) {
        return target;
    }
    return velocity + difference * (maxDelta / distance);
}

}  // namespace

void updateTopDownMovement(World& world, Entity actor, const PlayerInput& input,
                           const TileMap& collision, const TopDownConfig& config,
                           float fixedDelta) {
    if (!world.hasComponent<Transform>(actor) || !world.hasComponent<Velocity>(actor) ||
        !world.hasComponent<Collider>(actor) || !world.hasComponent<Actor>(actor)) {
        return;  // entite incomplete : rien a deplacer, jamais un plantage (EX-NFR-040)
    }

    Transform& transform = world.getComponent<Transform>(actor);
    Velocity& velocity = world.getComponent<Velocity>(actor);
    const Collider& collider = world.getComponent<Collider>(actor);
    Actor& actorState = world.getComponent<Actor>(actor);

    // 1. Intention -> vitesse visee. A intention nulle, la cible est l'arret, et c'est la friction
    // -- plus vive que l'acceleration -- qui gouverne le rapprochement.
    const Vector2 direction = desiredDirection(input);
    const bool moving = direction.lengthSquared() > 0.0F;
    const Vector2 targetVelocity = direction * config.moveSpeed;
    const float rate = moving ? config.acceleration : config.friction;
    velocity.value = approach(velocity.value, targetVelocity, rate * fixedDelta);
    if (!moving && velocity.value.length() < REST_SPEED) {
        velocity.value = Vector2{};  // pose l'arret net, sans derive residuelle
    }

    // 2. Orientation : la derniere direction de marche, CONSERVEE a l'arret (EX-EXP-004). Prise de
    // l'intention et non de la vitesse : un personnage qui pousse contre un mur regarde le mur,
    // meme si sa vitesse y est nulle.
    if (moving) {
        actorState.facing = direction.normalized();
    }

    // 3. Deplacement continu contre la grille de collision. Le balayage traite x et y de la meme
    // facon, sans axe privilegie : c'est ce qui rend un contrôleur de plateforme inutile ici, et
    // ce qui produit le glissement le long d'un mur pris en biais (EX-EXP-002, EX-EXP-003).
    const Aabb box = Aabb::fromTopLeftSize(transform.position, collider.size);
    const SweepResult swept = sweepAabb(box, velocity.value * fixedDelta, collision);
    transform.position = swept.position;

    // 4. Un axe bloque perd sa vitesse : la conserver ferait "pousser" contre le mur et relancerait
    // le personnage des que le mur cesse, comme s'il avait accumule de l'elan contre lui.
    if (swept.normal.x != 0.0F) {
        velocity.value.x = 0.0F;
    }
    if (swept.normal.y != 0.0F) {
        velocity.value.y = 0.0F;
    }
}

}  // namespace core
