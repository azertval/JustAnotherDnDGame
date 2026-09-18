// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <optional>
#include <string>
#include <vector>

#include "Core/Levels/GridPosition.h"
#include "HMI/Editor/EditorTool.h"

/**
 * @file HMI/Editor/EditorStatus.h
 * @brief Choix du contenu de la barre d'état de l'éditeur : zones permanentes et aide
 *        contextuelle à l'outil actif (`EX-IHM-060`).
 */

namespace hmi {

class Localization;

/// État affiché pour une carte en cours d'édition (barre d'état, `EX-EDIT-013`/`EX-EDIT-012`).
struct LevelStatusInfo {
    std::string name;                               ///< Nom de la carte ouverte.
    bool dirty = false;                             ///< Modifications non enregistrées.
    EditorTool tool = EditorTool::Paint;            ///< Outil d'édition actif.
    std::optional<core::GridPosition> hoveredCell;  ///< Case survolée, si le curseur est dessus.
    float zoom = 1.0F;                              ///< Facteur de zoom courant.
};

/// Contexte d'édition dont la barre d'état décide l'affichage ; absent hors édition.
struct EditorStatusContext {
    std::optional<LevelStatusInfo> level;
};

/// Lignes à afficher pour la barre d'état de l'éditeur, à un instant donné.
struct EditorStatusLines {
    /// Zones permanentes, dans l'ordre d'affichage : carte, modifications non enregistrées, outil
    /// actif, case survolée, zoom. Une zone vide (chaîne vide) quand l'information n'a pas de sens
    /// — jamais de libellé de remplacement.
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
 * @param context      Contexte d'édition courant ; `context.level` absent produit des zones et une
 *                     aide vides.
 * @param localization Catalogue de traduction (`EX-REN-033`) — aucune chaîne en dur.
 */
[[nodiscard]] EditorStatusLines editorStatusLines(const EditorStatusContext& context,
                                                  const Localization& localization);

}  // namespace hmi
