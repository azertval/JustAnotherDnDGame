// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_game_hud.cpp
 * @brief Tests unitaires du choix de contenu de l'affichage tête haute (LOT-52 TACHE-03,
 *        EX-IHM-003). Fonction pure, sans GPU.
 */

#include <filesystem>

#include <gtest/gtest.h>

#include "Core/Ecs/Components/Player.h"
#include "HMI/Game/GameHud.h"
#include "HMI/Localization/Localization.h"

namespace {

hmi::Localization testLocalization() {
    hmi::Localization localization;
    localization.setDefaultCatalog("fr", {{"hud.jumps_remaining", "Sauts : %1"},
                                          {"hud.dashes_remaining", "Dashs : %1"},
                                          {"hud.interact_prompt", "Interagir pour ramasser"}});
    return localization;
}

}  // namespace

/**
 * @brief Un niveau avec des budgets finis affiche les deux compteurs, puis le nom du tableau.
 * \castest{<b>Un niveau avec des budgets finis affiche les deux compteurs et le nom du
 * tableau.</b><br/>
 * \tcat Unitaire · HUD de jeu<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Les lignes « Sauts : 3 », « Dashs : 1 » et le nom du tableau sont produites, dans cet
 * ordre.
 * }
 */
TEST(GameHudTest, BudgetsFinisAffichentLesDeuxCompteurs) {
    core::Player player;
    player.jumpsRemaining = 3;
    player.dashesRemaining = 1;

    const std::vector<std::string> lines =
        hmi::gameHudLines(player, "Salle des epreuves", testLocalization());

    ASSERT_EQ(lines.size(), 3u);
    EXPECT_EQ(lines[0], "Sauts : 3");
    EXPECT_EQ(lines[1], "Dashs : 1");
    EXPECT_EQ(lines[2], "Salle des epreuves");
}

/**
 * @brief Un niveau sans budget (illimité aux deux mouvements) n'affiche aucun compteur, seulement
 *        le nom du tableau.
 * \castest{<b>Un niveau sans budget n'affiche aucun compteur.</b><br/>
 * \tcat Unitaire · HUD de jeu<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Seul le nom du tableau est produit.
 * }
 */
TEST(GameHudTest, NiveauSansBudgetNAfficheAucunCompteur) {
    core::Player player;  // jumpsRemaining/dashesRemaining par defaut : -1 (illimite)

    const std::vector<std::string> lines =
        hmi::gameHudLines(player, "Promenade", testLocalization());

    ASSERT_EQ(lines.size(), 1u);
    EXPECT_EQ(lines[0], "Promenade");
}

/**
 * @brief Un budget partiellement défini (sauts limités, dashs illimités) n'affiche qu'un seul
 *        compteur.
 * \castest{<b>Un budget partiellement défini n'affiche qu'un seul compteur.</b><br/>
 * \tcat Unitaire · HUD de jeu<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Seul le compteur de sauts est produit, avant le nom du tableau.
 * }
 */
TEST(GameHudTest, BudgetPartielNAfficheQuUnCompteur) {
    core::Player player;
    player.jumpsRemaining = 2;
    player.dashesRemaining = -1;

    const std::vector<std::string> lines =
        hmi::gameHudLines(player, "Chute libre", testLocalization());

    ASSERT_EQ(lines.size(), 2u);
    EXPECT_EQ(lines[0], "Sauts : 2");
    EXPECT_EQ(lines[1], "Chute libre");
}

/**
 * @brief Chaque clé de traduction utilisée par le HUD existe, traduite, dans les deux catalogues
 *        livrés (français et anglais).
 * \castest{<b>Les clés de traduction du HUD existent dans les deux catalogues livrés.</b><br/>
 * \tcat Unitaire · HUD de jeu<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Charger fr.lang puis en.lang depuis les catalogues livrés.<br/>2. Résoudre les clés
 * du HUD.<br/>
 * \tattendu Les deux clés résolvent vers un texte traduit, distinct de la clé elle-même.
 * }
 */
TEST(GameHudTest, ClesDeTraductionExistentDansLesDeuxCatalogues) {
    const std::filesystem::path directory(JADG_LOCALIZATION_DIR);
    for (const std::string& language : {"fr", "en"}) {
        hmi::Localization localization(directory);
        ASSERT_TRUE(localization.loadDefaultLanguage(language)) << language;
        EXPECT_NE(localization.text("hud.jumps_remaining"), "hud.jumps_remaining") << language;
        EXPECT_NE(localization.text("hud.dashes_remaining"), "hud.dashes_remaining") << language;
        EXPECT_NE(localization.text("hud.interact_prompt"), "hud.interact_prompt") << language;
    }
}

/**
 * @brief Invite « Interagir » (`LOT-65` TACHE-07) : affichée **seulement** au contact d'une clé non
 * ramassée, et en première ligne.
 *
 * Ramasser une clé exige le contact **et** l'action « Interagir » (`EX-GP-023`, `EX-CTRL-022`), la
 * seule entrée du jeu qu'aucun autre tableau ne demande. Sans invite, un joueur qui l'ignore reste
 * bloqué devant la porte verrouillée sans aucun retour. L'invite reste **contextuelle** : un
 * tableau sans clé, ou une clé déjà ramassée, n'affiche rien — le tutoriel demeure « sans texte »
 * partout ailleurs.
 * \castest{<b>L'invite « Interagir » n'apparaît qu'au contact d'une clé non ramassée, en première
 * ligne du HUD.</b><br/>
 * \tcat Unitaire · HUD de jeu<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Composer le HUD hors contact d'une clé.<br/>2. Le composer au contact d'une clé non
 * ramassée.<br/>
 * \tattendu Aucune invite dans le premier cas ; l'invite en première ligne dans le second, sans
 * modifier les autres lignes.
 * }
 */
TEST(GameHudTest, InviteInteragirSeulementAuContactDUneCle) {
    core::Player player;  // budgets illimites : seule la ligne du nom subsiste
    const hmi::Localization localization = testLocalization();

    const std::vector<std::string> sansCle =
        hmi::gameHudLines(player, "Cle", localization, /*overlappingKey=*/false);
    ASSERT_EQ(sansCle.size(), 1u);
    EXPECT_EQ(sansCle[0], "Cle");

    const std::vector<std::string> surUneCle =
        hmi::gameHudLines(player, "Cle", localization, /*overlappingKey=*/true);
    ASSERT_EQ(surUneCle.size(), 2u);
    EXPECT_EQ(surUneCle[0], "Interagir pour ramasser");
    EXPECT_EQ(surUneCle[1], "Cle");

    // L'invite s'ajoute AVANT les budgets, sans en modifier l'ordre ni le contenu.
    player.jumpsRemaining = 2;
    const std::vector<std::string> avecBudget =
        hmi::gameHudLines(player, "Cle", localization, /*overlappingKey=*/true);
    ASSERT_EQ(avecBudget.size(), 3u);
    EXPECT_EQ(avecBudget[0], "Interagir pour ramasser");
    EXPECT_EQ(avecBudget[1], "Sauts : 2");
    EXPECT_EQ(avecBudget[2], "Cle");
}
