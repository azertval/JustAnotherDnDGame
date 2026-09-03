// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_game_key_bindings.cpp
 * @brief Tests unitaires du remappage des touches de jeu (`GameKeyBindings`, `EX-CTRL-012`).
 */

#include <filesystem>
#include <fstream>

#include <gtest/gtest.h>

#include "HMI/Input/GameKeyBindings.h"

/**
 * @brief Les sept actions ont leurs valeurs par défaut (flèches/Espace/Maj/E) à la construction.
 * \castest{<b>Les sept actions ont leurs valeurs par défaut (flèches/Espace/Maj/E) à la
 * construction.</b><br/>
 * \tcat Unitaire · Game Key Bindings<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Les sept actions ont leurs valeurs par défaut (flèches/Espace/Maj/E) à la
 * construction.
 * }
 */
TEST(GameKeyBindingsTest, ValeursParDefautALaConstruction) {
    const hmi::GameKeyBindings bindings;
    EXPECT_EQ(bindings.key(hmi::GameAction::MoveLeft), hmi::Key::Left);
    EXPECT_EQ(bindings.key(hmi::GameAction::MoveRight), hmi::Key::Right);
    EXPECT_EQ(bindings.key(hmi::GameAction::AimUp), hmi::Key::Up);
    EXPECT_EQ(bindings.key(hmi::GameAction::AimDown), hmi::Key::Down);
    EXPECT_EQ(bindings.key(hmi::GameAction::Jump), hmi::Key::Space);
    EXPECT_EQ(bindings.key(hmi::GameAction::Dash), hmi::Key::Shift);
    EXPECT_EQ(bindings.key(hmi::GameAction::Interact), hmi::Key::E);
}

/**
 * @brief `setKey` sur une touche libre lie l'action sans toucher aux autres.
 * \castest{<b>`setKey` sur une touche libre lie l'action sans toucher aux autres.</b><br/>
 * \tcat Unitaire · Game Key Bindings<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu `setKey` sur une touche libre lie l'action sans toucher aux autres.
 * }
 */
TEST(GameKeyBindingsTest, SetKeySurToucheLibre) {
    hmi::GameKeyBindings bindings;
    bindings.setKey(hmi::GameAction::Jump, hmi::Key::F1);
    EXPECT_EQ(bindings.key(hmi::GameAction::Jump), hmi::Key::F1);
    EXPECT_EQ(bindings.key(hmi::GameAction::MoveLeft), hmi::Key::Left);
}

/**
 * @brief `setKey` sur une touche déjà liée à une autre action échange les deux touches.
 * \castest{<b>`setKey` sur une touche déjà liée à une autre action échange les deux
 * touches.</b><br/>
 * \tcat Unitaire · Game Key Bindings<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu `setKey` sur une touche déjà liée à une autre action échange les deux touches.
 * }
 */
TEST(GameKeyBindingsTest, SetKeyEchangeSurConflit) {
    hmi::GameKeyBindings bindings;
    // Lie Sauter sur la touche par defaut de Gauche : les deux doivent echanger.
    bindings.setKey(hmi::GameAction::Jump, hmi::Key::Left);
    EXPECT_EQ(bindings.key(hmi::GameAction::Jump), hmi::Key::Left);
    EXPECT_EQ(bindings.key(hmi::GameAction::MoveLeft), hmi::Key::Space);
}

/**
 * @brief `resetToDefaults` restaure les valeurs par défaut après un ou plusieurs remaps.
 * \castest{<b>`resetToDefaults` restaure les valeurs par défaut après un ou plusieurs
 * remaps.</b><br/>
 * \tcat Unitaire · Game Key Bindings<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu `resetToDefaults` restaure les valeurs par défaut après un ou plusieurs remaps.
 * }
 */
TEST(GameKeyBindingsTest, ResetToDefaultsRestaureLesDefauts) {
    hmi::GameKeyBindings bindings;
    bindings.setKey(hmi::GameAction::Jump, hmi::Key::F1);
    bindings.setKey(hmi::GameAction::Dash, hmi::Key::F2);
    bindings.resetToDefaults();
    EXPECT_EQ(bindings.key(hmi::GameAction::Jump), hmi::Key::Space);
    EXPECT_EQ(bindings.key(hmi::GameAction::Dash), hmi::Key::Shift);
}

/**
 * @brief Sauvegarder puis recharger préserve un jeu de bindings personnalisé (aller-retour JSON).
 * \castest{<b>Sauvegarder puis recharger préserve un jeu de bindings personnalisé (aller-retour
 * JSON).</b><br/>
 * \tcat Unitaire · Game Key Bindings<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Sauvegarder puis recharger préserve un jeu de bindings personnalisé (aller-retour
 * JSON).
 * }
 */
TEST(GameKeyBindingsTest, SaveEtLoadAllerRetour) {
    const std::filesystem::path path =
        std::filesystem::temp_directory_path() / "jadg_test_game_bindings.json";
    std::filesystem::remove(path);

    hmi::GameKeyBindings original;
    original.setKey(hmi::GameAction::Jump, hmi::Key::F1);
    original.setKey(hmi::GameAction::Dash, hmi::Key::F2);
    ASSERT_TRUE(original.save(path));

    const hmi::GameKeyBindings reloaded = hmi::GameKeyBindings::load(path);
    std::filesystem::remove(path);

    EXPECT_EQ(reloaded.key(hmi::GameAction::Jump), hmi::Key::F1);
    EXPECT_EQ(reloaded.key(hmi::GameAction::Dash), hmi::Key::F2);
    EXPECT_EQ(reloaded.key(hmi::GameAction::MoveLeft), hmi::Key::Left);
}

/**
 * @brief Charger depuis un fichier absent renvoie les valeurs par défaut, sans erreur.
 * \castest{<b>Charger depuis un fichier absent renvoie les valeurs par défaut, sans
 * erreur.</b><br/>
 * \tcat Unitaire · Game Key Bindings<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Charger depuis un fichier absent renvoie les valeurs par défaut, sans erreur.
 * }
 */
TEST(GameKeyBindingsTest, LoadFichierAbsentRenvoieLesDefauts) {
    const std::filesystem::path path =
        std::filesystem::temp_directory_path() / "jadg_test_game_bindings_absent.json";
    std::filesystem::remove(path);

    const hmi::GameKeyBindings bindings = hmi::GameKeyBindings::load(path);
    EXPECT_EQ(bindings.key(hmi::GameAction::MoveLeft), hmi::Key::Left);
    EXPECT_EQ(bindings.key(hmi::GameAction::Jump), hmi::Key::Space);
}

/**
 * @brief Charger un fichier JSON corrompu renvoie les valeurs par défaut, sans exception.
 * \castest{<b>Charger un fichier JSON corrompu renvoie les valeurs par défaut, sans
 * exception.</b><br/>
 * \tcat Unitaire · Game Key Bindings<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Charger un fichier JSON corrompu renvoie les valeurs par défaut, sans exception.
 * }
 */
TEST(GameKeyBindingsTest, LoadJsonCorrompuRenvoieLesDefauts) {
    const std::filesystem::path path =
        std::filesystem::temp_directory_path() / "jadg_test_game_bindings_corrompu.json";
    {
        std::ofstream corrupt(path, std::ios::binary);
        corrupt << "{ceci n'est pas du json valide";
    }

    const hmi::GameKeyBindings bindings = hmi::GameKeyBindings::load(path);
    std::filesystem::remove(path);

    EXPECT_EQ(bindings.key(hmi::GameAction::MoveLeft), hmi::Key::Left);
}

/**
 * @brief Une valeur hors bornes dans le fichier est ignorée : l'action garde sa valeur par défaut.
 * \castest{<b>Une valeur hors bornes dans le fichier est ignorée : l'action garde sa valeur par
 * défaut.</b><br/>
 * \tcat Unitaire · Game Key Bindings<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Une valeur hors bornes dans le fichier est ignorée : l'action garde sa valeur par
 * défaut.
 * }
 */
TEST(GameKeyBindingsTest, LoadValeurHorsBornesIgnoree) {
    const std::filesystem::path path = std::filesystem::temp_directory_path() /
                                       "jadg_test_game_bindings_hors_bornes.json";
    {
        std::ofstream file(path, std::ios::binary);
        file << R"({"jeu": {"sauter": 99999}})";
    }

    const hmi::GameKeyBindings bindings = hmi::GameKeyBindings::load(path);
    std::filesystem::remove(path);

    EXPECT_EQ(bindings.key(hmi::GameAction::Jump), hmi::Key::Space);
}

/**
 * @brief Un fichier écrit avant ce lot (section « jeu » sans « interagir ») se relit sans erreur,
 *        l'action prenant sa valeur par défaut.
 * \castest{<b>Un fichier écrit avant ce lot (sans « interagir ») se relit sans erreur, l'action
 * prenant sa valeur par défaut.</b><br/>
 * \tcat Unitaire · Game Key Bindings<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un fichier « jeu » antérieur à `LOT-63` (sans « interagir ») se relit sans erreur,
 * Interagir prenant sa touche par défaut (E), et les autres actions restent inchangées.
 * }
 */
TEST(GameKeyBindingsTest, LoadFichierAnterieurSansInteragirPrendLaValeurParDefaut) {
    const std::filesystem::path path =
        std::filesystem::temp_directory_path() / "jadg_test_game_bindings_anterieur.json";
    {
        std::ofstream file(path, std::ios::binary);
        // Section "jeu" telle qu'ecrite avant LOT-63 : aucune entree "interagir".
        file << R"({"jeu": {"gauche": 37, "droite": 39, "haut": 38, "bas": 40, "sauter": 32,
                             "dash": 16}})";
    }

    const hmi::GameKeyBindings bindings = hmi::GameKeyBindings::load(path);
    std::filesystem::remove(path);

    EXPECT_EQ(bindings.key(hmi::GameAction::Interact), hmi::Key::E);
    EXPECT_EQ(bindings.key(hmi::GameAction::Jump), hmi::Key::Space);
}

/**
 * @brief Sauvegarder les touches de jeu préserve une section « editeur » déjà présente.
 * \castest{<b>Sauvegarder les touches de jeu préserve une section « editeur » déjà
 * présente.</b><br/>
 * \tcat Unitaire · Game Key Bindings<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Sauvegarder les touches de jeu préserve une section « editeur » déjà présente.
 * }
 */
TEST(GameKeyBindingsTest, SavePreserveLaSectionEditeur) {
    const std::filesystem::path path =
        std::filesystem::temp_directory_path() / "jadg_test_game_bindings_partage.json";
    {
        std::ofstream file(path, std::ios::binary);
        file << R"({"editeur": {"sauvegarder": 83}})";
    }

    hmi::GameKeyBindings bindings;
    ASSERT_TRUE(bindings.save(path));

    std::string content;
    {
        std::ifstream reread(path, std::ios::binary);
        content.assign(std::istreambuf_iterator<char>(reread), std::istreambuf_iterator<char>());
    }
    std::filesystem::remove(path);

    EXPECT_NE(content.find("editeur"), std::string::npos);
    EXPECT_NE(content.find("sauvegarder"), std::string::npos);
}
