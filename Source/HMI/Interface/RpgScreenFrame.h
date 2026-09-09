// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QWidget>
#include <map>
#include <string>
#include <vector>

#include "HMI/Presentation/RpgScreens.h"

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

    /**
     * @brief Remplit les valeurs de l'écran (`LOT-38`).
     *
     * @param values Identifiant de valeur (`hmi::RpgField::valueId`) → texte déjà **formaté et
     *               traduit** par l'appelant. Une entrée dont l'identifiant n'est pas à l'écran
     *               est ignorée en silence : les valeurs sont produites par écran, et une fiche
     *               qui en porterait une de trop n'est pas une erreur.
     *
     * Ce qu'aucune entrée ne nomme **reste au tiret cadratin**. C'est ce qui distingue « ce champ
     * n'a pas de source » de « ce champ vaut zéro », et les confondre ferait lire un personnage
     * sans sorts comme un personnage dont les sorts sont épuisés.
     */
    void setValues(const std::map<std::string, std::string>& values);

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

    /// Construit le cadre commun : titre, corps défilant, pied d'actions.
    void buildChrome();

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
    /// Étiquettes de **valeur**, et l'identifiant sous lequel `setValues` les remplit. Un
    /// identifiant vide dit que rien n'alimente encore ce champ : l'étiquette garde alors le tiret
    /// cadratin, quoi qu'on lui passe.
    std::vector<TranslatedLabel> _values;
    /// Les dernières valeurs reçues, rejouées après un changement de langue (`retranslateUi`
    /// repasse tout au tiret avant de les reposer).
    std::map<std::string, std::string> _lastValues;
};

}  // namespace hmi
