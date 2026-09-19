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
        case EditorTool::Line:
            return "Line";
        case EditorTool::Bucket:
            return "Bucket";
        case EditorTool::Eraser:
            return "Eraser";
        case EditorTool::Pipette:
            return "Pipette";
        case EditorTool::Selection:
            return "Selection";
        case EditorTool::Entity:
            return "Entity";
        case EditorTool::Shape:
            return "Shape";
        case EditorTool::Measure:
            return "Measure";
        case EditorTool::Note:
            return "Note";
    }
    return "Brush";
}

const char* toolHelp(EditorTool tool) {
    switch (tool) {
        case EditorTool::Paint:
            return "Paint: left click/drag · Alt+click: pick · Ctrl+Z/Y: undo/redo";
        case EditorTool::Rectangle:
            return "Rectangle: drag to paint an area · Alt+click: pick · Ctrl+Z/Y: undo/redo";
        case EditorTool::Line:
            return "Line: drag to paint a line · Alt+click: pick · Ctrl+Z/Y: undo/redo";
        case EditorTool::Bucket:
            return "Bucket: click to fill the connected cells alike · Alt+click: pick";
        case EditorTool::Eraser:
            return "Eraser: left click/drag erases on the active layer · Ctrl+Z/Y: undo/redo";
        case EditorTool::Pipette:
            return "Pipette: click to pick what you see, then back to painting";
        case EditorTool::Selection:
            return "Selection: drag to select · Ctrl+C/V: copy/paste · Del: erase";
        case EditorTool::Measure:
            return "Measure: drag from a cell to another · 1 cell = 5 ft";
        case EditorTool::Note:
            return "Note: click a cell to write, edit or clear its author note";
        case EditorTool::Entity:
            return "Entity: click to place the chosen kind, drag to draw a zone or a route · "
                   "drag an entity to move the selection, a handle to resize · Shift+click: add "
                   "to the selection · Del removes · Ctrl+click places on an occupied cell";
        case EditorTool::Shape:
            return "Shape: drag to paint cells into the selected zone, Ctrl+drag to erase · "
                   "click to extend the selected route, drag a point to move it, Ctrl+click to "
                   "remove it";
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
    const bool brushes = paintsWithBrush(level.tool);
    if (brushes && !level.brush.empty()) {
        lines.permanent[2] += " · " + level.brush;
    }
    if (level.mirror) {
        lines.permanent[2] += " · Mirror";
    }
    if (!level.measure.empty()) {
        lines.permanent[2] += " · " + level.measure;
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
        if (!level.hoveredNote.empty()) {
            lines.permanent[3] += " · Note: " + level.hoveredNote;
        }
    }
    const int zoomPercent = static_cast<int>(std::lround(level.zoom * 100.0F));
    lines.permanent[4] =
        formatOne("Zoom: %1%", zoomPercent) + (level.isoView ? " · Iso" : " · Flat");

    lines.help = toolHelp(level.tool);
    if ((brushes || level.tool == EditorTool::Eraser) && level.collisionActive) {
        lines.help += " · On the collision, painting forces a cell; the eraser releases it";
    }
    return lines;
}

}  // namespace hmi
