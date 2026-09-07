// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <optional>

/**
 * @file HMI/Interface/ScreenFlow.h
 * @brief Machine à états des écrans de la fenêtre principale (`LOT-59` TACHE-01, `EX-GP-041`).
 *
 * Logique **pure** (aucune dépendance Qt), testable hors instance d'application (`EX-NFR-010`) —
 * même patron que `HMI/Editor/PanelFocus.h`/`HMI/Interface/ActionCatalog.h`. `MainWindow` ne fait
 * que suivre cette table : les `showXxx()` deviennent des consommatrices de `dressingFor()`, et
 * toute navigation passe par `resolveTransition()` plutôt que d'appeler une autre méthode
 * directement -- une transition non déclarée ici est **refusée**, jamais silencieusement acceptée.
 */

namespace hmi {

/// Écran actuellement affiché par la fenêtre principale.
enum class ScreenId {
    Menu,
    Editor,
    Game,
    Options,
    Pause,
    Credits,
    /// **Un** écran du RPG est ouvert (`LOT-68`, `EX-IHM-090`). Lequel des huit n'est pas la
    /// question de cette table : c'est celle de `hmi::RpgScreenHost`, qui les héberge tous sur une
    /// seule page. Les y déclarer un par un aurait multiplié par huit les transitions à écrire
    /// pour n'exprimer, huit fois, que la même règle.
    RpgScreen,
};

/// Événement pouvant déclencher une transition d'écran. Un seul événement `OpenOptions`/
/// `CloseOptions` sert Menu et Pause : c'est `ScreenState::optionsReturnTo` (pas l'événement) qui
/// porte la différence, cf. plus bas.
enum class ScreenEvent {
    OpenMenu,
    OpenEditor,
    OpenGame,
    OpenOptions,
    CloseOptions,
    OpenPause,
    ResumePause,
    QuitPauseToMenu,
    OpenCredits,
    CloseCredits,
    /// Ouvre un écran du RPG. Un seul événement pour les huit, et depuis trois écrans (Menu, Game,
    /// Pause) : c'est `ScreenState::rpgReturnTo` qui porte la différence, comme
    /// `optionsReturnTo` le fait pour Options.
    OpenRpgScreen,
    CloseRpgScreen,
};

/// Habillage de fenêtre associé à un écran : ce que chaque `showXxx()` répétait à la main
/// (bascule du `QStackedWidget`, docks, barre de menu, barres d'outils, navigation manette). Le
/// choix de la page du `QStackedWidget` reste dans `MainWindow` (pointeurs de widgets Qt, hors de
/// portée d'une table pure) ; `Pause` ne bascule d'ailleurs aucune page -- c'est un recouvrement
/// par-dessus `Game` (`overlayVisible`), pour que la scène reste dessinée derrière.
struct ScreenDressing {
    bool docksVisible = false;
    bool menuBarVisible = false;
    bool toolBarVisible = false;
    bool pixelToolBarVisible = false;
    bool editingCommandsEnabled = false;
    bool gamepadNavigationActive = false;
    bool overlayVisible = false;

    friend bool operator==(const ScreenDressing&, const ScreenDressing&) = default;
};

/// @return L'habillage de fenêtre attendu pour @p screen.
[[nodiscard]] ScreenDressing dressingFor(ScreenId screen) noexcept;

/// État complet de la machine. `optionsReturnTo` n'est pertinent que lorsque `screen ==
/// ScreenId::Options` : l'écran vers lequel `CloseOptions` revient (`Menu` ou `Pause`, selon
/// l'origine) -- porté ici plutôt que par une variable « écran précédent » posée à côté
/// (`TACHE-01`, voir epic.md).
struct ScreenState {
    ScreenId screen = ScreenId::Menu;
    ScreenId optionsReturnTo = ScreenId::Menu;
    /// Écran vers lequel `CloseRpgScreen` revient (`Menu`, `Game` ou `Pause`, selon l'origine).
    /// Même patron, et même raison, qu'`optionsReturnTo` : la provenance est un attribut de
    /// l'état, jamais une variable « écran précédent » posée à côté de la machine.
    ScreenId rpgReturnTo = ScreenId::Menu;

    friend bool operator==(const ScreenState&, const ScreenState&) = default;
};

/// Résout une transition d'écran à partir de l'état courant et d'un événement.
/// @return Le nouvel état, ou `std::nullopt` si @p event est interdit depuis `current.screen` --
///         l'appelant garde alors l'état courant inchangé, jamais de bascule silencieuse
///         (`EX-GP-041`).
[[nodiscard]] std::optional<ScreenState> resolveTransition(const ScreenState& current,
                                                           ScreenEvent event) noexcept;

}  // namespace hmi
