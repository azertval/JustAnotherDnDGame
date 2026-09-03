// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <optional>
#include <string>
#include <vector>

#include "Core/Ecs/Components/Sprite.h"
#include "Core/Levels/GridPosition.h"
#include "Core/Levels/Level.h"
#include "Core/Levels/TileType.h"

/**
 * @file HMI/Graphics/TileVisuals.h
 * @brief Correspondance type de tuile → région d'atlas, partagée entre le jeu et l'éditeur.
 */

namespace hmi {

/**
 * @brief Région d'atlas (couleur procédurale) associée à un type de tuile.
 *
 * **Unique** correspondance type → région, utilisée par `GameSession` (rendu d'un niveau joué),
 * `hmi::DraftRenderer` (canevas de l'éditeur) et la palette de l'éditeur (`hmi::PalettePanel`) :
 * le canevas de l'éditeur est un aperçu **fidèle** de ce que le joueur verra en jouant le niveau
 * (`P`), sans simplification qui pourrait induire en erreur pendant l'édition.
 * Ne dépend que de la **géométrie de grille** de `hmi::TextureAtlas` (constantes et découpage
 * statiques), jamais d'une texture chargée : la palette de l'éditeur peut donc l'appeler sans
 * device Direct3D, ce qu'un widget Qt ne doit de toute façon jamais exiger.
 * @param type Type de tuile (`Empty` renvoie une région arbitraire, jamais dessinée en pratique
 *             — les cases vides ne sont pas rendues par l'appelant).
 * @return La région d'atlas à échantillonner pour ce type.
 */
[[nodiscard]] core::AtlasRegion regionForTile(core::TileType type);

/**
 * @brief Cherche la surcharge de texture assignée à une case précise (`EX-EDIT-043`, `LOT-45`).
 *
 * Recherche linéaire : le nombre de surcharges d'un niveau reste modeste (donnée purement
 * visuelle, posée à la main), et cette fonction n'est appelée qu'à la **construction** de la scène
 * (une fois par tuile), jamais à chaque image — même principe que `solidNeighborMask`.
 * @param overrides Surcharges du niveau
 * (`core::Level::textureOverrides`/`core::LevelDraft::textureOverrides`).
 * @param position  Position de la case cherchée.
 * @return Le nom de l'asset assigné à cette case, ou `std::nullopt` si elle n'en a pas.
 */
[[nodiscard]] std::optional<std::string> textureOverrideAt(
    const std::vector<core::TileTextureOverride>& overrides, core::GridPosition position);

}  // namespace hmi
