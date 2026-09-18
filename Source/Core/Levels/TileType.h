// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

/**
 * @file Core/Levels/TileType.h
 * @brief Types de tuiles d'une carte et utilitaires associés.
 */

namespace core {

/**
 * @brief Type d'une tuile de la grille d'une carte (`EX-LVL-002`).
 *
 * `Empty` est la case traversable par défaut ; `Solid` bloque le déplacement ; `Entry` est le point
 * d'arrivée par défaut du héros sur la carte. Le reste est le vocabulaire du **terrain** en vue de
 * dessus (`EX-EXP-005`, `LOT-08`) : `Grass`, `Dirt`, `Sand`, `Water` et `DeepWater` (sols, les
 * quatre premiers traversables), `Wall` et `Cliff` (obstacles), `Bridge` et `Stairs` (passages).
 *
 * Ce qui **agit** sur une carte — PNJ, coffres, portails, rencontres — n'est pas un type de tuile
 * mais une entité (`core::MapEntity`, `EX-LVL-017`) : une grille ne retient qu'un type par case.
 */
enum class TileType {
    Empty,
    Solid,
    Entry,
    // --- Terrain du RPG en vue de dessus (LOT-08) ---
    /// Herbe : le sol par défaut d'une carte extérieure.
    Grass,
    /// Terre battue : chemins, cours, sols d'intérieur sommaires.
    Dirt,
    /// Sable : plages et déserts. Traversable, comme l'herbe et la terre.
    Sand,
    /// Eau **peu profonde** : traversable — on y patauge. Sa distinction d'avec `DeepWater` est
    /// la seule chose qui permette à une rive d'exister.
    Water,
    /// Eau **profonde** : infranchissable tant qu'aucune règle de nage n'existe (`isSolid`).
    DeepWater,
    /// Mur : la matière pleine bâtie, distincte de `Solid` (matière générique du socle) parce que
    /// l'éditeur et l'habillage doivent pouvoir les vêtir différemment.
    Wall,
    /// Falaise : rupture de relief infranchissable, l'obstacle **naturel** du décor extérieur.
    Cliff,
    /// Pont : franchit l'eau ou un ravin. Traversable, et c'est tout son intérêt.
    Bridge,
    /// Escalier : liaison verticale d'une carte à l'autre, traversable. Ce qu'il **relie** est une
    /// donnée du graphe de cartes (`LOT-09`), jamais du type de tuile.
    Stairs,
};

/**
 * @brief Nombre de valeurs de `core::TileType` — **seule** source de vérité de la fin de
 *        l'énumération (`LOT-74` TACHE-02).
 *
 * Ajouter un type ne demande rien d'autre que de l'ajouter ci-dessus, **avant** le dernier
 * énumérateur : cette constante suit, et ses consommateurs avec elle. L'ajouter en **fin** de
 * liste demande en plus de re-pointer cette seule ligne.
 *
 * Volontairement une constante libre plutôt qu'un énumérateur `Count` : les `switch` sur `TileType`
 * du projet sont **exhaustifs et sans `default`** (c'est ce qui fait que le compilateur désigne
 * lui-même les points à mettre à jour), et un énumérateur sentinelle les obligerait tous à traiter
 * un cas qui ne décrit aucune tuile.
 */
inline constexpr int TILE_TYPE_COUNT = static_cast<int>(TileType::Stairs) + 1;

/**
 * @brief Indique si un type de tuile bloque le déplacement de manière **statique**.
 *
 * @note `DeepWater` **bloque** : ce n'est pas de la matière, mais rien ne permet encore de la
 *       franchir. Le jour où une règle de nage existera, elle sortira d'ici — et ce sera le seul
 *       endroit à changer, ce qui est précisément l'intérêt de l'y mettre plutôt que de parsemer
 *       le code de tests « sauf si c'est de l'eau ».
 * @param type Type de tuile.
 * @return `true` pour la matière pleine (`Solid`, `Wall`, `Cliff`) et pour l'eau profonde.
 */
[[nodiscard]] constexpr bool isSolid(TileType type) noexcept {
    return type == TileType::Solid || type == TileType::Wall || type == TileType::Cliff ||
           type == TileType::DeepWater;
}

}  // namespace core
