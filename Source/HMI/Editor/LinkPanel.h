// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QWidget>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include "Core/Levels/GridPosition.h"
#include "HMI/Editor/LinkGeometry.h"

/**
 * @file HMI/Editor/LinkPanel.h
 * @brief Panneau « Liens » : liste/surbrillance/suppression des liaisons de mécanismes (LOT-37).
 */

class QModelIndex;
class QPushButton;
class QStandardItemModel;
class QTableView;

namespace core {
class LevelDraft;
}

namespace Ui {
class LinkPanel;
}

namespace hmi {

class Localization;

/**
 * @brief Vue tabulaire des liaisons (`Mechanism`) du brouillon courant (`EX-IHM-031`).
 *
 * **Vue du modèle**, pas un état : `refresh` reconstruit la liste depuis un `core::LevelDraft`
 * (`hmi::buildLinkRows`), aucune copie de l'état des liaisons n'est conservée entre deux appels.
 * Sélectionner une ligne émet `linkSelected` (surbrillance dans le viewport) ; le bouton
 * « Supprimer » émet `deleteRequested` (la suppression effective passe par
 * `GameViewport::unlinkMechanism`, seul propriétaire du brouillon).
 */
class LinkPanel : public QWidget {
    Q_OBJECT

public:
    explicit LinkPanel(QWidget* parent = nullptr);

    /// Hors-ligne : `std::unique_ptr<Ui::LinkPanel>` porte un type incomplet.
    ~LinkPanel() override;

    /// Reconstruit la liste depuis @p draft (après toute mutation du brouillon).
    void refresh(const core::LevelDraft& draft);

    /// Applique la langue active (en-têtes de colonnes, libellés de type, bouton).
    void retranslateUi(const Localization& loc);

signals:
    /// Émis quand la sélection change : la paire (déclencheur, cible), ou `std::nullopt` si aucune
    /// ligne n'est sélectionnée.
    void linkSelected(std::optional<std::pair<core::GridPosition, core::GridPosition>> link);
    /// Émis par le bouton « Supprimer » pour la ligne sélectionnée (position de la cible).
    void deleteRequested(core::GridPosition targetPosition);

private:
    void rebuildRows();
    void onSelectionChanged();
    void onDeleteClicked();

    /// Mise en page issue de `LinkPanel.ui` (`LOT-68`) : le C++ ne branche plus que le
    /// fonctionnel, conformément à la convention du projet.
    std::unique_ptr<Ui::LinkPanel> _ui;
    QTableView* _table;
    QStandardItemModel* _model;
    QPushButton* _deleteButton;
    std::vector<LinkRow> _rows;  ///< Ligne de tableau (index) -> (type, déclencheur, cible).
    const Localization* _loc = nullptr;
};

}  // namespace hmi
