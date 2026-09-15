// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file fuzz_skins.cpp
 * @brief Cible libFuzzer de `hmi::SkinCatalog::loadFromString`.
 *
 * Le catalogue d'habillage des tuiles, représentant des catalogues de HmiLib.
 *
 * Le lecteur promet de rendre une erreur décrite, jamais de lever : toute exception qui s'échappe,
 * lecture hors bornes ou allocation démesurée est un défaut que le job `fuzz` de nuit rapporte avec
 * l'entrée qui le reproduit. Rejouer une entrée : `fuzz_skins.exe crash-<empreinte>`.
 */

#include <cstddef>
#include <cstdint>
#include <string_view>

#include "HMI/Graphics/SkinCatalog.h"

/// @brief Point d'entrée appelé par libFuzzer pour chaque entrée générée.
extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t* data, std::size_t size) {
    const std::string_view text(static_cast<const char*>(static_cast<const void*>(data)), size);
    (void)hmi::SkinCatalog::loadFromString(text);
    return 0;
}
