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
 * @note Ce vocabulaire est celui du **socle** hérité du moteur. Le `LOT-01` a retiré les types
 *       propres au jeu de plateforme (pentes, arrondis, blocs réduits, plateformes mobiles, blocs
 *       volatils, dangers directionnels, mobiles et temporisés), sans objet en vue de dessus. Le
 *       vocabulaire propre au RPG (terrains, obstacles, escaliers, ponts) est introduit par le
 *       `LOT-08`.
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
};

/**
 * @brief Nombre de valeurs de `core::TileType` — **seule** source de vérité de la fin de
 *        l'énumération (`LOT-74` TACHE-02).
 *
 * Ajouter un type ne demande rien d'autre que de l'ajouter ci-dessus, **avant** le dernier
 * énumérateur : cette constante suit, et ses consommateurs avec elle.
 *
 * Volontairement une constante libre plutôt qu'un énumérateur `Count` : les `switch` sur `TileType`
 * du projet sont **exhaustifs et sans `default`** (c'est ce qui fait que le compilateur désigne
 * lui-même les points à mettre à jour), et un énumérateur sentinelle les obligerait tous à traiter
 * un cas qui ne décrit aucune tuile.
 */
inline constexpr int TILE_TYPE_COUNT = static_cast<int>(TileType::LockedDoor) + 1;

/**
 * @brief Indique si un type de tuile bloque le déplacement de manière **statique**.
 *
 * @note La solidité d'une porte dépend de son **état** (ouverte/fermée) et la position d'un bloc
 *       poussable évolue en jeu : les deux sont résolues par la simulation, jamais par ce test
 *       statique.
 * @param type Type de tuile.
 * @return `true` pour `Solid` et `Block`, la matière pleine par nature.
 */
[[nodiscard]] constexpr bool isSolid(TileType type) noexcept {
    return type == TileType::Solid || type == TileType::Block;
}

}  // namespace core
