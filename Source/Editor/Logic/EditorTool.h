// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <cstddef>

/**
 * @file Editor/Logic/EditorTool.h
 * @brief Outil actif dans la grille de l'éditeur (`EX-EDIT-014`, `EX-EDIT-066`).
 */

namespace hmi {

/**
 * @brief Outil actif dans la grille de l'éditeur, choisi par la barre d'outils ou sa touche
 *        (`hmi::EditorActions`).
 *
 * Les outils **du peintre** donnent le pinceau armé (un type, une pièce) : `Paint` case par case au
 * clic et au glisser, `Rectangle` sur un rectangle tiré, `Line` sur une ligne tirée, `Bucket` sur
 * la région de même contenu (`LOT-EDITOR-04`). `Eraser` gomme au clic et au glisser, quel que soit
 * le pinceau armé. `Pipette` prend le pinceau de ce qu'on voit sous la case, puis rend la main à
 * l'outil du peintre d'avant (`Alt` + clic fait de même depuis n'importe quel outil).
 * `Selection` définit une zone qu'on copie (`Ctrl+C`), colle (`Ctrl+V`) ou gomme (`Suppr`).
 * `Entity` (`LOT-11`) pose, sélectionne, déplace et retire les entités de carte, tire les zones et
 * leurs poignées (voir `hmi::resolveEntityPress`) ; `Shape` peint une zone de règles case par case
 * et trace un trajet point par point (`LOT-EDITOR-05`, `hmi::resolveShapePress`). `Measure` mesure
 * en cases et en pieds, `Note` épingle une note d'auteur à une case.
 */
enum class EditorTool {
    Paint,
    Rectangle,
    Line,
    Bucket,
    Eraser,
    Pipette,
    Selection,
    Entity,
    Shape,
    Measure,
    Note,
};

/// Nombre d'outils d'édition, déclaré au plus près de l'énumération qu'il compte.
inline constexpr std::size_t EDITOR_TOOL_COUNT = 11;

/// @return Vrai si @p tool pose le pinceau armé (pinceau, rectangle, ligne, seau).
[[nodiscard]] constexpr bool paintsWithBrush(EditorTool tool) noexcept {
    return tool == EditorTool::Paint || tool == EditorTool::Rectangle || tool == EditorTool::Line ||
           tool == EditorTool::Bucket;
}

}  // namespace hmi
