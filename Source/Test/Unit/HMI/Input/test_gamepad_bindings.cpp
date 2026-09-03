// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_gamepad_bindings.cpp
 * @brief Tests unitaires du remappage manette (`GamepadBindings`, `EX-CTRL-002`, `EX-CTRL-012`).
 */

#include <filesystem>
#include <fstream>

#include <gtest/gtest.h>

#include "HMI/Input/GamepadBindings.h"

/**
 * @brief Les sept actions ont leurs boutons par défaut (D-pad/stick, A, épaule droite, X) à la
 *        construction.
 * \castest{<b>Les sept actions ont leurs boutons par défaut à la construction.</b><br/>
 * \tcat Unitaire · Gamepad Bindings<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Les sept actions ont leurs boutons par défaut à la construction.
 * }
 */
TEST(GamepadBindingsTest, ValeursParDefautALaConstruction) {
    const hmi::GamepadBindings bindings;
    EXPECT_EQ(bindings.button(hmi::GameAction::MoveLeft), hmi::GamepadButton::Left);
    EXPECT_EQ(bindings.button(hmi::GameAction::MoveRight), hmi::GamepadButton::Right);
    EXPECT_EQ(bindings.button(hmi::GameAction::AimUp), hmi::GamepadButton::Up);
    EXPECT_EQ(bindings.button(hmi::GameAction::AimDown), hmi::GamepadButton::Down);
    EXPECT_EQ(bindings.button(hmi::GameAction::Jump), hmi::GamepadButton::A);
    EXPECT_EQ(bindings.button(hmi::GameAction::Dash), hmi::GamepadButton::RightShoulder);
    EXPECT_EQ(bindings.button(hmi::GameAction::Interact), hmi::GamepadButton::X);
}

/**
 * @brief `setKey` sur un bouton libre lie l'action sans toucher aux autres.
 * \castest{<b>`setKey` sur un bouton libre lie l'action sans toucher aux autres.</b><br/>
 * \tcat Unitaire · Gamepad Bindings<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu `setKey` sur un bouton libre lie l'action sans toucher aux autres.
 * }
 */
TEST(GamepadBindingsTest, SetKeySurBoutonLibre) {
    hmi::GamepadBindings bindings;
    bindings.setKey(hmi::GameAction::Jump, hmi::GamepadButton::X);
    EXPECT_EQ(bindings.button(hmi::GameAction::Jump), hmi::GamepadButton::X);
    EXPECT_EQ(bindings.button(hmi::GameAction::MoveLeft), hmi::GamepadButton::Left);
}

/**
 * @brief `setKey` sur un bouton déjà lié à une autre action échange les deux boutons.
 * \castest{<b>`setKey` sur un bouton déjà lié à une autre action échange les deux boutons.</b><br/>
 * \tcat Unitaire · Gamepad Bindings<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu `setKey` sur un bouton déjà lié à une autre action échange les deux boutons.
 * }
 */
TEST(GamepadBindingsTest, SetKeyEchangeSurConflit) {
    hmi::GamepadBindings bindings;
    // Lie Sauter sur le bouton par defaut de Dash (epaule droite) : les deux doivent echanger.
    bindings.setKey(hmi::GameAction::Jump, hmi::GamepadButton::RightShoulder);
    EXPECT_EQ(bindings.button(hmi::GameAction::Jump), hmi::GamepadButton::RightShoulder);
    EXPECT_EQ(bindings.button(hmi::GameAction::Dash), hmi::GamepadButton::A);
}

/**
 * @brief `resetToDefaults` restaure les valeurs par défaut après un ou plusieurs remaps.
 * \castest{<b>`resetToDefaults` restaure les valeurs par défaut après un ou plusieurs
 * remaps.</b><br/>
 * \tcat Unitaire · Gamepad Bindings<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu `resetToDefaults` restaure les valeurs par défaut après un ou plusieurs remaps.
 * }
 */
TEST(GamepadBindingsTest, ResetToDefaultsRestaureLesDefauts) {
    hmi::GamepadBindings bindings;
    bindings.setKey(hmi::GameAction::Jump, hmi::GamepadButton::X);
    bindings.setKey(hmi::GameAction::Dash, hmi::GamepadButton::Y);
    bindings.resetToDefaults();
    EXPECT_EQ(bindings.button(hmi::GameAction::Jump), hmi::GamepadButton::A);
    EXPECT_EQ(bindings.button(hmi::GameAction::Dash), hmi::GamepadButton::RightShoulder);
}

/**
 * @brief Sauvegarder puis recharger préserve un jeu de bindings personnalisé (aller-retour JSON).
 * \castest{<b>Sauvegarder puis recharger préserve un jeu de bindings personnalisé (aller-retour
 * JSON).</b><br/>
 * \tcat Unitaire · Gamepad Bindings<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Sauvegarder puis recharger préserve un jeu de bindings personnalisé (aller-retour
 * JSON).
 * }
 */
TEST(GamepadBindingsTest, SaveEtLoadAllerRetour) {
    const std::filesystem::path path =
        std::filesystem::temp_directory_path() / "jadg_test_gamepad_bindings.json";
    std::filesystem::remove(path);

    hmi::GamepadBindings original;
    original.setKey(hmi::GameAction::Jump, hmi::GamepadButton::X);
    original.setKey(hmi::GameAction::Dash, hmi::GamepadButton::Y);
    ASSERT_TRUE(original.save(path));

    const hmi::GamepadBindings reloaded = hmi::GamepadBindings::load(path);
    std::filesystem::remove(path);

    EXPECT_EQ(reloaded.button(hmi::GameAction::Jump), hmi::GamepadButton::X);
    EXPECT_EQ(reloaded.button(hmi::GameAction::Dash), hmi::GamepadButton::Y);
    EXPECT_EQ(reloaded.button(hmi::GameAction::MoveLeft), hmi::GamepadButton::Left);
}

/**
 * @brief Charger depuis un fichier absent renvoie les valeurs par défaut, sans erreur.
 * \castest{<b>Charger depuis un fichier absent renvoie les valeurs par défaut, sans
 * erreur.</b><br/>
 * \tcat Unitaire · Gamepad Bindings<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Charger depuis un fichier absent renvoie les valeurs par défaut, sans erreur.
 * }
 */
TEST(GamepadBindingsTest, LoadFichierAbsentRenvoieLesDefauts) {
    const std::filesystem::path path =
        std::filesystem::temp_directory_path() / "jadg_test_gamepad_bindings_absent.json";
    std::filesystem::remove(path);

    const hmi::GamepadBindings bindings = hmi::GamepadBindings::load(path);
    EXPECT_EQ(bindings.button(hmi::GameAction::Jump), hmi::GamepadButton::A);
}

/**
 * @brief Un nom de bouton inconnu dans le fichier est ignoré : l'action garde sa valeur par
 *        défaut.
 * \castest{<b>Un nom de bouton inconnu est ignoré : l'action garde sa valeur par défaut.</b><br/>
 * \tcat Unitaire · Gamepad Bindings<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un nom de bouton inconnu est ignoré : l'action garde sa valeur par défaut.
 * }
 */
TEST(GamepadBindingsTest, LoadNomBoutonInconnuIgnore) {
    const std::filesystem::path path =
        std::filesystem::temp_directory_path() / "jadg_test_gamepad_bindings_inconnu.json";
    {
        std::ofstream file(path, std::ios::binary);
        file << R"({"manette": {"sauter": "bouton_qui_n_existe_pas"}})";
    }

    const hmi::GamepadBindings bindings = hmi::GamepadBindings::load(path);
    std::filesystem::remove(path);

    EXPECT_EQ(bindings.button(hmi::GameAction::Jump), hmi::GamepadButton::A);
}

/**
 * @brief Un fichier écrit avant ce lot (section « manette » sans « interagir ») se relit sans
 *        erreur, l'action prenant son bouton par défaut.
 * \castest{<b>Un fichier écrit avant ce lot (sans « interagir ») se relit sans erreur, l'action
 * prenant son bouton par défaut.</b><br/>
 * \tcat Unitaire · Gamepad Bindings<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un fichier « manette » antérieur à `LOT-63` (sans « interagir ») se relit sans erreur,
 * Interagir prenant son bouton par défaut (X).
 * }
 */
TEST(GamepadBindingsTest, LoadFichierAnterieurSansInteragirPrendLaValeurParDefaut) {
    const std::filesystem::path path =
        std::filesystem::temp_directory_path() / "jadg_test_gamepad_bindings_anterieur.json";
    {
        std::ofstream file(path, std::ios::binary);
        // Section "manette" telle qu'ecrite avant LOT-63 : aucune entree "interagir".
        file << R"({"manette": {"gauche": "gauche", "droite": "droite", "haut": "haut",
                                 "bas": "bas", "sauter": "a", "dash": "rb"}})";
    }

    const hmi::GamepadBindings bindings = hmi::GamepadBindings::load(path);
    std::filesystem::remove(path);

    EXPECT_EQ(bindings.button(hmi::GameAction::Interact), hmi::GamepadButton::X);
    EXPECT_EQ(bindings.button(hmi::GameAction::Jump), hmi::GamepadButton::A);
}

/**
 * @brief Sauvegarder les bindings manette préserve les sections « jeu »/« editeur » déjà
 *        présentes.
 * \castest{<b>Sauvegarder les bindings manette préserve les sections « jeu »/« editeur »
 * déjà présentes.</b><br/>
 * \tcat Unitaire · Gamepad Bindings<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Sauvegarder les bindings manette préserve les sections « jeu »/« editeur ».
 * }
 */
TEST(GamepadBindingsTest, SavePreserveLesAutresSections) {
    const std::filesystem::path path =
        std::filesystem::temp_directory_path() / "jadg_test_gamepad_bindings_partage.json";
    {
        std::ofstream file(path, std::ios::binary);
        file << R"({"jeu": {"sauter": 32}, "editeur": {"sauvegarder": 83}})";
    }

    hmi::GamepadBindings bindings;
    ASSERT_TRUE(bindings.save(path));

    std::string content;
    {
        std::ifstream reread(path, std::ios::binary);
        content.assign(std::istreambuf_iterator<char>(reread), std::istreambuf_iterator<char>());
    }
    std::filesystem::remove(path);

    EXPECT_NE(content.find("\"jeu\""), std::string::npos);
    EXPECT_NE(content.find("\"editeur\""), std::string::npos);
    EXPECT_NE(content.find("\"manette\""), std::string::npos);
}
