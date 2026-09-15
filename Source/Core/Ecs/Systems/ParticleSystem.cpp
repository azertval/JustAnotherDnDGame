// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "Core/Ecs/Systems/ParticleSystem.h"

#include <algorithm>
#include <cmath>

#include "Core/Ecs/World.h"
#include "Core/Math/DeterministicRandom.h"

namespace core {

namespace {

// Trainee de dash (LOT-53 TACHE-02) : emission CONTINUE (l'appelant invoque emitDashTrail a
// chaque pas fixe ou le dash est actif) -- quelques particules par pas suffisent, un flux dense
// nait de la repetition, pas d'un gros paquet ponctuel.
constexpr ParticleEffect DASH_TRAIL_EFFECT{
    /*count*/ 2,       /*speedMin*/ 0.3f, /*speedMax*/ 0.8f,
    /*lifeMin*/ 0.15f, /*lifeMax*/ 0.3f,  /*spreadRadians*/ 0.5f};

// Bouffee de poussiere a l'atterrissage : vitesse/duree de vie fixes, seul le NOMBRE varie avec
// l'intensite de l'impact (emitLanding calcule le compte, puis construit l'effet).
constexpr float LANDING_DUST_SPEED_MIN = 0.4f;
constexpr float LANDING_DUST_SPEED_MAX = 1.2f;
constexpr float LANDING_DUST_LIFE_MIN = 0.2f;
constexpr float LANDING_DUST_LIFE_MAX = 0.4f;
constexpr float LANDING_DUST_SPREAD_RADIANS = 0.9f;

// Eclatement a la mort : rafale en cercle complet (spreadRadians = PI rend la direction de base
// sans effet, voir ParticleSystem::emit).
constexpr float FULL_CIRCLE_RADIANS = 3.14159265358979323846f;
constexpr ParticleEffect DEATH_BURST_EFFECT{
    /*count*/ 16,      /*speedMin*/ 1.0f, /*speedMax*/ 3.0f,
    /*lifeMin*/ 0.25f, /*lifeMax*/ 0.6f,  /*spreadRadians*/ FULL_CIRCLE_RADIANS};

}  // namespace

ParticleSystem::ParticleSystem(std::uint64_t seed) noexcept : _baseSeed(seed) {}

Entity ParticleSystem::reserveSlot(World& world) {
    if (static_cast<int>(_order.size()) >= MAX_PARTICLES) {
        // Recyclage deterministe de la plus ANCIENNE (front de _order) : jamais dependant du
        // sparse set de l'ECS, qui ne garantit aucun ordre stable apres un swap-and-pop.
        const Entity recycled = _order.front();
        _order.erase(_order.begin());
        world.destroyEntity(recycled);
    }
    const Entity entity = world.createEntity();
    _order.push_back(entity);
    return entity;
}

void ParticleSystem::spawn(World& world, const ParticleEffect& effect, Vector2 origin,
                           Vector2 direction, ParticleKind kind) {
    const float baseAngle = std::atan2(direction.y, direction.x);
    for (int i = 0; i < effect.count; ++i) {
        const Entity entity = reserveSlot(world);
        // Reseede POUR CETTE particule : le resultat ne depend que du triplet (graine de base,
        // pas courant, identifiant d'entite), jamais de l'ordre/nombre d'appels a spawn() au sein
        // du meme pas (EX-NFR-002).
        DeterministicRandom rng{deriveSeed(_baseSeed, _stepIndex, entity.index)};
        const float speed = rng.nextRange(effect.speedMin, effect.speedMax);
        const float angle = baseAngle + rng.nextRange(-effect.spreadRadians, effect.spreadRadians);
        const float life = rng.nextRange(effect.lifeMin, effect.lifeMax);
        const Vector2 velocity{std::cos(angle) * speed, std::sin(angle) * speed};
        world.addComponent(entity, Particle{origin, velocity, life, life, kind});
    }
}

void ParticleSystem::emitDashTrail(World& world, Vector2 position, float facing) {
    // Part de l'ARRIERE du mouvement (oppose a l'orientation) : une trainee, pas un sillage
    // devant le personnage.
    const Vector2 direction{-facing, 0.0f};
    spawn(world, DASH_TRAIL_EFFECT, position, direction, ParticleKind::DashTrail);
}

void ParticleSystem::emitLanding(World& world, Vector2 position, float impactSpeed) {
    if (impactSpeed < LANDING_MIN_IMPACT_SPEED) {
        return;  // petit pas : aucun effet (evite un nuage de poussiere permanent).
    }
    const float range = LANDING_MAX_IMPACT_SPEED - LANDING_MIN_IMPACT_SPEED;
    const float intensity = (std::min)(1.0f, (impactSpeed - LANDING_MIN_IMPACT_SPEED) / range);
    const int countRange = LANDING_DUST_MAX_COUNT - LANDING_DUST_MIN_COUNT;
    const int count =
        LANDING_DUST_MIN_COUNT + static_cast<int>(intensity * static_cast<float>(countRange));
    const ParticleEffect effect{count,
                                LANDING_DUST_SPEED_MIN,
                                LANDING_DUST_SPEED_MAX,
                                LANDING_DUST_LIFE_MIN,
                                LANDING_DUST_LIFE_MAX,
                                LANDING_DUST_SPREAD_RADIANS};
    // Dispersion autour de la verticale (poussiere qui se souleve), calque/teinte resolus au
    // rendu (hmi::ParticleRenderer, LOT-53 TACHE-03).
    spawn(world, effect, position, Vector2{0.0f, -1.0f}, ParticleKind::LandingDust);
}

void ParticleSystem::emitDeath(World& world, Vector2 position) {
    // Direction de base sans importance : spreadRadians == PI couvre le cercle complet.
    spawn(world, DEATH_BURST_EFFECT, position, Vector2{0.0f, -1.0f}, ParticleKind::Death);
}

void ParticleSystem::update(World& world, float fixedDelta) {
    ++_stepIndex;
    // Compaction en place, dans l'ordre d'emission (_order reste la source de verite, jamais
    // world.view<Particle>() -- voir en-tete de la classe).
    std::size_t writeIndex = 0;
    for (std::size_t i = 0; i < _order.size(); ++i) {
        const Entity entity = _order[i];
        Particle& particle = world.getComponent<Particle>(entity);
        particle.life -= fixedDelta;
        if (particle.life <= 0.0f) {
            world.destroyEntity(entity);
            continue;  // disparait CE pas : pas recopiee dans le tableau compacte.
        }
        particle.position += particle.velocity * fixedDelta;
        _order[writeIndex++] = entity;
    }
    _order.resize(writeIndex);
}

void ParticleSystem::clear(World& world) {
    for (const Entity entity : _order) {
        world.destroyEntity(entity);
    }
    _order.clear();
}

}  // namespace core
