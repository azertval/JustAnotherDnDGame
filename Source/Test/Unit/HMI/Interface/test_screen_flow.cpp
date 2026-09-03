// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_screen_flow.cpp
 * @brief Tests unitaires de la machine à états des écrans (LOT-59 TACHE-01, EX-GP-041). Logique
 *        pure, sans Qt.
 */

#include <gtest/gtest.h>

#include "HMI/Interface/ScreenFlow.h"

namespace {

using hmi::dressingFor;
using hmi::resolveTransition;
using hmi::ScreenDressing;
using hmi::ScreenEvent;
using hmi::ScreenId;
using hmi::ScreenState;

}  // namespace

/**
 * @brief Chaque transition autorisée mène à l'écran attendu.
 * \castest{<b>Chaque transition autorisée mène à l'écran attendu.</b><br/>
 * \tcat Unitaire · Machine à états des écrans<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Résoudre chacune des transitions autorisées listées.<br/>2. Vérifier l'écran
 * résultant.<br/>
 * \tattendu Chaque transition mène à l'écran attendu.
 * }
 */
TEST(ScreenFlowTest, TransitionsAutoriseesMenentALEcranAttendu) {
    const ScreenState menu{.screen = ScreenId::Menu, .optionsReturnTo = ScreenId::Menu};
    const ScreenState editor{.screen = ScreenId::Editor, .optionsReturnTo = ScreenId::Menu};
    const ScreenState game{.screen = ScreenId::Game, .optionsReturnTo = ScreenId::Menu};
    const ScreenState pause{.screen = ScreenId::Pause, .optionsReturnTo = ScreenId::Menu};
    const ScreenState niveauTermine{.screen = ScreenId::LevelComplete,
                                    .optionsReturnTo = ScreenId::Menu};
    const ScreenState levelSelect{.screen = ScreenId::LevelSelect,
                                  .optionsReturnTo = ScreenId::Menu};
    const ScreenState credits{.screen = ScreenId::Credits, .optionsReturnTo = ScreenId::Menu};
    const ScreenState optionsFromMenu{.screen = ScreenId::Options,
                                      .optionsReturnTo = ScreenId::Menu};
    const ScreenState optionsFromPause{.screen = ScreenId::Options,
                                       .optionsReturnTo = ScreenId::Pause};

    EXPECT_EQ(resolveTransition(menu, ScreenEvent::OpenEditor)->screen, ScreenId::Editor);
    EXPECT_EQ(resolveTransition(menu, ScreenEvent::OpenGame)->screen, ScreenId::Game);
    EXPECT_EQ(resolveTransition(menu, ScreenEvent::OpenOptions)->screen, ScreenId::Options);
    EXPECT_EQ(resolveTransition(editor, ScreenEvent::OpenMenu)->screen, ScreenId::Menu);
    EXPECT_EQ(resolveTransition(game, ScreenEvent::OpenMenu)->screen, ScreenId::Menu);
    EXPECT_EQ(resolveTransition(game, ScreenEvent::OpenPause)->screen, ScreenId::Pause);
    EXPECT_EQ(resolveTransition(game, ScreenEvent::LevelSucceeded)->screen,
              ScreenId::LevelComplete);
    EXPECT_EQ(resolveTransition(pause, ScreenEvent::ResumePause)->screen, ScreenId::Game);
    EXPECT_EQ(resolveTransition(pause, ScreenEvent::RestartFromPause)->screen, ScreenId::Game);
    EXPECT_EQ(resolveTransition(pause, ScreenEvent::QuitPauseToMenu)->screen, ScreenId::Menu);
    EXPECT_EQ(resolveTransition(pause, ScreenEvent::OpenOptions)->screen, ScreenId::Options);
    EXPECT_EQ(resolveTransition(optionsFromMenu, ScreenEvent::CloseOptions)->screen,
              ScreenId::Menu);
    EXPECT_EQ(resolveTransition(optionsFromPause, ScreenEvent::CloseOptions)->screen,
              ScreenId::Pause);
    EXPECT_EQ(resolveTransition(niveauTermine, ScreenEvent::ContinueAfterLevel)->screen,
              ScreenId::Game);
    EXPECT_EQ(resolveTransition(niveauTermine, ScreenEvent::ReplayLevel)->screen, ScreenId::Game);
    EXPECT_EQ(resolveTransition(niveauTermine, ScreenEvent::ReturnToMenuFromLevelComplete)->screen,
              ScreenId::Menu);
    EXPECT_EQ(resolveTransition(menu, ScreenEvent::OpenLevelSelect)->screen, ScreenId::LevelSelect);
    EXPECT_EQ(resolveTransition(levelSelect, ScreenEvent::LevelChosen)->screen, ScreenId::Game);
    EXPECT_EQ(resolveTransition(levelSelect, ScreenEvent::CloseLevelSelect)->screen,
              ScreenId::Menu);
    EXPECT_EQ(resolveTransition(menu, ScreenEvent::OpenCredits)->screen, ScreenId::Credits);
    EXPECT_EQ(resolveTransition(credits, ScreenEvent::CloseCredits)->screen, ScreenId::Menu);
    // Aperçu en direct / onglet Rejeu (LOT-ANNEXE-21) : ramène au Menu une fois la lecture
    // terminée (même convention que l'ancien "Regarder l'IA jouer", LOT-ANNEXE-18).
}

/**
 * @brief Options revient vers l'écran d'où il a été ouvert : Menu si ouvert depuis Menu, Pause si
 *        ouvert depuis Pause -- porté par l'état, pas par une variable à part.
 * \castest{<b>Options revient vers son écran d'origine (Menu ou Pause).</b><br/>
 * \tcat Unitaire · Machine à états des écrans<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Ouvrir Options depuis Menu, puis le fermer : vérifier le retour au Menu.<br/>2.
 * Ouvrir Options depuis Pause, puis le fermer : vérifier le retour à Pause.<br/>
 * \tattendu Chaque fermeture revient à l'écran d'origine respectif.
 * }
 */
TEST(ScreenFlowTest, OptionsRevientVersSonEcranDOrigine) {
    const ScreenState menu{.screen = ScreenId::Menu, .optionsReturnTo = ScreenId::Menu};
    const ScreenState pause{.screen = ScreenId::Pause, .optionsReturnTo = ScreenId::Menu};

    const std::optional<ScreenState> openedFromMenu =
        resolveTransition(menu, ScreenEvent::OpenOptions);
    ASSERT_TRUE(openedFromMenu.has_value());
    EXPECT_EQ(resolveTransition(*openedFromMenu, ScreenEvent::CloseOptions)->screen,
              ScreenId::Menu);

    const std::optional<ScreenState> openedFromPause =
        resolveTransition(pause, ScreenEvent::OpenOptions);
    ASSERT_TRUE(openedFromPause.has_value());
    EXPECT_EQ(resolveTransition(*openedFromPause, ScreenEvent::CloseOptions)->screen,
              ScreenId::Pause);
}

/**
 * @brief Une transition interdite est refusée (std::nullopt), notamment Editor -> Pause et
 *        Menu -> LevelComplete (exemples cités par TACHE-01).
 * \castest{<b>Une transition interdite est refusée.</b><br/>
 * \tcat Unitaire · Machine à états des écrans<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Tenter Editor -> Pause (via OpenPause).<br/>2. Tenter Menu -> LevelComplete (via
 * LevelSucceeded).<br/>3. Tenter Menu -> Pause directement.<br/>
 * \tattendu Chaque tentative renvoie std::nullopt.
 * }
 */
TEST(ScreenFlowTest, TransitionInterditeEstRefusee) {
    const ScreenState editor{.screen = ScreenId::Editor, .optionsReturnTo = ScreenId::Menu};
    const ScreenState menu{.screen = ScreenId::Menu, .optionsReturnTo = ScreenId::Menu};
    const ScreenState game{.screen = ScreenId::Game, .optionsReturnTo = ScreenId::Menu};

    EXPECT_EQ(resolveTransition(editor, ScreenEvent::OpenPause), std::nullopt);
    EXPECT_EQ(resolveTransition(menu, ScreenEvent::LevelSucceeded), std::nullopt);
    EXPECT_EQ(resolveTransition(menu, ScreenEvent::OpenPause), std::nullopt);
    EXPECT_EQ(resolveTransition(menu, ScreenEvent::ResumePause), std::nullopt);
    EXPECT_EQ(resolveTransition(editor, ScreenEvent::OpenOptions), std::nullopt);
    // Sélection de niveau (LOT-59 TACHE-06) : atteignable seulement depuis le menu, jamais en jeu
    // ni en édition.
    EXPECT_EQ(resolveTransition(editor, ScreenEvent::OpenLevelSelect), std::nullopt);
    EXPECT_EQ(resolveTransition(game, ScreenEvent::OpenLevelSelect), std::nullopt);
    // Crédits (LOT-60) : même règle, atteignable seulement depuis le menu.
    EXPECT_EQ(resolveTransition(editor, ScreenEvent::OpenCredits), std::nullopt);
    EXPECT_EQ(resolveTransition(game, ScreenEvent::OpenCredits), std::nullopt);
    // Mode IA (LOT-ANNEXE-21) : même règle, atteignable seulement depuis le menu.
}

/**
 * @brief L'habillage de fenêtre attendu pour chacun des sept écrans correspond à ce que
 *        `MainWindow` appliquait à la main avant l'extraction de la table (docks, barres,
 *        commandes d'édition, navigation manette).
 * \castest{<b>L'habillage de fenêtre est celui attendu pour chaque écran.</b><br/>
 * \tcat Unitaire · Machine à états des écrans<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Interroger l'habillage de chacun des sept écrans.<br/>2. Vérifier chaque champ.<br/>
 * \tattendu L'habillage correspond au comportement historique de chaque `showXxx()`.
 * }
 */
TEST(ScreenFlowTest, HabillageDeFenetreEstCeluiAttenduParEcran) {
    const ScreenDressing menu = dressingFor(ScreenId::Menu);
    EXPECT_FALSE(menu.docksVisible);
    EXPECT_FALSE(menu.menuBarVisible);
    EXPECT_FALSE(menu.toolBarVisible);
    EXPECT_FALSE(menu.pixelToolBarVisible);
    EXPECT_FALSE(menu.editingCommandsEnabled);
    EXPECT_TRUE(menu.gamepadNavigationActive);
    EXPECT_FALSE(menu.overlayVisible);

    const ScreenDressing editor = dressingFor(ScreenId::Editor);
    EXPECT_TRUE(editor.docksVisible);
    EXPECT_TRUE(editor.menuBarVisible);
    EXPECT_TRUE(editor.toolBarVisible);
    EXPECT_TRUE(editor.pixelToolBarVisible);
    EXPECT_TRUE(editor.editingCommandsEnabled);
    EXPECT_FALSE(editor.gamepadNavigationActive);
    EXPECT_FALSE(editor.overlayVisible);

    const ScreenDressing game = dressingFor(ScreenId::Game);
    EXPECT_FALSE(game.docksVisible);
    EXPECT_FALSE(game.menuBarVisible);
    EXPECT_FALSE(game.toolBarVisible);
    EXPECT_FALSE(game.pixelToolBarVisible);
    EXPECT_FALSE(game.editingCommandsEnabled);
    EXPECT_FALSE(game.gamepadNavigationActive);
    EXPECT_FALSE(game.overlayVisible);

    const ScreenDressing options = dressingFor(ScreenId::Options);
    EXPECT_FALSE(options.docksVisible);
    EXPECT_FALSE(options.menuBarVisible);
    EXPECT_FALSE(options.toolBarVisible);
    EXPECT_FALSE(options.pixelToolBarVisible);
    EXPECT_FALSE(options.editingCommandsEnabled);
    EXPECT_TRUE(options.gamepadNavigationActive);
    EXPECT_FALSE(options.overlayVisible);

    const ScreenDressing levelSelect = dressingFor(ScreenId::LevelSelect);
    EXPECT_FALSE(levelSelect.docksVisible);
    EXPECT_FALSE(levelSelect.menuBarVisible);
    EXPECT_FALSE(levelSelect.toolBarVisible);
    EXPECT_FALSE(levelSelect.pixelToolBarVisible);
    EXPECT_FALSE(levelSelect.editingCommandsEnabled);
    EXPECT_TRUE(levelSelect.gamepadNavigationActive);
    EXPECT_FALSE(levelSelect.overlayVisible);

    const ScreenDressing credits = dressingFor(ScreenId::Credits);
    EXPECT_FALSE(credits.docksVisible);
    EXPECT_FALSE(credits.menuBarVisible);
    EXPECT_FALSE(credits.toolBarVisible);
    EXPECT_FALSE(credits.pixelToolBarVisible);
    EXPECT_FALSE(credits.editingCommandsEnabled);
    EXPECT_TRUE(credits.gamepadNavigationActive);
    EXPECT_FALSE(credits.overlayVisible);

    for (const ScreenId overlayScreen : {ScreenId::Pause, ScreenId::LevelComplete}) {
        const ScreenDressing overlay = dressingFor(overlayScreen);
        EXPECT_FALSE(overlay.docksVisible) << static_cast<int>(overlayScreen);
        EXPECT_FALSE(overlay.menuBarVisible) << static_cast<int>(overlayScreen);
        EXPECT_FALSE(overlay.toolBarVisible) << static_cast<int>(overlayScreen);
        EXPECT_FALSE(overlay.pixelToolBarVisible) << static_cast<int>(overlayScreen);
        EXPECT_FALSE(overlay.editingCommandsEnabled) << static_cast<int>(overlayScreen);
        EXPECT_TRUE(overlay.gamepadNavigationActive) << static_cast<int>(overlayScreen);
        EXPECT_TRUE(overlay.overlayVisible) << static_cast<int>(overlayScreen);
    }
}

/**
 * @brief Rejouer ou continuer depuis l'écran de fin de niveau ramène toujours à Game -- jamais
 *        directement à un autre écran (le contenu réel du niveau suivant est décidé ailleurs,
 *        hors de la responsabilité de la machine à états).
 * \castest{<b>Continuer/Rejouer depuis LevelComplete ramène à Game.</b><br/>
 * \tcat Unitaire · Machine à états des écrans<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Résoudre ContinueAfterLevel depuis LevelComplete.<br/>2. Résoudre ReplayLevel depuis
 * LevelComplete.<br/>
 * \tattendu Les deux transitions mènent à Game.
 * }
 */
TEST(ScreenFlowTest, ContinuerOuRejouerDepuisLevelCompleteRameneAGame) {
    const ScreenState niveauTermine{.screen = ScreenId::LevelComplete,
                                    .optionsReturnTo = ScreenId::Menu};
    EXPECT_EQ(resolveTransition(niveauTermine, ScreenEvent::ContinueAfterLevel)->screen,
              ScreenId::Game);
    EXPECT_EQ(resolveTransition(niveauTermine, ScreenEvent::ReplayLevel)->screen, ScreenId::Game);
}
