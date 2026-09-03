// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_shadow_render.cpp
 * @brief Tests unitaires des ombres du plan physique (LOT-55, EX-REN-045).
 */

#include <optional>

#include <gtest/gtest.h>

#include "Core/Ecs/Components/Transform.h"
#include "Core/Ecs/World.h"
#include "Core/Levels/TileMap.h"
#include "Core/Levels/TileType.h"
#include "HMI/Graphics/PreviousPosition.h"
#include "HMI/Graphics/QuadRecorder.h"
#include "HMI/Graphics/ShadowRenderer.h"
#include "HMI/Graphics/TileSilhouette.h"
#include "HMI/Graphics/TileSkinTag.h"
#include "HMI/Graphics/TileVisuals.h"

namespace {

// Texture factice : la composition ne fait que comparer des identites (cf. test_quad_recorder).
int textureStorage = 0;
hmi::TextureHandle texture = &textureStorage;

hmi::SceneTextures testTextures() {
    hmi::SceneTextures textures;
    textures.atlas = texture;
    textures.atlasWidth = 80;  // 5 colonnes x 16 px (hmi::TextureAtlas::TILES_PER_SIDE)
    textures.atlasHeight = 160;
    return textures;
}

// Ajoute une entite-tuile (Transform + TileSkinTag), meme placement que
// hmi::DraftRenderer::rebuild / core::buildLevelScene : une tuile occupe sa case entiere.
core::Entity addTile(core::World& world, core::TileType type, int column, int row) {
    const core::Entity entity = world.createEntity();
    world.addComponent(
        entity, core::Transform{core::Vector2{static_cast<float>(column), static_cast<float>(row)},
                                core::Vector2{1.0f, 1.0f}, 0.0f});
    world.addComponent(entity, hmi::TileSkinTag{type, 0, std::nullopt, std::nullopt});
    return entity;
}

}  // namespace

/**
 * @brief Une tuile pleine (Solid) projette un quad d'ombre, decale, sur RenderLayer::Shadow.
 * \castest{<b>Tuile pleine : quad d'ombre decale sur Shadow.</b><br/>
 * \tcat Unitaire · Ombres du plan physique<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Peupler une tuile Solid.<br/>2. Composer les ombres en mode Texture.<br/>
 * \tattendu Un quad est emis sur RenderLayer::Shadow, decale de l'offset attendu.
 * }
 */
TEST(ShadowRenderTest, TuilePleineProjetteUnQuadDecaleSurShadow) {
    core::World world;
    addTile(world, core::TileType::Solid, 3, 2);

    hmi::ComposedScene scene;
    hmi::composeShadows(scene, world, hmi::RenderMode::Texture, testTextures(), 0.0f);

    hmi::QuadRecorder recorder;
    recorder.record(scene);
    ASSERT_EQ(recorder.size(), 1u) << recorder.describe();
    EXPECT_EQ(recorder.countOnLayer(hmi::RenderLayer::Shadow), 1);
    const hmi::ComposedQuad& quad = recorder.quads().front();
    EXPECT_FLOAT_EQ(quad.sprite.x, 3.0f + hmi::SHADOW_OFFSET_X);
    EXPECT_FLOAT_EQ(quad.sprite.y, 2.0f + hmi::SHADOW_OFFSET_Y);
    EXPECT_FLOAT_EQ(quad.sprite.width, 1.0f);
    EXPECT_FLOAT_EQ(quad.sprite.height, 1.0f);
    EXPECT_LT(quad.sprite.a, 1.0f);  // semi-transparente
}

/**
 * @brief Chacun des douze types a silhouette projette une ombre echantillonnant EXACTEMENT la
 * meme region d'atlas que hmi::regionForTile — la meme forme que la silhouette affichee, sans
 * reimplementer la geometrie (LOT-42).
 * \castest{<b>Silhouettes : ombre a la meme region d'atlas que la tuile.</b><br/>
 * \tcat Unitaire · Ombres du plan physique<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Peupler chacun des douze types a silhouette.<br/>2. Composer les ombres.<br/>
 * \tattendu Les UV du quad d'ombre correspondent a hmi::regionForTile(type).
 * }
 */
TEST(ShadowRenderTest, DouzeSilhouettesMemeRegionQueRegionForTile) {
    for (const core::TileType type : hmi::SILHOUETTE_TILE_TYPES) {
        core::World world;
        addTile(world, type, 0, 0);

        const hmi::SceneTextures textures = testTextures();
        hmi::ComposedScene scene;
        hmi::composeShadows(scene, world, hmi::RenderMode::Texture, textures, 0.0f);

        hmi::QuadRecorder recorder;
        recorder.record(scene);
        ASSERT_EQ(recorder.size(), 1u) << recorder.describe();

        const core::AtlasRegion region = hmi::regionForTile(type);
        const float expectedU0 = static_cast<float>(region.x) / textures.atlasWidth;
        const float expectedV0 = static_cast<float>(region.y) / textures.atlasHeight;
        const float expectedU1 = static_cast<float>(region.x + region.width) / textures.atlasWidth;
        const float expectedV1 =
            static_cast<float>(region.y + region.height) / textures.atlasHeight;
        const hmi::ComposedQuad& quad = recorder.quads().front();
        EXPECT_FLOAT_EQ(quad.sprite.u0, expectedU0);
        EXPECT_FLOAT_EQ(quad.sprite.v0, expectedV0);
        EXPECT_FLOAT_EQ(quad.sprite.u1, expectedU1);
        EXPECT_FLOAT_EQ(quad.sprite.v1, expectedV1);
    }
}

/**
 * @brief Un bloc poussable en mouvement (PreviousPosition distincte de la position courante) voit
 * son ombre interpolee exactement comme son propre sprite (EX-ARCH-031).
 * \castest{<b>Bloc poussable en mouvement : ombre interpolee.</b><br/>
 * \tcat Unitaire · Ombres du plan physique<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Peupler un Block avec une PreviousPosition distincte.<br/>2. Composer a
 * interpolationAlpha = 0.5.<br/>
 * \tattendu Le quad d'ombre est a mi-chemin entre les deux positions, plus l'offset.
 * }
 */
TEST(ShadowRenderTest, BlocPoussableEnMouvementOmbreInterpolee) {
    core::World world;
    const core::Entity block = addTile(world, core::TileType::Block, 5, 5);
    world.addComponent(block, hmi::PreviousPosition{core::Vector2{4.0f, 5.0f}});

    hmi::ComposedScene scene;
    hmi::composeShadows(scene, world, hmi::RenderMode::Texture, testTextures(), 0.5f);

    hmi::QuadRecorder recorder;
    recorder.record(scene);
    ASSERT_EQ(recorder.size(), 1u) << recorder.describe();
    EXPECT_FLOAT_EQ(recorder.quads().front().sprite.x, 4.5f + hmi::SHADOW_OFFSET_X);
    EXPECT_FLOAT_EQ(recorder.quads().front().sprite.y, 5.0f + hmi::SHADOW_OFFSET_Y);
}

/**
 * @brief Une porte suit l'etat COURANT de la grille de collision des mecanismes, pas son type
 * statique (toujours TileType::Door) : fermee (solide) elle projette une ombre, ouverte elle n'en
 * projette plus ; sans grille fournie, jamais d'ombre.
 * \castest{<b>Porte : ombre selon l'etat courant de la grille de collision.</b><br/>
 * \tcat Unitaire · Ombres du plan physique<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Peupler une porte.<br/>2. Composer sans grille, puis avec une grille ou elle est
 * fermee, puis ouverte.<br/>
 * \tattendu Aucune ombre sans grille ; une ombre fermee ; aucune ouverte.
 * }
 */
TEST(ShadowRenderTest, PorteSuitLEtatCourantDeLaGrilleDeCollision) {
    core::World world;
    addTile(world, core::TileType::Door, 2, 1);
    const hmi::SceneTextures textures = testTextures();

    // Sans grille de collision (editeur, aucune simulation de mecanisme) : jamais d'ombre.
    {
        hmi::ComposedScene scene;
        hmi::composeShadows(scene, world, hmi::RenderMode::Texture, textures, 0.0f, nullptr);
        EXPECT_EQ(scene.size(), 0u);
    }

    // Porte fermee dans la grille de collision (Solid) : une ombre.
    {
        core::TileMap collision(4, 4);
        collision.setTile(2, 1, core::TileType::Solid);
        hmi::ComposedScene scene;
        hmi::composeShadows(scene, world, hmi::RenderMode::Texture, textures, 0.0f, &collision);
        EXPECT_EQ(scene.size(), 1u);
    }

    // Porte ouverte dans la grille de collision (Door, jamais core::isSolid) : aucune ombre.
    {
        core::TileMap collision(4, 4);
        collision.setTile(2, 1, core::TileType::Door);
        hmi::ComposedScene scene;
        hmi::composeShadows(scene, world, hmi::RenderMode::Texture, textures, 0.0f, &collision);
        EXPECT_EQ(scene.size(), 0u);
    }
}
