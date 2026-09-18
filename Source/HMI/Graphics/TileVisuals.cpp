// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "HMI/Graphics/TileVisuals.h"

#include "HMI/Graphics/TextureAtlas.h"

namespace hmi {

// Region d'atlas (couleur) associee a chaque type de tuile, pour un rendu distinct.
core::AtlasRegion regionForTile(core::TileType type) {
    switch (type) {
        case core::TileType::Solid:
            return TextureAtlas::tile(0, 2);  // gris
        case core::TileType::Entry:
            return TextureAtlas::tile(1, 0);  // vert
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

}  // namespace hmi
