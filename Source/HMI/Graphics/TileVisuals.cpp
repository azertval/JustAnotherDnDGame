// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Graphics/TileVisuals.h"

#include "HMI/Graphics/TextureAtlas.h"

namespace hmi {

// Region d'atlas (couleur) associee a chaque type de tuile, pour un rendu distinct.
core::AtlasRegion regionForTile(core::TileType type) {
    switch (type) {
        case core::TileType::Solid:
            return TextureAtlas::tile(0, 2);  // gris
        case core::TileType::Danger:
            return TextureAtlas::tile(0, 0);  // rouge
        case core::TileType::Entry:
            return TextureAtlas::tile(1, 0);  // vert
        case core::TileType::Exit:
            return TextureAtlas::tile(2, 0);  // bleu
        case core::TileType::Switch:
            return TextureAtlas::tile(3, 0);  // jaune
        case core::TileType::PressurePlate:
            return TextureAtlas::tile(
                1, 1);  // cyan (libere par LOT-17 : ancien placeholder du personnage)
        case core::TileType::Door:
            return TextureAtlas::tile(2, 1);  // orange
        case core::TileType::Key:
            return TextureAtlas::tile(4, 0);  // or (EX-GP-023, LOT-63)
        case core::TileType::LockedDoor:
            return TextureAtlas::tile(3, 4);  // brun fonce (EX-GP-023, LOT-63)
        case core::TileType::Block:
            return TextureAtlas::tile(3, 1);  // violet
        case core::TileType::Empty:
            break;
    }
    return TextureAtlas::tile(0, 0);
}

// Cherche la surcharge de texture assignee a une case precise (EX-EDIT-043, LOT-45).
std::optional<std::string> textureOverrideAt(
    const std::vector<core::TileTextureOverride>& overrides, core::GridPosition position) {
    for (const core::TileTextureOverride& override : overrides) {
        if (override.position == position) {
            return override.assetName;
        }
    }
    return std::nullopt;
}

}  // namespace hmi
