// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QWidget>
#include <vector>

#include "HMI/Interface/RpgScreens.h"

/**
 * @file HMI/Interface/RpgScreenFrame.h
 * @brief Châssis commun des écrans du RPG (`LOT-68`, `EX-IHM-090`).
 */

class QLabel;
class QVBoxLayout;

namespace hmi {

class Localization;
class MenuEntryButton;
class TitleBanner;

/**
 * @brief Le **cadre** que les huit écrans du RPG partagent : panneau, titre, zone de contenu, pied
 *        d'actions.
 *
 * ## Pas de `.ui`, et c'est le sujet du lot
 *
 * Tous les autres écrans du jeu décrivent leur mise en page dans un fichier Qt Designer, et c'est
 * la convention du projet. Celui-ci ne le peut pas : huit `.ui` presque identiques seraient huit
 * endroits où corriger un pied de page, et le neuvième écran en ferait neuf — exactement ce
 * qu'`EX-IHM-090` interdit. La mise en page reste néanmoins **déclarative** : elle vit dans
 * `hmi::rpgScreenLayout` (données pures), et ce fichier n'est que le peintre qui la rend.
 *
 * Ce châssis ne connaît **aucun** écran par son nom. Il reçoit un descripteur, en peint l'ossature,
 * et n'émet que des intentions — `hmi::RpgScreenHost` décide où elles mènent.
 */
class RpgScreenFrame : public QWidget {
    Q_OBJECT

public:
    /// @param descriptor Écran à peindre (nom d'objet, titre, ossature).
    /// @param parent     Parent Qt.
    explicit RpgScreenFrame(const RpgScreenDescriptor& descriptor, QWidget* parent = nullptr);

    /// Applique la langue active à tous les libellés : titre d'écran, titres de blocs, libellés de
    /// champs, boutons et rappels de touches.
    void retranslateUi(const Localization& loc);

    /// Donne le focus clavier à la première entrée du pied d'actions — porte d'entrée du parcours
    /// de focus à la manette (`EX-IHM-071`), même patron que `PauseScreen::focusDefaultAction`.
    void focusDefaultAction();

    /// @return L'écran peint par ce châssis.
    [[nodiscard]] RpgScreenId screen() const {
        return _descriptor.id;
    }

protected:
    /// `Échap` ferme l'écran : la même intention que le bouton « Fermer », reçue ici tant que ce
    /// widget a le focus clavier.
    void keyPressEvent(QKeyEvent* event) override;

signals:
    /// L'écran doit se fermer (retour à l'écran d'où il a été ouvert).
    void closeRequested();
    /// Passer à l'écran suivant du cycle, **sans** repasser par le menu (`EX-IHM-090`).
    void nextScreenRequested();
    /// Passer à l'écran précédent du cycle.
    void previousScreenRequested();

private:
    /// Une étiquette et la clé qui la traduit. Retenues ensemble : `retranslateUi` ne sait pas
    /// autrement quoi réécrire dans une ossature construite à partir d'une table.
    struct TranslatedLabel {
        QLabel* label = nullptr;
        const char* key = "";
    };

    /// Construit les widgets d'un bloc et les ajoute à @p column.
    void buildBlock(QVBoxLayout* column, const RpgContentBlock& block);
    /// Construit une colonne de blocs ; @return le conteneur, ou `nullptr` si la colonne est vide.
    [[nodiscard]] QWidget* buildColumn(std::span<const RpgContentBlock> blocks);

    RpgScreenDescriptor _descriptor;
    TitleBanner* _title = nullptr;
    MenuEntryButton* _closeButton = nullptr;
    MenuEntryButton* _previousButton = nullptr;
    MenuEntryButton* _nextButton = nullptr;
    QLabel* _hints = nullptr;
    /// Étiquettes traduisibles de l'ossature (titres de blocs, libellés de champs).
    std::vector<TranslatedLabel> _translated;
    /// Étiquettes de **valeur** : elles ne portent aucune donnée à ce lot et affichent le tiret
    /// cadratin du catalogue. Retenues à part parce qu'elles partagent toutes la même clé.
    std::vector<QLabel*> _placeholders;
};

}  // namespace hmi
