// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include "Core/Math/Vector2.h"

/**
 * @file HMI/Graphics/PreviousPosition.h
 * @brief Composant de rendu : position d'une entité au pas de simulation précédent (interpolation).
 */

namespace hmi {

/**
 * @brief Position d'une entité au **pas de simulation précédent**, pour l'interpolation de rendu
 *        (`EX-ARCH-031`).
 *
 * La simulation avance par pas fixes discrets (`core::FixedTimestep`, @ref guide-boucle) tandis que
 * le rendu redessine une fois par frame réelle, souvent plus fréquente (écran 120/144 Hz). Sans
 * interpolation, une entité en mouvement continu (personnage, danger mobile, bloc poussé) apparaît
 * par « à-coups » figés à sa dernière position simulée. En conservant la position du pas précédent
 * en plus de la position courante (`core::Transform`), `SpriteRenderer` peut dessiner l'entité à
 * `lerp(précédente, courante, alpha)` où `alpha ∈ [0, 1[` est le facteur d'interpolation fourni par
 * `core::FixedTimestep::interpolationAlpha` — un mouvement lisse quel que soit le framerate de
 * rendu.
 *
 * C'est un composant **de présentation** (`hmi`, jamais `core`) : il vit néanmoins dans le
 * `core::World` (générique sur le type de composant), écrit par `HMI` (`hmi::GameSession` recopie
 * la position courante vers ce champ au début de chaque pas) et lu par `HMI`
 * (`hmi::SpriteRenderer`). `Core` l'ignore totalement — aucune règle de simulation n'en dépend, la
 * frontière `HMI → Core` reste intacte (`EX-NFR-010`, `EX-ARCH-012`). Une entité **sans** ce
 * composant (les tuiles fixes, l'immense majorité) est simplement dessinée à sa position courante,
 * sans interpolation.
 */
struct PreviousPosition {
    /// Position de l'entité (coin haut-gauche, unités monde) telle que laissée par le pas
    /// précédent.
    core::Vector2 value{};
};

}  // namespace hmi
