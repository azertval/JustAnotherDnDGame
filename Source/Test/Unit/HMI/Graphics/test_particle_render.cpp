// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

/**
 * @file test_particle_render.cpp
 * @brief Tests unitaires du rendu des particules (`LOT-53` TACHE-03, `EX-REN-008`).
 */

#include <gtest/gtest.h>

#include "Core/Ecs/Components/Particle.h"
#include "Core/Ecs/World.h"
#include "HMI/Graphics/ParticleRenderer.h"
#include "HMI/Graphics/QuadRecorder.h"

namespace {

int textureStorage = 0;
hmi::TextureHandle texture = &textureStorage;

hmi::SceneTextures testTextures() {
    hmi::SceneTextures textures;
    textures.atlas = texture;
    // Atlas FICTIF de 5 x 10 tuiles de 16 px, volontairement independant de
    // hmi::TextureAtlas::TILES_PER_SIDE : ce test verifie le decoupage des quads de particules a
    // partir d'une region deja calculee, jamais la geometrie de l'atlas reel.
    textures.atlasWidth = 80;
    textures.atlasHeight = 160;
    return textures;
}

core::Entity addParticle(core::World& world, core::Vector2 position, float life, float maxLife,
                         core::ParticleKind kind) {
    const core::Entity entity = world.createEntity();
    world.addComponent(entity,
                       core::Particle{position, core::Vector2{0.0f, 0.0f}, life, maxLife, kind});
    return entity;
}

}  // namespace

/**
 * @brief Une particule vivante produit un quad centré sur sa position, de taille
 * `PARTICLE_QUAD_SIZE`, teinté avec une opacité égale à `life / maxLife`.
 * \castest{<b>Une particule -> un quad centré, opacité = life/maxLife.</b><br/>
 * \tcat Unitaire · Rendu des particules<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Peupler une particule à mi-vie (life = maxLife / 2).<br/>2. Composer en mode
 * Texture.<br/>
 * \tattendu Un quad centré sur la position de la particule, de côté `PARTICLE_QUAD_SIZE`,
 * d'opacité 0,5.
 * }
 */
TEST(ParticleRenderTest, UneParticuleUnQuadCentreOpaciteProportionnelle) {
    core::World world;
    addParticle(world, core::Vector2{3.0f, 2.0f}, 0.5f, 1.0f, core::ParticleKind::LandingDust);

    hmi::ComposedScene scene;
    hmi::composeParticles(scene, world, hmi::RenderMode::Texture, testTextures());

    hmi::QuadRecorder recorder;
    recorder.record(scene);
    ASSERT_EQ(recorder.size(), 1u) << recorder.describe();
    const hmi::ComposedQuad& quad = recorder.quads().front();
    EXPECT_FLOAT_EQ(quad.sprite.x, 3.0f - hmi::PARTICLE_QUAD_SIZE * 0.5f);
    EXPECT_FLOAT_EQ(quad.sprite.y, 2.0f - hmi::PARTICLE_QUAD_SIZE * 0.5f);
    EXPECT_FLOAT_EQ(quad.sprite.width, hmi::PARTICLE_QUAD_SIZE);
    EXPECT_FLOAT_EQ(quad.sprite.height, hmi::PARTICLE_QUAD_SIZE);
    EXPECT_FLOAT_EQ(quad.sprite.a, 0.5f);
}

/**
 * @brief Une traînée de dash passe sur `RenderLayer::Object` (derrière le personnage) ; une
 * bouffée de poussière et un éclat de mort passent sur `RenderLayer::Foreground` (devant).
 * \castest{<b>Calque selon l'effet : Object pour le dash, Foreground pour le reste.</b><br/>
 * \tcat Unitaire · Rendu des particules<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Peupler une particule de chaque type.<br/>2. Composer en mode Texture.<br/>
 * \tattendu La traînée de dash est sur `Object` ; la poussière et l'éclat de mort sur
 * `Foreground`.
 * }
 */
TEST(ParticleRenderTest, CalqueSelonLEffet) {
    core::World world;
    addParticle(world, core::Vector2{}, 1.0f, 1.0f, core::ParticleKind::DashTrail);
    addParticle(world, core::Vector2{}, 1.0f, 1.0f, core::ParticleKind::LandingDust);
    addParticle(world, core::Vector2{}, 1.0f, 1.0f, core::ParticleKind::Death);

    hmi::ComposedScene scene;
    hmi::composeParticles(scene, world, hmi::RenderMode::Texture, testTextures());

    hmi::QuadRecorder recorder;
    recorder.record(scene);
    ASSERT_EQ(recorder.size(), 3u) << recorder.describe();
    EXPECT_EQ(recorder.countOnLayer(hmi::RenderLayer::Object), 1);
    EXPECT_EQ(recorder.countOnLayer(hmi::RenderLayer::Foreground), 2);
}

/**
 * @brief Aucun quad de particule n'est composé en mode Physique.
 * \castest{<b>Mode Physique : aucun quad de particule.</b><br/>
 * \tcat Unitaire · Rendu des particules<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Peupler une particule vivante.<br/>2. Composer en mode Physique.<br/>
 * \tattendu Aucun quad n'est émis (`EX-REN-046`).
 * }
 */
TEST(ParticleRenderTest, ModePhysiqueAucunQuad) {
    core::World world;
    addParticle(world, core::Vector2{}, 1.0f, 1.0f, core::ParticleKind::Death);

    hmi::ComposedScene scene;
    hmi::composeParticles(scene, world, hmi::RenderMode::Physique, testTextures());

    EXPECT_EQ(scene.size(), 0u);
}
