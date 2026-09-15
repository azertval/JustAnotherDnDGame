// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file fuzz_localization.cpp
 * @brief Cible libFuzzer de `hmi::Localization::parseCatalog`.
 *
 * Le lecteur des catalogues de traduction `clé = valeur`, le seul lecteur de texte qui ne soit
 * pas du JSON.
 *
 * Le lecteur promet de rendre une erreur décrite, jamais de lever : toute exception qui s'échappe,
 * lecture hors bornes ou allocation démesurée est un défaut que le job `fuzz` de nuit rapporte avec
 * l'entrée qui le reproduit. Rejouer une entrée : `fuzz_localization.exe crash-<empreinte>`.
 */

#include <cstddef>
#include <cstdint>
#include <string_view>

#include "HMI/Localization/Localization.h"

/// @brief Point d'entrée appelé par libFuzzer pour chaque entrée générée.
extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t* data, std::size_t size) {
    const std::string_view text(static_cast<const char*>(static_cast<const void*>(data)), size);
    (void)hmi::Localization::parseCatalog(text);
    return 0;
}
