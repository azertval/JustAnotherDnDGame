// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Graphics/ShadowRenderer.h"

#include <cmath>

#include "Core/Ecs/Components/Transform.h"
#include "Core/Ecs/World.h"
#include "Core/Levels/TileMap.h"
#include "Core/Levels/TileType.h"
#include "HMI/Graphics/Camera2D.h"
#include "HMI/Graphics/PreviousPosition.h"
#include "HMI/Graphics/TileSilhouette.h"
#include "HMI/Graphics/TileSkinTag.h"
#include "HMI/Graphics/TileVisuals.h"

namespace hmi {

namespace {

// Indique si une entite-tuile projette une ombre. Une porte n'est jamais core::isSolid (son type
// statique reste TileType::Door quel que soit l'etat du mecanisme) : sa solidite REELLE est lue
// dans doorCollision, si fourni -- seul cas ou l'ombre depend d'autre chose que du type statique.
bool castsShadow(const TileSkinTag& tag, const core::Transform& transform,
                 const core::TileMap* doorCollision) {
    if (core::isSolid(tag.type) || hasSilhouette(tag.type)) {
        return true;
    }
    if (tag.type == core::TileType::Door && doorCollision != nullptr) {
        const int column = static_cast<int>(std::lround(transform.position.x));
        const int row = static_cast<int>(std::lround(transform.position.y));
        return doorCollision->inBounds(column, row) && doorCollision->isSolid(column, row);
    }
    return false;
}

}  // namespace

// Compose une ombre decalee pour chaque tuile physique visible, sur RenderLayer::Shadow.
void composeShadows(ComposedScene& scene, core::World& world, RenderMode mode,
                    const SceneTextures& textures, float interpolationAlpha,
                    const core::TileMap* doorCollision) {
    if (mode != RenderMode::Texture) {
        return;  // mode Physique : lecture nue des collisions, aucune ombre (EX-REN-046).
    }

    world.view<core::Transform, TileSkinTag>().each(
        [&](core::Entity entity, const core::Transform& transform, const TileSkinTag& tag) {
            if (!castsShadow(tag, transform, doorCollision)) {
                return;
            }

            // Meme region que le mode Physique et que le detourage de silhouette (LOT-42) : deja
            // opaque exactement la ou la matiere est presente, transparente ailleurs -- reutilisee
            // telle quelle, jamais un skin (l'ombre ignore le jeu de skins courant).
            const core::AtlasRegion region = regionForTile(tag.type);
            const float atlasWidth = static_cast<float>(textures.atlasWidth);
            const float atlasHeight = static_cast<float>(textures.atlasHeight);
            const float worldWidth =
                static_cast<float>(region.width) / Camera2D::PIXELS_PER_UNIT * transform.scale.x;
            const float worldHeight =
                static_cast<float>(region.height) / Camera2D::PIXELS_PER_UNIT * transform.scale.y;

            // Interpolee comme le sprite de la meme entite (bloc poussable en mouvement,
            // EX-ARCH-031) : jamais recalculee, seulement lue.
            core::Vector2 position = transform.position;
            if (world.hasComponent<PreviousPosition>(entity)) {
                const core::Vector2 previous = world.getComponent<PreviousPosition>(entity).value;
                position = previous + (transform.position - previous) * interpolationAlpha;
            }
            position.x += SHADOW_OFFSET_X;
            position.y += SHADOW_OFFSET_Y;

            SpriteQuad quad;
            quad.x = position.x;
            quad.y = position.y;
            quad.width = worldWidth;
            quad.height = worldHeight;
            quad.u0 = static_cast<float>(region.x) / atlasWidth;
            quad.v0 = static_cast<float>(region.y) / atlasHeight;
            quad.u1 = static_cast<float>(region.x + region.width) / atlasWidth;
            quad.v1 = static_cast<float>(region.y + region.height) / atlasHeight;
            quad.r = 0.0f;
            quad.g = 0.0f;
            quad.b = 0.0f;
            quad.a = SHADOW_OPACITY;
            scene.addSprite(RenderLayer::Shadow, textures.atlas, 0, quad);
        });
}

}  // namespace hmi
