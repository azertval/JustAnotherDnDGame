// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "HMI/Editor/PixelPalettePanel.h"

#include <QCheckBox>
#include <QColor>
#include <QHBoxLayout>
#include <QIcon>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPixmap>
#include <QPushButton>
#include <QSignalBlocker>
#include <QString>
#include <QVBoxLayout>

#include "HMI/Localization/Localization.h"
#include "ui_PixelPalettePanel.h"

namespace hmi {

namespace {

// Couleur R8G8B8A8_UNORM (LOT-54 TACHE-01) -> QColor, pour peindre une pastille de palette. Seule
// la pastille montre la couleur elle-meme (EX-IHM-051) : tout le reste du panneau vient des jetons
// (feuille de style Qt, TACHE-01 de LOT-56).
QColor toQColor(std::uint32_t color) {
    return {static_cast<int>(color & 0xFFU), static_cast<int>((color >> 8) & 0xFFU),
            static_cast<int>((color >> 16) & 0xFFU), static_cast<int>((color >> 24) & 0xFFU)};
}

QIcon swatchIcon(std::uint32_t color) {
    QPixmap pixmap(16, 16);
    pixmap.fill(toQColor(color));
    return QIcon(pixmap);
}

}  // namespace

PixelPalettePanel::PixelPalettePanel(QWidget* parent)
    : QWidget(parent), _ui(std::make_unique<Ui::PixelPalettePanel>()) {
    _ui->setupUi(this);
    _list = _ui->list;
    _addButton = _ui->addButton;
    _removeButton = _ui->removeButton;
    _renameButton = _ui->renameButton;
    _moveUpButton = _ui->moveUpButton;
    _moveDownButton = _ui->moveDownButton;
    _extractButton = _ui->extractButton;
    _constrainCheck = _ui->constrainCheck;

    connect(_list, &QListWidget::itemSelectionChanged, this, [this] { updateButtonsEnabled(); });
    connect(_list, &QListWidget::itemActivated, this,
            [this](QListWidgetItem* item) { onItemActivated(_list->row(item)); });
    connect(_addButton, &QPushButton::clicked, this, [this] { emit addRequested(); });
    connect(_removeButton, &QPushButton::clicked, this, [this] { onRemoveClicked(); });
    connect(_renameButton, &QPushButton::clicked, this, [this] { onRenameClicked(); });
    connect(_moveUpButton, &QPushButton::clicked, this, [this] { onMoveUpClicked(); });
    connect(_moveDownButton, &QPushButton::clicked, this, [this] { onMoveDownClicked(); });
    connect(_extractButton, &QPushButton::clicked, this, [this] { emit extractRequested(); });
    connect(_constrainCheck, &QCheckBox::toggled, this,
            [this](bool enabled) { emit constrainToggled(enabled); });

    updateButtonsEnabled();
}

PixelPalettePanel::~PixelPalettePanel() = default;

void PixelPalettePanel::refresh(const PixelPalette& palette) {
    _entries = palette.entries();
    _list->clear();
    for (const PixelPaletteEntry& entry : _entries) {
        auto* const item =
            new QListWidgetItem(swatchIcon(entry.color), QString::fromStdString(entry.name));
        _list->addItem(item);
    }
    updateButtonsEnabled();
}

void PixelPalettePanel::retranslateUi(const Localization& loc) {
    _loc = &loc;
    _addButton->setText(QString::fromStdString(loc.text("pixel_palette.add")));
    _removeButton->setText(QString::fromStdString(loc.text("pixel_palette.remove")));
    _renameButton->setText(QString::fromStdString(loc.text("pixel_palette.rename")));
    _moveUpButton->setText(QString::fromStdString(loc.text("pixel_palette.move_up")));
    _moveDownButton->setText(QString::fromStdString(loc.text("pixel_palette.move_down")));
    _extractButton->setText(QString::fromStdString(loc.text("pixel_palette.extract")));
    _constrainCheck->setText(QString::fromStdString(loc.text("pixel_palette.constrain")));
}

void PixelPalettePanel::setConstrainEnabled(bool enabled) {
    const QSignalBlocker blocker(_constrainCheck);
    _constrainCheck->setChecked(enabled);
}

bool PixelPalettePanel::constrainEnabled() const {
    return _constrainCheck->isChecked();
}

void PixelPalettePanel::onRemoveClicked() {
    const int row = _list->currentRow();
    if (row < 0) {
        return;
    }
    emit removeRequested(static_cast<std::size_t>(row));
}

void PixelPalettePanel::onRenameClicked() {
    const int row = _list->currentRow();
    if (row < 0) {
        return;
    }
    emit renameRequested(static_cast<std::size_t>(row));
}

void PixelPalettePanel::onMoveUpClicked() {
    const int row = _list->currentRow();
    if (row < 0) {
        return;
    }
    emit moveRequested(static_cast<std::size_t>(row), true);
}

void PixelPalettePanel::onMoveDownClicked() {
    const int row = _list->currentRow();
    if (row < 0) {
        return;
    }
    emit moveRequested(static_cast<std::size_t>(row), false);
}

void PixelPalettePanel::onItemActivated(int row) {
    if (row < 0 || static_cast<std::size_t>(row) >= _entries.size()) {
        return;
    }
    emit colorActivated(_entries[static_cast<std::size_t>(row)].color);
}

void PixelPalettePanel::updateButtonsEnabled() {
    const int row = _list->currentRow();
    const bool hasSelection = row >= 0;
    _removeButton->setEnabled(hasSelection);
    _renameButton->setEnabled(hasSelection);
    _moveUpButton->setEnabled(hasSelection && row > 0);
    _moveDownButton->setEnabled(hasSelection && row + 1 < _list->count());
}

}  // namespace hmi
