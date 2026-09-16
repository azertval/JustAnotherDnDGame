// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "HMI/Editor/PlanesPanel.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <array>

#include "Core/Levels/LevelDraft.h"
#include "HMI/Graphics/Parallax.h"
#include "HMI/Localization/Localization.h"
#include "ui_PlanesPanel.h"

namespace hmi {

namespace {

// Densites proposees, dans l'ordre de la liste deroulante. Meme jeu que le format (EX-DEC-041) :
// une valeur en plus ici serait refusee au chargement, donc invisible jusqu'a la relecture.
constexpr std::array<int, 3> DENSITIES = {core::PLANE_NATIVE_PIXELS_PER_UNIT, 8, 4};

// Rang d'un plan porte par l'element de liste : l'ordre visuel EST l'ordre du modele, mais le lire
// depuis la donnee plutot que depuis la position evite de dependre d'un tri involontaire.
constexpr int RANK_ROLE = Qt::UserRole;

}  // namespace

PlanesPanel::PlanesPanel(QWidget* parent) : QWidget(parent), _ui(new Ui::PlanesPanel) {
    _ui->setupUi(this);
    for (const int density : DENSITIES) {
        _ui->densityCombo->addItem(QString::number(density), density);
    }
    _ui->depthCombo->addItem(QString{}, static_cast<int>(core::PlaneDepth::Behind));
    _ui->depthCombo->addItem(QString{}, static_cast<int>(core::PlaneDepth::Front));
    connectControls();
    refreshSettings(core::LevelDraft::empty("", 1, 1), std::nullopt);
}

PlanesPanel::~PlanesPanel() {
    delete _ui;
}

std::optional<std::size_t> PlanesPanel::selectedIndex() const {
    const QTreeWidgetItem* const item = _ui->planeTree->currentItem();
    if (item == nullptr) {
        return std::nullopt;
    }
    const QVariant rank = item->data(0, RANK_ROLE);
    if (!rank.isValid()) {
        return std::nullopt;
    }
    return static_cast<std::size_t>(rank.toULongLong());
}

void PlanesPanel::connectControls() {
    connect(_ui->planeTree, &QTreeWidget::currentItemChanged, this, [this] {
        if (_refreshing) {
            return;
        }
        emit planeSelected(selectedIndex());
    });
    // Case a cocher de la colonne « Visible » : aide d'edition, jamais un pas d'annulation.
    connect(_ui->planeTree, &QTreeWidget::itemChanged, this, [this](QTreeWidgetItem* item, int) {
        if (_refreshing || item == nullptr) {
            return;
        }
        const auto rank = static_cast<std::size_t>(item->data(0, RANK_ROLE).toULongLong());
        emit visibilityToggled(rank, item->checkState(3) == Qt::Checked);
    });

    connect(_ui->addButton, &QPushButton::clicked, this, [this] { emit addRequested(); });
    const auto withSelection = [this](auto&& action) {
        return [this, action] {
            if (const std::optional<std::size_t> index = selectedIndex()) {
                action(*index);
            }
        };
    };
    connect(_ui->removeButton, &QPushButton::clicked, this,
            withSelection([this](std::size_t index) { emit removeRequested(index); }));
    connect(_ui->paintButton, &QPushButton::clicked, this,
            withSelection([this](std::size_t index) { emit paintRequested(index); }));
    connect(_ui->forwardButton, &QPushButton::clicked, this,
            withSelection([this](std::size_t index) { emit reorderRequested(index, true); }));
    connect(_ui->backwardButton, &QPushButton::clicked, this,
            withSelection([this](std::size_t index) { emit reorderRequested(index, false); }));
    connect(_ui->frontButton, &QPushButton::clicked, this, withSelection([this](std::size_t index) {
                emit depthChangeRequested(index, core::PlaneDepth::Front);
            }));
    connect(_ui->behindButton, &QPushButton::clicked, this,
            withSelection([this](std::size_t index) {
                emit depthChangeRequested(index, core::PlaneDepth::Behind);
            }));
    connect(_ui->isolateButton, &QPushButton::toggled, this, [this](bool checked) {
        if (_refreshing) {
            return;
        }
        emit isolateToggled(selectedIndex().value_or(0), checked);
    });
    connectSettingControls();
}

void PlanesPanel::connectSettingControls() {
    connect(_ui->densityCombo, &QComboBox::currentIndexChanged, this, [this](int) {
        if (_refreshing) {
            return;
        }
        if (const std::optional<std::size_t> index = selectedIndex()) {
            emit densityChangeRequested(*index, _ui->densityCombo->currentData().toInt());
        }
    });
    connect(_ui->depthCombo, &QComboBox::currentIndexChanged, this, [this](int) {
        if (_refreshing) {
            return;
        }
        if (const std::optional<std::size_t> index = selectedIndex()) {
            emit depthChangeRequested(
                *index, static_cast<core::PlaneDepth>(_ui->depthCombo->currentData().toInt()));
        }
    });

    // editingFinished, jamais valueChanged : taper « 0.75 » empilerait sinon quatre pas
    // d'annulation pour un seul geste (lecon du LOT-67).
    const auto emitParallax = [this] {
        if (_refreshing) {
            return;
        }
        if (const std::optional<std::size_t> index = selectedIndex()) {
            emit parallaxChangeRequested(*index, static_cast<float>(_ui->parallaxXSpin->value()),
                                         static_cast<float>(_ui->parallaxYSpin->value()));
        }
    };
    connect(_ui->parallaxXSpin, &QDoubleSpinBox::editingFinished, this, emitParallax);
    connect(_ui->parallaxYSpin, &QDoubleSpinBox::editingFinished, this, emitParallax);
    connect(_ui->opacitySpin, &QDoubleSpinBox::editingFinished, this, [this] {
        if (_refreshing) {
            return;
        }
        if (const std::optional<std::size_t> index = selectedIndex()) {
            emit opacityChangeRequested(*index, static_cast<float>(_ui->opacitySpin->value()));
        }
    });
    connect(_ui->parallaxCheck, &QCheckBox::toggled, this, [this](bool checked) {
        if (_refreshing) {
            return;
        }
        emit levelParallaxToggled(checked);
    });
}

void PlanesPanel::refresh(const core::LevelDraft& draft, std::optional<std::size_t> selected,
                          const PlaneVisibility& visibility) {
    _refreshing = true;
    _framingMode = draft.cameraFraming().mode;
    rebuildTree(draft, selected, visibility);
    refreshSettings(draft, selected);

    _ui->parallaxCheck->setChecked(draft.parallaxEnabled());
    // Grisee dans le mode ou le moteur neutralise la parallaxe (EX-DEC-043) : cocher la case n'y
    // produirait rien de visible, ce qui se lirait comme un defaut plutot que comme une regle.
    const bool parallaxUsable = planeParallaxActive(_framingMode, true);
    _ui->parallaxCheck->setEnabled(parallaxUsable);
    _ui->parallaxHint->setVisible(!parallaxUsable);

    _ui->isolateButton->setChecked(visibility.isolatedRank().has_value());
    _refreshing = false;
}

void PlanesPanel::rebuildTree(const core::LevelDraft& draft, std::optional<std::size_t> selected,
                              const PlaneVisibility& visibility) {
    _ui->planeTree->clear();
    const std::vector<core::Plane>& planes = draft.planes();
    for (std::size_t rank = 0; rank < planes.size(); ++rank) {
        const core::Plane& plane = planes[rank];
        auto* const item = new QTreeWidgetItem(_ui->planeTree);
        item->setText(0, QString::fromStdString(plane.fileName));
        item->setText(1, QString::number(plane.pixelsPerUnit));
        item->setText(2, plane.depth == core::PlaneDepth::Front ? tr("Devant") : tr("Derrière"));
        item->setData(0, RANK_ROLE, static_cast<qulonglong>(rank));
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(3, visibility.visible(rank) ? Qt::Checked : Qt::Unchecked);
        if (selected && *selected == rank) {
            _ui->planeTree->setCurrentItem(item);
        }
    }
}

void PlanesPanel::refreshSettings(const core::LevelDraft& draft,
                                  std::optional<std::size_t> selected) {
    const std::vector<core::Plane>& planes = draft.planes();
    const bool hasSelection = selected && *selected < planes.size();
    _ui->settingsGroup->setEnabled(hasSelection);
    _ui->removeButton->setEnabled(hasSelection);
    _ui->paintButton->setEnabled(hasSelection);
    _ui->forwardButton->setEnabled(hasSelection);
    _ui->backwardButton->setEnabled(hasSelection);
    _ui->frontButton->setEnabled(hasSelection);
    _ui->behindButton->setEnabled(hasSelection);
    if (!hasSelection) {
        // Champs ramenes aux valeurs par defaut d'un plan, pas laisses sur ce qu'ils affichaient.
        // Grises, ils restent lisibles : y laisser « Parallaxe 0,01 · Opacite 0,00 » -- les bornes
        // basses des champs -- se lit comme une configuration que quelqu'un aurait choisie.
        const core::Plane defaults;
        _ui->densityCombo->setCurrentIndex(_ui->densityCombo->findData(defaults.pixelsPerUnit));
        _ui->depthCombo->setCurrentIndex(
            _ui->depthCombo->findData(static_cast<int>(defaults.depth)));
        _ui->parallaxXSpin->setValue(static_cast<double>(defaults.parallaxX));
        _ui->parallaxYSpin->setValue(static_cast<double>(defaults.parallaxY));
        _ui->opacitySpin->setValue(static_cast<double>(defaults.opacity));
        return;
    }

    const core::Plane& plane = planes[*selected];
    _ui->densityCombo->setCurrentIndex(_ui->densityCombo->findData(plane.pixelsPerUnit));
    _ui->depthCombo->setCurrentIndex(_ui->depthCombo->findData(static_cast<int>(plane.depth)));
    _ui->parallaxXSpin->setValue(static_cast<double>(plane.parallaxX));
    _ui->parallaxYSpin->setValue(static_cast<double>(plane.parallaxY));
    _ui->opacitySpin->setValue(static_cast<double>(plane.opacity));
}

void PlanesPanel::retranslateUi(const Localization& loc) {
    const auto text = [&loc](const char* key) { return QString::fromStdString(loc.text(key)); };
    _ui->planeTree->setHeaderLabels({text("planes.column_file"), text("planes.column_density"),
                                     text("planes.column_depth"), text("planes.column_visible")});
    _ui->addButton->setText(text("planes.add"));
    _ui->removeButton->setText(text("planes.remove"));
    _ui->paintButton->setText(text("planes.paint"));
    _ui->isolateButton->setText(text("planes.isolate"));
    _ui->forwardButton->setText(text("planes.forward"));
    _ui->backwardButton->setText(text("planes.backward"));
    _ui->frontButton->setText(text("planes.front"));
    _ui->behindButton->setText(text("planes.behind"));
    _ui->settingsGroup->setTitle(text("planes.settings"));
    _ui->densityLabel->setText(text("planes.density"));
    _ui->depthLabel->setText(text("planes.depth"));
    _ui->parallaxXLabel->setText(text("planes.parallax_x"));
    _ui->parallaxYLabel->setText(text("planes.parallax_y"));
    _ui->opacityLabel->setText(text("planes.opacity"));
    _ui->levelGroup->setTitle(text("planes.level_group"));
    _ui->parallaxCheck->setText(text("planes.parallax_enabled"));
    _ui->parallaxHint->setText(text("planes.parallax_whole_level_hint"));
    _ui->depthCombo->setItemText(0, text("planes.depth_behind"));
    _ui->depthCombo->setItemText(1, text("planes.depth_front"));
}

}  // namespace hmi
