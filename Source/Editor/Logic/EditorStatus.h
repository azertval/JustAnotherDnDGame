// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <optional>
#include <string>
#include <vector>

#include "Core/Levels/GridPosition.h"
#include "Editor/Logic/EditorTool.h"

/**
 * @file Editor/Logic/EditorStatus.h
 * @brief Choix du contenu de la barre d'état de l'éditeur : zones permanentes et aide
 *        contextuelle à l'outil actif.
 */

namespace hmi {

/// État affiché pour une carte en cours d'édition (barre d'état, `EX-EDIT-013`/`EX-EDIT-012`).
struct LevelStatusInfo {
    std::string name;                               ///< Nom de la carte ouverte.
    bool dirty = false;                             ///< Modifications non enregistrées.
    EditorTool tool = EditorTool::Paint;            ///< Outil d'édition actif.
    std::optional<core::GridPosition> hoveredCell;  ///< Case survolée, si le curseur est dessus.
    /// Pièces de la case survolée (`street · wall-left`), vide sans lieu (LOT-EDITOR-02).
    std::string hoveredPieces;
    /// La collision de la case survolée est forcée à la main (LOT-EDITOR-03).
    bool hoveredForced = false;
    /// Le pinceau armé (`wall-left`, `grass`, `Eraser`), vide s'il n'y a rien à dire.
    std::string brush;
    /// La couche active est la grille de collision.
    bool collisionActive = false;
    /// Le miroir est actif (LOT-EDITOR-04).
    bool mirror = false;
    /// La mesure en cours (`hmi::measureLabel`), vide sinon (LOT-EDITOR-04).
    std::string measure;
    /// La note d'auteur de la case survolée, vide sinon (LOT-EDITOR-04).
    std::string hoveredNote;
    float zoom = 1.0F;  ///< Facteur de zoom courant.
    /// Vue du canevas : iso (le lieu) ou à plat (les types), LOT-EDITOR-02.
    bool isoView = true;
};

/// Contexte d'édition dont la barre d'état décide l'affichage ; absent hors édition.
struct EditorStatusContext {
    std::optional<LevelStatusInfo> level;
};

/// Lignes à afficher pour la barre d'état de l'éditeur, à un instant donné.
struct EditorStatusLines {
    /// Zones permanentes, dans l'ordre d'affichage : carte, modifications non enregistrées, outil
    /// actif, case survolée et ses pièces, zoom et vue. Une zone vide (chaîne vide) quand
    /// l'information n'a pas de sens — jamais de libellé de remplacement.
    std::vector<std::string> permanent;
    /// Aide contextuelle à l'outil actif ; vide hors édition.
    std::string help;
};

/// Nombre de zones permanentes de la barre d'état.
inline constexpr std::size_t EDITOR_STATUS_ZONE_COUNT = 5;

/**
 * @brief Décide le contenu de la barre d'état de l'éditeur pour @p context.
 *
 * Fonction **pure** (`EX-NFR-010`) : ne lit que ce qu'on lui passe, aucune dépendance Qt/GPU.
 * Les textes sont en anglais, écrits ici : l'éditeur est un outil interne, sans traduction
 * (`LOT-EDITOR-01`).
 * @param context Contexte d'édition courant ; `context.level` absent produit des zones et une aide
 *                vides.
 */
[[nodiscard]] EditorStatusLines editorStatusLines(const EditorStatusContext& context);

}  // namespace hmi
