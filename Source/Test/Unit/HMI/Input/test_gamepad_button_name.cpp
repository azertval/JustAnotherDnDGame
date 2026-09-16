// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

/**
 * @file test_gamepad_button_name.cpp
 * @brief Tests unitaires de l'affichage et de la capture de bouton manette (`GamepadButtonName`,
 *        `LOT-30`).
 */

#include <gtest/gtest.h>

#include "HMI/Input/GamepadButtonName.h"
#include "HMI/Input/InputState.h"

/**
 * @brief Chaque bouton manette a un libellé lisible dédié.
 * \castest{<b>Chaque bouton manette a un libellé lisible dédié.</b><br/>
 * \tcat Unitaire · Gamepad Button Name<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Chaque bouton manette a un libellé lisible dédié.
 * }
 */
TEST(GamepadButtonNameTest, ChaqueBoutonALibelleDedie) {
    EXPECT_EQ(hmi::gamepadButtonDisplayName(hmi::GamepadButton::A), "A");
    EXPECT_EQ(hmi::gamepadButtonDisplayName(hmi::GamepadButton::Left), "Gauche");
    EXPECT_EQ(hmi::gamepadButtonDisplayName(hmi::GamepadButton::RightShoulder), "Epaule droite");
}

/**
 * @brief `capturedGamepadButton` renvoie le bouton pressé à la frame courante.
 * \castest{<b>`capturedGamepadButton` renvoie le bouton pressé à la frame courante.</b><br/>
 * \tcat Unitaire · Gamepad Button Name<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu `capturedGamepadButton` renvoie le bouton pressé à la frame courante.
 * }
 */
TEST(GamepadButtonNameTest, CapturedGamepadButtonRenvoieLeBoutonEnfonce) {
    hmi::InputState input;
    input.onGamepadButtonDown(hmi::GamepadButton::X);
    const std::optional<hmi::GamepadButton> captured = hmi::capturedGamepadButton(input);
    ASSERT_TRUE(captured.has_value());
    EXPECT_EQ(*captured, hmi::GamepadButton::X);
}

/**
 * @brief `capturedGamepadButton` renvoie vide quand aucun bouton n'est pressé.
 * \castest{<b>`capturedGamepadButton` renvoie vide quand aucun bouton n'est pressé.</b><br/>
 * \tcat Unitaire · Gamepad Button Name<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu `capturedGamepadButton` renvoie vide quand aucun bouton n'est pressé.
 * }
 */
TEST(GamepadButtonNameTest, CapturedGamepadButtonVideSansBouton) {
    const hmi::InputState input;
    EXPECT_FALSE(hmi::capturedGamepadButton(input).has_value());
}
