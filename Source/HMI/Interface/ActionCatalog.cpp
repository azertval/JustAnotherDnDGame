// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Interface/ActionCatalog.h"

#include <stdexcept>

namespace hmi {

const std::array<EditorActionSpec, EDITOR_ACTION_CATALOG_COUNT>& editorActionCatalog() {
    static const std::array<EditorActionSpec, EDITOR_ACTION_CATALOG_COUNT> catalog{{
        // Outils (ordre de la palette/du panneau Outils historique, EX-EDIT-014) : groupe
        // exclusif. Seul TextureAssign porte un raccourci -- la "touche dédiée" remappable
        // (EditorKeyBindings::TextureAssignTool), déjà gérée par hmi::GameViewport ; les cinq
        // autres n'ont pas de raccourci clavier aujourd'hui, et ce lot n'en introduit pas.
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
        {.id = IconId::ToolLink,
         .labelKey = "tool.link",
         .shortcut = "",
         .checkable = true,
         .group = EditorActionGroup::LevelTools,
         .surface = ActionSurface::ToolBarAndMenu},
        {.id = IconId::ToolTextureAssign,
         .labelKey = "tool.texture_assign",
         .shortcut = "",
         .checkable = true,
         .group = EditorActionGroup::LevelTools,
         .surface = ActionSurface::ToolBarAndMenu},
        {.id = IconId::ToolCameraZone,
         .labelKey = "tool.camera_zone",
         .shortcut = "",
         .checkable = true,
         .group = EditorActionGroup::LevelTools,
         .surface = ActionSurface::ToolBarAndMenu},
        // Outils du canevas pixel art (LOT-54 TACHE-04) : groupe exclusif SEPARE des outils de
        // niveau ci-dessus -- les deux groupes ne s'excluent jamais entre eux. Aucun raccourci
        // clavier dedie aujourd'hui (choix par la barre d'outils du canevas uniquement).
        {.id = IconId::PixelBrush,
         .labelKey = "pixel_tool.brush",
         .shortcut = "",
         .checkable = true,
         .group = EditorActionGroup::PixelTools,
         .surface = ActionSurface::ToolBarAndMenu},
        {.id = IconId::PixelEraser,
         .labelKey = "pixel_tool.eraser",
         .shortcut = "",
         .checkable = true,
         .group = EditorActionGroup::PixelTools,
         .surface = ActionSurface::ToolBarAndMenu},
        {.id = IconId::PixelFill,
         .labelKey = "pixel_tool.fill",
         .shortcut = "",
         .checkable = true,
         .group = EditorActionGroup::PixelTools,
         .surface = ActionSurface::ToolBarAndMenu},
        {.id = IconId::PixelEyedropper,
         .labelKey = "pixel_tool.eyedropper",
         .shortcut = "",
         .checkable = true,
         .group = EditorActionGroup::PixelTools,
         .surface = ActionSurface::ToolBarAndMenu},
        {.id = IconId::PixelSelectionTool,
         .labelKey = "pixel_tool.selection",
         .shortcut = "",
         .checkable = true,
         .group = EditorActionGroup::PixelTools,
         .surface = ActionSurface::ToolBarAndMenu},
        // Commandes de fichier de l'atelier pixel art (LOT-54 TACHE-05) : aucun raccourci clavier
        // dedie aujourd'hui, comme les outils de canevas ci-dessus.
        {.id = IconId::PixelOpen,
         .labelKey = "action.pixel_open",
         .shortcut = "",
         .checkable = false,
         .group = EditorActionGroup::PixelCommands,
         .surface = ActionSurface::ToolBarAndMenu},
        {.id = IconId::PixelCreate,
         .labelKey = "action.pixel_create",
         .shortcut = "",
         .checkable = false,
         .group = EditorActionGroup::PixelCommands,
         .surface = ActionSurface::ToolBarAndMenu},
        {.id = IconId::PixelSave,
         .labelKey = "action.pixel_save",
         .shortcut = "",
         .checkable = false,
         .group = EditorActionGroup::PixelCommands,
         .surface = ActionSurface::ToolBarAndMenu},
        {.id = IconId::PixelSaveAs,
         .labelKey = "action.pixel_save_as",
         .shortcut = "",
         .checkable = false,
         .group = EditorActionGroup::PixelCommands,
         .surface = ActionSurface::MenuOnly},
        // Commandes de region (LOT-54 TACHE-06) : Copier/Coller reutilisent les actions
        // deduplique deja definies (IconId::Copy/Paste, EX-IHM-062) -- rien a ajouter ici pour
        // elles. Deplacement : geste de glisser de l'outil Selection, pas une commande.
        {.id = IconId::PixelFlipHorizontal,
         .labelKey = "action.pixel_flip_horizontal",
         .shortcut = "",
         .checkable = false,
         .group = EditorActionGroup::PixelCommands,
         .surface = ActionSurface::MenuOnly},
        {.id = IconId::PixelFlipVertical,
         .labelKey = "action.pixel_flip_vertical",
         .shortcut = "",
         .checkable = false,
         .group = EditorActionGroup::PixelCommands,
         .surface = ActionSurface::MenuOnly},
        {.id = IconId::PixelRotateClockwise,
         .labelKey = "action.pixel_rotate_cw",
         .shortcut = "",
         .checkable = false,
         .group = EditorActionGroup::PixelCommands,
         .surface = ActionSurface::MenuOnly},
        {.id = IconId::PixelRotateCounterClockwise,
         .labelKey = "action.pixel_rotate_ccw",
         .shortcut = "",
         .checkable = false,
         .group = EditorActionGroup::PixelCommands,
         .surface = ActionSurface::MenuOnly},
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
        {.id = IconId::ToggleRenderMode,
         .labelKey = "action.toggle_render_mode",
         .shortcut = "F8",
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
        case IconId::ToolLink:
            return EditorTool::Link;
        case IconId::ToolTextureAssign:
            return EditorTool::TextureAssign;
        case IconId::ToolCameraZone:
            return EditorTool::CameraZone;
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
        case EditorTool::Link:
            return IconId::ToolLink;
        case EditorTool::TextureAssign:
            return IconId::ToolTextureAssign;
        case EditorTool::CameraZone:
            return IconId::ToolCameraZone;
    }
    return IconId::ToolPaint;
}

std::optional<PixelTool> editorActionPixelTool(IconId id) {
    switch (id) {
        case IconId::PixelBrush:
            return PixelTool::Brush;
        case IconId::PixelEraser:
            return PixelTool::Eraser;
        case IconId::PixelFill:
            return PixelTool::Fill;
        case IconId::PixelEyedropper:
            return PixelTool::Eyedropper;
        case IconId::PixelSelectionTool:
            return PixelTool::Selection;
        default:
            return std::nullopt;
    }
}

IconId editorActionForPixelTool(PixelTool tool) {
    switch (tool) {
        case PixelTool::Brush:
            return IconId::PixelBrush;
        case PixelTool::Eraser:
            return IconId::PixelEraser;
        case PixelTool::Fill:
            return IconId::PixelFill;
        case PixelTool::Eyedropper:
            return IconId::PixelEyedropper;
        case PixelTool::Selection:
            return IconId::PixelSelectionTool;
    }
    return IconId::PixelBrush;
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
    return IconId::Save;  // inatteignable pour les 9 actions couvertes ; TextureAssignTool exclue.
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
