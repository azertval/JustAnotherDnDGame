// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Editor/TileTaxonomy.h"

namespace hmi {

std::vector<TileCategory> tileTaxonomy() {
    using core::TileType;
    return {
        {.label = "Tuile",
         .tiles = {{.type = TileType::Empty, .label = "Vide (gomme)"},
                   {.type = TileType::Solid, .label = "Plein"},
                   {.type = TileType::Block, .label = "Bloc poussable"}},
         .subgroups = {}},
        {.label = "Interactif",
         .tiles = {{.type = TileType::Switch, .label = "Interrupteur"},
                   {.type = TileType::PressurePlate, .label = "Plaque de pression"},
                   {.type = TileType::Door, .label = "Porte"},
                   {.type = TileType::Key, .label = "Clé"},
                   {.type = TileType::LockedDoor, .label = "Porte verrouillée"}},
         .subgroups = {}},
        {.label = "Piège",
         .tiles = {{.type = TileType::Danger, .label = "Danger"}},
         .subgroups = {}},
        {.label = "Jalon",
         .tiles = {{.type = TileType::Entry, .label = "Entrée"},
                   {.type = TileType::Exit, .label = "Sortie"}},
         .subgroups = {}},
    };
}

}  // namespace hmi
