// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "HMI/Graphics/ParticleRenderer.h"

#include "Core/Ecs/Components/Particle.h"
#include "Core/Ecs/World.h"
#include "HMI/Graphics/TextureAtlas.h"

namespace hmi {

namespace {

// Calque de dessin d'un effet (EX-REN-043) : une trainee de dash passe DERRIERE le personnage,
// une bouffee de poussiere ou un eclat de mort DEVANT (perimetre du lot, epic.md).
RenderLayer layerForKind(core::ParticleKind kind) {
    switch (kind) {
        case core::ParticleKind::DashTrail:
            return RenderLayer::Object;
        case core::ParticleKind::LandingDust:
        case core::ParticleKind::Death:
            return RenderLayer::Foreground;
    }
    return RenderLayer::Foreground;  // inatteignable : le switch couvre tout l'enum.
}

struct ParticleColor {
    float r;
    float g;
    float b;
};

// Teinte par effet : porte entierement l'apparence, aucun asset dedie (voir en-tete).
ParticleColor colorForKind(core::ParticleKind kind) {
    switch (kind) {
        case core::ParticleKind::DashTrail:
            return ParticleColor{0.75f, 0.9f, 1.0f};  // trainee claire et froide
        case core::ParticleKind::LandingDust:
            return ParticleColor{0.75f, 0.65f, 0.45f};  // poussiere terreuse
        case core::ParticleKind::Death:
            return ParticleColor{0.95f, 0.25f, 0.2f};  // eclat rouge
    }
    return ParticleColor{1.0f, 1.0f, 1.0f};  // inatteignable : le switch couvre tout l'enum.
}

}  // namespace

void composeParticles(ComposedScene& scene, core::World& world, RenderMode mode,
                      const SceneTextures& textures) {
    if (mode != RenderMode::Texture) {
        return;  // mode Physique : lecture nue des collisions, aucun effet (EX-REN-046).
    }

    // Region opaque unie (teintee), meme patron que les segments unis de hmi::DraftRenderer :
    // aucun asset dedie, la teinte porte l'effet.
    const core::AtlasRegion solid = TextureAtlas::tile(0, 0);
    const float atlasWidth = static_cast<float>(textures.atlasWidth);
    const float atlasHeight = static_cast<float>(textures.atlasHeight);
    const float u0 = static_cast<float>(solid.x) / atlasWidth;
    const float v0 = static_cast<float>(solid.y) / atlasHeight;
    const float u1 = static_cast<float>(solid.x + solid.width) / atlasWidth;
    const float v1 = static_cast<float>(solid.y + solid.height) / atlasHeight;

    world.view<core::Particle>().each([&](core::Entity, const core::Particle& particle) {
        const float fade = (particle.maxLife > 0.0f) ? (particle.life / particle.maxLife) : 0.0f;
        const ParticleColor color = colorForKind(particle.kind);

        SpriteQuad quad;
        quad.x = particle.position.x - (PARTICLE_QUAD_SIZE * 0.5f);
        quad.y = particle.position.y - (PARTICLE_QUAD_SIZE * 0.5f);
        quad.width = PARTICLE_QUAD_SIZE;
        quad.height = PARTICLE_QUAD_SIZE;
        quad.u0 = u0;
        quad.v0 = v0;
        quad.u1 = u1;
        quad.v1 = v1;
        quad.r = color.r;
        quad.g = color.g;
        quad.b = color.b;
        quad.a = fade;
        scene.addSprite(layerForKind(particle.kind), textures.atlas, 0, quad);
    });
}

}  // namespace hmi
