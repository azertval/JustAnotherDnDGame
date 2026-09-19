// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "Editor/Logic/EditorStatus.h"

#include <cmath>
#include <cstddef>

namespace hmi {

namespace {

// Remplace la premiere occurrence de "%1" (et "%2" pour les gabarits a deux valeurs) -- pas de
// dependance Qt.
std::string replacePlaceholder(std::string text, const std::string& placeholder,
                               const std::string& value) {
    const std::size_t position = text.find(placeholder);
    if (position == std::string::npos) {
        return text;
    }
    text.replace(position, placeholder.size(), value);
    return text;
}

std::string formatOne(const std::string& templateText, const std::string& value) {
    return replacePlaceholder(templateText, "%1", value);
}

std::string formatOne(const std::string& templateText, int value) {
    return formatOne(templateText, std::to_string(value));
}

std::string formatTwo(const std::string& templateText, int first, int second) {
    return replacePlaceholder(formatOne(templateText, first), "%2", std::to_string(second));
}

// Cle de traduction du libelle court d'un outil (deja utilisees par EditorActions).
const char* toolLabel(EditorTool tool) {
    switch (tool) {
        case EditorTool::Paint:
            return "Brush";
        case EditorTool::Rectangle:
            return "Rectangle";
        case EditorTool::Selection:
            return "Selection";
        case EditorTool::Entity:
            return "Entity";
    }
    return "Brush";
}

const char* toolHelp(EditorTool tool) {
    switch (tool) {
        case EditorTool::Paint:
            return "Paint: left click/drag · Ctrl+Z/Y: undo/redo";
        case EditorTool::Rectangle:
            return "Rectangle: drag to paint an area · Ctrl+Z/Y: undo/redo";
        case EditorTool::Selection:
            return "Selection: drag to select · Ctrl+C/V: copy/paste";
        case EditorTool::Entity:
            return "Entity: click an empty cell to place the chosen kind · click an entity to "
                   "select it, drag to move it · Del removes it · Ctrl+click places on an "
                   "occupied cell";
    }
    return "";
}

}  // namespace

// Contenu de la barre d'etat de l'editeur (voir en-tete) : zones permanentes puis aide.
EditorStatusLines editorStatusLines(const EditorStatusContext& context) {
    EditorStatusLines lines;
    lines.permanent.assign(EDITOR_STATUS_ZONE_COUNT, std::string{});
    if (!context.level) {
        return lines;  // aucun contexte actif : zones et aide vides, jamais de libelle de repli.
    }
    const LevelStatusInfo& level = *context.level;

    lines.permanent[0] = formatOne("Map: %1", level.name);
    if (level.dirty) {
        lines.permanent[1] = "Modified";
    }
    lines.permanent[2] = toolLabel(level.tool);
    const bool brushes = level.tool == EditorTool::Paint || level.tool == EditorTool::Rectangle;
    if (brushes && !level.brush.empty()) {
        lines.permanent[2] += " · " + level.brush;
    }
    if (level.hoveredCell) {
        lines.permanent[3] =
            formatTwo("(%1, %2)", level.hoveredCell->column, level.hoveredCell->row);
        if (!level.hoveredPieces.empty()) {
            lines.permanent[3] += " " + level.hoveredPieces;
        }
        if (level.hoveredForced) {
            lines.permanent[3] += " · forced collision";
        }
    }
    const int zoomPercent = static_cast<int>(std::lround(level.zoom * 100.0F));
    lines.permanent[4] =
        formatOne("Zoom: %1%", zoomPercent) + (level.isoView ? " · Iso" : " · Flat");

    lines.help = toolHelp(level.tool);
    if (brushes && level.collisionActive) {
        lines.help += " · On the collision, painting forces a cell; the eraser releases it";
    }
    return lines;
}

}  // namespace hmi
