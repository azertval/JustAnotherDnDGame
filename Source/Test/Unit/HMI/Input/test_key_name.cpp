// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

/**
 * @file test_key_name.cpp
 * @brief Tests unitaires de l'affichage et de la capture de touche (`KeyName`, `LOT-29`).
 */

#include <gtest/gtest.h>

#include "HMI/Input/InputState.h"
#include "HMI/Input/KeyName.h"

/**
 * @brief Une touche nommée de l'énumération a un libellé lisible dédié.
 * \castest{<b>Une touche nommée de l'énumération a un libellé lisible dédié.</b><br/>
 * \tcat Unitaire · Key Name<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Une touche nommée de l'énumération a un libellé lisible dédié.
 * }
 */
TEST(KeyNameTest, ToucheNommeeLibelleDedie) {
    EXPECT_EQ(hmi::keyDisplayName(hmi::Key::Left), "Fleche gauche");
    EXPECT_EQ(hmi::keyDisplayName(hmi::Key::Space), "Espace");
    EXPECT_EQ(hmi::keyDisplayName(hmi::Key::Shift), "Maj");
    EXPECT_EQ(hmi::keyDisplayName(hmi::Key::F10), "F10");
}

/**
 * @brief Une lettre ou un chiffre non nommé explicitement s'affiche par son propre caractère.
 * \castest{<b>Une lettre ou un chiffre non nommé explicitement s'affiche par son propre
 * caractère.</b><br/>
 * \tcat Unitaire · Key Name<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Une lettre ou un chiffre non nommé explicitement s'affiche par son propre caractère.
 * }
 */
TEST(KeyNameTest, LettreNonNommeeAfficheSonCaractere) {
    // 'B' (0x42) n'est pas un enumerateur nomme de Key : repli generique attendu.
    EXPECT_EQ(hmi::keyDisplayName(static_cast<hmi::Key>('B')), "B");
    EXPECT_EQ(hmi::keyDisplayName(static_cast<hmi::Key>('5')), "5");
}

/**
 * @brief Une touche totalement inconnue s'affiche via un repli hexadécimal.
 * \castest{<b>Une touche totalement inconnue s'affiche via un repli hexadécimal.</b><br/>
 * \tcat Unitaire · Key Name<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Une touche totalement inconnue s'affiche via un repli hexadécimal.
 * }
 */
TEST(KeyNameTest, ToucheInconnueRepliHexadecimal) {
    EXPECT_EQ(hmi::keyDisplayName(static_cast<hmi::Key>(0x12)), "Touche 0x12");
}

/**
 * @brief `capturedKey` renvoie la touche pressée à la frame courante.
 * \castest{<b>`capturedKey` renvoie la touche pressée à la frame courante.</b><br/>
 * \tcat Unitaire · Key Name<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu `capturedKey` renvoie la touche pressée à la frame courante.
 * }
 */
TEST(KeyNameTest, CapturedKeyRenvoieLaToucheEnfoncee) {
    hmi::InputState input;
    input.onKeyDown(hmi::Key::F1);
    const std::optional<hmi::Key> captured = hmi::capturedKey(input);
    ASSERT_TRUE(captured.has_value());
    EXPECT_EQ(*captured, hmi::Key::F1);
}

/**
 * @brief `capturedKey` renvoie vide quand aucune touche n'est pressée.
 * \castest{<b>`capturedKey` renvoie vide quand aucune touche n'est pressée.</b><br/>
 * \tcat Unitaire · Key Name<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu `capturedKey` renvoie vide quand aucune touche n'est pressée.
 * }
 */
TEST(KeyNameTest, CapturedKeyVideSansTouche) {
    const hmi::InputState input;
    EXPECT_FALSE(hmi::capturedKey(input).has_value());
}

/**
 * @brief `Échap` et `Entrée` sont réservées : leur pression seule ne capture rien.
 * \castest{<b>`Échap` et `Entrée` sont réservées : leur pression seule ne capture rien.</b><br/>
 * \tcat Unitaire · Key Name<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu `Échap` et `Entrée` sont réservées : leur pression seule ne capture rien.
 * }
 */
TEST(KeyNameTest, EchapEtEntreeReserveesNonCapturees) {
    hmi::InputState escapeOnly;
    escapeOnly.onKeyDown(hmi::Key::Escape);
    EXPECT_FALSE(hmi::capturedKey(escapeOnly).has_value());

    hmi::InputState enterOnly;
    enterOnly.onKeyDown(hmi::Key::Enter);
    EXPECT_FALSE(hmi::capturedKey(enterOnly).has_value());
}
