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
        // Terrain du RPG (LOT-08) : cases jusqu'ici libres de la grille, chacune peinte d'une
        // teinte distincte par l'atlas procedural -- le jeu reste lancable sans aucun fichier
        // d'image (EX-NFR-040).
        case core::TileType::Grass:
            return TextureAtlas::tile(4, 1);  // vert herbe
        case core::TileType::Dirt:
            return TextureAtlas::tile(4, 2);  // terre
        case core::TileType::Sand:
            return TextureAtlas::tile(2, 3);  // sable
        case core::TileType::Water:
            return TextureAtlas::tile(3, 3);  // eau claire
        case core::TileType::DeepWater:
            return TextureAtlas::tile(4, 3);  // eau profonde
        case core::TileType::Wall:
            return TextureAtlas::tile(0, 4);  // pierre batie, distincte du gris de Solid
        case core::TileType::Cliff:
            return TextureAtlas::tile(1, 4);  // roche sombre
        case core::TileType::Bridge:
            return TextureAtlas::tile(2, 4);  // bois
        case core::TileType::Stairs:
            return TextureAtlas::tile(4, 4);  // pierre claire
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
