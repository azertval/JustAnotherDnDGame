// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "HMI/Input/PlayerInputMapper.h"

#include "Core/Physics/PlayerInput.h"
#include "HMI/Input/InputState.h"

namespace hmi {

// Traduit l'etat clavier/manette en intention de deplacement (voir en-tete).
core::PlayerInput toPlayerInput(const InputState& input, const GameKeyBindings& gameKeyBindings,
                                const GamepadBindings& gamepadBindings) {
    // Chaque action : sa touche clavier liee OU son bouton manette lie (EX-CTRL-012).
    const bool left = input.keyDown(gameKeyBindings.key(GameAction::MoveLeft)) ||
                      input.gamepadButtonDown(gamepadBindings.button(GameAction::MoveLeft));
    const bool right = input.keyDown(gameKeyBindings.key(GameAction::MoveRight)) ||
                       input.gamepadButtonDown(gamepadBindings.button(GameAction::MoveRight));

    core::PlayerInput result;
    // Gauche et droite se neutralisent (-1 + 1 = 0) : comportement deterministe.
    result.moveX = (right ? 1.0F : 0.0F) - (left ? 1.0F : 0.0F);
    // Saut : jumpPressed = front (declenche/bufferise), jumpHeld = maintenu (hauteur variable).
    const Key jumpKey = gameKeyBindings.key(GameAction::Jump);
    const GamepadButton jumpButton = gamepadBindings.button(GameAction::Jump);
    result.jumpPressed = input.keyPressed(jumpKey) || input.gamepadButtonPressed(jumpButton);
    result.jumpHeld = input.keyDown(jumpKey) || input.gamepadButtonDown(jumpButton);
    // Visee verticale du dash (y vers le bas) : Bas = +1, Haut = -1, sinon 0.
    const bool aimDown = input.keyDown(gameKeyBindings.key(GameAction::AimDown)) ||
                         input.gamepadButtonDown(gamepadBindings.button(GameAction::AimDown));
    const bool aimUp = input.keyDown(gameKeyBindings.key(GameAction::AimUp)) ||
                       input.gamepadButtonDown(gamepadBindings.button(GameAction::AimUp));
    result.moveY = (aimDown ? 1.0F : 0.0F) - (aimUp ? 1.0F : 0.0F);
    // Dash : dashPressed = front (declenche, EX-CTRL-013), dashHeld = maintenu (charge, EX-GP-056).
    const Key dashKey = gameKeyBindings.key(GameAction::Dash);
    const GamepadButton dashButton = gamepadBindings.button(GameAction::Dash);
    result.dashPressed = input.keyPressed(dashKey) || input.gamepadButtonPressed(dashButton);
    result.dashHeld = input.keyDown(dashKey) || input.gamepadButtonDown(dashButton);
    // Interagir : pressee/maintenue/relachee (EX-CTRL-011), complete l'activation par contact
    // sans la remplacer (EX-CTRL-022).
    const Key interactKey = gameKeyBindings.key(GameAction::Interact);
    const GamepadButton interactButton = gamepadBindings.button(GameAction::Interact);
    result.interactPressed =
        input.keyPressed(interactKey) || input.gamepadButtonPressed(interactButton);
    result.interactHeld = input.keyDown(interactKey) || input.gamepadButtonDown(interactButton);
    result.interactReleased =
        input.keyReleased(interactKey) || input.gamepadButtonReleased(interactButton);
    return result;
}

}  // namespace hmi
