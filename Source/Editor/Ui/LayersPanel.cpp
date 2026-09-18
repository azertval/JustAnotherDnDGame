// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "Editor/Ui/LayersPanel.h"

#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSlider>
#include <algorithm>
#include <cmath>

#include "Core/Levels/LevelDraft.h"
#include "HMI/Localization/Localization.h"
#include "ui_LayersPanel.h"

namespace hmi {

namespace {

// Rang de couche porte par un element de liste ; la grille racine n'en a pas.
constexpr int ROOT_SLOT = -1;

[[nodiscard]] int slotData(LayerSlot slot) {
    return slot ? static_cast<int>(*slot) : ROOT_SLOT;
}

}  // namespace

LayersPanel::LayersPanel(QWidget* parent)
    : QWidget(parent), _ui(std::make_unique<Ui::LayersPanel>()) {
    _ui->setupUi(this);

    connect(_ui->layerList, &QListWidget::currentItemChanged, this,
            [this](QListWidgetItem* current, QListWidgetItem*) {
                updateButtons();
                if (_rebuilding || current == nullptr) {
                    return;
                }
                // Differe : la demande reconstruit la liste, ce qui ne peut pas se faire pendant
                // l'emission d'un de ses propres signaux.
                const LayerSlot slot = slotOf(current);
                QMetaObject::invokeMethod(
                    this, [this, slot] { emit activeLayerRequested(slot); }, Qt::QueuedConnection);
            });
    connect(_ui->layerList, &QListWidget::itemChanged, this, [this](QListWidgetItem* item) {
        if (_rebuilding || item == nullptr) {
            return;
        }
        const LayerSlot slot = slotOf(item);
        const bool visible = item->checkState() == Qt::Checked;
        const auto position = static_cast<std::size_t>(_ui->layerList->row(item));
        if (position < _snapshot.displays.size() &&
            _snapshot.displays[position].visible != visible) {
            QMetaObject::invokeMethod(
                this, [this, slot, visible] { emit visibilityRequested(slot, visible); },
                Qt::QueuedConnection);
            return;
        }
        // Sinon, c'est le nom qui a change (edition en place, couches visuelles seulement).
        if (slot && position < _snapshot.rows.size() &&
            item->text() != QString::fromStdString(_snapshot.rows[position].name)) {
            const QString name = item->text().trimmed();
            const std::size_t index = *slot;
            QMetaObject::invokeMethod(
                this, [this, index, name] { emit renameRequested(index, name); },
                Qt::QueuedConnection);
        }
    });
    connect(_ui->opacitySlider, &QSlider::valueChanged, this, [this](int value) {
        _ui->opacityValue->setText(QStringLiteral("%1 %").arg(value));
        if (_rebuilding || _ui->layerList->currentItem() == nullptr) {
            return;
        }
        emit opacityRequested(slotOf(_ui->layerList->currentItem()),
                              static_cast<float>(value) / 100.0F);
    });
    connect(_ui->addGroundButton, &QPushButton::clicked, this,
            [this] { emit addRequested(core::LayerKind::Ground); });
    connect(_ui->addDecorButton, &QPushButton::clicked, this,
            [this] { emit addRequested(core::LayerKind::Decor); });
    connect(_ui->removeButton, &QPushButton::clicked, this, [this] {
        if (const LayerSlot slot = slotOf(_ui->layerList->currentItem())) {
            emit removeRequested(*slot);
        }
    });
    connect(_ui->moveForwardButton, &QPushButton::clicked, this, [this] {
        if (const LayerSlot slot = slotOf(_ui->layerList->currentItem())) {
            emit moveRequested(*slot, /*forward=*/true);
        }
    });
    connect(_ui->moveBackwardButton, &QPushButton::clicked, this, [this] {
        if (const LayerSlot slot = slotOf(_ui->layerList->currentItem())) {
            emit moveRequested(*slot, /*forward=*/false);
        }
    });
    updateButtons();
}

LayersPanel::~LayersPanel() = default;

void LayersPanel::refresh(const core::LevelDraft& draft, LayerSlot active,
                          const LayerViewState& view) {
    std::vector<LayerRow> rows = layerRows(draft.layers());
    _hasVisualLayers = rows.size() > 1;
    // Ordre d'affichage : du dessus vers le dessous. La racine reste en tete -- dans l'editeur, la
    // collision se dessine PAR-DESSUS les couches visuelles -- puis les couches visuelles, de la
    // plus en avant a la plus en arriere.
    std::reverse(rows.begin() + 1, rows.end());

    Snapshot next;
    next.active = active;
    for (const LayerRow& row : rows) {
        next.displays.push_back(view.display(row.slot, _hasVisualLayers));
    }
    next.rows = std::move(rows);
    if (next == _snapshot && _ui->layerList->count() > 0) {
        return;
    }
    _snapshot = std::move(next);
    rebuild();
}

void LayersPanel::rebuild() {
    _rebuilding = true;
    const QSignalBlocker blockList(_ui->layerList);
    _ui->layerList->clear();
    for (std::size_t position = 0; position < _snapshot.rows.size(); ++position) {
        const LayerRow& row = _snapshot.rows[position];
        auto* const item = new QListWidgetItem(rowLabel(row), _ui->layerList);
        item->setData(Qt::UserRole, slotData(row.slot));
        Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;
        if (row.slot) {
            flags |= Qt::ItemIsEditable;
        }
        item->setFlags(flags);
        item->setCheckState(_snapshot.displays[position].visible ? Qt::Checked : Qt::Unchecked);
        QString kindLabel = rowLabel(row);
        if (row.kind == core::LayerKind::Decor) {
            kindLabel = text("layers.kind.decor", QStringLiteral("Décor"));
        } else if (row.kind == core::LayerKind::Ground) {
            kindLabel = text("layers.kind.ground", QStringLiteral("Sol"));
        }
        item->setToolTip(kindLabel + QStringLiteral(" — ") + text("layers.visible_tooltip", {}));
        if (row.slot == _snapshot.active) {
            _ui->layerList->setCurrentItem(item);
        }
    }
    _rebuilding = false;
    updateButtons();
}

void LayersPanel::updateButtons() {
    const QListWidgetItem* const current = _ui->layerList->currentItem();
    const LayerSlot slot = current != nullptr ? slotOf(current) : LayerSlot{};
    const int position = current != nullptr ? _ui->layerList->row(current) : -1;
    const int count = _ui->layerList->count();

    _ui->removeButton->setEnabled(slot.has_value());
    // La liste va du dessus vers le dessous : « monter » rapproche du rang 1 (le premier visuel).
    _ui->moveForwardButton->setEnabled(slot.has_value() && position > 1);
    _ui->moveBackwardButton->setEnabled(slot.has_value() && position >= 1 && position < count - 1);

    const bool hasRow =
        position >= 0 && static_cast<std::size_t>(position) < _snapshot.displays.size();
    _ui->opacitySlider->setEnabled(hasRow);
    const int value =
        hasRow ? static_cast<int>(std::lround(
                     _snapshot.displays[static_cast<std::size_t>(position)].opacity * 100.0F))
               : 100;
    const bool previous = _rebuilding;
    _rebuilding = true;
    _ui->opacitySlider->setValue(value);
    _ui->opacityValue->setText(QStringLiteral("%1 %").arg(value));
    _rebuilding = previous;
}

QString LayersPanel::rowLabel(const LayerRow& row) const {
    switch (row.kind) {
        case core::LayerKind::Legacy:
            return text("layers.root_legacy", QStringLiteral("Grille unique"));
        case core::LayerKind::Collision:
            return text("layers.root_collision", QStringLiteral("Collision"));
        case core::LayerKind::Ground:
        case core::LayerKind::Decor:
            break;
    }
    // Le nom est editable en place : c'est lui, et lui seul, qui porte le texte de l'element. Le
    // role se lit dans l'infobulle, pour que renommer n'ait jamais a retirer un suffixe.
    return QString::fromStdString(row.name);
}

LayerSlot LayersPanel::slotOf(const QListWidgetItem* item) {
    if (item == nullptr) {
        return std::nullopt;
    }
    const int slotValue = item->data(Qt::UserRole).toInt();
    return slotValue == ROOT_SLOT ? LayerSlot{} : LayerSlot{static_cast<std::size_t>(slotValue)};
}

QString LayersPanel::text(const char* key, const QString& fallback) const {
    return _loc != nullptr ? QString::fromStdString(_loc->text(key)) : fallback;
}

void LayersPanel::retranslateUi(const Localization& loc) {
    _loc = &loc;
    _ui->opacityLabel->setText(text("layers.opacity", {}));
    _ui->addGroundButton->setText(text("layers.add_ground", {}));
    _ui->addDecorButton->setText(text("layers.add_decor", {}));
    _ui->removeButton->setText(text("layers.remove", {}));
    _ui->moveForwardButton->setText(text("layers.move_forward", {}));
    _ui->moveBackwardButton->setText(text("layers.move_backward", {}));
    _ui->layerList->setToolTip(text("layers.active_tooltip", {}));
    rebuild();
}

}  // namespace hmi
