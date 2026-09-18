// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

/**
 * @file test_audio_engine.cpp
 * @brief Tests unitaires du moteur audio (EX-REN-047, EX-NFR-040).
 */

#include <gtest/gtest.h>

#include "HMI/Audio/AudioEngine.h"

/**
 * @brief Un moteur muet accepte precharge et lecture sans erreur ni plantage.
 * \castest{<b>Un moteur audio muet accepte precharge et lecture sans erreur.</b><br/>
 * \tcat Unitaire · Moteur audio<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Construire un AudioEngine force en etat muet (ForceMuted::Yes).<br/>
 * 2. Precharger un echantillon sur un chemin inexistant.<br/>
 * 3. Jouer cet echantillon, puis un identifiant jamais precharge.<br/>
 * \tattendu Aucune exception, aucun plantage : le peripherique absent ne bloque jamais le jeu
 * (EX-NFR-040).
 * }
 */
TEST(AudioEngine, MutedEngineAcceptsPreloadAndPlayWithoutError) {
    hmi::AudioEngine engine(hmi::AudioEngine::ForceMuted::Yes);
    EXPECT_TRUE(engine.muted());

    engine.preload("saut", "chemin/inexistant.wav");
    engine.play("saut");
    engine.play("evenement_jamais_precharge");
    // Aucune exception, aucun plantage : le test reussit s'il atteint ce point.
}

/**
 * @brief Le volume regle est toujours ramene dans [0, 1], jamais propage hors bornes.
 * \castest{<b>Le volume du moteur audio est borne a [0, 1].</b><br/>
 * \tcat Unitaire · Moteur audio<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Regler un volume negatif, puis un volume superieur a 1, puis une valeur
 * intermediaire.<br/>
 * \tattendu La valeur lue est ramenee a l'extremite la plus proche pour les valeurs hors bornes,
 * et inchangee pour une valeur deja valide.
 * }
 */
TEST(AudioEngine, VolumeIsClampedToUnitRange) {
    hmi::AudioEngine engine(hmi::AudioEngine::ForceMuted::Yes);

    engine.setVolume(-5.0f);
    EXPECT_FLOAT_EQ(engine.volume(), 0.0f);

    engine.setVolume(5.0f);
    EXPECT_FLOAT_EQ(engine.volume(), 1.0f);

    engine.setVolume(0.42f);
    EXPECT_FLOAT_EQ(engine.volume(), 0.42f);
}

/**
 * @brief Le volume par defaut d'un moteur fraichement construit est au maximum.
 * \castest{<b>Le volume par defaut du moteur audio est au maximum.</b><br/>
 * \tcat Unitaire · Moteur audio<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Construire un AudioEngine sans regler de volume.<br/>
 * \tattendu Le volume lu vaut 1.0.
 * }
 */
TEST(AudioEngine, DefaultVolumeIsFull) {
    hmi::AudioEngine engine(hmi::AudioEngine::ForceMuted::Yes);
    EXPECT_FLOAT_EQ(engine.volume(), 1.0f);
}
