// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "HMI/Interface/ActionCatalog.h"

#include <stdexcept>

namespace hmi {

const std::array<EditorActionSpec, EDITOR_ACTION_CATALOG_COUNT>& editorActionCatalog() {
    static const std::array<EditorActionSpec, EDITOR_ACTION_CATALOG_COUNT> catalog{{
        // Outils (EX-EDIT-014) : groupe exclusif, sans raccourci clavier.
        {.id = IconId::ToolPaint,
         .labelKey = "tool.brush",
         .shortcut = "",
         .checkable = true,
         .group = EditorActionGroup::LevelTools,
         .surface = ActionSurface::ToolBarAndMenu},
        {.id = IconId::ToolRectangle,
         .labelKey = "tool.rectangle",
         .shortcut = "",
         .checkable = true,
         .group = EditorActionGroup::LevelTools,
         .surface = ActionSurface::ToolBarAndMenu},
        {.id = IconId::ToolSelection,
         .labelKey = "tool.selection",
         .shortcut = "",
         .checkable = true,
         .group = EditorActionGroup::LevelTools,
         .surface = ActionSurface::ToolBarAndMenu},
        // Outil Entite (LOT-11) : poser et editer les entites de carte.
        {.id = IconId::ToolEntity,
         .labelKey = "tool.entity",
         .shortcut = "",
         .checkable = true,
         .group = EditorActionGroup::LevelTools,
         .surface = ActionSurface::ToolBarAndMenu},
        // Commandes principales : aucun groupe, aucune n'est cochable.
        {.id = IconId::Save,
         .labelKey = "action.save",
         .shortcut = "Ctrl+S",
         .checkable = false,
         .group = EditorActionGroup::None,
         .surface = ActionSurface::ToolBarAndMenu},
        {.id = IconId::Playtest,
         .labelKey = "action.playtest",
         .shortcut = "P",
         .checkable = false,
         .group = EditorActionGroup::None,
         .surface = ActionSurface::ToolBarAndMenu},
        {.id = IconId::Undo,
         .labelKey = "action.undo",
         .shortcut = "Ctrl+Z",
         .checkable = false,
         .group = EditorActionGroup::None,
         .surface = ActionSurface::ToolBarAndMenu},
        {.id = IconId::Redo,
         .labelKey = "action.redo",
         .shortcut = "Ctrl+Y",
         .checkable = false,
         .group = EditorActionGroup::None,
         .surface = ActionSurface::ToolBarAndMenu},
        {.id = IconId::ToggleGrid,
         .labelKey = "action.toggle_grid",
         .shortcut = "F10",
         .checkable = false,
         .group = EditorActionGroup::None,
         .surface = ActionSurface::MenuOnly},
        {.id = IconId::ResetCamera,
         .labelKey = "action.reset_camera",
         .shortcut = "0",
         .checkable = false,
         .group = EditorActionGroup::None,
         .surface = ActionSurface::MenuOnly},
        // Déduplication des commandes (LOT-57 TACHE-04) : branchées sur les actions d'éditeur
        // remappables jusqu'ici définies et jamais lues (`EditorKeyBindings`). Valeurs par défaut
        // alignées sur `EditorKeyBindings::defaultKey` ; la valeur effective vient toujours
        // d'`EditorActions::applyShortcuts`, jamais de ce littéral seul (cf. son commentaire).
        {.id = IconId::Copy,
         .labelKey = "action.copy",
         .shortcut = "Ctrl+C",
         .checkable = false,
         .group = EditorActionGroup::None,
         .surface = ActionSurface::MenuOnly},
        {.id = IconId::Paste,
         .labelKey = "action.paste",
         .shortcut = "Ctrl+V",
         .checkable = false,
         .group = EditorActionGroup::None,
         .surface = ActionSurface::MenuOnly},
        {.id = IconId::Rename,
         .labelKey = "action.rename",
         .shortcut = "F2",
         .checkable = false,
         .group = EditorActionGroup::None,
         .surface = ActionSurface::MenuOnly},
        {.id = IconId::ShortcutsOverview,
         .labelKey = "action.shortcuts_overview",
         .shortcut = "F1",
         .checkable = false,
         .group = EditorActionGroup::None,
         .surface = ActionSurface::MenuOnly},
    }};
    return catalog;
}

const EditorActionSpec& editorActionSpec(IconId id) {
    for (const EditorActionSpec& spec : editorActionCatalog()) {
        if (spec.id == id) {
            return spec;
        }
    }
    throw std::out_of_range("editorActionSpec: identifiant d'action inconnu");
}

std::optional<EditorTool> editorActionTool(IconId id) {
    switch (id) {
        case IconId::ToolPaint:
            return EditorTool::Paint;
        case IconId::ToolRectangle:
            return EditorTool::Rectangle;
        case IconId::ToolSelection:
            return EditorTool::Selection;
        case IconId::ToolEntity:
            return EditorTool::Entity;
        default:
            return std::nullopt;
    }
}

IconId editorActionForTool(EditorTool tool) {
    switch (tool) {
        case EditorTool::Paint:
            return IconId::ToolPaint;
        case EditorTool::Rectangle:
            return IconId::ToolRectangle;
        case EditorTool::Selection:
            return IconId::ToolSelection;
        case EditorTool::Entity:
            return IconId::ToolEntity;
    }
    return IconId::ToolPaint;
}

const std::array<KeyBindingIconEntry, KEY_BINDING_ICON_COUNT>& keyBindingIconCatalog() {
    static const std::array<KeyBindingIconEntry, KEY_BINDING_ICON_COUNT> catalog{{
        {.action = EditorAction::Save, .id = IconId::Save},
        {.action = EditorAction::Undo, .id = IconId::Undo},
        {.action = EditorAction::Redo, .id = IconId::Redo},
        {.action = EditorAction::Copy, .id = IconId::Copy},
        {.action = EditorAction::Paste, .id = IconId::Paste},
        {.action = EditorAction::Playtest, .id = IconId::Playtest},
        {.action = EditorAction::ToggleGrid, .id = IconId::ToggleGrid},
        {.action = EditorAction::ToggleHelp, .id = IconId::ShortcutsOverview},
        {.action = EditorAction::Rename, .id = IconId::Rename},
    }};
    return catalog;
}

IconId iconForKeyBindingAction(EditorAction action) {
    for (const KeyBindingIconEntry& entry : keyBindingIconCatalog()) {
        if (entry.action == action) {
            return entry.id;
        }
    }
    return IconId::Save;  // inatteignable : toute action remappable a son icone.
}

std::optional<EditorAction> keyBindingActionForIcon(IconId id) {
    for (const KeyBindingIconEntry& entry : keyBindingIconCatalog()) {
        if (entry.id == id) {
            return entry.action;
        }
    }
    return std::nullopt;
}

}  // namespace hmi
