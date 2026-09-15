// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

/**
 * @file test_gamepad_probe.cpp
 * @brief Tests unitaires de l'espacement des sondages XInput (`EX-CTRL-002`, LOT-69).
 */

#include <chrono>

#include <gtest/gtest.h>

#include "HMI/Input/GamepadPoller.h"

using namespace std::chrono_literals;

/**
 * @brief Manette présente : le sondage est systématique, l'anti-saccade n'a plus lieu d'être.
 * \castest{<b>Une manette connectee est sondee a chaque appel.</b><br/>
 * \tcat Unitaire · Manette<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Demander la decision avec une manette connectee et un delai nul.<br/>
 * \tattendu Le sondage est du.
 * }
 */
TEST(GamepadProbeTest, ManetteConnecteeSondeeAChaqueAppel) {
    EXPECT_TRUE(hmi::gamepadProbeDue(/*wasConnected=*/true, 0ms));
    EXPECT_TRUE(hmi::gamepadProbeDue(/*wasConnected=*/true, 1ms));
}

/**
 * @brief Manette absente : le sondage est espacé, mais **en temps réel** — c'est ce qui rend le
 * délai de détection indépendant de la cadence de l'appelant.
 * \castest{<b>Une manette absente n'est re-sondee qu'apres le delai d'anti-saccade.</b><br/>
 * \tcat Unitaire · Manette<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Demander la decision juste avant le delai, puis juste apres.<br/>
 * \tattendu Refuse avant, du apres.
 * }
 */
TEST(GamepadProbeTest, ManetteAbsenteReSondeeApresLeDelai) {
    EXPECT_FALSE(hmi::gamepadProbeDue(/*wasConnected=*/false, 0ms));
    EXPECT_FALSE(
        hmi::gamepadProbeDue(/*wasConnected=*/false, hmi::GAMEPAD_DISCONNECTED_PROBE_PERIOD - 1ms));
    EXPECT_TRUE(
        hmi::gamepadProbeDue(/*wasConnected=*/false, hmi::GAMEPAD_DISCONNECTED_PROBE_PERIOD));
}

/**
 * @brief Le délai borne la détection **quelle que soit la cadence de l'appelant** : c'est
 * précisément ce qu'un compteur d'appels ne savait pas faire. À 500 ms par appel (onglet des
 * options), quatre appels suffisent ; l'ancien compteur en exigeait cent vingt, soit une minute.
 * \castest{<b>Le delai de detection ne depend pas de la cadence de l'appelant.</b><br/>
 * \tcat Unitaire · Manette<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Simuler des appels espaces de 500 ms et compter ceux qui precedent le premier
 * sondage du.<br/>2. Recommencer avec des appels espaces de 16 ms (cadence de rendu).<br/>
 * \tattendu Dans les deux cas, le sondage a lieu au plus tard au bout du delai d'anti-saccade.
 * }
 */
TEST(GamepadProbeTest, DelaiIndependantDeLaCadenceDAppel) {
    const auto firstDueAfter = [](std::chrono::milliseconds step) {
        std::chrono::milliseconds elapsed{0};
        while (!hmi::gamepadProbeDue(/*wasConnected=*/false, elapsed)) {
            elapsed += step;
        }
        return elapsed;
    };
    EXPECT_LE(firstDueAfter(500ms), hmi::GAMEPAD_DISCONNECTED_PROBE_PERIOD + 500ms);
    EXPECT_LE(firstDueAfter(16ms), hmi::GAMEPAD_DISCONNECTED_PROBE_PERIOD + 16ms);
}
