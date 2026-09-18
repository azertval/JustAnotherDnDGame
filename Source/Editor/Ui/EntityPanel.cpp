// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "Editor/Ui/EntityPanel.h"

#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QLabel>
#include <QLayoutItem>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <cstdint>
#include <limits>
#include <variant>

#include "Core/Levels/LevelDraft.h"
#include "Editor/Logic/EntityReferences.h"
#include "HMI/Localization/Localization.h"
#include "ui_EntityPanel.h"

namespace hmi {

namespace {

// Bornes du champ entier : un rang d'arene ou un compteur, jamais une coordonnee.
constexpr int INTEGER_MINIMUM = -9999;
constexpr int INTEGER_MAXIMUM = 9999;

[[nodiscard]] bool sameEntity(const core::MapEntity& a, const core::MapEntity& b) {
    return a.type == b.type && a.position == b.position && a.properties == b.properties;
}

[[nodiscard]] QString valueText(const core::PropertyValue& value) {
    if (const auto* const flag = std::get_if<bool>(&value)) {
        return *flag ? QStringLiteral("true") : QStringLiteral("false");
    }
    if (const auto* const text = std::get_if<std::string>(&value)) {
        return QString::fromStdString(*text);
    }
    if (const auto* const integer = std::get_if<std::int64_t>(&value)) {
        return QString::number(*integer);
    }
    return QString::number(std::get<double>(value));
}

[[nodiscard]] QString cellText(core::GridPosition position) {
    return QStringLiteral("(%1, %2)").arg(position.column).arg(position.row);
}

}  // namespace

EntityPanel::EntityPanel(QWidget* parent)
    : QWidget(parent), _ui(std::make_unique<Ui::EntityPanel>()), _form(nullptr) {
    _ui->setupUi(this);
    // Le conteneur du formulaire n'existe qu'apres setupUi : pas d'initialisation de membre.
    // NOLINTNEXTLINE(cppcoreguidelines-prefer-member-initializer)
    _form = new QFormLayout(_ui->propertiesForm);
    _form->setContentsMargins(0, 0, 0, 0);

    _ui->entityTable->horizontalHeader()->setStretchLastSection(true);
    _ui->entityTable->verticalHeader()->setVisible(false);

    connect(_ui->kindCombo, &QComboBox::currentIndexChanged, this, [this](int) {
        if (!_rebuilding) {
            emit kindToPlaceChanged(QString::fromStdString(kindToPlace()));
        }
    });
    connect(_ui->entityTable->selectionModel(), &QItemSelectionModel::selectionChanged, this,
            [this] {
                if (_rebuilding) {
                    return;
                }
                const QModelIndexList rows = _ui->entityTable->selectionModel()->selectedRows();
                emit entitySelected(rows.isEmpty() ? std::nullopt
                                                   : std::make_optional(static_cast<std::size_t>(
                                                         rows.constFirst().row())));
            });
    connect(_ui->removeButton, &QPushButton::clicked, this, [this] {
        if (_selected) {
            emit removeRequested(*_selected);
        }
    });
    connect(_ui->warningList, &QListWidget::itemActivated, this, [this](QListWidgetItem* item) {
        if (item != nullptr) {
            emit entitySelected(static_cast<std::size_t>(item->data(Qt::UserRole).toULongLong()));
        }
    });
    rebuildKinds();
}

EntityPanel::~EntityPanel() = default;

std::string EntityPanel::kindToPlace() const {
    return _ui->kindCombo->currentData().toString().toStdString();
}

void EntityPanel::refresh(const core::LevelDraft& draft, std::optional<std::size_t> selected,
                          const core::EntityReferenceContext& context,
                          const std::vector<EditorDiagnostic>& diagnostics) {
    _entities = draft.entities();
    _selected = selected && *selected < _entities.size() ? selected : std::nullopt;
    _context = context;
    _diagnostics = diagnostics;
    rebuildTable();
    rebuildForm();
    rebuildWarnings();
}

void EntityPanel::rebuildKinds() {
    _rebuilding = true;
    const QString current = _ui->kindCombo->currentData().toString();
    _ui->kindCombo->clear();
    _ui->kindCombo->addItem(text("entities.select_only", QStringLiteral("(sélection seule)")),
                            QString{});
    for (const core::EntityKind& kind : core::knownEntityKinds()) {
        const QString type =
            QString::fromUtf8(kind.type.data(), static_cast<qsizetype>(kind.type.size()));
        _ui->kindCombo->addItem(kindLabel(type.toStdString()), type);
    }
    const int index = _ui->kindCombo->findData(current);
    _ui->kindCombo->setCurrentIndex(index >= 0 ? index : 0);
    _rebuilding = false;
}

void EntityPanel::rebuildTable() {
    _rebuilding = true;
    const QSignalBlocker block(_ui->entityTable);
    _ui->entityTable->clearContents();
    _ui->entityTable->setRowCount(static_cast<int>(_entities.size()));
    for (std::size_t row = 0; row < _entities.size(); ++row) {
        const core::MapEntity& entity = _entities[row];
        _ui->entityTable->setItem(static_cast<int>(row), 0,
                                  new QTableWidgetItem(kindLabel(entity.type)));
        _ui->entityTable->setItem(static_cast<int>(row), 1,
                                  new QTableWidgetItem(cellText(entity.position)));
    }
    _ui->entityTable->clearSelection();
    if (_selected) {
        _ui->entityTable->selectRow(static_cast<int>(*_selected));
    }
    _rebuilding = false;
}

void EntityPanel::rebuildForm() {
    const core::MapEntity* const entity = _selected ? &_entities[*_selected] : nullptr;
    const core::EntityKind* const kind =
        entity != nullptr ? core::findEntityKind(entity->type) : nullptr;

    std::vector<std::vector<std::string>> choices;
    if (kind != nullptr) {
        for (const core::EntityPropertySpec& spec : kind->properties) {
            choices.push_back(entityChoices(spec, *entity, _context));
        }
    }
    const bool unchanged =
        _formIndex == _selected && _formChoices == choices &&
        ((entity == nullptr && !_formEntity) ||
         (entity != nullptr && _formEntity && sameEntity(*entity, *_formEntity)));
    if (unchanged && _form->rowCount() > 0) {
        return;
    }
    _formIndex = _selected;
    _formEntity = entity != nullptr ? std::make_optional(*entity) : std::nullopt;
    _formChoices = choices;

    clearForm();
    _ui->removeButton->setEnabled(entity != nullptr);
    if (entity == nullptr) {
        _ui->selectionLabel->setText(text("entities.no_selection", {}));
        return;
    }
    const std::size_t index = *_selected;
    _ui->selectionLabel->setText(
        kind != nullptr ? kindLabel(entity->type) + QStringLiteral(" ") + cellText(entity->position)
                        : text("entities.unknown_kind", QStringLiteral("%1"))
                              .arg(QString::fromStdString(entity->type)));

    if (kind != nullptr) {
        for (std::size_t specIndex = 0; specIndex < kind->properties.size(); ++specIndex) {
            const core::EntityPropertySpec& spec = kind->properties[specIndex];
            const auto found = entity->properties.find(std::string{spec.key});
            addPropertyRow(index, spec,
                           found != entity->properties.end() ? found->second : spec.defaultValue,
                           choices[specIndex]);
        }
    }
    // Proprietes que la table ne declare pas : transportees, montrees, jamais editees ici.
    for (const auto& [key, value] : entity->properties) {
        if (kind != nullptr && kind->find(key) != nullptr) {
            continue;
        }
        auto* const shown = new QLabel(valueText(value), _ui->propertiesForm);
        shown->setTextInteractionFlags(Qt::TextSelectableByMouse);
        _form->addRow(QString::fromStdString(key), shown);
    }
}

void EntityPanel::clearForm() {
    // Les champs sont retires puis detruits APRES le retour a la boucle d'evenements : une
    // reconstruction declenchee par le `editingFinished` d'un champ du formulaire se deroule
    // pendant l'emission de son signal, et `removeRow` le detruirait sous ses pieds.
    while (_form->rowCount() > 0) {
        const QFormLayout::TakeRowResult taken = _form->takeRow(0);
        for (QLayoutItem* const item : {taken.labelItem, taken.fieldItem}) {
            if (item == nullptr) {
                continue;
            }
            if (QWidget* const widget = item->widget()) {
                widget->hide();
                widget->deleteLater();
            }
            delete item;
        }
    }
}

void EntityPanel::addPropertyRow(std::size_t index, const core::EntityPropertySpec& spec,
                                 const core::PropertyValue& value,
                                 const std::vector<std::string>& choices) {
    const QString key = QString::fromUtf8(spec.key.data(), static_cast<qsizetype>(spec.key.size()));
    const QString label = propertyLabel(key.toStdString());
    switch (spec.kind) {
        case core::EntityPropertyKind::Choice: {
            addChoiceRow(index, spec, value, choices);
            break;
        }
        case core::EntityPropertyKind::Text: {
            auto* const edit = new QLineEdit(valueText(value), _ui->propertiesForm);
            connect(edit, &QLineEdit::editingFinished, this, [this, index, key, edit] {
                emit propertyChanged(index, key,
                                     core::PropertyValue{edit->text().trimmed().toStdString()});
            });
            _form->addRow(label, edit);
            break;
        }
        case core::EntityPropertyKind::Integer: {
            auto* const spin = new QSpinBox(_ui->propertiesForm);
            spin->setRange(INTEGER_MINIMUM, INTEGER_MAXIMUM);
            const auto* const held = std::get_if<std::int64_t>(&value);
            spin->setValue(held != nullptr ? static_cast<int>(*held) : 0);
            connect(spin, &QSpinBox::editingFinished, this, [this, index, key, spin] {
                emit propertyChanged(index, key, core::PropertyValue{std::int64_t{spin->value()}});
            });
            _form->addRow(label, spin);
            break;
        }
        case core::EntityPropertyKind::Boolean: {
            auto* const check = new QCheckBox(_ui->propertiesForm);
            const auto* const held = std::get_if<bool>(&value);
            check->setChecked(held != nullptr && *held);
            connect(check, &QCheckBox::toggled, this, [this, index, key](bool checked) {
                emit propertyChanged(index, key, core::PropertyValue{checked});
            });
            _form->addRow(label, check);
            break;
        }
    }
}

void EntityPanel::addChoiceRow(std::size_t index, const core::EntityPropertySpec& spec,
                               const core::PropertyValue& value,
                               const std::vector<std::string>& choices) {
    const QString key = QString::fromUtf8(spec.key.data(), static_cast<qsizetype>(spec.key.size()));
    const QString label = propertyLabel(key.toStdString());
    auto* const combo = new QComboBox(_ui->propertiesForm);
    // Editable : l'auteur peut nommer une cible qui n'existe pas encore -- la carte
    // qu'il ecrira ensuite. L'avertissement le lui rappellera.
    combo->setEditable(spec.source != core::EntityChoiceSource::Fixed);
    if (!spec.required || spec.source != core::EntityChoiceSource::Fixed) {
        combo->addItem(text("entities.none", QStringLiteral("(aucun)")), QString{});
    }
    for (const std::string& choice : choices) {
        combo->addItem(QString::fromStdString(choice), QString::fromStdString(choice));
    }
    const QString current = valueText(value);
    int currentIndex = combo->findData(current);
    if (currentIndex < 0 && !current.isEmpty()) {
        combo->addItem(current, current);
        currentIndex = combo->count() - 1;
    }
    combo->setCurrentIndex((std::max)(currentIndex, 0));
    const auto commit = [this, index, key, combo] {
        const QString chosen =
            combo->currentIndex() >= 0 &&
                    combo->currentText() == combo->itemText(combo->currentIndex())
                ? combo->currentData().toString()
                : combo->currentText().trimmed();
        emit propertyChanged(index, key, core::PropertyValue{chosen.toStdString()});
    };
    connect(combo, &QComboBox::activated, this, [commit](int) { commit(); });
    if (combo->isEditable()) {
        connect(combo->lineEdit(), &QLineEdit::editingFinished, this, commit);
    }
    _form->addRow(label, combo);
}

void EntityPanel::rebuildWarnings() {
    _ui->warningList->clear();
    if (_diagnostics.empty()) {
        auto* const none =
            new QListWidgetItem(text("entities.warnings_none", {}), _ui->warningList);
        none->setFlags(Qt::ItemIsEnabled);
        return;
    }
    for (const EditorDiagnostic& diagnostic : _diagnostics) {
        QString message = text(diagnostic.key.c_str(), QString::fromStdString(diagnostic.key));
        for (std::size_t position = 0; position < diagnostic.args.size(); ++position) {
            // Le premier argument d'une reference est le type d'entite : il se lit traduit. Les
            // autres sont des identifiants de donnee, montres tels quels.
            const std::string& argument = diagnostic.args[position];
            const bool entityType =
                diagnostic.kind == EditorDiagnosticKind::Reference && position == 0;
            message =
                message.arg(entityType ? kindLabel(argument) : QString::fromStdString(argument));
        }
        auto* const item = new QListWidgetItem(
            cellText(diagnostic.cell) + QStringLiteral(" ") + message, _ui->warningList);
        item->setData(Qt::UserRole, QVariant::fromValue<qulonglong>(diagnostic.entityIndex));
    }
}

QString EntityPanel::text(const char* key, const QString& fallback) const {
    return _loc != nullptr ? QString::fromStdString(_loc->text(key)) : fallback;
}

QString EntityPanel::kindLabel(const std::string& type) const {
    if (core::findEntityKind(type) == nullptr) {
        return QString::fromStdString(type);
    }
    const std::string key = "entities.kind." + type;
    return text(key.c_str(), QString::fromStdString(type));
}

QString EntityPanel::propertyLabel(const std::string& key) const {
    const std::string localizationKey = "entities.property." + key;
    return text(localizationKey.c_str(), QString::fromStdString(key));
}

void EntityPanel::retranslateUi(const Localization& loc) {
    _loc = &loc;
    _ui->placeLabel->setText(text("entities.place", {}));
    _ui->propertiesBox->setTitle(text("entities.properties", {}));
    _ui->removeButton->setText(text("entities.remove", {}));
    _ui->warningsBox->setTitle(text("entities.warnings", {}));
    _ui->entityTable->setHorizontalHeaderLabels(
        {text("entities.column.type", {}), text("entities.column.cell", {})});
    rebuildKinds();
    rebuildTable();
    _formIndex.reset();
    _formEntity.reset();
    rebuildForm();
    rebuildWarnings();
}

}  // namespace hmi
