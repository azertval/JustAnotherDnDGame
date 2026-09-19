// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "Core/Levels/CollisionDerivation.h"

#include <cstddef>
#include <optional>

#include "Core/Levels/PieceFootprint.h"

namespace core {

namespace {

[[nodiscard]] PieceTactical strongest(PieceTactical left, PieceTactical right) noexcept {
    return static_cast<int>(left) >= static_cast<int>(right) ? left : right;
}

}  // namespace

TileType collisionTileOf(PieceTactical tactical) noexcept {
    switch (tactical) {
        case PieceTactical::Solid:
            return TileType::Wall;
        case PieceTactical::Obstacle:
            return TileType::Cliff;
        case PieceTactical::Open:
        case PieceTactical::Difficult:
        case PieceTactical::Cover:
            return TileType::Empty;
    }
    return TileType::Empty;
}

PieceTactical tacticalOfTileType(TileType type) noexcept {
    switch (type) {
        case TileType::Solid:
        case TileType::Wall:
            return PieceTactical::Solid;
        case TileType::DeepWater:
        case TileType::Cliff:
            return PieceTactical::Obstacle;
        case TileType::Empty:
        case TileType::Entry:
        case TileType::Grass:
        case TileType::Dirt:
        case TileType::Sand:
        case TileType::Water:
        case TileType::Bridge:
        case TileType::Stairs:
            return PieceTactical::Open;
    }
    return PieceTactical::Open;
}

CollisionDerivation deriveCollision(const std::vector<TileLayer>& layers, int width, int height,
                                    const ScenePieceManifest* manifest) {
    const auto cellCount = static_cast<std::size_t>(width) * static_cast<std::size_t>(height);
    // Contribution la plus forte par case ; std::nullopt tant que rien ne couvre la case (vide).
    std::vector<std::optional<PieceTactical>> contributions(cellCount);
    std::vector<bool> unknown(cellCount, false);
    TileMap bounds(width, height);
    const auto indexOf = [width](int column, int row) {
        return (static_cast<std::size_t>(row) * static_cast<std::size_t>(width)) +
               static_cast<std::size_t>(column);
    };
    const auto contribute = [&](int column, int row, PieceTactical tactical) {
        if (!bounds.inBounds(column, row)) {
            return;  // une emprise qui déborde de la carte n'y occupe que ce qui y tient
        }
        std::optional<PieceTactical>& cell = contributions[indexOf(column, row)];
        cell = cell ? strongest(*cell, tactical) : tactical;
    };

    for (const TileLayer& layer : layers) {
        if (!isVisualLayerKind(layer.kind) || layer.tiles.width() != width ||
            layer.tiles.height() != height) {
            continue;
        }
        for (int row = 0; row < height; ++row) {
            for (int column = 0; column < width; ++column) {
                const std::string_view name = layer.pieceAt(column, row);
                const ScenePiece* piece =
                    name.empty() || manifest == nullptr ? nullptr : manifest->find(name);
                if (piece != nullptr) {
                    for (const GridPosition cell : footprintCells(
                             GridPosition{.column = column, .row = row}, piece->footprint())) {
                        contribute(cell.column, cell.row, piece->tactical);
                    }
                    continue;
                }
                if (!name.empty()) {
                    unknown[indexOf(column, row)] = true;
                }
                const TileType type = layer.tiles.tile(column, row);
                if (type != TileType::Empty) {
                    contribute(column, row, tacticalOfTileType(type));
                } else if (!name.empty()) {
                    contribute(column, row, PieceTactical::Open);  // une pièce inconnue couvre
                }
            }
        }
    }

    CollisionDerivation derivation{.collision = TileMap(width, height)};
    for (int row = 0; row < height; ++row) {
        for (int column = 0; column < width; ++column) {
            const std::size_t index = indexOf(column, row);
            const PieceTactical tactical = contributions[index].value_or(PieceTactical::Solid);
            derivation.collision.setTile(column, row, collisionTileOf(tactical));
            const GridPosition cell{.column = column, .row = row};
            if (tactical == PieceTactical::Difficult || tactical == PieceTactical::Cover) {
                derivation.unplayed.push_back(cell);
            }
            if (unknown[index]) {
                derivation.unknownPieces.push_back(cell);
            }
        }
    }
    return derivation;
}

bool collisionAgrees(const TileMap& written, const TileMap& derived, int column, int row) {
    const TileType value = written.tile(column, row);
    return (value == TileType::Entry ? TileType::Empty : value) == derived.tile(column, row);
}

}  // namespace core
