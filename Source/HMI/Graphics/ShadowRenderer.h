// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "HMI/Graphics/ComposedScene.h"
#include "HMI/Graphics/RenderMode.h"
#include "HMI/Graphics/TileAppearance.h"

/**
 * @file HMI/Graphics/ShadowRenderer.h
 * @brief Ombres du plan physique, sur `RenderLayer::Shadow` (`LOT-55`, `EX-REN-045`).
 */

namespace core {
class World;
class TileMap;
}  // namespace core

namespace hmi {

/// Décalage de l'ombre par rapport à sa tuile, en unités monde (1 pixel à 16 px/unité).
inline constexpr float SHADOW_OFFSET_X = 1.0f / 16.0f;
inline constexpr float SHADOW_OFFSET_Y = 1.0f / 16.0f;

/// Opacité de l'ombre (constante, cf. décisions de cadrage du LOT-55 : pas de réglage designer).
inline constexpr float SHADOW_OPACITY = 0.4f;

/**
 * @brief Compose une ombre décalée pour chaque tuile physique visible, sur `RenderLayer::Shadow`.
 *
 * Parcourt les mêmes entités que `hmi::composeWorldSprites` (`core::Transform` + `hmi::
 * TileSkinTag`) mais n'en retient que les tuiles qui **projettent une ombre** : pleines
 * (`core::isSolid`, `EX-GP-001`) ou à silhouette inclinée/courbe (`hmi::hasSilhouette`, `LOT-42`).
 * La forme échantillonnée est celle de `hmi::regionForTile` — **le même** atlas procédural que le
 * mode Physique et que le masque de silhouette (`hmi::isInsideSilhouette`) : une case carrée
 * pleine ou arrondi/pente est déjà, dans cet atlas, opaque **exactement** là où la matière est
 * présente et transparente ailleurs (`hmi::isInsideSilhouette`, `hmi::
 * buildProceduralAtlasImage`). Teinter cette région en noir semi-transparent donne donc l'ombre à
 * sa forme réelle sans réimplémenter la géométrie, et sans le moindre nouveau prédicat de solidité
 * dans `Core` — un bloc réduit (`BlockHalf`/`BlockQuarter`) hérite de la même façon de sa taille
 * réelle via `core::Transform::scale` (`core::tileVisualScale`), déjà porté par l'entité comme pour
 * son propre sprite.
 *
 * Une porte (`core::TileType::Door`) n'est **jamais** solide au sens de `core::isSolid` — sa
 * solidité réelle dépend de l'état du mécanisme qui la commande, jamais reflété par `hmi::
 * TileSkinTag::type` (figé au chargement). @p doorCollision, quand fourni, tranche donc son ombre
 * d'après l'état **courant** (`core::MechanismController::collisionMap`) plutôt que d'après le
 * type statique — une porte fermée projette une ombre, une porte ouverte n'en projette plus.
 * `nullptr` (l'éditeur, qui ne simule aucun mécanisme) exclut simplement les portes de l'ombrage.
 *
 * Un bloc poussable (`EX-GP-022`) suit son ombre automatiquement : son entité porte la même
 * position interpolée (`hmi::PreviousPosition`, `EX-ARCH-031`) que celle utilisée pour son propre
 * sprite, jamais recalculée ici.
 * @param scene         Scène à remplir (non vidée : la composition peut être cumulative).
 * @param world         Monde dont on lit `Transform`, `TileSkinTag`, `PreviousPosition`.
 * @param mode           Mode de rendu courant : sans effet en dehors de `RenderMode::Texture`
 *                       (`EX-REN-046`) — le mode Physique reste la lecture nue des collisions.
 * @param textures       Textures liables (seul `hmi::SceneTextures::atlas` est utilisé : l'ombre
 *                       n'est jamais celle d'un skin, toujours celle de l'atlas procédural).
 * @param interpolationAlpha Facteur d'interpolation `[0, 1[` (`EX-ARCH-031`), identique à celui de
 *                       `hmi::composeWorldSprites`.
 * @param doorCollision  Grille de collision courante des mécanismes (`core::MechanismController::
 *                       collisionMap`), ou `nullptr` pour ignorer les portes (aucune simulation de
 *                       mécanisme, comme dans l'éditeur).
 */
void composeShadows(ComposedScene& scene, core::World& world, RenderMode mode,
                    const SceneTextures& textures, float interpolationAlpha,
                    const core::TileMap* doorCollision = nullptr);

}  // namespace hmi
