// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Core/Levels/LevelOutcome.h"

#include <algorithm>  // std::clamp
#include <cmath>      // std::floor

#include "Core/Levels/GridPosition.h"
#include "Core/Levels/Level.h"
#include "Core/Levels/TileMap.h"
#include "Core/Levels/TileType.h"
#include "Core/Physics/Aabb.h"

namespace core {
namespace {

// La boîte recouvre-t-elle la case (column, row) ? Chevauchement d'aire strictement positive :
// se contenter d'effleurer un bord ne compte pas (évite les faux positifs aux frontières).
bool overlapsCell(const Aabb& box, int column, int row) {
    const auto left = static_cast<float>(column);
    const auto top = static_cast<float>(row);
    return box.min.x < left + 1.0F && box.max.x > left && box.min.y < top + 1.0F && box.max.y > top;
}

// La boîte recouvre-t-elle @p other ? Généralisation de overlapsCell à un rectangle arbitraire
// (boîte fournie par extraDangerBoxes).
bool overlapsBox(const Aabb& box, const Aabb& other) {
    return box.min.x < other.max.x && box.max.x > other.min.x && box.min.y < other.max.y &&
           box.max.y > other.min.y;
}

}  // namespace

LevelOutcome evaluateOutcome(const Aabb& playerBox, const Level& level,
                             const std::vector<Aabb>& extraDangerBoxes) {
    const TileMap& map = level.tileMap();

    // Échec prioritaire n°1 : le personnage est passé sous la limite basse du niveau (chute).
    if (playerBox.min.y >= static_cast<float>(map.height())) {
        return LevelOutcome::Lost;
    }

    // Échec prioritaire n°2 : contact avec un danger statique. On ne teste que les cases
    // recouvertes : une tuile `Danger` occupe la case pleine.
    const int columnBegin =
        std::clamp(static_cast<int>(std::floor(playerBox.min.x)), 0, map.width() - 1);
    const int columnEnd =
        std::clamp(static_cast<int>(std::floor(playerBox.max.x)), 0, map.width() - 1);
    const int rowBegin =
        std::clamp(static_cast<int>(std::floor(playerBox.min.y)), 0, map.height() - 1);
    const int rowEnd =
        std::clamp(static_cast<int>(std::floor(playerBox.max.y)), 0, map.height() - 1);
    for (int row = rowBegin; row <= rowEnd; ++row) {
        for (int column = columnBegin; column <= columnEnd; ++column) {
            const TileType type = map.tile(column, row);
            if (type == TileType::Danger && overlapsCell(playerBox, column, row)) {
                return LevelOutcome::Lost;
            }
        }
    }

    // Échec prioritaire n°3 : contact avec un danger à état, dont la boîte est
    // assemblée par l appelant.
    for (const Aabb& dangerBox : extraDangerBoxes) {
        if (overlapsBox(playerBox, dangerBox)) {
            return LevelOutcome::Lost;
        }
    }

    // Succès : la boîte recouvre la case de sortie.
    const GridPosition exit = level.exit();
    if (overlapsCell(playerBox, exit.column, exit.row)) {
        return LevelOutcome::Won;
    }

    return LevelOutcome::Playing;
}

}  // namespace core
