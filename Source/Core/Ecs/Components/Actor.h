// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "Core/Math/Vector2.h"

/**
 * @file Core/Ecs/Components/Actor.h
 * @brief Composant du personnage qui se déplace sur la carte (données pures).
 */

namespace core {

/**
 * @brief Marque une entité **acteur** — un personnage qui occupe une case et se déplace — et porte
 *        ce que ce déplacement laisse derrière lui : son orientation et sa masse.
 *
 * Donnée pure sans logique (`EX-ARCH-011`). Remplace `core::Player`, dont les trente champs
 * décrivaient un personnage de **plateforme** (contact au sol, coyote time, jump buffering, dash,
 * wall jump, combos) et que le `LOT-01` avait laissés **inertes** en retirant le système qui les
 * calculait. Un RPG en vue de dessus n'a besoin d'aucun d'eux : pas de gravité, donc pas de sol à
 * quitter, pas de saut à amortir, pas de mur où glisser.
 *
 * S'appelle `Actor` et non `Player` parce que rien ici ne suppose **le** personnage du joueur :
 * les PNJ (`LOT-15`) et les alliés du groupe (`LOT-29`) portent le même composant. Ce qui distingue
 * l'entité pilotée par les entrées, c'est de recevoir une intention, pas d'être d'un autre type.
 */
struct Actor {
    /**
     * @brief Orientation courante, en vecteur unitaire du monde (`y` vers le bas).
     *
     * Un **vecteur**, et non le `-1`/`+1` du platformer : en vue de dessus, un personnage regarde
     * dans huit directions, et « la dernière direction de marche » est ce dont ont besoin le sprite
     * (`LOT-08`), l'interaction avec ce qui est devant (`LOT-10`) et l'attaque au corps à corps
     * (`LOT-21`). **Conservée à l'arrêt** : un personnage immobile regarde là où il allait, jamais
     * vers une direction par défaut (`EX-EXP-004`).
     */
    Vector2 facing{1.0F, 0.0F};

    /**
     * @brief Masse, en unité de jeu arbitraire.
     *
     * Seuil des mécanismes sensibles au poids — une plaque de pression ne s'enfonce que sous une
     * masse suffisante (`EX-GP-025`). Sans gravité, elle ne sert plus à rien d'autre : la vitesse
     * terminale de chute qu'elle réglait autrefois n'existe plus.
     */
    float mass = 1.0F;
};

}  // namespace core
