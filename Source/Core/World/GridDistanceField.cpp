// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Core/World/GridDistanceField.h"

#include <array>
#include <queue>

namespace core {

namespace {
constexpr int UNREACHABLE = -1;
}  // namespace

GridDistanceField::GridDistanceField(const core::TileMap& tileMap, const core::GridPosition& target)
    : GridDistanceField(tileMap, std::vector<core::GridPosition>{target}) {}

GridDistanceField::GridDistanceField(const core::TileMap& tileMap,
                                     const std::vector<core::GridPosition>& targets)
    : _width(tileMap.width()),
      _height(tileMap.height()),
      _distances(static_cast<std::size_t>(_width) * static_cast<std::size_t>(_height),
                 UNREACHABLE) {
    const auto index = [this](int column, int row) {
        return static_cast<std::size_t>(row) * static_cast<std::size_t>(_width) +
               static_cast<std::size_t>(column);
    };

    std::queue<core::GridPosition> frontier;
    for (const core::GridPosition& target : targets) {
        if (!tileMap.inBounds(target.column, target.row) ||
            tileMap.isSolid(target.column, target.row)) {
            // Cible hors-grille ou solide (porte verrouillée pas encore ouverte, par exemple) :
            // ignorée plutôt que de faire échouer les autres cibles -- garde défensive.
            continue;
        }
        const std::size_t targetIndex = index(target.column, target.row);
        if (_distances[targetIndex] != UNREACHABLE) {
            continue;  // Cible dupliquée (plusieurs mécanismes partageant la même position).
        }
        _distances[targetIndex] = 0;
        frontier.push(target);
    }

    static constexpr std::array<int, 4> DELTA_COLUMN{0, 0, -1, 1};
    static constexpr std::array<int, 4> DELTA_ROW{-1, 1, 0, 0};

    while (!frontier.empty()) {
        const core::GridPosition current = frontier.front();
        frontier.pop();
        const int currentDistance = _distances[index(current.column, current.row)];

        for (std::size_t direction = 0; direction < DELTA_COLUMN.size(); ++direction) {
            const int neighborColumn = current.column + DELTA_COLUMN[direction];
            const int neighborRow = current.row + DELTA_ROW[direction];
            if (!tileMap.inBounds(neighborColumn, neighborRow) ||
                tileMap.isSolid(neighborColumn, neighborRow)) {
                continue;
            }
            const std::size_t neighborIndex = index(neighborColumn, neighborRow);
            if (_distances[neighborIndex] != UNREACHABLE) {
                continue;
            }
            _distances[neighborIndex] = currentDistance + 1;
            frontier.push(core::GridPosition{neighborColumn, neighborRow});
        }
    }
}

int GridDistanceField::distance(const core::GridPosition& position) const noexcept {
    const int sentinel = unreachableDistance();
    if (position.column < 0 || position.row < 0 || position.column >= _width ||
        position.row >= _height) {
        return sentinel;
    }
    const std::size_t linearIndex =
        static_cast<std::size_t>(position.row) * static_cast<std::size_t>(_width) +
        static_cast<std::size_t>(position.column);
    const int value = _distances[linearIndex];
    return value == UNREACHABLE ? sentinel : value;
}

bool GridDistanceField::isReachable(const core::GridPosition& position) const noexcept {
    if (position.column < 0 || position.row < 0 || position.column >= _width ||
        position.row >= _height) {
        return false;
    }
    const std::size_t linearIndex =
        static_cast<std::size_t>(position.row) * static_cast<std::size_t>(_width) +
        static_cast<std::size_t>(position.column);
    return _distances[linearIndex] != UNREACHABLE;
}

}  // namespace core
