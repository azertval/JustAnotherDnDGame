// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file fuzz_json.cpp
 * @brief Cible libFuzzer de `core::readJsonObject`.
 *
 * L'enveloppe commune de tous les catalogues : JSON bien formé, racine objet, garde de version.
 * Tout lecteur de données du jeu passe d'abord par elle.
 *
 * Le lecteur promet de rendre une erreur décrite, jamais de lever : toute exception qui s'échappe,
 * lecture hors bornes ou allocation démesurée est un défaut que le job `fuzz` de nuit rapporte avec
 * l'entrée qui le reproduit. Rejouer une entrée : `fuzz_json.exe crash-<empreinte>`.
 */

#include <cstddef>
#include <cstdint>
#include <string_view>

#include "Core/Data/JsonDocument.h"

/// @brief Point d'entrée appelé par libFuzzer pour chaque entrée générée.
extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t* data, std::size_t size) {
    const std::string_view text(static_cast<const char*>(static_cast<const void*>(data)), size);
    (void)core::readJsonObject(text, 1, "fuzz");
    return 0;
}
