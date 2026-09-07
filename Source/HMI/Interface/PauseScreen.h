// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QWidget>
#include <memory>

/**
 * @file HMI/Interface/PauseScreen.h
 * @brief Écran de pause (LOT-59 TACHE-02). Mise en page dans `PauseScreen.ui` (Qt Designer).
 */

namespace Ui {
class PauseScreen;
}

namespace hmi {

class Localization;

/**
 * @brief Recouvrement de pause affiché par-dessus la scène de jeu figée (`EX-GP-041`).
 *
 * Widget **enfant du même parent que le conteneur du viewport** (`MainWindow::_stack`), jamais
 * une page du `QStackedWidget` : c'est ce qui permet à la scène de rester dessinée derrière lui
 * (`MainWindow` garde `_editorContainer` comme page courante et se contente de montrer/cacher ce
 * recouvrement par-dessus, geometrie synchronisee a chaque redimensionnement). N'émet que des
 * intentions ; `MainWindow` décide (dont la confirmation avant de quitter vers le menu, qui perd
 * la progression du tableau en cours).
 */
class PauseScreen : public QWidget {
    Q_OBJECT

public:
    explicit PauseScreen(QWidget* parent = nullptr);
    ~PauseScreen() override;

    /// Applique la langue active aux libellés.
    void retranslateUi(const Localization& loc);
    /// Donne le focus clavier au premier bouton (« Reprendre ») : `Tab` navigue ensuite entre les
    /// boutons, `Entrée`/`Espace` les active, `Échap` remonte à `keyPressEvent` (aucun bouton ne
    /// gère cette touche -- propagation standard Qt vers le parent).
    void focusDefaultAction();

protected:
    /// `Échap` depuis la pause reprend (même intention que le bouton « Reprendre »,
    /// `LOT-59` TACHE-02) -- reçu ici, pas par `GameViewport`, tant que ce widget a le focus
    /// clavier (`MainWindow::applyScreenDressing`).
    void keyPressEvent(QKeyEvent* event) override;

signals:
    void resumeRequested();
    void optionsRequested();
    void quitToMenuRequested();

private:
    std::unique_ptr<Ui::PauseScreen> _ui;
};

}  // namespace hmi
