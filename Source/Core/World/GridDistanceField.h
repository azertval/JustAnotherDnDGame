// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <vector>

#include "Core/Levels/GridPosition.h"
#include "Core/Levels/TileMap.h"

/**
 * @file Core/World/GridDistanceField.h
 * @brief Champ de distances de plus court chemin sur la grille, respectant les murs (amendement
 * `LOT-ANNEXE-08`, `EX-IA-023`).
 */

namespace core {

/**
 * @brief Distance de plus court chemin en nombre de cases (4-connexité, respectant les cases
 * statiquement solides d'un `core::TileMap`), pré-calculée une fois par BFS multi-source depuis une
 * case cible.
 *
 * Sert de base à la récompense de progression (`EX-IA-023`). La propriété recherchée est que
 * *tout pas rapprochant du but sur un chemin réellement empruntable* soit récompensé, y compris
 * un détour imposé par un mur : une distance à vol d'oiseau pénaliserait ce détour, et
 * dirigerait donc l'agent contre la seule solution disponible.
 *
 * Le coût est borné par construction : un unique BFS à quatre voisins, sans pondération ni
 * replanification, puis des lectures `O(1)`. La distance se compte en nombre de cases, jamais en
 * coût de déplacement -- il n'y a pas de planificateur de chemin ici.
 *
 * Accepte n'importe quelle `core::TileMap`, et c'est le choix de cette grille qui fixe ce que
 * « empruntable » veut dire :
 * - `core::Level::tileMap()` ne connaît que la solidité **statique** ;
 * - `core::MechanismController::collisionMap()` connaît en plus l'état **courant** des portes,
 *   solides tant qu'elles sont fermées (le contrôleur les gère dynamiquement, `TileMap::isSolid`
 *   les ignore).
 *
 * Une porte verrouillée encore fermée doit être vue comme un mur, sans quoi le champ mesure la
 * progression le long d'un chemin qui n'existe pas et la clé qui l'ouvre ne raccourcit rien.
 */
class GridDistanceField {
public:
    /**
     * @brief Calcule le champ par BFS à partir de l'unique case @p target.
     * @param tileMap Grille de collision (cases solides/non-solides) --
     * `core::MechanismController:: collisionMap()` pour respecter l'état courant des portes,
     * `core::Level::tileMap()` pour la seule solidité statique.
     * @param target  Case cible (typiquement `core::Level::exit()`), point de départ du BFS.
     */
    GridDistanceField(const core::TileMap& tileMap, const core::GridPosition& target);

    /**
     * @brief Calcule le champ par BFS **multi-source** à partir de @p targets.
     *
     * Généralise le constructeur à cible unique : la distance renvoyée par `distance()` est celle
     * au **plus proche** élément de @p targets, ce qui permet de faire cohabiter la sortie du
     * niveau avec les positions des déclencheurs (`core::Mechanism::switchPosition`) pas encore
     * résolus comme cibles concurrentes -- l'objectif immédiat le plus proche, sans hiérarchie ni
     * connaissance de l'ordre de résolution attendu.
     * @param tileMap Grille de collision (voir constructeur à cible unique).
     * @param targets Cases cibles ; une case hors-grille ou solide est simplement ignorée (garde
     *        défensive, ne fait pas échouer les autres cibles). Vide -> champ entièrement
     *        inatteignable.
     */
    GridDistanceField(const core::TileMap& tileMap, const std::vector<core::GridPosition>& targets);

    /**
     * @brief Distance de plus court chemin, en nombre de cases, depuis @p position jusqu'à la case
     * cible passée au constructeur.
     * @param position Case dont on veut la distance ; hors-grille traitée comme inatteignable.
     * @return La distance en cases, ou la sentinelle `largeur * hauteur` (borne supérieure stricte
     *         de toute distance atteignable sur la grille) si @p position est hors-grille ou ne
     * peut atteindre la cible par aucun chemin de cases non solides.
     */
    [[nodiscard]] int distance(const core::GridPosition& position) const noexcept;

    /**
     * @brief La cible est-elle atteignable depuis @p position par un chemin de cases non solides ?
     *
     * Distingue une distance réelle de la **sentinelle** rendue par `distance()`, que rien ne
     * permet sinon de reconnaître : la sentinelle vaut `largeur × hauteur`, soit `576` sur une
     * grille `24 × 24`, et une progression calculée entre une case atteignable et une case qui ne
     * l'est pas vaudrait donc `±576` sur un seul pas.
     * @param position Case à tester ; hors-grille traitée comme inatteignable.
     */
    [[nodiscard]] bool isReachable(const core::GridPosition& position) const noexcept;

    /// @return La sentinelle rendue par `distance()` pour une case inatteignable
    /// (`largeur × hauteur`), borne supérieure stricte de toute distance atteignable — donc aussi
    /// la seule constante de normalisation qui ne dépende pas du niveau chargé.
    [[nodiscard]] int unreachableDistance() const noexcept {
        return _width * _height;
    }

private:
    int _width;
    int _height;
    std::vector<int> _distances;
};

}  // namespace core
