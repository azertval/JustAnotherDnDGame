// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <QPixmap>
#include <QWidget>

#include "Core/Levels/TileType.h"

/**
 * @file Editor/Ui/PalettePanel.h
 * @brief Panneau « Palette » : arbre de sélection du type de tuile à peindre.
 */

class QEvent;
class QModelIndex;
class QStandardItemModel;
class QTreeView;

namespace hmi {

/**
 * @brief Palette de tuiles en **arbre** (`QTreeView`) : catégories → sous-groupes → tuiles.
 *
 * Alimentée par la taxonomie pure (`hmi::tileTaxonomy`), elle remplace l'accordéon « maison »
 * (retiré) par un contrôle Qt natif (`EX-EDIT-018`). Sélectionner une
 * **feuille** met à jour le type de tuile actif (`selectedTile`) et émet `tileSelected` — consommé
 * par l'outil de peinture. Les en-têtes (catégories, sous-groupes) ne sont pas
 * sélectionnables comme tuile.
 */
class PalettePanel : public QWidget {
    Q_OBJECT

public:
    explicit PalettePanel(QWidget* parent = nullptr);

    /// @return Le type de tuile actuellement sélectionné (`Solid` par défaut).
    [[nodiscard]] core::TileType selectedTile() const noexcept {
        return _selected;
    }

signals:
    /// Émis quand l'utilisateur sélectionne une tuile (feuille) dans l'arbre.
    void tileSelected(core::TileType type);

protected:
    /// Régénère les vignettes lors d'un changement d'écran (`QEvent::ScreenChangeInternal`) :
    /// l'échelle d'affichage a pu changer.
    bool event(QEvent* event) override;

private:
    void buildModel();
    void onCurrentChanged(const QModelIndex& current);
    /// Vignette d'un type : sa couleur dans l'atlas procédural.
    [[nodiscard]] QPixmap thumbnailFor(core::TileType type);

    QTreeView* _tree;
    QStandardItemModel* _model;
    core::TileType _selected = core::TileType::Solid;
};

}  // namespace hmi
