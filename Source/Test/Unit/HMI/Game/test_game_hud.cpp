// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_game_hud.cpp
 * @brief Tests unitaires du choix de contenu de l'affichage tête haute (LOT-52 TACHE-03,
 *        EX-IHM-003). Fonction pure, sans GPU.
 */

#include <filesystem>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "HMI/Game/GameHud.h"
#include "HMI/Localization/Localization.h"

namespace {

hmi::Localization testLocalization() {
    hmi::Localization localization;
    localization.setDefaultCatalog("fr", {{"hud.interact_prompt", "Interagir pour ramasser"}});
    return localization;
}

}  // namespace

/**
 * @brief Hors contexte particulier, le HUD n'affiche que le nom du tableau.
 *
 * Les compteurs de sauts et de dashs ont disparu avec le personnage de plateforme (`LOT-06`) :
 * ils comptaient des ressources que le déplacement en vue de dessus n'a plus.
 * \castest{<b>Le HUD n'affiche que le nom du tableau hors contexte particulier.</b><br/>
 * \tcat Unitaire · HUD de jeu<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Composer le HUD d'un tableau, sans contact avec une cle.<br/>
 * \tattendu Une seule ligne : le nom du tableau.
 * }
 */
TEST(GameHudTest, HorsContexteSeulLeNomDuTableauEstAffiche) {
    const std::vector<std::string> lines = hmi::gameHudLines("Promenade", testLocalization());

    ASSERT_EQ(lines.size(), 1u);
    EXPECT_EQ(lines[0], "Promenade");
}

/**
 * @brief Chaque clé de traduction utilisée par le HUD existe, traduite, dans les deux catalogues
 *        livrés (français et anglais).
 * \castest{<b>Les cles de traduction du HUD existent dans les deux catalogues livres.</b><br/>
 * \tcat Unitaire · HUD de jeu<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Charger fr.lang puis en.lang depuis les catalogues livres.<br/>2. Resoudre les cles
 * du HUD.<br/>
 * \tattendu La cle resout vers un texte traduit, distinct de la cle elle-meme.
 * }
 */
TEST(GameHudTest, ClesDeTraductionExistentDansLesDeuxCatalogues) {
    const std::filesystem::path directory(JADG_LOCALIZATION_DIR);
    for (const std::string& language : {"fr", "en"}) {
        hmi::Localization localization(directory);
        ASSERT_TRUE(localization.loadDefaultLanguage(language)) << language;
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
 * \castest{<b>L'invite Interagir n'apparait qu'au contact d'une cle non ramassee, en premiere
 * ligne du HUD.</b><br/>
 * \tcat Unitaire · HUD de jeu<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Composer le HUD hors contact d'une cle.<br/>2. Le composer au contact d'une cle non
 * ramassee.<br/>
 * \tattendu Aucune invite dans le premier cas ; l'invite en premiere ligne dans le second, le nom
 * du tableau restant en derniere ligne.
 * }
 */
TEST(GameHudTest, InviteInteragirSeulementAuContactDUneCle) {
    const hmi::Localization localization = testLocalization();

    const std::vector<std::string> sansCle =
        hmi::gameHudLines("Cle", localization, /*overlappingKey=*/false);
    ASSERT_EQ(sansCle.size(), 1u);
    EXPECT_EQ(sansCle[0], "Cle");

    const std::vector<std::string> surUneCle =
        hmi::gameHudLines("Cle", localization, /*overlappingKey=*/true);
    ASSERT_EQ(surUneCle.size(), 2u);
    EXPECT_EQ(surUneCle[0], "Interagir pour ramasser");
    EXPECT_EQ(surUneCle[1], "Cle");
}
