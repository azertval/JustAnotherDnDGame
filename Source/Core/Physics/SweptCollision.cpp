// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Core/Physics/SweptCollision.h"

#include <algorithm>  // std::clamp
#include <cmath>      // std::floor

#include "Core/Levels/TileMap.h"
#include "Core/Math/Vector2.h"
#include "Core/Physics/Aabb.h"
#include "Core/Physics/PhysicsConfig.h"

namespace core {
namespace {

// Fine « peau » : évite d'accrocher la tuile que le bord de la boîte ne fait qu'effleurer
// (bord exactement sur une frontière de cellule). Détermine la portée PERPENDICULAIRE au balayage.

// Choix technique (voir en-tête) : on résout AXE PAR AXE. Chaque axe est un balayage 1D continu.
// L'avantage décisif sur la méthode diagonale « Minkowski + slabs » est le CLAMP DIRECT : on cale
// la position sur la coordonnée entière du mur (ex. `column - size.x`), sans jamais faire
// `position += delta * t`. INVARIANT : une position résolue est une coordonnée de mur exacte,
// jamais une interpolation — donc aucune dérive flottante ne peut faire « coller » au mur.

// Balayage 1D sur l'axe X. Renvoie l'abscisse résolue du coin haut-gauche ; règle sign à
// -1 (mur à droite → normale vers la gauche), +1 (mur à gauche) ou 0 (aucun contact).
float sweepX(const TileMap& tiles, const Vector2& position, const Vector2& size, float dx,
             float& sign) {
    sign = 0.0F;
    const float newX = position.x + dx;
    const int width = tiles.width();
    const int height = tiles.height();

    // Lignes réellement occupées par la boîte (la peau évite de mordre la ligne juste effleurée).
    const int rowMin = std::clamp(static_cast<int>(std::floor(position.y)), 0, height - 1);
    const int rowMax = std::clamp(
        static_cast<int>(std::floor(position.y + size.y - COLLISION_SKIN)), 0, height - 1);

    if (dx > 0.0F) {
        // On balaie de la colonne du bord droit actuel jusqu'à celle du bord droit visé : le
        // premier solide rencontré est le plus proche (aucune traversée, même si dx ≫ 1 tuile).
        const int columnStart =
            std::clamp(static_cast<int>(std::floor(position.x + size.x)), 0, width - 1);
        const int columnEnd = std::clamp(static_cast<int>(std::floor(newX + size.x)), 0, width - 1);
        for (int column = columnStart; column <= columnEnd; ++column) {
            for (int row = rowMin; row <= rowMax; ++row) {
                if (tiles.isSolid(column, row)) {
                    sign = -1.0F;
                    return static_cast<float>(column) - size.x;  // bord droit collé au mur (column)
                }
            }
        }
    } else {
        const int columnStart = std::clamp(static_cast<int>(std::floor(position.x)), 0, width - 1);
        const int columnEnd = std::clamp(static_cast<int>(std::floor(newX)), 0, width - 1);
        for (int column = columnStart; column >= columnEnd; --column) {
            for (int row = rowMin; row <= rowMax; ++row) {
                if (tiles.isSolid(column, row)) {
                    sign = 1.0F;
                    return static_cast<float>(column + 1);  // bord gauche collé au mur (column + 1)
                }
            }
        }
    }
    return newX;  // rien sur le chemin : déplacement complet
}

// Symétrique sur l'axe Y (lignes ↔ colonnes). sign : -1 (sol dessous), +1 (plafond dessus), 0.
float sweepY(const TileMap& tiles, const Vector2& position, const Vector2& size, float dy,
             float& sign) {
    sign = 0.0F;
    const float newY = position.y + dy;
    const int width = tiles.width();
    const int height = tiles.height();

    const int columnMin = std::clamp(static_cast<int>(std::floor(position.x)), 0, width - 1);
    const int columnMax = std::clamp(
        static_cast<int>(std::floor(position.x + size.x - COLLISION_SKIN)), 0, width - 1);

    if (dy > 0.0F) {
        const int rowStart =
            std::clamp(static_cast<int>(std::floor(position.y + size.y)), 0, height - 1);
        const int rowEnd = std::clamp(static_cast<int>(std::floor(newY + size.y)), 0, height - 1);
        for (int row = rowStart; row <= rowEnd; ++row) {
            for (int column = columnMin; column <= columnMax; ++column) {
                if (tiles.isSolid(column, row)) {
                    sign = -1.0F;
                    return static_cast<float>(row) - size.y;  // bord bas posé sur le sol (row)
                }
            }
        }
    } else {
        const int rowStart = std::clamp(static_cast<int>(std::floor(position.y)), 0, height - 1);
        const int rowEnd = std::clamp(static_cast<int>(std::floor(newY)), 0, height - 1);
        for (int row = rowStart; row >= rowEnd; --row) {
            for (int column = columnMin; column <= columnMax; ++column) {
                if (tiles.isSolid(column, row)) {
                    sign = 1.0F;
                    return static_cast<float>(row + 1);  // bord haut sous le plafond (row + 1)
                }
            }
        }
    }
    return newY;
}

}  // namespace

// Déplace la boîte de `delta` en s'arrêtant aux tuiles solides, puis en glissant (voir en-tête).
SweepResult sweepAabb(const Aabb& box, const Vector2& delta, const TileMap& tiles) {
    const Vector2 size = box.max - box.min;  // dimensions pleines
    Vector2 position = box.min;              // coin haut-gauche
    Vector2 blocked{};

    // Résolution X puis Y. La passe Y part de `position.x` DÉJÀ résolu : c'est ce qui permet le
    // glissement (buter sur un mur en X, puis continuer à tomber en Y le long de ce mur).
    if (delta.x != 0.0F) {
        position.x = sweepX(tiles, position, size, delta.x, blocked.x);
    }
    if (delta.y != 0.0F) {
        position.y = sweepY(tiles, position, size, delta.y, blocked.y);
    }

    SweepResult result;
    result.position = position;
    result.normal = blocked;
    result.hit = (blocked.x != 0.0F) || (blocked.y != 0.0F);
    return result;
}

}  // namespace core
