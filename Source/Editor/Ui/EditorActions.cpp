// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "Editor/Ui/EditorActions.h"

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QKeyCombination>
#include <QKeySequence>
#include <QSignalBlocker>
#include <QString>
#include <QStyle>
#include <QToolBar>

#include "Editor/Logic/EditorKeyBindings.h"
#include "HMI/Input/QtKeyMap.h"

namespace hmi {

namespace {

/// Description d'une commande : de quoi construire son `QAction`.
struct CommandSpec {
    EditorCommand command;
    const char* label;
    /// Raccourci par défaut (`QKeySequence`), vide si aucun. Celui d'une commande remappable est
    /// remplacé par `applyShortcuts`.
    const char* shortcut;
    /// Icône standard du style, ou `QStyle::SP_CustomBase` pour un simple libellé.
    QStyle::StandardPixmap icon;
    /// Dans la barre d'outils (usage continu), ou au menu seul.
    bool onToolBar;
    /// Commande remappable qui la déclenche, s'il y en a une.
    std::optional<EditorAction> binding;
};

constexpr QStyle::StandardPixmap TEXT_ONLY = QStyle::SP_CustomBase;

const std::array<CommandSpec, EDITOR_COMMAND_COUNT>& commandSpecs() {
    static const std::array<CommandSpec, EDITOR_COMMAND_COUNT> specs{{
        {EditorCommand::ToolPaint, "Brush", "", TEXT_ONLY, true, std::nullopt},
        {EditorCommand::ToolRectangle, "Rectangle", "", TEXT_ONLY, true, std::nullopt},
        {EditorCommand::ToolSelection, "Selection", "", TEXT_ONLY, true, std::nullopt},
        {EditorCommand::ToolEntity, "Entity", "", TEXT_ONLY, true, std::nullopt},
        {EditorCommand::Save, "Save", "Ctrl+S", QStyle::SP_DialogSaveButton, true,
         EditorAction::Save},
        {EditorCommand::Playtest, "Playtest", "P", QStyle::SP_MediaPlay, true,
         EditorAction::Playtest},
        {EditorCommand::Undo, "Undo", "Ctrl+Z", QStyle::SP_ArrowBack, true, EditorAction::Undo},
        {EditorCommand::Redo, "Redo", "Ctrl+Y", QStyle::SP_ArrowForward, true, EditorAction::Redo},
        {EditorCommand::ToggleGrid, "Grid", "F10", TEXT_ONLY, false, EditorAction::ToggleGrid},
        {EditorCommand::ResetCamera, "Reset camera", "0", TEXT_ONLY, false, std::nullopt},
        {EditorCommand::Copy, "Copy", "Ctrl+C", TEXT_ONLY, false, EditorAction::Copy},
        {EditorCommand::Paste, "Paste", "Ctrl+V", TEXT_ONLY, false, EditorAction::Paste},
        {EditorCommand::Rename, "Rename", "F2", TEXT_ONLY, false, EditorAction::Rename},
        {EditorCommand::ShortcutsOverview, "Shortcuts overview", "F1", TEXT_ONLY, false,
         EditorAction::ToggleHelp},
    }};
    return specs;
}

// Le modificateur Ctrl de Save/Undo/Redo/Copy/Paste reste câblé en dur (EditorKeyBindings.h) :
// seule la touche-lettre associée est remappable.
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

[[nodiscard]] std::size_t indexOf(EditorCommand command) {
    return static_cast<std::size_t>(command);
}

}  // namespace

EditorActions::EditorActions(QObject* parent)
    : QObject(parent), _toolGroup(new QActionGroup(this)) {
    _toolGroup->setExclusive(true);
    for (const CommandSpec& spec : commandSpecs()) {
        auto* const act = new QAction(QString::fromUtf8(spec.label), this);
        if (spec.icon != TEXT_ONLY) {
            act->setIcon(QApplication::style()->standardIcon(spec.icon));
        }
        if (spec.shortcut[0] != '\0') {
            act->setShortcut(QKeySequence(QString::fromLatin1(spec.shortcut)));
        }
        if (toolOf(spec.command)) {
            act->setCheckable(true);
            act->setActionGroup(_toolGroup);
        }
        _actions[indexOf(spec.command)] = act;
    }
    // Pinceau actif par défaut (EX-EDIT-014).
    action(EditorCommand::ToolPaint)->setChecked(true);
    refreshToolTips();
}

QAction* EditorActions::action(EditorCommand command) const {
    return _actions[indexOf(command)];
}

std::optional<EditorTool> EditorActions::toolOf(EditorCommand command) {
    switch (command) {
        case EditorCommand::ToolPaint:
            return EditorTool::Paint;
        case EditorCommand::ToolRectangle:
            return EditorTool::Rectangle;
        case EditorCommand::ToolSelection:
            return EditorTool::Selection;
        case EditorCommand::ToolEntity:
            return EditorTool::Entity;
        default:
            return std::nullopt;
    }
}

QAction* EditorActions::toolAction(EditorTool tool) const {
    for (const CommandSpec& spec : commandSpecs()) {
        if (toolOf(spec.command) == tool) {
            return action(spec.command);
        }
    }
    return nullptr;
}

void EditorActions::populateToolBar(QToolBar& toolBar) const {
    bool separatorInserted = false;
    for (const CommandSpec& spec : commandSpecs()) {
        if (!spec.onToolBar) {
            continue;
        }
        if (!separatorInserted && !toolOf(spec.command)) {
            toolBar.addSeparator();
            separatorInserted = true;
        }
        toolBar.addAction(action(spec.command));
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

void EditorActions::applyShortcuts(const EditorKeyBindings& bindings) {
    for (const CommandSpec& spec : commandSpecs()) {
        if (!spec.binding) {
            continue;
        }
        const auto qtKey = static_cast<Qt::Key>(hmiKeyToQtKey(bindings.key(*spec.binding)));
        const Qt::KeyboardModifiers modifiers =
            carriesImplicitCtrl(*spec.binding) ? Qt::ControlModifier : Qt::NoModifier;
        action(spec.command)->setShortcut(QKeySequence(QKeyCombination(modifiers, qtKey)));
    }
    refreshToolTips();
}

void EditorActions::refreshToolTips() const {
    // Infobulle = libellé + raccourci de l'action elle-même : un remappage ne peut pas la rendre
    // fausse.
    for (QAction* const act : _actions) {
        const QKeySequence shortcut = act->shortcut();
        act->setToolTip(shortcut.isEmpty() ? act->text()
                                           : act->text() + QStringLiteral(" (") +
                                                 shortcut.toString(QKeySequence::NativeText) +
                                                 QStringLiteral(")"));
    }
}

}  // namespace hmi
