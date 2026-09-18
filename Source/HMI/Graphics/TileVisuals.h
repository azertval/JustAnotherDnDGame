// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include "Core/Ecs/Components/Sprite.h"
#include "Core/Levels/TileType.h"

/**
 * @file HMI/Graphics/TileVisuals.h
 * @brief Correspondance type de tuile → région d'atlas, partagée par le canevas et la palette de
 *        l'éditeur.
 */

namespace hmi {

/**
 * @brief Région d'atlas (couleur procédurale) associée à un type de tuile.
 *
 * **Unique** correspondance type → région, utilisée par `hmi::DraftRenderer` (canevas de
 * l'éditeur) et la palette de l'éditeur (`hmi::PalettePanel`) : la vignette de la palette et la
 * case peinte sur le canevas ont toujours la même couleur.
 * Ne dépend que de la **géométrie de grille** de `hmi::TextureAtlas` (constantes et découpage
 * statiques), jamais d'une texture chargée : la palette de l'éditeur peut donc l'appeler sans
 * contexte GPU, ce qu'un widget Qt ne doit de toute façon jamais exiger.
 * @param type Type de tuile (`Empty` renvoie une région arbitraire, jamais dessinée en pratique
 *             — les cases vides ne sont pas rendues par l'appelant).
 * @return La région d'atlas à échantillonner pour ce type.
 */
[[nodiscard]] core::AtlasRegion regionForTile(core::TileType type);

}  // namespace hmi
