// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Runtime/ScreenRouter.h"

#include <optional>

#include "Core/BuildConfig.h"
#include "HMI/HmiLog.h"

namespace hmi {

bool ScreenRouter::developerBuild() noexcept {
    return core::DEVELOPER_BUILD;
}

namespace {

/// Correspondance entre l'état de la table et celui que le QML lit. `Editor` n'y figure pas : le
/// jeu n'a aucun chemin qui y mène, et l'y traduire aurait donné un état que rien ne peut afficher.
[[nodiscard]] ScreenRouter::Screen toRouterScreen(ScreenId screen) noexcept {
    switch (screen) {
        case ScreenId::Menu:
            return ScreenRouter::Screen::Menu;
        case ScreenId::Game:
            return ScreenRouter::Screen::Game;
        case ScreenId::Options:
            return ScreenRouter::Screen::Options;
        case ScreenId::Pause:
            return ScreenRouter::Screen::Pause;
        case ScreenId::Credits:
            return ScreenRouter::Screen::Credits;
        case ScreenId::RpgScreen:
            return ScreenRouter::Screen::RpgScreen;
        case ScreenId::Editor:
            break;
    }
    // Inatteignable : le jeu n'émet aucun événement qui mène à l'éditeur. Retomber sur le menu
    // plutôt que sur un état indéfini — et le dire, parce qu'y arriver signifierait que la table
    // a changé sans que ce routeur le sache.
    HMI_LOG_WARNING("Routeur d'ecrans : etat 'Editeur' atteint depuis le jeu, retour au menu.");
    return ScreenRouter::Screen::Menu;
}

// Les deux énumérations décrivent les mêmes neuf écrans, dans le même ordre : la conversion est
// donc un simple changement de type. C'est une hypothèse, et une hypothèse tacite se casse en
// silence — le jour où quelqu'un insère un écran au milieu d'une des deux, la fiche s'ouvrirait
// à la place de l'inventaire, sans la moindre erreur. Ces vérifications la rendent explicite, et
// leur échec se lit à la compilation.
static_assert(static_cast<int>(ScreenRouter::RpgScreen::CharacterSheet) ==
              static_cast<int>(RpgScreenId::CharacterSheet));
static_assert(static_cast<int>(ScreenRouter::RpgScreen::Inventory) ==
              static_cast<int>(RpgScreenId::Inventory));
static_assert(static_cast<int>(ScreenRouter::RpgScreen::CombatHud) ==
              static_cast<int>(RpgScreenId::CombatHud));
static_assert(static_cast<int>(ScreenRouter::RpgScreen::TeamSheet) ==
              static_cast<int>(RpgScreenId::TeamSheet));

[[nodiscard]] RpgScreenId toRpgScreenId(ScreenRouter::RpgScreen screen) noexcept {
    return static_cast<RpgScreenId>(screen);
}

[[nodiscard]] ScreenRouter::RpgScreen toRouterRpgScreen(RpgScreenId screen) noexcept {
    return static_cast<ScreenRouter::RpgScreen>(screen);
}

}  // namespace

ScreenRouter::ScreenRouter(QObject* parent) : QObject(parent) {}

ScreenRouter::Screen ScreenRouter::currentScreen() const noexcept {
    return toRouterScreen(_state.screen);
}

ScreenRouter::RpgScreen ScreenRouter::currentRpgScreen() const noexcept {
    return toRouterRpgScreen(_rpgScreen);
}

bool ScreenRouter::apply(ScreenEvent event) {
    // Toute la règle est dans la table. Une transition non déclarée rend `nullopt`, et l'état ne
    // bouge pas : c'est ce qui empêche un geste illégitime de produire un état d'où l'on ne sait
    // pas revenir (EX-GP-041).
    const std::optional<ScreenState> next = resolveTransition(_state, event);
    if (!next.has_value()) {
        return false;
    }
    _state = *next;
    emit changed();
    return true;
}

void ScreenRouter::openMenu() {
    static_cast<void>(apply(ScreenEvent::OpenMenu));
}
void ScreenRouter::openGame() {
    static_cast<void>(apply(ScreenEvent::OpenGame));
}
void ScreenRouter::openOptions() {
    static_cast<void>(apply(ScreenEvent::OpenOptions));
}
void ScreenRouter::closeOptions() {
    static_cast<void>(apply(ScreenEvent::CloseOptions));
}
void ScreenRouter::openPause() {
    static_cast<void>(apply(ScreenEvent::OpenPause));
}
void ScreenRouter::resume() {
    static_cast<void>(apply(ScreenEvent::ResumePause));
}
void ScreenRouter::quitToMenu() {
    static_cast<void>(apply(ScreenEvent::QuitPauseToMenu));
}
void ScreenRouter::openCredits() {
    static_cast<void>(apply(ScreenEvent::OpenCredits));
}
void ScreenRouter::closeCredits() {
    static_cast<void>(apply(ScreenEvent::CloseCredits));
}

void ScreenRouter::openRpgScreen(RpgScreen screen) {
    // L'écran demandé est retenu MÊME si la transition échoue déjà parce qu'on y est : ouvrir la
    // fiche puis l'inventaire depuis le châssis ne repasse pas par un changement d'état global.
    _rpgScreen = toRpgScreenId(screen);
    if (!apply(ScreenEvent::OpenRpgScreen)) {
        emit changed();  // même écran global, écran du RPG différent : la vue doit suivre.
    }
}

void ScreenRouter::closeRpgScreen() {
    static_cast<void>(apply(ScreenEvent::CloseRpgScreen));
}

void ScreenRouter::nextRpgScreen() {
    _rpgScreen = hmi::nextRpgScreen(_rpgScreen);
    emit changed();
}

void ScreenRouter::previousRpgScreen() {
    _rpgScreen = hmi::previousRpgScreen(_rpgScreen);
    emit changed();
}

}  // namespace hmi
