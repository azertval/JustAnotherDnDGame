// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

/**
 * @file HMI/Input/GamepadButton.h
 * @brief Bouton manette logique, indépendant de toute action de jeu ou touche clavier.
 */

namespace hmi {

/**
 * @brief Bouton (ou direction) manette remappable à une action de jeu (`EX-CTRL-002`).
 *
 * `Up`/`Down`/`Left`/`Right` fusionnent le D-pad et le stick gauche en une seule notion logique
 * par direction — le joueur ne perçoit pas la différence entre les deux, et
 * `hmi::GamepadPoller` les a toujours traités comme équivalents. `Start`/`Back`, les clics de
 * stick et les gâchettes analogiques ne sont pas représentés : conventions globales ou hors
 * périmètre (voir l'épic du lot).
 */
enum class GamepadButton {
    Up,
    Down,
    Left,
    Right,
    A,
    B,
    X,
    Y,
    LeftShoulder,
    RightShoulder,
};

/// Nombre de boutons manette remappables (`GamepadButton`).
constexpr int GAMEPAD_BUTTON_COUNT = 10;

}  // namespace hmi
