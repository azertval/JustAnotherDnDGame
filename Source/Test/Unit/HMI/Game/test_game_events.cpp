// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_game_events.cpp
 * @brief Tests unitaires de la détection d'événements de jeu (LOT-60, EX-REN-047).
 */

#include <algorithm>
#include <optional>
#include <vector>

#include <gtest/gtest.h>

#include "HMI/Game/GameEvents.h"

namespace {

using hmi::GameEvent;
using hmi::MechanismEventState;

bool contains(const std::vector<GameEvent>& events, GameEvent event) {
    return std::find(events.begin(), events.end(), event) != events.end();
}

}  // namespace

/**
 * @brief Un interrupteur (non continu) qui bascule produit SwitchToggled.
 * \castest{<b>Un interrupteur bascule produit l'evenement SwitchToggled.</b><br/>
 * \tcat Unitaire · Detection d'evenements<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Un mecanisme non continu passe de ferme a ouvert.<br/>
 * \tattendu La liste contient exactement un evenement : SwitchToggled.
 * }
 */
TEST(GameEventsTest, InterrupteurBasculeProduitSwitchToggled) {
    const MechanismEventState previous{.doorOpen = {false}};
    const MechanismEventState current{.doorOpen = {true}};
    const std::vector<bool> continuous = {false};

    const std::vector<GameEvent> events = hmi::detectMechanismEvents(previous, current, continuous);
    ASSERT_EQ(events.size(), 1U);
    EXPECT_EQ(events[0], GameEvent::SwitchToggled);
}

/**
 * @brief Une plaque de pression enfoncee produit PressurePlatePressed, pas SwitchToggled.
 * \castest{<b>Une plaque de pression enfoncee produit PressurePlatePressed.</b><br/>
 * \tcat Unitaire · Detection d'evenements<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Un mecanisme continu passe de ferme a ouvert.<br/>
 * \tattendu La liste contient exactement un evenement : PressurePlatePressed.
 * }
 */
TEST(GameEventsTest, PlaquePresseeProduitPressurePlatePressed) {
    const MechanismEventState previous{.doorOpen = {false}};
    const MechanismEventState current{.doorOpen = {true}};
    const std::vector<bool> continuous = {true};

    const std::vector<GameEvent> events = hmi::detectMechanismEvents(previous, current, continuous);
    ASSERT_EQ(events.size(), 1U);
    EXPECT_EQ(events[0], GameEvent::PressurePlatePressed);
}

/**
 * @brief Une plaque de pression relachee produit PressurePlateReleased.
 * \castest{<b>Une plaque de pression relachee produit PressurePlateReleased.</b><br/>
 * \tcat Unitaire · Detection d'evenements<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Un mecanisme continu passe d'ouvert a ferme.<br/>
 * \tattendu La liste contient exactement un evenement : PressurePlateReleased.
 * }
 */
TEST(GameEventsTest, PlaqueRelacheeProduitPressurePlateReleased) {
    const MechanismEventState previous{.doorOpen = {true}};
    const MechanismEventState current{.doorOpen = {false}};
    const std::vector<bool> continuous = {true};

    const std::vector<GameEvent> events = hmi::detectMechanismEvents(previous, current, continuous);
    ASSERT_EQ(events.size(), 1U);
    EXPECT_EQ(events[0], GameEvent::PressurePlateReleased);
}

/**
 * @brief Un mecanisme dont l'etat ne change pas ne produit aucun evenement.
 * \castest{<b>Un mecanisme stable ne produit aucun evenement.</b><br/>
 * \tcat Unitaire · Detection d'evenements<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. previous et current identiques pour un jeu de mecanismes.<br/>
 * \tattendu La liste d'evenements est vide.
 * }
 */
TEST(GameEventsTest, MecanismeStableNeProduitRien) {
    const MechanismEventState state{.doorOpen = {true, false, true}};
    const std::vector<bool> continuous = {false, true, false};

    EXPECT_TRUE(hmi::detectMechanismEvents(state, state, continuous).empty());
}

/**
 * @brief Seul le mecanisme qui transitionne produit un evenement, les autres restent muets.
 * \castest{<b>Un seul mecanisme sur plusieurs qui change produit un seul evenement.</b><br/>
 * \tcat Unitaire · Detection d'evenements<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Trois mecanismes, un seul change d'etat.<br/>
 * \tattendu Exactement un evenement, correspondant au mecanisme qui a change.
 * }
 */
TEST(GameEventsTest, SeulLeMecanismeQuiChangeProduitUnEvenement) {
    const MechanismEventState previous{.doorOpen = {false, false, false}};
    const MechanismEventState current{.doorOpen = {false, true, false}};
    const std::vector<bool> continuous = {false, false, false};

    const std::vector<GameEvent> events = hmi::detectMechanismEvents(previous, current, continuous);
    ASSERT_EQ(events.size(), 1U);
    EXPECT_EQ(events[0], GameEvent::SwitchToggled);
}

/**
 * @brief Une taille incoherente entre les vecteurs ignore l'index en trop sans planter.
 * \castest{<b>Des vecteurs de tailles differentes ne provoquent aucun acces hors bornes.</b><br/>
 * \tcat Unitaire · Detection d'evenements<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. previous a 2 elements, current en a 3, isContinuous en a 2.<br/>
 * \tattendu Aucun plantage ; seuls les index communs aux trois vecteurs sont consideres.
 * }
 */
TEST(GameEventsTest, TaillesIncoherentesNIgnorentSansPlanter) {
    const MechanismEventState previous{.doorOpen = {false, false}};
    const MechanismEventState current{.doorOpen = {false, false, true}};
    const std::vector<bool> continuous = {false, false};

    EXPECT_TRUE(hmi::detectMechanismEvents(previous, current, continuous).empty());
}

/**
 * @brief L'issue Playing ne produit aucun evenement.
 * \castest{<b>Une partie en cours ne produit aucun evenement d'issue.</b><br/>
 * \tcat Unitaire · Detection d'evenements<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Traduire core::LevelOutcome::Playing.<br/>
 * \tattendu std::nullopt.
 * }
 */
TEST(GameEventsTest, IssueEnCoursNeProduitRien) {
    EXPECT_EQ(hmi::detectOutcomeEvent(core::LevelOutcome::Playing), std::nullopt);
}

/**
 * @brief L'issue Won produit ExitReached.
 * \castest{<b>Une issue gagnee produit l'evenement ExitReached.</b><br/>
 * \tcat Unitaire · Detection d'evenements<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Traduire core::LevelOutcome::Won.<br/>
 * \tattendu GameEvent::ExitReached.
 * }
 */
TEST(GameEventsTest, IssueGagneeProduitExitReached) {
    EXPECT_EQ(hmi::detectOutcomeEvent(core::LevelOutcome::Won), GameEvent::ExitReached);
}

/**
 * @brief L'issue Lost produit Died.
 * \castest{<b>Une issue perdue produit l'evenement Died.</b><br/>
 * \tcat Unitaire · Detection d'evenements<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Traduire core::LevelOutcome::Lost.<br/>
 * \tattendu GameEvent::Died.
 * }
 */
TEST(GameEventsTest, IssuePerdueProduitDied) {
    EXPECT_EQ(hmi::detectOutcomeEvent(core::LevelOutcome::Lost), GameEvent::Died);
}
