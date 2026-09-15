// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include "Core/Physics/PlayerInput.h"
#include "HMI/Input/GameKeyBindings.h"
#include "HMI/Input/GamepadBindings.h"

/**
 * @file HMI/Input/PlayerInputMapper.h
 * @brief Traduction des entrées clavier/manette en intention de déplacement (action logique).
 */

namespace hmi {

class InputState;

/**
 * @brief Traduit l'état d'entrée en ::core::PlayerInput, dissocié des touches/boutons physiques
 *        (`EX-CTRL-010`, `EX-CTRL-012`).
 *
 * Le reste du moteur ne connaît que l'**intention** (`moveX`), pas les touches ni les boutons.
 * Chaque action est déclenchée par **deux** sources indépendantes, chacune remappable
 * séparément : la touche clavier liée dans @p gameKeyBindings (Options → Touches de jeu) **ou**
 * le bouton manette lié dans @p gamepadBindings (Options → Touches de la manette) — l'une des
 * deux suffit, aucune des deux n'est prioritaire sur l'autre.
 *
 * Gauche et droite simultanées se neutralisent (`moveX == 0`). Fonction **pure**, testable avec un
 * `InputState` injecté (`EX-NFR-010`) ; appelée une fois par frame en amont de la logique
 * (`EX-CTRL-020`, `EX-CTRL-021`).
 *
 * @param input           État clavier/manette échantillonné de la frame.
 * @param gameKeyBindings Association action de jeu -> touche clavier courante.
 * @param gamepadBindings Association action de jeu -> bouton manette courant.
 * @return L'intention de déplacement correspondante.
 */
[[nodiscard]] core::PlayerInput toPlayerInput(const InputState& input,
                                              const GameKeyBindings& gameKeyBindings,
                                              const GamepadBindings& gamepadBindings);

}  // namespace hmi
