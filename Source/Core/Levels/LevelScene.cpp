// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Core/Levels/LevelScene.h"

#include <cstdint>

#include "Core/Ecs/Components/Transform.h"
#include "Core/Ecs/World.h"
#include "Core/Levels/Level.h"
#include "Core/Levels/TileLayer.h"
#include "Core/Levels/TileMap.h"
#include "Core/Math/Vector2.h"
#include "Core/Physics/Aabb.h"

namespace core {

namespace {

// Peuple le monde des tuiles non vides d'UNE grille. @p order ordonne les sprites entre couches :
// le sol est dessine avant le decor, et sous lui.
void buildLayerEntities(
    World& world, const TileMap& map, LayerKind kind, std::int32_t order,
    const std::function<AtlasRegion(TileType)>& regionForTile,
    const std::function<void(Entity, LayerKind, TileType, int, int)>& onTileEntity) {
    for (int row = 0; row < map.height(); ++row) {
        for (int column = 0; column < map.width(); ++column) {
            const TileType type = map.tile(column, row);
            if (type == TileType::Empty) {
                continue;
            }
            const Entity entity = world.createEntity();

            Transform transform;
            transform.position = Vector2{static_cast<float>(column), static_cast<float>(row)};
            world.addComponent(entity, transform);

            Sprite sprite;
            sprite.region = regionForTile(type);
            // `layer` ordonne les sprites d'un **meme** calque de rendu : le rang de la couche y
            // suffit, une carte a une couche (le cas legacy) retombant sur 0 comme avant le
            // LOT-04. Le calque lui-meme est une notion de presentation (hmi::RenderLayer,
            // LOT-40) que `Core` ignore (EX-NFR-011).
            sprite.layer = order;
            world.addComponent(entity, sprite);

            // La presentation attache ici ses propres composants (habillage) : Core fournit le
            // point d'accroche, sans rien savoir de ce qui s'y greffe.
            if (onTileEntity) {
                onTileEntity(entity, kind, type, column, row);
            }
        }
    }
}

}  // namespace

// Peuple un World d'une entite (Transform + Sprite) par tuile non vide de chaque couche VISIBLE du
// niveau. Les plans picturaux (LOT-69) ne sont pas des entites : HMI les compose directement.
void buildLevelScene(
    World& world, const Level& level, const std::function<AtlasRegion(TileType)>& regionForTile,
    const std::function<void(Entity, LayerKind, TileType, int, int)>& onTileEntity) {
    // Une carte sans couche declaree -- un Level construit directement, sans passer par le
    // chargeur -- n'a que sa grille racine : c'est le comportement d'avant le LOT-04, conserve tel
    // quel plutot que de faire dependre le rendu d'une promotion faite ailleurs.
    if (level.layers().empty()) {
        buildLayerEntities(world, level.tileMap(), LayerKind::Legacy, 0, regionForTile,
                           onTileEntity);
        return;
    }

    std::int32_t order = 0;
    for (const TileLayer& layer : level.layers()) {
        // La couche de collision est un MASQUE, pas une image : un tonneau infranchissable pose
        // sur une dalle se voit par le decor, jamais par la collision. La dessiner recouvrirait le
        // sol de tuiles fantomes (EX-LVL-016).
        if (layer.kind == LayerKind::Collision) {
            continue;
        }
        buildLayerEntities(world, layer.tiles, layer.kind, order, regionForTile, onTileEntity);
        ++order;
    }
}

}  // namespace core
