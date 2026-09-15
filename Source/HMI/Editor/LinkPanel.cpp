// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "HMI/Editor/LinkPanel.h"

#include <QAbstractItemView>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QPushButton>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QString>
#include <QTableView>
#include <QVBoxLayout>

#include "Core/Levels/LevelDraft.h"
#include "HMI/Localization/Localization.h"
#include "ui_LinkPanel.h"

namespace hmi {

namespace {

constexpr int COLUMN_COUNT = 3;

// Libellé "(colonne, ligne)" d'une position de grille, pour affichage dans le tableau.
QString positionText(core::GridPosition position) {
    return QStringLiteral("(%1, %2)").arg(position.column).arg(position.row);
}

}  // namespace

LinkPanel::LinkPanel(QWidget* parent)
    : QWidget(parent),
      _ui(std::make_unique<Ui::LinkPanel>()),
      _model(new QStandardItemModel(0, COLUMN_COUNT, this)) {
    _ui->setupUi(this);
    _table = _ui->table;
    _deleteButton = _ui->deleteButton;

    _table->setModel(_model);
    // Comportement de selection et en-tetes : du fonctionnel, pas de la mise en page -- il reste
    // donc ici, la ou le .ui ne porte que la structure.
    _table->horizontalHeader()->setStretchLastSection(true);
    _table->verticalHeader()->setVisible(false);
    _model->setHorizontalHeaderLabels(
        {QStringLiteral("Type"), QStringLiteral("Déclencheur"), QStringLiteral("Cible")});

    connect(_table->selectionModel(), &QItemSelectionModel::selectionChanged, this,
            [this] { onSelectionChanged(); });
    connect(_deleteButton, &QPushButton::clicked, this, [this] { onDeleteClicked(); });
}

LinkPanel::~LinkPanel() = default;

void LinkPanel::refresh(const core::LevelDraft& draft) {
    _rows = buildLinkRows(draft);
    rebuildRows();
}

void LinkPanel::rebuildRows() {
    _model->removeRows(0, _model->rowCount());
    for (const LinkRow& row : _rows) {
        const bool mechanism = row.kind == LinkKind::Mechanism;
        QString typeLabel;
        if (_loc != nullptr) {
            typeLabel = QString::fromStdString(
                _loc->text(mechanism ? "link.type.mechanism" : "link.type.danger"));
        } else {
            typeLabel = mechanism ? QStringLiteral("Mécanisme") : QStringLiteral("Danger commuté");
        }
        auto* const typeItem = new QStandardItem(typeLabel);
        auto* const triggerItem = new QStandardItem(positionText(row.trigger));
        auto* const targetItem = new QStandardItem(positionText(row.target));
        typeItem->setEditable(false);
        triggerItem->setEditable(false);
        targetItem->setEditable(false);
        _model->appendRow({typeItem, triggerItem, targetItem});
    }
    _deleteButton->setEnabled(false);
}

void LinkPanel::retranslateUi(const Localization& loc) {
    _loc = &loc;
    _model->setHorizontalHeaderLabels({
        QString::fromStdString(loc.text("link.column.type")),
        QString::fromStdString(loc.text("link.column.trigger")),
        QString::fromStdString(loc.text("link.column.target")),
    });
    _deleteButton->setText(QString::fromStdString(loc.text("link.delete_button")));
    rebuildRows();  // réapplique les libellés de type (mécanisme/danger) traduits.
}

void LinkPanel::onSelectionChanged() {
    const QModelIndexList selected = _table->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        _deleteButton->setEnabled(false);
        emit linkSelected(std::nullopt);
        return;
    }
    const int row = selected.first().row();
    if (row < 0 || static_cast<std::size_t>(row) >= _rows.size()) {
        _deleteButton->setEnabled(false);
        emit linkSelected(std::nullopt);
        return;
    }
    _deleteButton->setEnabled(true);
    const LinkRow& selectedRow = _rows[static_cast<std::size_t>(row)];
    emit linkSelected(std::make_pair(selectedRow.trigger, selectedRow.target));
}

void LinkPanel::onDeleteClicked() {
    const QModelIndexList selected = _table->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        return;
    }
    const int row = selected.first().row();
    if (row < 0 || static_cast<std::size_t>(row) >= _rows.size()) {
        return;
    }
    emit deleteRequested(_rows[static_cast<std::size_t>(row)].target);
}

}  // namespace hmi
