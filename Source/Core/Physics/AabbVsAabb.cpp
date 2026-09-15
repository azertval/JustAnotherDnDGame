// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "Core/Physics/AabbVsAabb.h"

#include "Core/Physics/PhysicsConfig.h"

namespace core {
namespace {

// Même tolérance que COLLISION_SKIN (SweptCollision.cpp) : évite qu'un simple effleurement de
// frontière ne compte comme un chevauchement (cohérence des réglages entre les deux routines de
// balayage).

// Chevauchement d'aire strictement positive sur un axe, en ignorant une fine peau à la frontière.
[[nodiscard]] bool overlaps(float aMin, float aMax, float bMin, float bMax) noexcept {
    return aMin < bMax - COLLISION_SKIN && aMax > bMin + COLLISION_SKIN;
}

// Résout le blocage sur UN axe (le second appelé voit déjà la position de l'axe précédent résolue
// dans @p position, cf. sweepAabbVsAabb) : chevauchement testé sur l'axe PERPENDICULAIRE (@p
// otherPos/
// @p otherSize contre @p obstacleOtherMin/Max), avance @p position de @p delta sauf si un contact
// avec
// @p obstacleMin/Max l'en empêche -- @p position est alors calé à la frontière. @return le normal
// signé (0 si non bloqué).
[[nodiscard]] float resolveAxisBlock(float& position, float size, float delta, float otherPos,
                                     float otherSize, float obstacleOtherMin,
                                     float obstacleOtherMax, float obstacleMin,
                                     float obstacleMax) noexcept {
    float blocked = 0.0F;
    if (overlaps(otherPos, otherPos + otherSize, obstacleOtherMin, obstacleOtherMax)) {
        if (delta > 0.0F && position + size <= obstacleMin + COLLISION_SKIN) {
            if (position + size + delta > obstacleMin) {
                position = obstacleMin - size;
                blocked = -1.0F;
            }
        } else if (delta < 0.0F && position >= obstacleMax - COLLISION_SKIN) {
            if (position + delta < obstacleMax) {
                position = obstacleMax;
                blocked = 1.0F;
            }
        }
    }
    if (blocked == 0.0F) {
        position += delta;
    }
    return blocked;
}

}  // namespace

SweepResult sweepAabbVsAabb(const Aabb& box, const Vector2& delta, const Aabb& obstacle) noexcept {
    Vector2 position = box.min;
    const Vector2 size = box.max - box.min;
    Vector2 blocked{};

    // Axe X : perpendiculaire testé avec la position Y COURANTE (avant résolution Y), comme
    // sweepX le fait avec la grille — permet le glissement le long de l'obstacle.
    if (delta.x != 0.0F) {
        blocked.x =
            resolveAxisBlock(position.x, size.x, delta.x, position.y, size.y, obstacle.min.y,
                             obstacle.max.y, obstacle.min.x, obstacle.max.x);
    }

    // Axe Y : perpendiculaire testé avec la position X DÉJÀ résolue ci-dessus (même ordre X→Y que
    // sweepAabb, EX-NFR-002 : déterministe).
    if (delta.y != 0.0F) {
        blocked.y =
            resolveAxisBlock(position.y, size.y, delta.y, position.x, size.x, obstacle.min.x,
                             obstacle.max.x, obstacle.min.y, obstacle.max.y);
    }

    SweepResult result;
    result.position = position;
    result.normal = blocked;
    result.hit = (blocked.x != 0.0F) || (blocked.y != 0.0F);
    return result;
}

}  // namespace core
