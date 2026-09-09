// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QWidget>
#include <filesystem>
#include <memory>

/**
 * @file HMI/Interface/OptionsPage.h
 * @brief Écran Options à onglets, intégré à la fenêtre (LOT-38). Layout dans `OptionsPage.ui`.
 */

namespace Ui {
class OptionsPage;
}

namespace hmi {

class GameViewport;
class Localization;
class KeybindingsWidget;
class EditorKeybindingsWidget;
class GamepadBindingsWidget;
class AudioEngine;

/**
 * @brief Page Options **unique**, affichée dans la fenêtre principale (pas en pop-up), au style du
 *        menu principal (thème `theme-identity.qss`).
 *
 * Mise en page dans `OptionsPage.ui` (éditable dans Qt Designer) : titre + `QTabWidget` avec les
 * onglets **Vidéo** et **Audio** (contenu statique). Les onglets **Commande clavier** et **Commande
 * manette** (contenu généré) sont ajoutés en code (`EX-IHM-040`). N'émet que des intentions non
 * triviales (retour, plein écran) ; les réglages simples agissent directement sur le viewport.
 *
 * ## Ce que la mise en page porte, et que le `.ui` ne dit plus lui-meme
 *
 * Le `.ui` de cet ecran est un fichier **produit par Qt Designer** (`EX-IHM-006`) : Designer
 * efface les commentaires XML a chaque enregistrement, et la justification de la mise en
 * page vit donc ici, ou elle survit a l'edition.
 *
 * - La racine — Les marges sont posees en code (`OptionsPage::OptionsPage`, `LOT-56`) depuis
 *   l'echelle d'espacement des jetons. Aucun nombre n'est fige dans le `.ui`.
 * - `optionsTitle` — Taille et graisse viennent du theme (`#OptionsPage QLabel#optionsTitle`,
 *   `LOT-56 TACHE-03`), depuis l'echelle typographique des jetons. Aucune propriete de police n'est
 *   figee dans le `.ui`.
 * - `fullscreenCheck` — Les selecteurs de resolution et de limite d'images/s ont ete **retires** au
 *   `LOT-68` (`EX-IHM-072`) : grises et non branches depuis leur creation, ils promettaient un
 *   reglage inexistant. A leur place, une bascule qui, elle, agit reellement.
 */
class OptionsPage : public QWidget {
    Q_OBJECT

public:
    /**
     * @param viewport        Viewport de jeu (V-Sync, remappage).
     * @param audio           Moteur audio (`LOT-60` TACHE-04), non possédé ; jamais nul en usage
     *                        réel (`MainWindow` le construit avant la page). Le curseur de volume
     *                        n'a d'effet que si non nul.
     * @param keybindingsPath Chemin de persistance des remappages.
     * @param parent          Widget parent Qt (propriété standard, `nullptr` par défaut).
     */
    OptionsPage(GameViewport* viewport, AudioEngine* audio, std::filesystem::path keybindingsPath,
                QWidget* parent = nullptr);
    ~OptionsPage() override;

    /// Applique la langue active à tous les libellés (onglets, champs, boutons).
    void retranslateUi(const Localization& loc);

signals:
    void backRequested();
    void fullscreenRequested(bool enabled);
    /// Émis quand l'utilisateur choisit une langue (« fr »/« en »).
    void languageChanged(const QString& code);
    /// Émis quand l'utilisateur demande l'enregistrement des journaux de session.
    void saveLogsRequested();
    /// Émis après chaque remappage réussi d'une touche d'éditeur (`LOT-57` TACHE-04) : l'appelant
    /// doit resynchroniser les raccourcis effectifs des actions (`EditorActions::applyShortcuts`).
    void editorBindingsChanged();

private:
    std::unique_ptr<Ui::OptionsPage> _ui;
    AudioEngine* _audio = nullptr;  ///< Non possédé (`EX-NFR-040` : nul = curseur sans effet).
    KeybindingsWidget* _keyboard = nullptr;  ///< Onglet remappage clavier (contenu dynamique).
    EditorKeybindingsWidget* _editorKeyboard = nullptr;  ///< Onglet remappage éditeur (LOT-57).
    GamepadBindingsWidget* _gamepad = nullptr;  ///< Onglet remappage manette (contenu dynamique).
    int _keyboardTabIndex = -1;
    int _editorKeyboardTabIndex = -1;
    int _gamepadTabIndex = -1;
};

}  // namespace hmi
