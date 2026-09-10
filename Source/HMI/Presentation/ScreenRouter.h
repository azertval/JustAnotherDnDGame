// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QString>
#include <QtQmlIntegration>

#include "HMI/Presentation/RpgScreens.h"
#include "HMI/Presentation/ScreenFlow.h"

/**
 * @file HMI/Presentation/ScreenRouter.h
 * @brief La navigation entre écrans, telle que le QML la pilote (`LOT-86`).
 */

namespace hmi {

/**
 * @brief Pilote la machine à états des écrans, et publie l'écran courant.
 *
 * ## Ce qu'il n'est pas
 *
 * Il ne décide **rien**. Toute la règle vit dans `hmi::resolveTransition` — table pure, sans Qt,
 * couverte par ses tests — et ce routeur ne fait que l'appeler et diffuser le résultat. C'est ce
 * qui permet de vérifier la navigation sans ouvrir une fenêtre, et qui l'a déjà été.
 *
 * **Une transition non déclarée est refusée**, jamais silencieusement acceptée :
 * `resolveTransition` rend alors `std::nullopt` et l'état ne bouge pas. Sans cette discipline, un
 * `openOptions()` appelé depuis un écran d'où les options ne s'ouvrent pas produirait un état que
 * la table ne décrit pas, et dont personne ne saurait comment revenir.
 *
 * ## Pourquoi une énumération et non un nom de fichier
 *
 * Le routeur publie un **état**, pas un chemin. Faire transiter « `Screens/MainMenu.qml` » ferait
 * connaître à la présentation le nom des fichiers de la conception — que l'artiste doit pouvoir
 * renommer sans toucher au C++. La correspondance entre état et fichier vit donc en QML, dans
 * `Logic/ScreenStack.qml`, du côté développeur mais du bon côté de la frontière.
 */
class ScreenRouter : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(Screen currentScreen READ currentScreen NOTIFY changed)
    Q_PROPERTY(RpgScreen currentRpgScreen READ currentRpgScreen NOTIFY changed)

public:
    /// Écran affiché. Reprend `hmi::ScreenId`, moins `Editor` : l'éditeur de niveaux est un
    /// binaire séparé depuis le `LOT-86`, et le jeu n'a aucun chemin qui y mène.
    enum class Screen {
        Menu,
        Game,
        Options,
        Pause,
        Credits,
        RpgScreen,
    };
    Q_ENUM(Screen)

    /// Lequel des neuf écrans du RPG est ouvert, quand `currentScreen` vaut `RpgScreen`.
    enum class RpgScreen {
        CharacterSheet,
        Inventory,
        QuestJournal,
        WorldMap,
        Dialogue,
        Merchant,
        GuildBoard,
        CombatHud,
        TeamSheet,
    };
    Q_ENUM(RpgScreen)

    explicit ScreenRouter(QObject* parent = nullptr);

    [[nodiscard]] Screen currentScreen() const noexcept;
    [[nodiscard]] RpgScreen currentRpgScreen() const noexcept;

    Q_INVOKABLE void openMenu();
    Q_INVOKABLE void openGame();
    Q_INVOKABLE void openOptions();
    Q_INVOKABLE void closeOptions();
    Q_INVOKABLE void openPause();
    Q_INVOKABLE void resume();
    Q_INVOKABLE void quitToMenu();
    Q_INVOKABLE void openCredits();
    Q_INVOKABLE void closeCredits();

    /// Ouvre un écran du RPG. L'écran d'où l'on vient est retenu par la table : refermer y revient,
    /// qu'on soit venu du menu, du jeu ou de la pause.
    Q_INVOKABLE void openRpgScreen(RpgScreen screen);
    Q_INVOKABLE void closeRpgScreen();

    /// Passe à l'écran suivant/précédent du RPG **sans repasser par le menu** (`EX-IHM-090`).
    /// C'est le geste des gâchettes de la manette.
    Q_INVOKABLE void nextRpgScreen();
    Q_INVOKABLE void previousRpgScreen();

signals:
    /// Émis quand l'écran courant change. Rien n'est émis si la transition a été **refusée** :
    /// l'interface ne doit pas se rafraîchir pour un geste que la table n'autorise pas.
    void changed();

private:
    /// Applique @p event si la table l'autorise. @return `true` si l'état a changé.
    bool apply(ScreenEvent event);

    ScreenState _state{};
    RpgScreenId _rpgScreen = RpgScreenId::CharacterSheet;
};

}  // namespace hmi
