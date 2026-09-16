// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <cstddef>
#include <optional>
#include <string>

#include "Core/Levels/GridPosition.h"

namespace core {
class LevelDraft;
}

/**
 * @file HMI/Editor/EntityGesture.h
 * @brief Geste de l'outil « Entité » (`LOT-11`) : sélectionner, poser, déplacer — pur et testable.
 */

namespace hmi {

/// @brief Ce que l'outil « Entité » fait d'un clic ou d'un glisser.
enum class EntityGestureAction {
    Ignore,    ///< Rien à faire.
    Select,    ///< Sélectionner l'entité `entityIndex`.
    Deselect,  ///< Case libre sans famille à poser : la sélection se vide.
    Place,     ///< Poser une entité de la famille choisie en `cell`.
    Move,      ///< Déplacer l'entité `entityIndex` en `cell`.
};

/// @brief Décision rendue par `resolveEntityPress` ou `resolveEntityRelease`.
struct EntityGestureDecision {
    EntityGestureAction action = EntityGestureAction::Ignore;
    std::size_t entityIndex = 0;
    core::GridPosition cell{};

    [[nodiscard]] bool operator==(const EntityGestureDecision&) const = default;
};

/**
 * @brief Résout l'appui du bouton gauche sur @p cell.
 *
 * Une case **occupée** sélectionne l'entité du dessus (la dernière posée), et c'est elle qu'un
 * glisser emportera ; une case **libre** pose la famille choisie, ou vide la sélection s'il n'y en
 * a pas. Poser **sur** une entité existante demande @p forcePlace (`Ctrl`) : sans quoi un clic pour
 * sélectionner un coffre en empilerait un second dessus, et l'erreur ne se verrait pas — deux
 * entités sur une case se dessinent comme une.
 *
 * @param draft       Le brouillon.
 * @param cell        La case cliquée, dans la grille.
 * @param kindToPlace Le type à poser, ou vide pour l'outil en simple sélection.
 * @param forcePlace  Vrai pour poser même sur une case occupée.
 */
[[nodiscard]] EntityGestureDecision resolveEntityPress(const core::LevelDraft& draft,
                                                       core::GridPosition cell,
                                                       const std::string& kindToPlace,
                                                       bool forcePlace);

/**
 * @brief Résout le relâchement d'un glisser commencé sur l'entité @p grabbed.
 * @return `Move` si le glisser a changé de case, `Ignore` sinon (un clic sur place reste une
 *         sélection, déjà faite à l'appui).
 */
[[nodiscard]] EntityGestureDecision resolveEntityRelease(std::optional<std::size_t> grabbed,
                                                         core::GridPosition pressCell,
                                                         core::GridPosition releaseCell);

}  // namespace hmi
