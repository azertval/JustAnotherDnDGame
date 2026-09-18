// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "Editor/Logic/PanelFocus.h"

namespace hmi {

const std::array<PanelFocusEntry, PANEL_FOCUS_CATALOG_COUNT>& panelFocusCatalog() {
    static const std::array<PanelFocusEntry, PANEL_FOCUS_CATALOG_COUNT> catalog{{
        {.tool = EditorTool::Entity, .panel = PanelId::Entities},
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

}  // namespace hmi
