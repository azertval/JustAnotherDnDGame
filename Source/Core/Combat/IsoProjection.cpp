// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Core/Combat/IsoProjection.h"

#include <algorithm>
#include <cmath>

namespace core {

IsoProjection::IsoProjection(int columns, int rows, float tileWidth, float diamondRatio,
                             float wallRise) noexcept
    : _columns(std::max(0, columns)),
      _rows(std::max(0, rows)),
      _tileWidth(tileWidth > 0.0f ? tileWidth : ARENA_TILE_WIDTH_UNITS),
      _diamondRatio(diamondRatio > 0.0f ? diamondRatio : ARENA_DIAMOND_RATIO),
      _wallRise(std::max(0.0f, wallRise)) {}

int IsoProjection::diagonals() const noexcept {
    return std::max(1, _columns + _rows);
}

Vector2 IsoProjection::sceneSize() const noexcept {
    const float demiDiagonales = static_cast<float>(diagonals()) / 2.0f;
    return {demiDiagonales * _tileWidth, demiDiagonales * tileHeight() + wallHeight()};
}

Vector2 IsoProjection::origin() const noexcept {
    // La case (0, rows - 1) touche le bord gauche : son losange commence en x = 0.
    return {static_cast<float>(_rows - 1) * _tileWidth / 2.0f, wallHeight()};
}

Vector2 IsoProjection::gridToWorld(const Vector2& gridPoint) const noexcept {
    const Vector2 depart = origin();
    return {depart.x + _tileWidth / 2.0f + (gridPoint.x - gridPoint.y) * _tileWidth / 2.0f,
            depart.y + (gridPoint.x + gridPoint.y) * tileHeight() / 2.0f};
}

Vector2 IsoProjection::worldToGrid(const Vector2& world) const noexcept {
    const Vector2 depart = origin();
    const float difference = (world.x - depart.x - _tileWidth / 2.0f) / (_tileWidth / 2.0f);
    const float somme = (world.y - depart.y) / (tileHeight() / 2.0f);
    return {(somme + difference) / 2.0f, (somme - difference) / 2.0f};
}

Vector2 IsoProjection::tileToWorld(GridPosition tile) const noexcept {
    return gridToWorld(
        {static_cast<float>(tile.column) + 0.5f, static_cast<float>(tile.row) + 0.5f});
}

Rect IsoProjection::tileBounds(GridPosition tile) const noexcept {
    const Vector2 depart = origin();
    const Vector2 coin{depart.x + static_cast<float>(tile.column - tile.row) * _tileWidth / 2.0f,
                       depart.y + static_cast<float>(tile.column + tile.row) * tileHeight() / 2.0f};
    return {coin, {_tileWidth, tileHeight()}};
}

std::optional<GridPosition> IsoProjection::worldToTile(const Vector2& world) const noexcept {
    const Vector2 grille = worldToGrid(world);
    const GridPosition tile{static_cast<int>(std::floor(grille.x)),
                            static_cast<int>(std::floor(grille.y))};
    if (!contains(tile)) {
        return std::nullopt;
    }
    return tile;
}

bool IsoProjection::contains(GridPosition tile) const noexcept {
    return tile.column >= 0 && tile.row >= 0 && tile.column < _columns && tile.row < _rows;
}

}  // namespace core
