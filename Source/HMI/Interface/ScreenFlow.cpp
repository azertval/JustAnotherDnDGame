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
        case ScreenId::Credits:
        case ScreenId::RpgScreen:
            // Même habillage que Menu : page du QStackedWidget, jamais un recouvrement.
            //
            // Les écrans du RPG (LOT-68) partagent ce cas, et c'est une DECISION, pas un
            // rangement : ceux qui se consultent en marchant (EX-IHM-091) mériteront un
            // recouvrement le jour où il y aura une scène à laisser voir derrière eux. Tant que
            // « Nouvelle partie » n'a pas de carte à charger, ce recouvrement se peindrait sur du
            // vide -- on l'écrira avec le contenu du LOT-27, qui pourra le montrer.
            return ScreenDressing{.docksVisible = false,
                                  .menuBarVisible = false,
                                  .toolBarVisible = false,
                                  .pixelToolBarVisible = false,
                                  .editingCommandsEnabled = false,
                                  .gamepadNavigationActive = true,
                                  .overlayVisible = false};
        case ScreenId::Pause:
            // Recouvrement par-dessus Game (EX-GP-041) : même habillage que Game (la
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
                case ScreenEvent::OpenCredits:
                    return ScreenState{.screen = ScreenId::Credits,
                                       .optionsReturnTo = ScreenId::Menu};
                // Depuis le menu, les écrans du RPG s'ouvrent en ECHAFAUDAGE (LOT-68) : « Nouvelle
                // partie » n'a pas de carte à charger tant que le LOT-27 n'en livre pas une, et
                // huit écrans qu'on ne peut pas atteindre ne se valident pas.
                case ScreenEvent::OpenRpgScreen:
                    return ScreenState{.screen = ScreenId::RpgScreen,
                                       .optionsReturnTo = ScreenId::Menu,
                                       .rpgReturnTo = ScreenId::Menu};
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
                // Depuis le jeu : la fiche, l'inventaire ou la carte s'ouvrent et se referment sur
                // la partie en cours. C'est `hmi::pausesGame` qui dit lequel suspend la simulation
                // (EX-IHM-091), pas cette table -- elle ne connaît pas les huit écrans.
                case ScreenEvent::OpenRpgScreen:
                    return ScreenState{.screen = ScreenId::RpgScreen,
                                       .optionsReturnTo = ScreenId::Menu,
                                       .rpgReturnTo = ScreenId::Game};
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
                    return ScreenState{.screen = ScreenId::Game, .optionsReturnTo = ScreenId::Menu};
                case ScreenEvent::QuitPauseToMenu:
                    return ScreenState{.screen = ScreenId::Menu, .optionsReturnTo = ScreenId::Menu};
                case ScreenEvent::OpenOptions:
                    return ScreenState{.screen = ScreenId::Options,
                                       .optionsReturnTo = ScreenId::Pause};
                case ScreenEvent::OpenRpgScreen:
                    return ScreenState{.screen = ScreenId::RpgScreen,
                                       .optionsReturnTo = ScreenId::Menu,
                                       .rpgReturnTo = ScreenId::Pause};
                default:
                    return std::nullopt;
            }
        case ScreenId::RpgScreen:
            switch (event) {
                // Un seul retour, vers l'écran d'où l'on vient. Le PASSAGE d'un écran du RPG à un
                // autre n'est pas une transition de cette machine : les huit vivent sur une seule
                // page (`hmi::RpgScreenHost`), et c'est ce qui permet d'aller de la fiche au
                // journal sans repasser par le menu (EX-IHM-090).
                case ScreenEvent::CloseRpgScreen:
                    return ScreenState{.screen = current.rpgReturnTo,
                                       .optionsReturnTo = ScreenId::Menu,
                                       .rpgReturnTo = ScreenId::Menu};
                default:
                    return std::nullopt;
            }
    }
    return std::nullopt;
}

}  // namespace hmi
