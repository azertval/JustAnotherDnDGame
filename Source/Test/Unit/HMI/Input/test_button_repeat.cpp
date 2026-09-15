// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_button_repeat.cpp
 * @brief Tests unitaires de la répétition d'un bouton de manette tenu (`LOT-24`).
 */

#include <chrono>
#include <vector>

#include <gtest/gtest.h>

#include "HMI/Input/ButtonRepeat.h"

/**
 * @brief Un bouton tenu avance d'un pas a l'appui, puis a intervalle regulier apres un delai.
 * \castest{<b>La croix de la manette tenue deplace le curseur d'une case a l'appui, puis se repete
 * apres le delai, a intervalle regulier ; relachee, elle s'arrete, et un nouvel appui repart du
 * debut.</b><br/>
 * \tcat Unitaire · IHM<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Appuyer a t = 0, tenir jusqu'a 600 ms en sondant toutes les 16 ms.<br/>2. Relacher,
 * puis rappuyer.<br/>
 * \tattendu Un pas a 0, rien avant 350 ms, un pas a 352 ms puis a 464 ms et 576 ms ; rien relache ;
 * un pas au nouvel appui.
 * }
 */
TEST(ButtonRepeatTest, UnAppuiPuisUneRepetitionReguliere) {
    using namespace std::chrono_literals;
    hmi::ButtonRepeat bouton;
    const auto debut = std::chrono::steady_clock::time_point{} + 1h;
    std::vector<long long> pas;
    for (long long t = 0; t <= 600; t += 16) {
        if (bouton.update(true, debut + std::chrono::milliseconds(t))) {
            pas.push_back(t);
        }
    }
    EXPECT_EQ(pas, (std::vector<long long>{0, 352, 464, 576}));
    EXPECT_FALSE(bouton.update(false, debut + 620ms));
    EXPECT_FALSE(bouton.update(false, debut + 700ms));
    EXPECT_TRUE(bouton.update(true, debut + 716ms));
    EXPECT_FALSE(bouton.update(true, debut + 732ms));
}
