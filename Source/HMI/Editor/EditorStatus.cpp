// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Editor/EditorStatus.h"

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <vector>

#include "HMI/Localization/Localization.h"

namespace hmi {

namespace {

// Remplace la premiere occurrence de "%1" (et "%2" pour les gabarits a deux valeurs) -- pas de
// dependance Qt, meme discipline que hmi::gameHudLines (LOT-52).
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

// Remplace %1, %2, ... par les valeurs donnees, dans l'ordre -- generalisation de formatOne/
// formatTwo pour la fiche d'un plan, qui en porte quatre.
std::string formatAll(const std::string& templateText, const std::vector<std::string>& values) {
    std::string text = templateText;
    for (std::size_t index = 0; index < values.size(); ++index) {
        text = replacePlaceholder(text, "%" + std::to_string(index + 1), values[index]);
    }
    return text;
}

// Poids d'une texture en mebioctets, deux decimales. Une seule unite pour toute l'echelle utile :
// le plus petit plan du depot pese deja 0,08 Mio et le plafond est a 16 Mio -- basculer en kibi
// sous le mebi rendrait deux plans incomparables d'un coup d'oeil, ce qui est tout ce qu'on
// demande a cette zone.
std::string formatMebibytes(std::size_t bytes) {
    const double mebibytes = static_cast<double>(bytes) / (1024.0 * 1024.0);
    std::array<char, 32> buffer{};
    std::snprintf(buffer.data(), buffer.size(), "%.2f", mebibytes);
    return std::string(buffer.data());
}

// Cle de traduction du libelle d'un mode de cadrage (LOT-64) -- memes cles que TexturePanel.cpp
// (section « Cadrage »), reprises ici plutot que dupliquees : un seul point de vocabulaire.
const char* cameraFramingLabelKey(core::CameraFramingMode mode) {
    switch (mode) {
        case core::CameraFramingMode::WholeLevel:
            return "camera_framing.whole_level";
        case core::CameraFramingMode::PerRoom:
            return "camera_framing.per_room";
        case core::CameraFramingMode::Follow:
            return "camera_framing.follow";
    }
    return "camera_framing.whole_level";
}

// Cle de traduction du libelle court d'un outil (deja utilisees par EditorActions).
const char* toolLabelKey(EditorTool tool) {
    switch (tool) {
        case EditorTool::Paint:
            return "tool.brush";
        case EditorTool::Rectangle:
            return "tool.rectangle";
        case EditorTool::Selection:
            return "tool.selection";
        case EditorTool::Link:
            return "tool.link";
        case EditorTool::TextureAssign:
            return "tool.texture_assign";
        case EditorTool::CameraZone:
            return "tool.camera_zone";
    }
    return "tool.brush";
}

// Cle de traduction de l'aide contextuelle d'un outil (LOT-57 TACHE-01, retire status.edit_help).
const char* toolHelpKey(EditorTool tool) {
    switch (tool) {
        case EditorTool::Paint:
            return "status.help_paint";
        case EditorTool::Rectangle:
            return "status.help_rectangle";
        case EditorTool::Selection:
            return "status.help_selection";
        case EditorTool::Link:
            return "status.help_link";
        case EditorTool::TextureAssign:
            return "status.help_texture_assign";
        case EditorTool::CameraZone:
            return "status.help_camera_zone";
    }
    return "status.help_paint";
}

// Cle de traduction du libelle court d'un outil de canevas pixel art (LOT-54 TACHE-04) -- deja
// utilisees par EditorActions (memes cles que les actions de la barre d'outils du canevas).
const char* pixelToolLabelKey(PixelTool tool) {
    switch (tool) {
        case PixelTool::Brush:
            return "pixel_tool.brush";
        case PixelTool::Eraser:
            return "pixel_tool.eraser";
        case PixelTool::Fill:
            return "pixel_tool.fill";
        case PixelTool::Eyedropper:
            return "pixel_tool.eyedropper";
        case PixelTool::Selection:
            return "pixel_tool.selection";
    }
    return "pixel_tool.brush";
}

// Cle de traduction de l'aide contextuelle d'un outil de canevas pixel art (LOT-54 TACHE-04).
const char* pixelToolHelpKey(PixelTool tool) {
    switch (tool) {
        case PixelTool::Brush:
            return "status.help_pixel_brush";
        case PixelTool::Eraser:
            return "status.help_pixel_eraser";
        case PixelTool::Fill:
            return "status.help_pixel_fill";
        case PixelTool::Eyedropper:
            return "status.help_pixel_eyedropper";
        case PixelTool::Selection:
            return "status.help_pixel_selection";
    }
    return "status.help_pixel_brush";
}

// Couleur R8G8B8A8_UNORM (ordre memoire R,G,B,A, meme convention que decodeImageFile/
// encodeImageFile, LOT-54 TACHE-01) -> chaine hexadecimale "#rrggbbaa", alpha compris (une
// pipette peut prelever une couleur partiellement transparente).
std::string formatColorHex(std::uint32_t color) {
    std::array<char, 10> buffer{};
    std::snprintf(buffer.data(), buffer.size(), "#%02x%02x%02x%02x",
                  static_cast<unsigned>(color & 0xFFU), static_cast<unsigned>((color >> 8) & 0xFFU),
                  static_cast<unsigned>((color >> 16) & 0xFFU),
                  static_cast<unsigned>((color >> 24) & 0xFFU));
    return std::string(buffer.data());
}

}  // namespace

// Contenu de la barre d'etat de l'editeur (voir en-tete) : zones permanentes puis aide.
EditorStatusLines editorStatusLines(const EditorStatusContext& context,
                                    const Localization& localization) {
    EditorStatusLines lines;
    // niveau/asset, modifie, outil, case/pixel survole, zoom, couleur courante (atelier
    // seulement), cadrage de camera (contexte niveau seulement).
    lines.permanent.assign(7, std::string{});

    if (context.pixelEdit) {
        const PixelEditStatusInfo& pixel = *context.pixelEdit;
        if (!pixel.assetName.empty()) {
            // Un plan n'est pas un asset : il ne vit pas sous `Assets/` et n'est reutilisable par
            // aucun autre niveau. Annoncer « Asset : ... » en mode creation dirait le contraire de
            // ce que le lot etablit.
            const char* const nameKey =
                pixel.plane ? "status.zone.plane_file" : "status.zone.asset";
            lines.permanent[0] = formatOne(localization.text(nameKey), pixel.assetName);
        }
        if (pixel.dirty) {
            lines.permanent[1] = localization.text("status.zone.dirty");
        }
        lines.permanent[2] = localization.text(pixelToolLabelKey(pixel.tool));
        if (pixel.hoveredPixel) {
            lines.permanent[3] = formatTwo(localization.text("status.zone.hover"),
                                           pixel.hoveredPixel->first, pixel.hoveredPixel->second);
        }
        lines.permanent[4] = formatOne(localization.text("status.zone.zoom"), pixel.zoom * 100);
        // Mode contraint (LOT-54 TACHE-07) : integre a la meme zone couleur plutot qu'une septieme
        // zone -- l'information n'a de sens qu'accolee a la couleur qu'elle affecte.
        const char* const colorKey =
            pixel.paletteConstrained ? "status.zone.color_constrained" : "status.zone.color";
        lines.permanent[5] =
            formatOne(localization.text(colorKey), formatColorHex(pixel.currentColor));
        // Fiche du plan (LOT-69 TACHE-08) : la septieme zone, libre en contexte plan puisque le
        // cadrage de camera qu'elle porte ailleurs n'a de sens que sur un niveau.
        if (pixel.plane) {
            lines.permanent[6] = formatAll(localization.text("status.zone.plane"),
                                           {std::to_string(pixel.plane->widthPixels),
                                            std::to_string(pixel.plane->heightPixels),
                                            std::to_string(pixel.plane->pixelsPerUnit),
                                            formatMebibytes(pixel.plane->textureBytes)});
        }
        lines.help = localization.text(pixelToolHelpKey(pixel.tool));
        return lines;
    }

    if (!context.level) {
        return lines;  // aucun contexte actif : zones et aide vides, jamais de libelle de repli.
    }
    const LevelStatusInfo& level = *context.level;

    lines.permanent[0] = formatOne(localization.text("status.zone.level"), level.name);
    if (level.dirty) {
        lines.permanent[1] = localization.text("status.zone.dirty");
    }
    lines.permanent[2] = localization.text(toolLabelKey(level.tool));
    if (level.hoveredCell) {
        lines.permanent[3] = formatTwo(localization.text("status.zone.hover"),
                                       level.hoveredCell->column, level.hoveredCell->row);
    }
    const int zoomPercent = static_cast<int>(std::lround(level.zoom * 100.0F));
    lines.permanent[4] = formatOne(localization.text("status.zone.zoom"), zoomPercent);
    // permanent[5] (couleur) reste vide : sans objet hors atelier pixel art.
    lines.permanent[6] = formatOne(localization.text("status.zone.camera_framing"),
                                   localization.text(cameraFramingLabelKey(level.cameraFraming)));

    lines.help = localization.text(toolHelpKey(level.tool));
    return lines;
}

}  // namespace hmi
