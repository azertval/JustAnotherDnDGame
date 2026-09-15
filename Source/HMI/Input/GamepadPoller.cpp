// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "HMI/Input/GamepadPoller.h"

#include <Windows.h>  // doit précéder <Xinput.h> (définit l'architecture cible).
#include <Xinput.h>

#include "HMI/HmiLog.h"
#include "HMI/Input/GamepadButton.h"

namespace hmi {

namespace {
// Sens du stick gauche sur un axe, au-dela de la zone morte XInput ; 0 si dans la zone morte
// (evite un deplacement fantome au repos, jitter materiel).
[[nodiscard]] int stickDirection(SHORT axis) noexcept {
    if (axis > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) {
        return 1;
    }
    if (axis < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) {
        return -1;
    }
    return 0;
}
}  // namespace

void GamepadPoller::poll(InputState& input) {
    // Espacement des sondages d'un slot vide : decide EN TEMPS REEL (hmi::gamepadProbeDue), jamais
    // en nombre d'appels -- poll() est appele tantot par la boucle de rendu, tantot par un
    // temporisateur d'interface, et un compteur d'appels donnait alors une detection de manette
    // allant de deux secondes a une minute selon l'appelant.
    const std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    if (!gamepadProbeDue(_wasConnected, now - _lastProbe)) {
        input.setGamepadConnected(false);
        return;
    }
    _lastProbe = now;

    XINPUT_STATE state{};
    const bool connected = XInputGetState(0, &state) == ERROR_SUCCESS;
    input.setGamepadConnected(connected);
    if (connected != _wasConnected) {
        HMI_LOG_INFO(connected ? "Manette connectee" : "Manette deconnectee");
        _wasConnected = connected;
    }

    const XINPUT_GAMEPAD& pad = state.Gamepad;
    const int stickX = stickDirection(pad.sThumbLX);
    const int stickY = stickDirection(pad.sThumbLY);  // XInput : Y positif = vers le haut

    auto setKey = [&input](Key key, bool pressed) {
        if (pressed) {
            input.onGamepadKeyDown(key);
        } else {
            input.onGamepadKeyUp(key);
        }
    };

    // D-pad ou stick gauche : les deux pilotent les memes directions (EX-CTRL-002).
    const bool left = (pad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) != 0 || stickX < 0;
    const bool right = (pad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) != 0 || stickX > 0;
    const bool up = (pad.wButtons & XINPUT_GAMEPAD_DPAD_UP) != 0 || stickY > 0;
    const bool down = (pad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) != 0 || stickY < 0;
    setKey(Key::Left, left);
    setKey(Key::Right, right);
    setKey(Key::Up, up);
    setKey(Key::Down, down);
    // A valide (menu) ET saute (jeu) : meme bouton physique pour les deux usages, comme au clavier.
    const bool aHeld = (pad.wButtons & XINPUT_GAMEPAD_A) != 0;
    setKey(Key::Enter, aHeld);
    setKey(Key::Space, aHeld);
    // B et Start reproduisent tous deux Echap (retour/pause) : pas d'ecran de pause dedie.
    const bool bHeld = (pad.wButtons & XINPUT_GAMEPAD_B) != 0;
    setKey(Key::Escape, bHeld || (pad.wButtons & XINPUT_GAMEPAD_START) != 0);
    const bool rightShoulderHeld = (pad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) != 0;
    setKey(Key::Shift, rightShoulderHeld);  // dash, EX-CTRL-013

    // Piste manette brute (GamepadButton), independante de la fusion Key ci-dessus : consommee par
    // PlayerInputMapper via GamepadBindings (EX-CTRL-002, EX-CTRL-012), pas par la navigation de
    // menu (qui reste sur la fusion Key, non remappable).
    auto setButton = [&input](GamepadButton button, bool pressed) {
        if (pressed) {
            input.onGamepadButtonDown(button);
        } else {
            input.onGamepadButtonUp(button);
        }
    };
    setButton(GamepadButton::Left, left);
    setButton(GamepadButton::Right, right);
    setButton(GamepadButton::Up, up);
    setButton(GamepadButton::Down, down);
    setButton(GamepadButton::A, aHeld);
    setButton(GamepadButton::B, bHeld);
    setButton(GamepadButton::X, (pad.wButtons & XINPUT_GAMEPAD_X) != 0);
    setButton(GamepadButton::Y, (pad.wButtons & XINPUT_GAMEPAD_Y) != 0);
    setButton(GamepadButton::LeftShoulder, (pad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) != 0);
    setButton(GamepadButton::RightShoulder, rightShoulderHeld);
}

}  // namespace hmi
