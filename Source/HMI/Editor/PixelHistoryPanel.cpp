// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "HMI/Editor/PixelHistoryPanel.h"

#include <QListWidget>
#include <QListWidgetItem>
#include <QString>
#include <QVBoxLayout>

#include "HMI/Localization/Localization.h"
#include "ui_PixelHistoryPanel.h"

namespace hmi {

PixelHistoryPanel::PixelHistoryPanel(QWidget* parent)
    : QWidget(parent), _ui(std::make_unique<Ui::PixelHistoryPanel>()), _list(_ui->list) {
    _ui->setupUi(this);

    connect(_list, &QListWidget::itemActivated, this,
            [this](QListWidgetItem*) { onItemActivated(); });
}

PixelHistoryPanel::~PixelHistoryPanel() = default;

void PixelHistoryPanel::refresh(const PixelHistory& history) {
    _entries = history.appliedEntries();

    _list->clear();
    for (const PixelHistoryEntry& entry : _entries) {
        const QString label =
            _loc != nullptr
                ? QString::fromStdString(
                      _loc->text(std::string(pixelOperationTranslationKey(entry.kind))))
                : QString::fromStdString(std::string(pixelOperationTranslationKey(entry.kind)));
        _list->addItem(label);
    }
    if (!_entries.empty()) {
        // La derniere entree est toujours l'etat courant (aucune branche de refaire visible dans
        // cette liste, cf. en-tete) : la selectionner indique la position courante sans etat
        // supplementaire a maintenir.
        _list->setCurrentRow(static_cast<int>(_entries.size()) - 1);
    }
}

void PixelHistoryPanel::retranslateUi(const Localization& loc) {
    _loc = &loc;
    const std::vector<PixelHistoryEntry> entries =
        _entries;  // refresh() les reconstruit a l'identique.
    for (int row = 0; row < _list->count(); ++row) {
        if (static_cast<std::size_t>(row) >= entries.size()) {
            break;
        }
        _list->item(row)->setText(QString::fromStdString(loc.text(std::string(
            pixelOperationTranslationKey(entries[static_cast<std::size_t>(row)].kind)))));
    }
}

void PixelHistoryPanel::onItemActivated() {
    const int row = _list->currentRow();
    if (row < 0 || static_cast<std::size_t>(row) >= _entries.size()) {
        return;
    }
    emit jumpRequested(static_cast<std::size_t>(row));
}

}  // namespace hmi
