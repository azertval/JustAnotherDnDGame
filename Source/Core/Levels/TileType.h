// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

/**
 * @file Core/Levels/TileType.h
 * @brief Types de tuiles d'une carte et utilitaires associés.
 */

namespace core {

/**
 * @brief Type d'une tuile de la grille d'une carte (`EX-GP-001`).
 *
 * `Empty` est la case traversable par défaut ; `Solid` bloque le déplacement ; `Danger`
 * provoque l'échec au contact ; `Entry`/`Exit` sont l'apparition et la sortie. `Switch`,
 * `PressurePlate` et `Door` sont les mécanismes classiques — l'interrupteur bascule au contact,
 * la plaque reste active tant qu'un poids y repose, les deux ouvrant une porte liée.
 * `Key`/`LockedDoor` sont la seconde paire déclencheur↔cible, la clé ouvrant sa porte
 * **définitivement**. `Block` est un bloc poussable.
 *
 * Le `LOT-08` y ajoute le vocabulaire du **terrain** en vue de dessus (`EX-EXP-005`) : `Grass`,
 * `Dirt`, `Sand`, `Water` et `DeepWater` (sols, les quatre premiers traversables), `Wall` et
 * `Cliff` (obstacles), `Bridge` et `Stairs` (passages). Il **ajoute** sans rien remplacer : le
 * vocabulaire de puzzle hérité sert tel quel au RPG.
 *
 * @note Le `LOT-01` avait retiré les types propres au jeu de plateforme (pentes, arrondis, blocs
 *       réduits, plateformes mobiles, blocs volatils, dangers directionnels, mobiles et
 *       temporisés), sans objet en vue de dessus.
 */
enum class TileType {
    Empty,
    Solid,
    Danger,
    Entry,
    Exit,
    Switch,
    Door,
    PressurePlate,
    Block,
    Key,
    LockedDoor,
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
 * liste, comme l'a fait le `LOT-08`, demande en plus de re-pointer cette seule ligne — c'est le
 * prix d'un vocabulaire qui se lit dans son ordre d'apparition plutôt qu'en désordre.
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
 * @note La solidité d'une porte dépend de son **état** (ouverte/fermée) et la position d'un bloc
 *       poussable évolue en jeu : les deux sont résolues par la simulation, jamais par ce test
 *       statique.
 * @note `DeepWater` **bloque** : ce n'est pas de la matière, mais rien ne permet encore de la
 *       franchir. Le jour où une règle de nage existera, elle sortira d'ici — et ce sera le seul
 *       endroit à changer, ce qui est précisément l'intérêt de l'y mettre plutôt que de parsemer
 *       le code de tests « sauf si c'est de l'eau ».
 * @param type Type de tuile.
 * @return `true` pour la matière pleine (`Solid`, `Block`, `Wall`, `Cliff`) et pour l'eau
 *         profonde.
 */
[[nodiscard]] constexpr bool isSolid(TileType type) noexcept {
    return type == TileType::Solid || type == TileType::Block || type == TileType::Wall ||
           type == TileType::Cliff || type == TileType::DeepWater;
}

}  // namespace core
