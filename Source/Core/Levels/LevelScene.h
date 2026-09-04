// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstddef>
#include <functional>

#include "Core/Ecs/Components/Sprite.h"  // core::AtlasRegion
#include "Core/Ecs/Entity.h"
#include "Core/Levels/TileLayer.h"
#include "Core/Levels/TileType.h"

/**
 * @file Core/Levels/LevelScene.h
 * @brief Projection d'un niveau en entités ECS (une tuile non vide = un sprite).
 */

namespace core {

class World;
class Level;

/**
 * @brief Peuple un `World` d'une **entité par tuile non vide** du niveau.
 *
 * Chaque tuile non `Empty` devient une entité portant un `Transform` (position = colonne, ligne
 * en unités monde) et un `Sprite` (couche 0, région fournie par @p regionForTile). Logique pure,
 * indépendante du rendu (`EX-ARCH-011`) : la correspondance **type de tuile → région d'atlas**
 * est **injectée**, ce qui rend la projection testable sans GPU (une fausse correspondance suffit).
 *
 * @param world         Monde à peupler.
 * @param level         Niveau source.
 * @param regionForTile Correspondance type de tuile → région d'atlas (dépendance de rendu
 * injectée).
 * @param onTileEntity  Rappel invoqué après la création de chaque entité tuile, avec le **rôle
 *                      de sa couche**, son type et sa case. Permet à la couche de présentation d'y
 *                      attacher ses **propres** composants (`hmi::TileSkinTag`, `LOT-42`, et
 *                      depuis le `LOT-07` le calque de dessin d'une tuile de décor) sans que
 *                      `Core` connaisse ni l'habillage ni les calques (`EX-NFR-011`) — même
 *                      principe d'injection que @p regionForTile. Vide par défaut.
 *
 *                      Le rôle est passé plutôt que l'indice de couche : ce qui décide de la
 *                      présentation d'une tuile, c'est ce qu'elle **est** (un sol, un décor), pas
 *                      son rang dans une liste que l'auteur de la carte peut réordonner.
 *
 * Ne peuple **que** les tuiles. Les plans picturaux (`EX-DEC-040`, LOT-69) qui ont remplacé les
 * décors-sprites ne sont **pas** des entités : ce sont des données d'habillage du niveau, composées
 * directement par `HMI` sans passer par l'ECS.
 */
void buildLevelScene(
    World& world, const Level& level, const std::function<AtlasRegion(TileType)>& regionForTile,
    const std::function<void(Entity, LayerKind, TileType, int, int)>& onTileEntity = {});

}  // namespace core
