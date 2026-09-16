// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "HMI/Interface/EditorKeybindingsWidget.h"

#include <QFormLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QPushButton>
#include <QString>
#include <QVBoxLayout>
#include <array>
#include <utility>

#include "HMI/Input/KeyName.h"
#include "HMI/Input/QtKeyMap.h"
#include "HMI/Interface/DesignTokens.h"
#include "HMI/Localization/Localization.h"

namespace hmi {

namespace {

// Clés de traduction des actions d'éditeur (même ordre que `hmi::EditorAction`).
constexpr std::array<const char*, hmi::EDITOR_ACTION_COUNT> ACTION_KEYS{
    "keybindings.action.save",        "keybindings.action.undo",  "keybindings.action.redo",
    "keybindings.action.copy",        "keybindings.action.paste", "keybindings.action.playtest",
    "keybindings.action.grid",        "keybindings.action.help",  "keybindings.action.rename",
    "keybindings.action.texture_tool"};

[[nodiscard]] hmi::EditorAction actionAt(int index) {
    return static_cast<hmi::EditorAction>(index);
}

}  // namespace

EditorKeybindingsWidget::EditorKeybindingsWidget(hmi::EditorKeyBindings& bindings,
                                                 std::filesystem::path savePath, QWidget* parent)
    : QWidget(parent), _bindings(bindings), _savePath(std::move(savePath)) {
    // Capte les touches même sans clic préalable dans un champ (pour la capture de remappage).
    setFocusPolicy(Qt::StrongFocus);

    auto* const form = new QFormLayout();
    for (int index = 0; index < hmi::EDITOR_ACTION_COUNT; ++index) {
        auto* const button = new QPushButton(this);
        // Largeur minimale unifiee avec KeybindingsWidget/GamepadBindingsWidget (LOT-56, jeton
        // controlMinWidth) : trois widgets jumeaux affichant la meme colonne de boutons.
        button->setMinimumWidth(editorDarkTokens().size.controlMinWidth);
        connect(button, &QPushButton::clicked, this, [this, index] {
            _capturing = index;
            _buttons[static_cast<std::size_t>(index)]->setText(
                _loc != nullptr ? QString::fromStdString(_loc->text("keybindings.press_key"))
                                : QStringLiteral("…"));
            setFocus();  // capter la prochaine touche via keyPressEvent
        });
        _buttons[static_cast<std::size_t>(index)] = button;
        auto* const label = new QLabel(this);
        _actionLabels[static_cast<std::size_t>(index)] = label;
        form->addRow(label, button);
    }

    auto* const layout = new QVBoxLayout(this);
    layout->addLayout(form);
    _help = new QLabel(this);
    _help->setWordWrap(true);
    layout->addWidget(_help);
    layout->addStretch();

    refresh();
}

void EditorKeybindingsWidget::refresh() {
    for (int index = 0; index < hmi::EDITOR_ACTION_COUNT; ++index) {
        _buttons[static_cast<std::size_t>(index)]->setText(
            QString::fromStdString(hmi::keyDisplayName(_bindings.key(actionAt(index)))));
    }
}

void EditorKeybindingsWidget::retranslateUi(const Localization& loc) {
    _loc = &loc;
    for (int index = 0; index < hmi::EDITOR_ACTION_COUNT; ++index) {
        _actionLabels[static_cast<std::size_t>(index)]->setText(
            QString::fromStdString(loc.text(ACTION_KEYS[static_cast<std::size_t>(index)])));
    }
    _help->setText(QString::fromStdString(loc.text("keybindings.key_hint")));
    refresh();  // les libellés de touches sont indépendants de la langue ; rétablit l'état courant.
}

void EditorKeybindingsWidget::keyPressEvent(QKeyEvent* event) {
    if (_capturing < 0) {
        QWidget::keyPressEvent(event);
        return;
    }
    if (event->key() == Qt::Key_Escape) {
        _capturing = -1;  // annule la capture
        refresh();
        return;
    }
    if (const std::optional<hmi::Key> key = qtKeyToHmiKey(event->key())) {
        _bindings.setKey(actionAt(_capturing), *key);
        static_cast<void>(_bindings.save(_savePath));  // persistance (échec non bloquant)
        _capturing = -1;
        refresh();
        emit bindingsChanged();
    }
}

}  // namespace hmi
