// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "Editor/Ui/EditorActions.h"

#include <QAction>
#include <QActionGroup>
#include <QKeyCombination>
#include <QKeySequence>
#include <QSignalBlocker>
#include <QString>
#include <QToolBar>

#include "Editor/Logic/EditorKeyBindings.h"
#include "HMI/Input/QtKeyMap.h"
#include "Editor/Logic/DesignTokens.h"
#include "Editor/Ui/ThemeIcons.h"
#include "HMI/Localization/Localization.h"

namespace hmi {

namespace {

// Le modificateur Ctrl de Save/Undo/Redo/Copy/Paste reste cable en dur (EditorKeyBindings.h) :
// seule la touche-lettre associee est remappable. Les autres actions n'ont pas de modificateur.
[[nodiscard]] bool carriesImplicitCtrl(EditorAction action) {
    switch (action) {
        case EditorAction::Save:
        case EditorAction::Undo:
        case EditorAction::Redo:
        case EditorAction::Copy:
        case EditorAction::Paste:
            return true;
        default:
            return false;
    }
}

}  // namespace

EditorActions::EditorActions(const DesignTokens& tokens, QObject* parent)
    : QObject(parent), _toolGroup(new QActionGroup(this)) {
    _toolGroup->setExclusive(true);

    for (std::size_t i = 0; i < editorActionCatalog().size(); ++i) {
        const EditorActionSpec& spec = editorActionCatalog()[i];
        auto* const act = new QAction(this);
        act->setIcon(themeIcon(spec.id, tokens.size.iconMedium, tokens));
        act->setCheckable(spec.checkable);
        if (spec.shortcut[0] != '\0') {
            act->setShortcut(QKeySequence(QString::fromLatin1(spec.shortcut)));
        }
        if (spec.group == EditorActionGroup::LevelTools) {
            act->setActionGroup(_toolGroup);
        }
        _actions[i] = act;
    }
    // Pinceau actif par défaut (EX-EDIT-014).
    action(IconId::ToolPaint)->setChecked(true);
}

QAction* EditorActions::action(IconId id) const {
    const std::array<EditorActionSpec, EDITOR_ACTION_CATALOG_COUNT>& catalog =
        editorActionCatalog();
    for (std::size_t i = 0; i < catalog.size(); ++i) {
        if (catalog[i].id == id) {
            return _actions[i];
        }
    }
    return nullptr;
}

QAction* EditorActions::toolAction(EditorTool tool) const {
    return action(editorActionForTool(tool));
}

void EditorActions::populateToolBar(QToolBar& toolBar) const {
    // Outils de niveau puis commandes. Le filtre sur `surface` (EX-IHM-074) est ce qui ramene la barre a l'essentiel : les
    // commandes ponctuelles (grille, recadrage, copier/coller, renommer, apercu des raccourcis)
    // restent atteignables par le menu et leur raccourci, sans occuper l'ecran en permanence.
    bool separatorInserted = false;
    for (const EditorActionSpec& spec : editorActionCatalog()) {
        if (spec.surface != ActionSurface::ToolBarAndMenu) {
            continue;
        }
        if (!separatorInserted && spec.group != EditorActionGroup::LevelTools) {
            toolBar.addSeparator();
            separatorInserted = true;
        }
        toolBar.addAction(action(spec.id));
    }
}

void EditorActions::retranslateUi(const Localization& loc) {
    const std::array<EditorActionSpec, EDITOR_ACTION_CATALOG_COUNT>& catalog =
        editorActionCatalog();
    for (std::size_t i = 0; i < catalog.size(); ++i) {
        const QString label = QString::fromStdString(loc.text(catalog[i].labelKey));
        _actions[i]->setText(label);
        // Infobulle = libelle + raccourci de l'ACTION elle-meme (jamais une chaine traduite
        // decrivant la touche) : le remappage ne peut donc jamais la rendre fausse.
        const QKeySequence shortcut = _actions[i]->shortcut();
        _actions[i]->setToolTip(shortcut.isEmpty()
                                    ? label
                                    : label + QStringLiteral(" (") +
                                          shortcut.toString(QKeySequence::NativeText) +
                                          QStringLiteral(")"));
    }
}

void EditorActions::setActiveTool(EditorTool tool) const {
    QAction* const act = toolAction(tool);
    if (act == nullptr || act->isChecked()) {
        return;
    }
    const QSignalBlocker blocker(act);
    act->setChecked(true);
}

void EditorActions::setEditingCommandsEnabled(bool enabled) const {
    for (const EditorActionSpec& spec : editorActionCatalog()) {
        if (spec.group == EditorActionGroup::None) {
            action(spec.id)->setEnabled(enabled);
        }
    }
}

void EditorActions::refreshIcons(const DesignTokens& tokens) {
    const std::array<EditorActionSpec, EDITOR_ACTION_CATALOG_COUNT>& catalog =
        editorActionCatalog();
    for (std::size_t i = 0; i < catalog.size(); ++i) {
        _actions[i]->setIcon(themeIcon(catalog[i].id, tokens.size.iconMedium, tokens));
    }
}

void EditorActions::applyShortcuts(const EditorKeyBindings& bindings, const Localization& loc) {
    for (const KeyBindingIconEntry& entry : keyBindingIconCatalog()) {
        const auto qtKey = static_cast<Qt::Key>(hmiKeyToQtKey(bindings.key(entry.action)));
        const Qt::KeyboardModifiers modifiers =
            carriesImplicitCtrl(entry.action) ? Qt::ControlModifier : Qt::NoModifier;
        action(entry.id)->setShortcut(QKeySequence(QKeyCombination(modifiers, qtKey)));
    }
    retranslateUi(loc);  // les infobulles incluent le raccourci : les refaire pour rester exactes.
}

}  // namespace hmi
