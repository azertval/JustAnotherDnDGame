// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstddef>

/**
 * @file HMI/Editor/EditorTool.h
 * @brief Outil actif dans la grille de l'éditeur (LOT-15, `EX-EDIT-014`).
 */

namespace hmi {

/**
 * @brief Outil actif dans la grille de l'éditeur, changé via `Tab` ou la barre d'outils
 *        (`hmi::EditorActions`).
 *
 * `Paint` peint case par case au clic/glisser (comportement LOT-14, inchangé). `Rectangle` peint
 * un rectangle entier au relâchement d'un glisser. `Selection` définit une zone (glisser) dont le
 * contenu peut être copié (`Ctrl+C`) puis collé ailleurs (`Ctrl+V`), sans peindre directement.
 * `Link` (`LOT-37`, `EX-IHM-030`) lie/délie un mécanisme : cliquer un déclencheur (interrupteur/
 * plaque) passe en attente de cible, cliquer une cible (porte/danger commuté) crée la liaison ;
 * refaire la même paire la supprime (bascule) — voir `hmi::resolveLinkClick`. `Échap` annule une
 * attente en cours (revient à aucune case sélectionnée), sans toucher au brouillon.
 * `TextureAssign` (`LOT-45`, `EX-EDIT-043`) assigne une texture par instance à une case : clic
 * gauche assigne/remplace l'asset sélectionné dans la bibliothèque, reclic du même asset ou clic
 * droit retire l'assignation — voir `hmi::resolveTextureAssignClick`.
 * `CameraZone` (`LOT-64`, `EX-LVL-007`, `EX-EDIT-029`) dessine une zone de caméra du mode *par
 * salle* : un glisser (comme `Rectangle`) définit un rectangle de cases, ajouté à
 * `core::CameraFramingConfig::zones` au relâchement (`core::LevelDraft::addCameraZone`) ; les
 * zones existantes se retirent depuis le tableau de la section « Cadrage » du panneau Textures,
 * pas depuis le canevas.
 */
enum class EditorTool { Paint, Rectangle, Selection, Link, TextureAssign, CameraZone };

/// Nombre d'outils d'édition, déclaré au plus près de l'énumération qu'il compte. Sert de garde
/// de **complétude** : le catalogue d'actions doit exposer exactement autant d'outils de niveau
/// (`hmi::editorActionCatalog`), ce qu'un test vérifie — sans quoi un outil ajouté à
/// l'énumération peut apparaître dans la barre d'outils sans jamais être relié au viewport.
inline constexpr std::size_t EDITOR_TOOL_COUNT = 6;

}  // namespace hmi
