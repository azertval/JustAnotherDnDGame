// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "Editor/Logic/TileTaxonomy.h"

namespace hmi {

std::vector<TileCategory> tileTaxonomy() {
    using core::TileType;
    return {
        {.label = "Tuile",
         .tiles = {{.type = TileType::Empty, .label = "Vide (gomme)"},
                   {.type = TileType::Solid, .label = "Plein"}},
         .subgroups = {}},
        {.label = "Jalon",
         .tiles = {{.type = TileType::Entry, .label = "Entrée"}},
         .subgroups = {}},
        // Terrain du RPG (LOT-08). Trois categories plutot qu'une : ce qui se marche, ce qui
        // arrete, ce qui fait franchir -- c'est la question que se pose le level designer devant
        // sa palette, bien avant celle de la matiere representee.
        {.label = "Sol",
         .tiles = {{.type = TileType::Grass, .label = "Herbe"},
                   {.type = TileType::Dirt, .label = "Terre"},
                   {.type = TileType::Sand, .label = "Sable"},
                   {.type = TileType::Water, .label = "Eau"},
                   {.type = TileType::DeepWater, .label = "Eau profonde"}},
         .subgroups = {}},
        {.label = "Obstacle",
         .tiles = {{.type = TileType::Wall, .label = "Mur"},
                   {.type = TileType::Cliff, .label = "Falaise"}},
         .subgroups = {}},
        {.label = "Passage",
         .tiles = {{.type = TileType::Bridge, .label = "Pont"},
                   {.type = TileType::Stairs, .label = "Escalier"}},
         .subgroups = {}},
    };
}

}  // namespace hmi
