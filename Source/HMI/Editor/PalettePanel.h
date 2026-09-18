// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <QPixmap>
#include <QWidget>
#include <memory>

#include "Core/Levels/TileType.h"

/**
 * @file HMI/Editor/PalettePanel.h
 * @brief Panneau « Palette » : arbre de sélection du type de tuile à peindre (LOT-35).
 */

class QEvent;
class QModelIndex;
class QStandardItemModel;
class QTreeView;

namespace Ui {
class PalettePanel;
}

namespace hmi {

class Localization;

/**
 * @brief Palette de tuiles en **arbre** (`QTreeView`) : catégories → sous-groupes → tuiles.
 *
 * Alimentée par la taxonomie pure (`hmi::tileTaxonomy`), elle remplace l'accordéon « maison »
 * (retiré au `LOT-38`) par un contrôle Qt natif (`EX-EDIT-018`, `EX-IHM-010`). Sélectionner une
 * **feuille** met à jour le type de tuile actif (`selectedTile`) et émet `tileSelected` — consommé
 * par l'outil de peinture (LOT-35 TACHE-03). Les en-têtes (catégories, sous-groupes) ne sont pas
 * sélectionnables comme tuile.
 */
class PalettePanel : public QWidget {
    Q_OBJECT

public:
    explicit PalettePanel(QWidget* parent = nullptr);

    /// Hors-ligne : `std::unique_ptr<Ui::PalettePanel>` porte un type incomplet.
    ~PalettePanel() override;

    /// @return Le type de tuile actuellement sélectionné (`Solid` par défaut).
    [[nodiscard]] core::TileType selectedTile() const noexcept {
        return _selected;
    }

    /// Applique la langue active (reconstruit l'arbre avec les libellés traduits).
    void retranslateUi(const Localization& loc);

signals:
    /// Émis quand l'utilisateur sélectionne une tuile (feuille) dans l'arbre.
    void tileSelected(core::TileType type);

protected:
    /// Régénère les vignettes lors d'un changement d'écran (`QEvent::ScreenChangeInternal`) :
    /// l'échelle d'affichage a pu changer (`LOT-56` TACHE-05).
    bool event(QEvent* event) override;

private:
    void buildModel();
    void onCurrentChanged(const QModelIndex& current);
    /// Vignette d'un type : sa couleur dans l'atlas procédural.
    [[nodiscard]] QPixmap thumbnailFor(core::TileType type);

    /// Mise en page issue de `PalettePanel.ui` (`LOT-68`) : le C++ ne branche plus que le
    /// fonctionnel, conformément à la convention du projet.
    std::unique_ptr<Ui::PalettePanel> _ui;
    QTreeView* _tree;
    QStandardItemModel* _model;
    core::TileType _selected = core::TileType::Solid;
    const Localization* _loc = nullptr;  ///< Catalogue courant (nul avant première retraduction).

};

}  // namespace hmi
