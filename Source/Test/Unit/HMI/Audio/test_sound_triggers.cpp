// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_sound_triggers.cpp
 * @brief Tests unitaires de la table événement → son (LOT-60, EX-REN-047).
 */

#include <gtest/gtest.h>

#include "HMI/Audio/SoundTriggers.h"

/**
 * @brief La table événement → son est exhaustive : chaque valeur de l'énumération a une entrée.
 * \castest{<b>La table evenement vers son est exhaustive.</b><br/>
 * \tcat Unitaire · Declencheurs de son<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Parcourir toutes les valeurs de GameEvent (0 a GAME_EVENT_COUNT - 1).<br/>
 * 2. Appeler soundForEvent sur chacune.<br/>
 * \tattendu Aucun plantage : le switch interne couvre chaque cas (verifie par construction, pas
 * par relecture -- un GameEvent ajoute sans entree casserait la compilation avant ce test).
 * }
 */
TEST(SoundTriggersTest, TableExhaustive) {
    for (int i = 0; i < hmi::GAME_EVENT_COUNT; ++i) {
        const auto event = static_cast<hmi::GameEvent>(i);
        // Ne verifie pas la valeur (silence volontaire pour certains evenements) : verifie
        // seulement que l'appel ne plante jamais, quel que soit l'evenement.
        (void)hmi::soundForEvent(event);
    }
}

/**
 * @brief Les evenements de personnage resolvent vers les identifiants de sounds.json attendus.
 * \castest{<b>Les evenements de personnage resolvent les bons identifiants de son.</b><br/>
 * \tcat Unitaire · Declencheurs de son<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Resoudre Jumped, Landed, Dashed, Died, ExitReached.<br/>
 * \tattendu Chacun resout l'identifiant attendu (saut, atterrissage, dash, mort,
 * sortie_atteinte).
 * }
 */
TEST(SoundTriggersTest, EvenementsDePersonnageResolvent) {
    EXPECT_EQ(hmi::soundForEvent(hmi::GameEvent::Jumped), "saut");
    EXPECT_EQ(hmi::soundForEvent(hmi::GameEvent::Landed), "atterrissage");
    EXPECT_EQ(hmi::soundForEvent(hmi::GameEvent::Dashed), "dash");
    EXPECT_EQ(hmi::soundForEvent(hmi::GameEvent::Died), "mort");
    EXPECT_EQ(hmi::soundForEvent(hmi::GameEvent::ExitReached), "sortie_atteinte");
}

/**
 * @brief Les evenements de mecanismes resolvent vers les identifiants attendus.
 * \castest{<b>Les evenements de mecanismes resolvent les bons identifiants de son.</b><br/>
 * \tcat Unitaire · Declencheurs de son<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Resoudre SwitchToggled, PressurePlatePressed, PressurePlateReleased.<br/>
 * \tattendu interrupteur, plaque_pression (les deux fronts de la plaque partagent le meme son).
 * }
 */
TEST(SoundTriggersTest, EvenementsDeMecanismesResolvent) {
    EXPECT_EQ(hmi::soundForEvent(hmi::GameEvent::SwitchToggled), "interrupteur");
    EXPECT_EQ(hmi::soundForEvent(hmi::GameEvent::PressurePlatePressed), "plaque_pression");
    EXPECT_EQ(hmi::soundForEvent(hmi::GameEvent::PressurePlateReleased), "plaque_pression");
}

/**
 * @brief Des evenements sans bruitage dedie resolvent explicitement vers le silence.
 * \castest{<b>Des evenements sans bruitage dedie resolvent vers std::nullopt.</b><br/>
 * \tcat Unitaire · Declencheurs de son<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Resoudre WallContactEnter et BlockPushed.<br/>
 * \tattendu std::nullopt pour les deux -- silence documente, pas un oubli.
 * }
 */
TEST(SoundTriggersTest, EvenementsSansBruitageResolventNullopt) {
    EXPECT_EQ(hmi::soundForEvent(hmi::GameEvent::WallContactEnter), std::nullopt);
    EXPECT_EQ(hmi::soundForEvent(hmi::GameEvent::BlockPushed), std::nullopt);
}
