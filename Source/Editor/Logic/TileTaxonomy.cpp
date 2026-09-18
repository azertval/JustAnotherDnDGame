// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "Editor/Logic/TileTaxonomy.h"

namespace hmi {

std::vector<TileCategory> tileTaxonomy() {
    using core::TileType;
    return {
        {.label = "Tile",
         .tiles = {{.type = TileType::Empty, .label = "Empty (eraser)"},
                   {.type = TileType::Solid, .label = "Solid"}},
         .subgroups = {}},
        {.label = "Marker",
         .tiles = {{.type = TileType::Entry, .label = "Entry"}},
         .subgroups = {}},
        // Terrain du RPG (LOT-08). Trois categories plutot qu'une : ce qui se marche, ce qui
        // arrete, ce qui fait franchir -- c'est la question que se pose le level designer devant
        // sa palette, bien avant celle de la matiere representee.
        {.label = "Ground",
         .tiles = {{.type = TileType::Grass, .label = "Grass"},
                   {.type = TileType::Dirt, .label = "Dirt"},
                   {.type = TileType::Sand, .label = "Sand"},
                   {.type = TileType::Water, .label = "Water"},
                   {.type = TileType::DeepWater, .label = "Deep water"}},
         .subgroups = {}},
        {.label = "Obstacle",
         .tiles = {{.type = TileType::Wall, .label = "Wall"},
                   {.type = TileType::Cliff, .label = "Cliff"}},
         .subgroups = {}},
        {.label = "Crossing",
         .tiles = {{.type = TileType::Bridge, .label = "Bridge"},
                   {.type = TileType::Stairs, .label = "Stairs"}},
         .subgroups = {}},
    };
}

}  // namespace hmi
