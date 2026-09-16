// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <optional>

#include "Core/Levels/GridPosition.h"
#include "Core/Levels/TileType.h"

/**
 * @file HMI/Editor/LinkGesture.h
 * @brief Machine à état du geste de l'outil « Lien » (LOT-37), pure et testable.
 */

namespace hmi {

/// @brief Case en attente d'appariement (premier clic de l'outil « Lien »), avec son type courant.
struct PendingLink {
    core::GridPosition cell;
    core::TileType tileType;
};

/// @brief Action à appliquer sur le brouillon/l'état d'édition suite à un clic de l'outil « Lien ».
enum class LinkGestureAction {
    Ignore,          ///< Case cliquée ni déclencheur ni cible : sans effet.
    SetPending,      ///< Aucune attente valide : la case cliquée devient l'attente.
    ReplacePending,  ///< Attente de même catégorie que le clic : remplacée par la case cliquée.
    Link,            ///< Catégories complémentaires, pas encore liées : créer la liaison.
    Unlink,          ///< Catégories complémentaires, déjà liées : supprimer la liaison (bascule).
};

/// @brief Décision résultant de `resolveLinkClick` : action à appliquer et paramètres associés.
struct LinkGestureDecision {
    LinkGestureAction action = LinkGestureAction::Ignore;
    core::GridPosition cell{};            ///< Valide pour `SetPending`/`ReplacePending`.
    core::GridPosition switchPosition{};  ///< Valide pour `Link`/`Unlink`.
    core::GridPosition targetPosition{};  ///< Valide pour `Link`/`Unlink`.
};

/**
 * @brief Résout le clic de l'outil « Lien » sur @p clickedCell selon l'attente courante.
 *
 * Reproduit le geste de liaison de l'éditeur (clic déclencheur → attente de cible, clic cible →
 * liaison créée ; refaire la même paire la supprime) : case ni déclencheur ni cible → ignorée ;
 * pas d'attente valide → la case cliquée devient l'attente ; attente de même catégorie que le clic
 * (deux déclencheurs ou deux cibles) → remplace l'attente ; catégories complémentaires → crée ou
 * retire la liaison (bascule) selon @p alreadyLinked. Une attente dont le type a changé entretemps
 * (case repeinte) n'est plus ni déclencheur ni cible : traitée comme aucune attente.
 * @param pending           Case en attente d'appariement, si une a été posée par un clic précédent.
 * @param clickedCell       Case visée par le clic courant.
 * @param clickedTileType   Type de tuile à @p clickedCell au moment du clic.
 * @param alreadyLinked     `true` si la paire (déclencheur, cible) résolue est déjà liée dans le
 *                          brouillon — non pertinent (ignoré) si les catégories ne se complètent
 *                          pas encore.
 * @return La décision à appliquer.
 */
[[nodiscard]] LinkGestureDecision resolveLinkClick(std::optional<PendingLink> pending,
                                                   core::GridPosition clickedCell,
                                                   core::TileType clickedTileType,
                                                   bool alreadyLinked) noexcept;

}  // namespace hmi
