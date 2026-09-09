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
 *
 * ## Ce que la mise en page porte, et que le `.ui` ne dit plus lui-meme
 *
 * Le `.ui` de cet ecran est un fichier **produit par Qt Designer** (`EX-IHM-006`) : Designer
 * efface les commentaires XML a chaque enregistrement, et la justification de la mise en
 * page vit donc ici, ou elle survit a l'edition.
 *
 * - La racine — Recouvrement plein cadre (`LOT-59 TACHE-02`) : le menu est centre par les
 *   extenseurs haut/bas, sur un fond translucide pose en code (`PauseScreen::PauseScreen`) qui
 *   laisse deviner la scene figee derriere.
 * - `hintsLabel` — Rappels de touches (`LOT-68`) : texte enrichi compose en code depuis les jetons
 *   (`hmi::keyHintText`). Vide dans le `.ui` — `retranslateUi` le remplit.
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
