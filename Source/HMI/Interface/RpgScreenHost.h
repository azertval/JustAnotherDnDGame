// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QHash>
#include <QWidget>
#include <map>
#include <string>

#include "HMI/Presentation/RpgScreens.h"

/**
 * @file HMI/Interface/RpgScreenHost.h
 * @brief Pile des huit écrans du RPG et navigation entre eux (`LOT-68`, `EX-IHM-090`).
 */

class QStackedWidget;

namespace hmi {

class Localization;
class RpgScreenFrame;

/**
 * @brief Héberge les écrans du RPG et fait passer de l'un à l'autre **sans repasser par le menu**.
 *
 * Une seule page du `QStackedWidget` de `MainWindow` porte les huit écrans : c'est ce qui rend le
 * passage de l'un à l'autre gratuit pour la machine à états d'écrans, qui n'a donc pas huit états
 * de plus à connaître. `hmi::ScreenFlow` sait qu'« un écran du RPG est ouvert » ; **lequel** est la
 * question de cet hôte, exactement comme `ScreenState::optionsReturnTo` porte la provenance
 * d'Options plutôt qu'une variable posée à côté.
 *
 * Les huit châssis sont construits **une fois**, à la construction : les reconstruire à chaque
 * ouverture rejouerait la mise en page complète pour un écran dont rien n'a changé.
 */
class RpgScreenHost : public QWidget {
    Q_OBJECT

public:
    explicit RpgScreenHost(QWidget* parent = nullptr);

    /// Applique la langue active aux huit écrans.
    void retranslateUi(const Localization& loc);

    /// Pose les valeurs de @p screen (`hmi::RpgScreenFrame::setValues`, `LOT-38`). Sans effet si
    /// cet écran n'est pas dans la table -- rien à remplir n'est pas une erreur.
    void setValues(RpgScreenId screen, const std::map<std::string, std::string>& values);

    /// Affiche @p screen et lui donne le focus clavier.
    void showScreen(RpgScreenId screen);

    /// @return L'écran affiché.
    [[nodiscard]] RpgScreenId currentScreen() const {
        return _current;
    }

    /// Passe à l'écran suivant du cycle (`hmi::nextRpgScreen`).
    void showNextScreen();
    /// Passe à l'écran précédent du cycle.
    void showPreviousScreen();

    /// Donne le focus clavier à l'écran affiché (`RpgScreenFrame::focusDefaultAction`).
    void focusDefaultAction();

signals:
    /// L'écran affiché demande à se fermer : `MainWindow` décide vers où l'on revient.
    void closeRequested();
    /// L'écran affiché a changé — porte la règle de superposition à qui doit l'appliquer
    /// (`EX-IHM-091`).
    void screenChanged(RpgScreenId screen);

private:
    QStackedWidget* _stack = nullptr;
    QHash<int, RpgScreenFrame*> _frames;  ///< `RpgScreenId` (en entier) -> châssis construit.
    RpgScreenId _current = RpgScreenId::CharacterSheet;
    /// Dernières valeurs de chaque écran, rejouées après un changement de langue : sans elles,
    /// changer de langue viderait une fiche remplie.
    QHash<int, std::map<std::string, std::string>> _screenValues;
};

}  // namespace hmi
