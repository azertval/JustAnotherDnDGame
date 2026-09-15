// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "HMI/Editor/PanelFocus.h"

namespace hmi {

const std::array<PanelFocusEntry, PANEL_FOCUS_CATALOG_COUNT>& panelFocusCatalog() {
    static const std::array<PanelFocusEntry, PANEL_FOCUS_CATALOG_COUNT> catalog{{
        {.tool = EditorTool::Link, .panel = PanelId::Links},
        {.tool = EditorTool::TextureAssign, .panel = PanelId::Textures},
    }};
    return catalog;
}

std::optional<PanelId> panelForTool(EditorTool tool) {
    for (const PanelFocusEntry& entry : panelFocusCatalog()) {
        if (entry.tool == tool) {
            return entry.panel;
        }
    }
    return std::nullopt;
}

const std::array<PixelPanelFocusEntry, PIXEL_PANEL_FOCUS_CATALOG_COUNT>& pixelPanelFocusCatalog() {
    // Les quatre outils du canevas mettent tous en avant le meme panneau (PixelCanvas) : des qu'un
    // outil de canevas est actif, on veut voir le canevas -- a la difference des outils de niveau,
    // aucun outil de canevas n'a de panneau "annexe" dedie.
    static const std::array<PixelPanelFocusEntry, PIXEL_PANEL_FOCUS_CATALOG_COUNT> catalog{{
        {.tool = PixelTool::Brush, .panel = PanelId::PixelCanvas},
        {.tool = PixelTool::Eraser, .panel = PanelId::PixelCanvas},
        {.tool = PixelTool::Fill, .panel = PanelId::PixelCanvas},
        {.tool = PixelTool::Eyedropper, .panel = PanelId::PixelCanvas},
        {.tool = PixelTool::Selection, .panel = PanelId::PixelCanvas},
    }};
    return catalog;
}

std::optional<PanelId> panelForPixelTool(PixelTool tool) {
    for (const PixelPanelFocusEntry& entry : pixelPanelFocusCatalog()) {
        if (entry.tool == tool) {
            return entry.panel;
        }
    }
    return std::nullopt;
}

}  // namespace hmi
