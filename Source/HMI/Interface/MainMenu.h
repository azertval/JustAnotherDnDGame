// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QPixmap>
#include <QWidget>
#include <memory>

/**
 * @file HMI/Interface/MainMenu.h
 * @brief Menu principal de l'application Qt (LOT-38). Mise en page dans `MainMenu.ui` (Qt
 * Designer).
 */

namespace Ui {
class MainMenu;
}

namespace hmi {

class Localization;

/**
 * @brief Menu principal : point d'entrée de l'application Qt (`EX-IHM-040`).
 *
 * La **mise en page** est décrite dans `MainMenu.ui` (éditable dans Qt Designer, compilée par
 * `uic`) ; ce code ne porte que la **logique** (émission des intentions). Le **thème** vient de
 * `resources/theme-identity.qss`. N'émet que des intentions (jouer, éditer, options, quitter) — la
 * navigation est appliquée par `MainWindow`.
 */
class MainMenu : public QWidget {
    Q_OBJECT

public:
    /// Nom de fichier du fond de menu, sous `Assets/UI/`. **Unique** endroit du code où il est
    /// écrit : `scripts/check_ui_assets.py` le recoupe avec le manifeste des illustrations dans
    /// les deux sens — un fond nommé ici mais absent du manifeste, et une illustration livrée que
    /// plus personne ne nomme, sont l'une et l'autre une erreur.
    static constexpr const char* BACKDROP_FILE = "world-map.jpg";

    explicit MainMenu(QWidget* parent = nullptr);
    ~MainMenu() override;

    /// Applique la langue active aux libellés (titre et boutons).
    void retranslateUi(const Localization& loc);

    /// Active/désactive « Continuer » (`LOT-59` TACHE-06) : grisé sans progression, seul bouton
    /// dont l'état dépend d'autre chose que la langue -- appelé par `MainWindow` à chaque retour

signals:
    void newGameRequested();
    void editorRequested();
    void optionsRequested();
    void creditsRequested();
    void quitRequested();

protected:
    /// Peint le décor pixel art du menu (`LOT-68`, `EX-IHM-070`) **derrière** les enfants : ciel
    /// en bandes, lune, silhouettes et sol, plus un voile de lisibilité sous le texte. La géométrie
    /// vient de `hmi::menuBackdropQuads` (pure) ; seules les couleurs sont résolues ici.
    void paintEvent(QPaintEvent* event) override;

private:
    /// Carte du monde de Tanares, fond du menu (`LOT-67`). **Nulle** si l'illustration est absente
    /// — cas attendu (`EX-NFR-040`), jamais une panne : le décor tracé de
    /// `hmi::menuBackdropQuads` reprend alors la main.
    QPixmap _backdrop;
    std::unique_ptr<Ui::MainMenu> _ui;
};

}  // namespace hmi
