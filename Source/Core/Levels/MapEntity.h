// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>

#include "Core/Levels/GridPosition.h"
#include "Core/Levels/LevelProperties.h"

/**
 * @file Core/Levels/MapEntity.h
 * @brief Entité placée sur une carte : PNJ, coffre, panneau, portail, déclencheur (`LOT-04`).
 */

namespace core {

/**
 * @brief Une entité de carte — ce qui n'est **pas** une tuile.
 *
 * La grille ne porte qu'un `core::TileType` par case, sans métadonnée : un PNJ a un nom et un
 * dialogue, un coffre un contenu et un état « déjà ouvert », un portail une carte cible et un
 * point d'arrivée. Le jeu de plateforme d'origine contournait cette limite avec des vecteurs
 * annexes indexés par position, un par famille — le `LOT-01` en a retiré quatre, tous devenus
 * caducs en même temps que leur type de tuile. Une **liste d'objets à propriétés** évite de
 * reconstruire ce schéma une famille à la fois.
 *
 * `Core` ne connaît **aucune** sémantique de `type` : c'est une chaîne, interprétée par le
 * gameplay (`LOT-10` et suivants). Le chargeur ne valide donc pas qu'un type existe — une entité
 * inconnue est une erreur de conception tolérée, pas une carte invalide (`EX-NFR-040`).
 */
struct MapEntity {
    /// Type de l'entité, libre : `"npc"`, `"chest"`, `"sign"`, `"portal"`, `"encounter"`…
    std::string type;
    /// Case occupée par l'entité.
    GridPosition position;
    /// Propriétés libres (`core::PropertyMap`), y compris les clés que le chargeur n'a pas
    /// reconnues — elles sont réémises telles quelles à l'écriture.
    PropertyMap properties;
};

}  // namespace core
