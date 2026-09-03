// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

/**
 * @file HMI/Editor/ThumbnailGeometry.h
 * @brief Dimensionnement des vignettes de l'éditeur à l'échelle d'affichage réelle (`LOT-56`
 *        TACHE-05, `EX-IHM-053`).
 *
 * Logique **pure** (aucune dépendance Qt/GPU), testable hors instance d'application
 * (`EX-NFR-010`) — compilée à la fois dans `JustAnotherDnDGame` et directement dans `UnitTests`.
 * Réutilisable par tout widget affichant du pixel art agrandi au plus proche voisin
 * (`PalettePanel`, `AssetThumbnailView`, `TexturePanel`, et le canevas de `LOT-54`) : une fonction
 * privée à l'un d'eux serait réécrite ailleurs, et l'erreur d'arrondi reviendrait par la porte de
 * service.
 */

namespace hmi {

/**
 * @brief Taille en pixels **réels** pour une taille logique et un facteur d'échelle d'affichage
 *        donnés.
 *
 * Ne produit jamais zéro, même pour un facteur non entier (125 %, 150 %) appliqué à la plus
 * petite taille d'icône des jetons : un défaut d'arrondi produirait sinon une vignette invisible
 * plutôt que simplement floue.
 * @param logicalSize  Taille demandée, en pixels logiques (jetons de design, `SizeTokens`).
 * @param scaleFactor  Facteur d'échelle du support d'affichage (`devicePixelRatio`).
 * @return `round(logicalSize * scaleFactor)`, borné à 1 au minimum.
 */
[[nodiscard]] int thumbnailPixelSize(int logicalSize, double scaleFactor) noexcept;

}  // namespace hmi
