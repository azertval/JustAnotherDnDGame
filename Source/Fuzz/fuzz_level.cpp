// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

/**
 * @file fuzz_level.cpp
 * @brief Cible libFuzzer de `core::LevelLoader::loadFromString`.
 *
 * Le lecteur de niveaux : le plus gros lecteur du dépôt, qui dimensionne une carte d'après le
 * fichier et n'attrape que les exceptions de nlohmann.
 *
 * Le lecteur promet de rendre une erreur décrite, jamais de lever : toute exception qui s'échappe,
 * lecture hors bornes ou allocation démesurée est un défaut que le job `fuzz` de nuit rapporte avec
 * l'entrée qui le reproduit. Rejouer une entrée : `fuzz_level.exe crash-<empreinte>`.
 *
 * Depuis le format v4 (`LOT-EDITOR-12`), une entrée peut être une **variante** : sa base se résout
 * vers une petite carte fixe, pour que le chemin des variantes soit parcouru comme les autres.
 */

#include <cstddef>
#include <cstdint>
#include <string_view>

#include "Core/Levels/LevelLoader.h"

/// @brief Point d'entrée appelé par libFuzzer pour chaque entrée générée.
extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t* data, std::size_t size) {
    const std::string_view text(static_cast<const char*>(static_cast<const void*>(data)), size);
    static const core::LevelLoader::BaseResolver RESOLVE_BASE = [](std::string_view) {
        return core::LevelLoader::loadFromString(
            R"({"version": 4, "width": 2, "height": 2, "tiles": [{"x": 0, "y": 0, "type": "entry"}],
                "layers": [{"kind": "ground", "scene": "martpart", "tiles": []}]})");
    };
    (void)core::LevelLoader::loadFromString(text, RESOLVE_BASE);
    return 0;
}
