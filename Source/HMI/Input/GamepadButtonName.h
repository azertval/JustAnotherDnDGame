// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <optional>
#include <string>

#include "HMI/Input/GamepadButton.h"

/**
 * @file HMI/Input/GamepadButtonName.h
 * @brief Nom affichable d'un bouton manette et capture générique « prochain bouton pressé »
 *        (`LOT-30`).
 */

namespace hmi {

class InputState;

/**
 * @brief Nom lisible d'un bouton manette, pour l'affichage dans l'écran de remappage.
 * @param button Bouton à nommer.
 * @return Un nom court (« A », « Épaule droite », « Flèche gauche »…).
 */
[[nodiscard]] std::string gamepadButtonDisplayName(GamepadButton button);

/**
 * @brief Cherche le premier bouton manette pressé cette frame, pour la capture d'un remappage.
 * @param input État des entrées de la frame (piste manette brute, `InputState::
 *              gamepadButtonPressed`).
 * @return Le bouton capturé, ou `std::nullopt` si aucun n'est pressé cette frame.
 */
[[nodiscard]] std::optional<GamepadButton> capturedGamepadButton(const InputState& input);

}  // namespace hmi
