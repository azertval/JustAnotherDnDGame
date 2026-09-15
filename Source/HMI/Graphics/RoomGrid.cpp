// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "HMI/Graphics/RoomGrid.h"

#include <algorithm>

namespace hmi {

namespace {
// Division entiere arrondie vers le haut (ceil), pour width/height > 0 et roomSize > 0.
int ceilDiv(int value, int roomSize) {
    return (value + roomSize - 1) / roomSize;
}
}  // namespace

// Construit la partition pour un niveau de levelWidth x levelHeight cases, a la taille de salle
// donnee (LOT-64 : reglable par niveau, ROOM_WIDTH_TILES/ROOM_HEIGHT_TILES n'en sont plus que la
// valeur par defaut).
RoomGrid::RoomGrid(int levelWidth, int levelHeight, int roomWidthTiles, int roomHeightTiles)
    : _levelWidth(levelWidth),
      _levelHeight(levelHeight),
      _roomWidthTiles(roomWidthTiles),
      _roomHeightTiles(roomHeightTiles),
      _columns(ceilDiv(levelWidth, roomWidthTiles)),
      _rows(ceilDiv(levelHeight, roomHeightTiles)) {}

// Indice (colonne, ligne) de la salle contenant tile, bornee aux salles existantes.
core::GridPosition RoomGrid::roomIndexAt(core::GridPosition tile) const noexcept {
    // Borne d'abord la case aux limites du niveau (jamais de comportement indefini pour une
    // position hors bornes) : la division entiere qui suit ne porte alors que sur des valeurs
    // non negatives, retombant toujours dans une salle existante.
    const int column = std::clamp(tile.column, 0, _levelWidth - 1);
    const int row = std::clamp(tile.row, 0, _levelHeight - 1);
    return core::GridPosition{.column = column / _roomWidthTiles, .row = row / _roomHeightTiles};
}

// Rectangle (en cases) de la salle d'indice roomIndex, rogne aux bornes du niveau.
RoomBounds RoomGrid::roomBounds(core::GridPosition roomIndex) const noexcept {
    const int originColumn = roomIndex.column * _roomWidthTiles;
    const int originRow = roomIndex.row * _roomHeightTiles;
    return RoomBounds{.column = originColumn,
                      .row = originRow,
                      .width = (std::min)(_roomWidthTiles, _levelWidth - originColumn),
                      .height = (std::min)(_roomHeightTiles, _levelHeight - originRow)};
}

}  // namespace hmi
