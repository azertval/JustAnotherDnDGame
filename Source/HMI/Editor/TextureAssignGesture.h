// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <optional>
#include <string>

#include "Core/Levels/GridPosition.h"
#include "Core/Levels/TileType.h"

/**
 * @file HMI/Editor/TextureAssignGesture.h
 * @brief Geste de l'outil « Texture par instance » (`LOT-45`), pur et testable.
 */

namespace hmi {

/// @brief Action à appliquer sur le brouillon suite à un clic de l'outil « Texture par instance ».
enum class TextureAssignAction {
    Ignore,  ///< Case vide, ou aucun asset sélectionné dans la bibliothèque : sans effet.
    Assign,  ///< Assigne (ou remplace) l'override de la case avec l'asset sélectionné.
    Remove,  ///< Retire l'override existant de la case.
};

/// @brief Décision résultant de `resolveTextureAssignClick`.
struct TextureAssignDecision {
    TextureAssignAction action = TextureAssignAction::Ignore;
    core::GridPosition cell{};  ///< Case visée ; valide pour `Assign`/`Remove`.
    std::string assetName;      ///< Asset à assigner ; valide pour `Assign` seulement.
};

/**
 * @brief Résout le clic de l'outil « Texture par instance » sur @p clickedCell.
 *
 * Ne modifie rien par lui-même : décrit l'action, l'appelant l'applique au brouillon (même
 * séparation que `hmi::resolveLinkClick`). Une case `Empty` est toujours ignorée — l'usage est
 * purement visuel, mais une case vide n'a rien à habiller. Un clic droit retire l'override existant
 * (sans effet si la case n'en a pas) ; un clic gauche sur l'asset déjà assigné retire aussi
 * l'override (bascule, évite un clic droit pour le cas courant) ; un clic gauche sans asset
 * sélectionné dans la bibliothèque est ignoré (rien à assigner).
 * @param clickedCell     Case visée par le clic.
 * @param clickedTileType Type de tuile à @p clickedCell au moment du clic.
 * @param existingOverride Nom de l'asset déjà assigné à @p clickedCell, si elle en a un.
 * @param selectedAsset   Asset actuellement sélectionné dans la bibliothèque « Objets », si un
 * l'est.
 * @param rightClick      `true` si le clic est un clic droit (retrait explicite).
 * @return La décision à appliquer.
 */
[[nodiscard]] TextureAssignDecision resolveTextureAssignClick(
    core::GridPosition clickedCell, core::TileType clickedTileType,
    const std::optional<std::string>& existingOverride,
    const std::optional<std::string>& selectedAsset, bool rightClick) noexcept;

}  // namespace hmi
