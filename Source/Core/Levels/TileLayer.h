// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <string>

#include "Core/Levels/LevelProperties.h"
#include "Core/Levels/TileMap.h"
#include "Core/Levels/TileType.h"

/**
 * @file Core/Levels/TileLayer.h
 * @brief Couche de tuiles typée d'une carte (`LOT-04`).
 */

namespace core {

/**
 * @brief Rôle d'une couche de tuiles dans une carte.
 *
 * Un RPG en vue de dessus a besoin d'au moins trois couches là où une grille unique ne suffit
 * pas :
 *
 * - `Ground` — ce qu'on voit sous les pieds (herbe, dalle, eau) ;
 * - `Decor` — dessiné **au-dessus** du sol, et devant ou derrière le personnage selon sa position
 *   (arbre, tonneau, tapis) ;
 * - `Collision` — masque **indépendant du visuel** : un tapis se traverse, un tonneau non, et les
 *   deux peuvent reposer sur la même image de sol. C'est cette couche, et elle seule, que
 *   consomment le déplacement en exploration et la grille de combat tactique (`LOT-19`).
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
 * @brief Vrai si une couche de rôle @p kind porte une **image** — le sol ou le décor —, et non le
 *        masque de collision ou la grille unique d'une carte `version: 2`.
 *
 * C'est la distinction que l'éditeur fait entre les couches qu'on peint à part (`LOT-11`) et la
 * grille racine, qui reste la seule où vit l'entrée.
 */
[[nodiscard]] constexpr bool isVisualLayerKind(LayerKind kind) noexcept {
    switch (kind) {
        case LayerKind::Ground:
        case LayerKind::Decor:
            return true;
        case LayerKind::Collision:
        case LayerKind::Legacy:
            return false;
    }
    return false;
}

/**
 * @brief Vrai si @p type se peint sur une couche **visuelle** (sol ou décor, `LOT-11`).
 *
 * Une couche visuelle est une image : elle accepte le vide, la matière générique et tout le
 * vocabulaire de terrain du `LOT-08`. Elle refuse l'entrée, qui n'a de sens que dans la grille de
 * collision : elle porte une **règle**, et une règle posée sur une couche que le jeu ne lit pas
 * serait un piège silencieux. Le `switch` est exhaustif : un type ajouté sans décision ne compile
 * pas.
 */
[[nodiscard]] constexpr bool isVisualLayerTileType(TileType type) noexcept {
    switch (type) {
        case TileType::Empty:
        case TileType::Solid:
        case TileType::Grass:
        case TileType::Dirt:
        case TileType::Sand:
        case TileType::Water:
        case TileType::DeepWater:
        case TileType::Wall:
        case TileType::Cliff:
        case TileType::Bridge:
        case TileType::Stairs:
            return true;
        case TileType::Entry:
            return false;
    }
    return false;
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
