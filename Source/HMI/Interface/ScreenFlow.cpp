// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Interface/ScreenFlow.h"

namespace hmi {

ScreenDressing dressingFor(ScreenId screen) noexcept {
    switch (screen) {
        case ScreenId::Menu:
            return ScreenDressing{.docksVisible = false,
                                  .menuBarVisible = false,
                                  .toolBarVisible = false,
                                  .pixelToolBarVisible = false,
                                  .editingCommandsEnabled = false,
                                  .gamepadNavigationActive = true,
                                  .overlayVisible = false};
        case ScreenId::Editor:
            return ScreenDressing{.docksVisible = true,
                                  .menuBarVisible = true,
                                  .toolBarVisible = true,
                                  .pixelToolBarVisible = true,
                                  .editingCommandsEnabled = true,
                                  .gamepadNavigationActive = false,
                                  .overlayVisible = false};
        case ScreenId::Game:
            return ScreenDressing{.docksVisible = false,
                                  .menuBarVisible = false,
                                  .toolBarVisible = false,
                                  .pixelToolBarVisible = false,
                                  .editingCommandsEnabled = false,
                                  .gamepadNavigationActive = false,
                                  .overlayVisible = false};
        case ScreenId::Options:
        case ScreenId::LevelSelect:
        case ScreenId::Credits:
            // Même habillage que Menu : page du QStackedWidget, jamais un recouvrement (LevelSelect
            // est atteint depuis le menu, pas en jeu -- LOT-59 TACHE-06 ; Credits de même, LOT-60).
            return ScreenDressing{.docksVisible = false,
                                  .menuBarVisible = false,
                                  .toolBarVisible = false,
                                  .pixelToolBarVisible = false,
                                  .editingCommandsEnabled = false,
                                  .gamepadNavigationActive = true,
                                  .overlayVisible = false};
        case ScreenId::Pause:
        case ScreenId::LevelComplete:
            // Recouvrement par-dessus Game (EX-GP-041, TACHE-02) : même habillage que Game (la
            // scène reste dessinée derrière), sauf navigation manette (menu de recouvrement,
            // pas de personnage à piloter) et overlayVisible.
            return ScreenDressing{.docksVisible = false,
                                  .menuBarVisible = false,
                                  .toolBarVisible = false,
                                  .pixelToolBarVisible = false,
                                  .editingCommandsEnabled = false,
                                  .gamepadNavigationActive = true,
                                  .overlayVisible = true};
    }
    return ScreenDressing{};
}

std::optional<ScreenState> resolveTransition(const ScreenState& current,
                                             ScreenEvent event) noexcept {
    switch (current.screen) {
        case ScreenId::Menu:
            switch (event) {
                case ScreenEvent::OpenMenu:
                    return ScreenState{.screen = ScreenId::Menu, .optionsReturnTo = ScreenId::Menu};
                case ScreenEvent::OpenEditor:
                    return ScreenState{.screen = ScreenId::Editor,
                                       .optionsReturnTo = ScreenId::Menu};
                case ScreenEvent::OpenGame:
                    return ScreenState{.screen = ScreenId::Game, .optionsReturnTo = ScreenId::Menu};
                case ScreenEvent::OpenOptions:
                    return ScreenState{.screen = ScreenId::Options,
                                       .optionsReturnTo = ScreenId::Menu};
                case ScreenEvent::OpenLevelSelect:
                    return ScreenState{.screen = ScreenId::LevelSelect,
                                       .optionsReturnTo = ScreenId::Menu};
                case ScreenEvent::OpenCredits:
                    return ScreenState{.screen = ScreenId::Credits,
                                       .optionsReturnTo = ScreenId::Menu};
                default:
                    return std::nullopt;
            }
        case ScreenId::Credits:
            switch (event) {
                case ScreenEvent::CloseCredits:
                    return ScreenState{.screen = ScreenId::Menu, .optionsReturnTo = ScreenId::Menu};
                default:
                    return std::nullopt;
            }
        case ScreenId::LevelSelect:
            switch (event) {
                case ScreenEvent::CloseLevelSelect:
                    return ScreenState{.screen = ScreenId::Menu, .optionsReturnTo = ScreenId::Menu};
                case ScreenEvent::LevelChosen:
                    return ScreenState{.screen = ScreenId::Game, .optionsReturnTo = ScreenId::Menu};
                default:
                    return std::nullopt;
            }
        case ScreenId::Editor:
            switch (event) {
                case ScreenEvent::OpenMenu:
                    return ScreenState{.screen = ScreenId::Menu, .optionsReturnTo = ScreenId::Menu};
                default:
                    return std::nullopt;
            }
        case ScreenId::Game:
            switch (event) {
                case ScreenEvent::OpenMenu:
                    return ScreenState{.screen = ScreenId::Menu, .optionsReturnTo = ScreenId::Menu};
                case ScreenEvent::OpenPause:
                    return ScreenState{.screen = ScreenId::Pause,
                                       .optionsReturnTo = ScreenId::Menu};
                case ScreenEvent::LevelSucceeded:
                    return ScreenState{.screen = ScreenId::LevelComplete,
                                       .optionsReturnTo = ScreenId::Menu};
                default:
                    return std::nullopt;
            }
        case ScreenId::Options:
            switch (event) {
                case ScreenEvent::CloseOptions:
                    return ScreenState{.screen = current.optionsReturnTo,
                                       .optionsReturnTo = ScreenId::Menu};
                default:
                    return std::nullopt;
            }
        case ScreenId::Pause:
            switch (event) {
                case ScreenEvent::ResumePause:
                case ScreenEvent::RestartFromPause:
                    return ScreenState{.screen = ScreenId::Game, .optionsReturnTo = ScreenId::Menu};
                case ScreenEvent::QuitPauseToMenu:
                    return ScreenState{.screen = ScreenId::Menu, .optionsReturnTo = ScreenId::Menu};
                case ScreenEvent::OpenOptions:
                    return ScreenState{.screen = ScreenId::Options,
                                       .optionsReturnTo = ScreenId::Pause};
                default:
                    return std::nullopt;
            }
        case ScreenId::LevelComplete:
            switch (event) {
                case ScreenEvent::ContinueAfterLevel:
                case ScreenEvent::ReplayLevel:
                    return ScreenState{.screen = ScreenId::Game, .optionsReturnTo = ScreenId::Menu};
                case ScreenEvent::ReturnToMenuFromLevelComplete:
                    return ScreenState{.screen = ScreenId::Menu, .optionsReturnTo = ScreenId::Menu};
                default:
                    return std::nullopt;
            }
    }
    return std::nullopt;
}

}  // namespace hmi
