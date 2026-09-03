// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>

#include "Core/Levels/LevelProperties.h"
#include "Core/Levels/TileMap.h"

/**
 * @file Core/Levels/TileLayer.h
 * @brief Couche de tuiles typée d'une carte (`LOT-04`).
 */

namespace core {

/**
 * @brief Rôle d'une couche de tuiles dans une carte.
 *
 * Un RPG en vue de dessus a besoin d'au moins trois couches là où un jeu de plateforme se
 * contentait d'une grille unique :
 *
 * - `Ground` — ce qu'on voit sous les pieds (herbe, dalle, eau) ;
 * - `Decor` — dessiné **au-dessus** du sol, et devant ou derrière le personnage selon sa position
 *   (arbre, tonneau, tapis) ;
 * - `Collision` — masque **indépendant du visuel** : un tapis se traverse, un tonneau non, et les
 *   deux peuvent reposer sur la même image de sol. C'est cette couche, et elle seule, que
 *   consomment le balayage AABB puis la grille de combat tactique (`LOT-19`).
 *
 * `Legacy` désigne la grille unique d'une carte au format `version: 2`, promue telle quelle au
 * chargement : elle vaut à la fois décor et collision, comme dans le format d'origine.
 */
enum class LayerKind {
    Ground,
    Decor,
    Collision,
    Legacy,
};

/// Nombre de valeurs de `core::LayerKind` — même convention que `core::TILE_TYPE_COUNT` : toute
/// valeur ajoutée doit l'être **avant** le dernier énumérateur.
inline constexpr int LAYER_KIND_COUNT = static_cast<int>(LayerKind::Legacy) + 1;

/**
 * @brief Nom JSON d'un rôle de couche.
 *
 * Déclaré ici, et non dans le chargeur ou l'écrivain, parce que les **deux** en ont besoin et
 * qu'un rôle nommé différemment de part et d'autre casserait l'aller-retour sans qu'aucun test
 * unitaire de l'un ou de l'autre pris isolément ne le voie. Le `switch` est exhaustif et sans
 * `default` : un rôle ajouté sans nom ne compile pas, plutôt que de retomber silencieusement sur
 * une valeur arbitraire.
 */
[[nodiscard]] constexpr const char* layerKindName(LayerKind kind) noexcept {
    switch (kind) {
        case LayerKind::Ground:
            return "ground";
        case LayerKind::Decor:
            return "decor";
        case LayerKind::Collision:
            return "collision";
        case LayerKind::Legacy:
            return "legacy";
    }
    return "ground";
}

/**
 * @brief Une couche de tuiles : son rôle, sa grille, et ses propriétés libres.
 *
 * Toutes les couches d'une carte partagent ses dimensions — le chargeur le vérifie, une couche
 * décalée d'une case rendrait la collision incohérente avec ce qui est affiché.
 */
struct TileLayer {
    /// Nom de la couche, libre et affiché par l'éditeur (« sol », « décor », « collision »…).
    std::string name;
    /// Rôle de la couche.
    LayerKind kind = LayerKind::Ground;
    /// Grille de tuiles de cette couche. Sans défaut : `core::TileMap` n'est pas constructible par
    /// défaut, ce qui rend l'omission détectable à la compilation (même parti que `LevelData`).
    TileMap tiles;
    /// Propriétés libres (`core::PropertyMap`), y compris les clés que le chargeur n'a pas
    /// reconnues — elles sont réémises telles quelles à l'écriture.
    PropertyMap properties;
};

}  // namespace core
