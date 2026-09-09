// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QWidget>
#include <map>
#include <string>
#include <vector>

#include "HMI/Interface/RpgScreenSurface.h"
#include "HMI/Interface/RpgScreens.h"

class QLabel;

namespace hmi {

class Localization;
class MenuEntryButton;
class TitleBanner;

/**
 * @file HMI/Interface/RpgUiScreen.h
 * @brief Le comportement commun d'un écran du RPG **décrit par son `.ui`** (`LOT-85`,
 *        `EX-IHM-006`).
 *
 * ## Pourquoi une seule classe pour huit écrans
 *
 * Les huit écrans n'ont pas de comportement propre : ils affichent des libellés traduits et des
 * valeurs nommées par la table, et se ferment comme leurs voisins. Ce qui les distingue est
 * entièrement dans leur **planche** — et c'est justement ce qu'on veut pouvoir retoucher dans Qt
 * Designer sans écrire une ligne de C++. Huit classes jumelles auraient été huit copies d'un même
 * corps, désynchronisées au premier ajout.
 *
 * Chaque écran n'apporte donc que son `Ui::` : c'est le rôle de `hmi::RpgUiPlate`, qui n'existe que
 * pour appeler `setupUi` puis `bind()`.
 *
 * ## Ce que la planche dit, et ce que ce code décide
 *
 * La planche porte la **structure**, et trois propriétés dynamiques :
 *
 * - `rpgRole` — le rôle, que le thème cible (`EX-IHM-051`) et dont ce code déduit les grandeurs ;
 * - `rpgKey` — la clé de traduction d'un libellé ;
 * - `rpgValue` — l'identifiant sous lequel `setValues` remplira le champ. Ceux de la table, mot
 *   pour mot : un identifiant qui en disparaîtrait cesserait d'arriver, et le tiret cadratin le
 *   dirait — c'est le comportement voulu.
 *
 * Ce code décide de **toutes les grandeurs** : espacements, retraits, côté des cases. Elles se
 * calculent depuis les jetons multipliés par le facteur d'agrandissement, et les figer dans le
 * fichier les arrêterait au facteur du jour où il a été écrit (`EX-IHM-082`).
 *
 * Une convention de nommage porte le reste : `*Inset` pour le retrait d'un parchemin, `*Layout`
 * d'un bloc pour son contenu, `*ColumnLayout` pour une colonne. Elle est **écrite dans le
 * générateur et lue ici** ; `scripts/check_ui_designer.py` la vérifie, faute de quoi un
 * renommage dans Designer laisserait un espacement au défaut de Qt sans que rien ne le dise.
 */
class RpgUiScreen : public QWidget, public RpgScreenSurface {
    Q_OBJECT

public:
    void retranslateUi(const Localization& loc) override;
    void setValues(const std::map<std::string, std::string>& values) override;
    void focusDefaultAction() override;

    [[nodiscard]] QWidget* widget() override {
        return this;
    }

    [[nodiscard]] RpgScreenId screen() const {
        return _descriptor.id;
    }

signals:
    /// L'écran doit se fermer (retour à l'écran d'où il a été ouvert).
    void closeRequested();
    /// Passer à l'écran suivant du cycle, **sans** repasser par le menu (`EX-IHM-090`).
    void nextScreenRequested();
    void previousScreenRequested();

protected:
    explicit RpgUiScreen(const RpgScreenDescriptor& descriptor, QWidget* parent = nullptr);

    /// À appeler **après** `setupUi` : relève ce que la planche décrit, pose les grandeurs et
    /// branche le pied d'actions. Séparé du constructeur parce que la planche n'existe pas encore
    /// quand celui-ci s'exécute.
    void bind();

    void keyPressEvent(QKeyEvent* event) override;

private:
    void applyMetrics();
    void collectBindings();

    struct Bound {
        QLabel* label = nullptr;
        std::string key;
    };

    RpgScreenDescriptor _descriptor;
    TitleBanner* _title = nullptr;
    MenuEntryButton* _closeButton = nullptr;
    MenuEntryButton* _previousButton = nullptr;
    MenuEntryButton* _nextButton = nullptr;
    QLabel* _hints = nullptr;
    std::vector<Bound> _translated;
    std::vector<Bound> _values;
    std::map<std::string, std::string> _lastValues;
};

/**
 * @brief Un écran du RPG, marié à sa planche.
 *
 * Un patron et non huit classes écrites à la main : tout le corps est dans `hmi::RpgUiScreen`, et
 * ceci n'apporte que le `Ui::` que `uic` a produit pour cet écran-là. Ajouter un écran coûte donc
 * un `.ui`, une ligne de table et une ligne de fabrique.
 *
 * Pas de `Q_OBJECT` ici : `moc` ne traite pas les patrons. Les signaux vivent dans la classe de
 * base, qui n'en est pas un — c'est la raison de cette découpe.
 */
template <typename UiT>
class RpgUiPlate : public RpgUiScreen {
public:
    explicit RpgUiPlate(const RpgScreenDescriptor& descriptor, QWidget* parent = nullptr)
        : RpgUiScreen(descriptor, parent) {
        _ui.setupUi(this);
        bind();
    }

private:
    UiT _ui;
};

}  // namespace hmi
