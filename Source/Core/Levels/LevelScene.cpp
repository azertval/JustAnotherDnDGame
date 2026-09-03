// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Core/Levels/LevelScene.h"

#include "Core/Ecs/Components/Transform.h"
#include "Core/Ecs/World.h"
#include "Core/Levels/Level.h"
#include "Core/Levels/TileMap.h"
#include "Core/Math/Vector2.h"
#include "Core/Physics/Aabb.h"

namespace core {

// Peuple un World d'une entite (Transform + Sprite) par tuile non vide du niveau. Les plans
// picturaux (LOT-69) ne sont pas des entites : HMI les compose directement.
void buildLevelScene(World& world, const Level& level,
                     const std::function<AtlasRegion(TileType)>& regionForTile,
                     const std::function<void(Entity, TileType, int, int)>& onTileEntity) {
    const TileMap& map = level.tileMap();
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
            // `layer` garde sa valeur par defaut : il n'ordonne que les sprites d'un **meme**
            // calque de rendu, et toutes les tuiles partagent le leur. Le calque lui-meme est une
            // notion de presentation (hmi::RenderLayer, LOT-40) que `Core` ignore (EX-NFR-011).
            world.addComponent(entity, sprite);

            // La presentation attache ici ses propres composants (habillage) : Core fournit le
            // point d'accroche, sans rien savoir de ce qui s'y greffe.
            if (onTileEntity) {
                onTileEntity(entity, type, column, row);
            }
        }
    }
}

}  // namespace core
