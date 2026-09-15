// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include "Core/Math/Vector2.h"

/**
 * @file Core/Ecs/Components/Collider.h
 * @brief Composant de boîte de collision (AABB) d'une entité (données pures).
 */

namespace core {

/**
 * @brief Étendue physique d'une entité pour la collision : une boîte **alignée aux axes** (AABB).
 *
 * **Convention d'ancrage (source de vérité du lot)** : `size` est la dimension **pleine**
 * (largeur, hauteur) en unités monde, et la boîte est ancrée sur `Transform.position` par son
 * **coin haut-gauche**. Elle couvre donc l'intervalle
 * `[position.x, position.x + size.x] × [position.y, position.y + size.y]`. Ce choix reprend la
 * convention des **tuiles** (origine haut-gauche, une tuile = 1 unité, `EX-ARCH-020`), ce qui rend
 * les tests de recouvrement personnage ↔ tuile uniformes. La primitive de balayage (TACHE-02)
 * pourra convertir en centre + demi-tailles à l'interne si son calcul le demande.
 *
 * Donnée pure sans logique (`EX-ARCH-011`) : le composant ne fait que **stocker** les dimensions ;
 * toute la résolution vit dans les systèmes.
 */
struct Collider {
    /// Dimensions pleines de la boîte (largeur, hauteur), en unités monde ; coin haut-gauche
    /// ancré sur `Transform.position`. Défaut nul : boîte à préciser au moment du spawn.
    Vector2 size{};
};

}  // namespace core
