// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Core/Levels/TileTypeName.h"

#include <cstddef>
#include <functional>
#include <unordered_map>

namespace core {

namespace {

// Hacheur transparent : permet d'interroger une table a cles std::string avec un std::string_view
// sans construire de chaine temporaire a chaque appel (C++20, heterogeneous lookup).
struct TransparentStringHash {
    using is_transparent = void;

    [[nodiscard]] std::size_t operator()(std::string_view text) const noexcept {
        return std::hash<std::string_view>{}(text);
    }
};

}  // namespace

std::string tileTypeName(TileType type) {
    // switch exhaustif sans default (meme patron que keyName/screenName ailleurs dans le projet) :
    // un TileType ajoute sans nom casse la compilation au lieu de retomber sur un nom arbitraire.
    switch (type) {
        case TileType::Empty:
            return "empty";  // jamais emis dans un niveau : les cases vides sont omises de 'tiles'.
        case TileType::Solid:
            return "solid";
        case TileType::Danger:
            return "danger";
        case TileType::Entry:
            return "entry";
        case TileType::Exit:
            return "exit";
        case TileType::Switch:
            return "switch";
        case TileType::Door:
            return "door";
        case TileType::PressurePlate:
            return "pressurePlate";
        case TileType::Block:
            return "block";
        case TileType::Key:
            return "key";
        case TileType::LockedDoor:
            return "lockedDoor";
    }
    return "empty";  // inatteignable : le switch ci-dessus couvre tout l'enum.
}

std::optional<TileType> parseTileType(std::string_view name) {
    // Table construite une fois a partir de tileTypeName : la reciprocite des deux conversions est
    // ainsi structurelle, pas seulement testee. Ajouter un TileType suffit donc a le rendre
    // lisible -- la borne d'iteration vient de core::TILE_TYPE_COUNT (Core/Levels/TileType.h),
    // seule source de verite de la fin de l'enumeration depuis le LOT-74 : il n'y a plus de
    // dernier enumerateur recopie ici, donc plus rien a mettre a jour a la main. Les cles sont des
    // std::string (proprietaires) : tileTypeName renvoie une valeur, dont un string_view ne
    // survivrait pas.
    using NameTable =
        std::unordered_map<std::string, TileType, TransparentStringHash, std::equal_to<>>;
    static const NameTable byName = [] {
        NameTable table;
        for (int raw = 0; raw < TILE_TYPE_COUNT; ++raw) {
            const auto type = static_cast<TileType>(raw);
            table.emplace(tileTypeName(type), type);
        }
        return table;
    }();

    const auto found = byName.find(name);
    if (found == byName.end()) {
        return std::nullopt;
    }
    return found->second;
}

}  // namespace core
