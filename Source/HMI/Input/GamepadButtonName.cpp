// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "HMI/Input/GamepadButtonName.h"

#include "HMI/Input/InputState.h"

namespace hmi {

std::string gamepadButtonDisplayName(GamepadButton button) {
    switch (button) {
        case GamepadButton::Up:
            return "Haut";
        case GamepadButton::Down:
            return "Bas";
        case GamepadButton::Left:
            return "Gauche";
        case GamepadButton::Right:
            return "Droite";
        case GamepadButton::A:
            return "A";
        case GamepadButton::B:
            return "B";
        case GamepadButton::X:
            return "X";
        case GamepadButton::Y:
            return "Y";
        case GamepadButton::LeftShoulder:
            return "Epaule gauche";
        case GamepadButton::RightShoulder:
            return "Epaule droite";
    }
    return "";  // inatteignable : switch exhaustif sur GamepadButton ci-dessus.
}

std::optional<GamepadButton> capturedGamepadButton(const InputState& input) {
    for (int code = 0; code < GAMEPAD_BUTTON_COUNT; ++code) {
        const auto candidate = static_cast<GamepadButton>(code);
        if (input.gamepadButtonPressed(candidate)) {
            return candidate;
        }
    }
    return std::nullopt;
}

}  // namespace hmi
