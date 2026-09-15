// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include "Core/Math/Vector2.h"
#include "Core/Physics/Aabb.h"
#include "Core/Physics/SweptCollision.h"

/**
 * @file Core/Physics/AabbVsAabb.h
 * @brief Collision **continue** d'une boîte AABB mobile contre une boîte AABB **fixe**
 *        (`EX-GP-005`), en complément du balayage sur grille (`sweepAabb`).
 */

namespace core {

/**
 * @brief Déplace @p box de @p delta en s'arrêtant au premier contact avec @p obstacle, une boîte
 *        **fixe** unique (pas une grille), puis en **glissant** le long de sa surface pour le
 *        déplacement résiduel.
 *
 * Complément de `core::sweepAabb` (`SweptCollision.h`), pas une extension : `sweepAabb` résout la
 * collision sur la **grille** (murs, sols, blocs pleins — solide/vide par case entière) ;
 * `sweepAabbVsAabb` résout la collision contre un obstacle dont la boîte réelle est plus **petite**
 * qu'une case (les blocs réduits `TileType::BlockHalf`/`BlockQuarter`, `core::BlockController`) et
 * ne peut donc pas être représentée sur la grille sans bloquer à tort l'espace vide qui l'entoure.
 * Un appelant qui doit composer les deux (`hmi::GameSession`) applique `sweepAabb` puis, sur le
 * déplacement réellement obtenu, `sweepAabbVsAabb` pour chaque bloc réduit concerné — la
 * restriction la plus stricte des deux l'emporte, par construction (chaque passe ne peut que
 * réduire encore le déplacement, jamais l'étendre).
 *
 * Même méthode que `sweepX`/`sweepY` : résolution **par axe**, et clamp direct sur la coordonnée
 * de contact plutôt qu'une interpolation `position + delta * t` — une position résolue est donc
 * une coordonnée exacte, jamais un résultat d'interpolation, et aucune dérive flottante ne peut
 * s'y accumuler (voir @ref guide-physique). L'axe X est résolu en premier à partir de la position
 * de départ, puis l'axe Y à partir de la position **déjà résolue** en X — ce qui produit le même
 * glissement le long de l'obstacle qu'un mur classique.
 *
 * Précondition : @p box ne **chevauche** pas @p obstacle au départ (état non pénétrant), comme
 * pour `sweepAabb`.
 *
 * @param box      Boîte mobile avant déplacement (coin haut-gauche = position de l'entité).
 * @param delta    Déplacement voulu sur le pas, en unités monde.
 * @param obstacle Boîte fixe contre laquelle résoudre la collision (déjà réduite/centrée si
 *                 pertinent — cette fonction ne connaît ni les tuiles ni les blocs, une boîte
 *                 quelconque suffit).
 * @return La position finale, l'indicateur de blocage par axe et l'occurrence d'un contact — même
 *         structure `core::SweepResult` que `sweepAabb`, pour une composition directe côté
 *         appelant.
 */
[[nodiscard]] SweepResult sweepAabbVsAabb(const Aabb& box, const Vector2& delta,
                                          const Aabb& obstacle) noexcept;

}  // namespace core
