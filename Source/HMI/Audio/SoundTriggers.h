// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <optional>
#include <string>

#include "HMI/Game/GameEvents.h"

/**
 * @file HMI/Audio/SoundTriggers.h
 * @brief Table événement de jeu → identifiant de son (`EX-REN-047`).
 */

namespace hmi {

/**
 * @brief Associe un `hmi::GameEvent` à un identifiant de `hmi::SoundCatalog`.
 *
 * Fonction **pure**, exhaustive (switch sans `default` : un événement ajouté sans entrée casse la
 * compilation). Distincte de la **détection** (`HMI/Game/GameEvents.h`) : ce qui **est arrivé** et
 * ce que **ça produit** sont deux questions séparées, pour que `LOT-53` puisse brancher des
 * particules sur la même détection sans la modifier.
 * @param event Événement survenu.
 * @return L'identifiant à passer à `hmi::SoundCatalog::resolve`, ou `std::nullopt` si cet
 *         événement ne produit **intentionnellement** aucun son — un silence documenté, pas un
 *         oubli (ex. `WallContactEnter`, `BlockPushed` : aucun bruitage dédié dans ce lot).
 */
[[nodiscard]] std::optional<std::string> soundForEvent(GameEvent event);

}  // namespace hmi
