// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <cstddef>

/**
 * @file Editor/Logic/EditorTool.h
 * @brief Outil actif dans la grille de l'éditeur (`EX-EDIT-014`).
 */

namespace hmi {

/**
 * @brief Outil actif dans la grille de l'éditeur, changé via `Tab` ou la barre d'outils
 *        (`hmi::EditorActions`).
 *
 * `Paint` peint case par case au clic/glisser. `Rectangle` peint
 * un rectangle entier au relâchement d'un glisser. `Selection` définit une zone (glisser) dont le
 * contenu peut être copié (`Ctrl+C`) puis collé ailleurs (`Ctrl+V`), sans peindre directement.
 * `Entity` (`LOT-11`) pose, sélectionne, déplace et retire les entités de carte — PNJ, coffres,
 * portails, points d'arrivée… — voir `hmi::resolveEntityPress` ; leurs propriétés s'éditent dans
 * le panneau « Entités ».
 */
enum class EditorTool { Paint, Rectangle, Selection, Entity };

/// Nombre d'outils d'édition, déclaré au plus près de l'énumération qu'il compte. Sert de garde
/// de **complétude** : le catalogue d'actions doit exposer exactement autant d'outils de niveau
/// (`hmi::editorActionCatalog`), ce qu'un test vérifie — sans quoi un outil ajouté à
/// l'énumération peut apparaître dans la barre d'outils sans jamais être relié au viewport.
inline constexpr std::size_t EDITOR_TOOL_COUNT = 4;

}  // namespace hmi
