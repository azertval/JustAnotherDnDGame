// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_particle_system.cpp
 * @brief Tests unitaires de l'émetteur de particules déterministe (`LOT-53` TACHE-01,
 *        `EX-REN-008`).
 */

#include <vector>

#include <gtest/gtest.h>

#include "Core/Ecs/Components/Actor.h"
#include "Core/Ecs/Components/Particle.h"
#include "Core/Ecs/Systems/ParticleSystem.h"
#include "Core/Ecs/World.h"
#include "Core/Math/Vector2.h"

namespace {

// Effet a duree de vie et vitesse FIXES (min == max) : rend la trajectoire et l'instant de
// disparition d'une particule exactement previsibles, sans que l'alea n'y interfere.
constexpr core::ParticleEffect FIXED_EFFECT{
    /*count*/ 1,      /*speedMin*/ 2.0f, /*speedMax*/ 2.0f,
    /*lifeMin*/ 0.5f, /*lifeMax*/ 0.5f,  /*spreadRadians*/ 0.0f};

std::vector<core::Particle> snapshot(core::World& world) {
    std::vector<core::Particle> particles;
    world.view<core::Particle>().each(
        [&](core::Entity, const core::Particle& particle) { particles.push_back(particle); });
    return particles;
}

}  // namespace

/**
 * @brief Deux exécutions de la même séquence d'émissions/pas produisent exactement les mêmes
 * particules (position, vitesse, durée de vie) à chaque pas : c'est le test central de la tâche.
 * \castest{<b>Déterminisme : même séquence -> mêmes particules.</b><br/>
 * \tcat Unitaire · Émetteur de particules<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Deux `ParticleSystem`/`World` construits à l'identique (graine par défaut).<br/>
 * 2. Rejouer sur chacun la même séquence : émettre un effet dispersé, avancer plusieurs pas,
 * émettre à nouveau, avancer encore.<br/>
 * \tattendu Les deux mondes contiennent, à chaque pas, des particules aux mêmes positions,
 * vitesses et durées de vie restantes.
 * }
 */
TEST(ParticleSystemTest, DeterminismeMemeSequenceMemesParticules) {
    constexpr core::ParticleEffect DISPERSED_EFFECT{
        /*count*/ 6,      /*speedMin*/ 1.0f, /*speedMax*/ 4.0f,
        /*lifeMin*/ 0.3f, /*lifeMax*/ 1.2f,  /*spreadRadians*/ 1.5f};
    constexpr float FIXED_DELTA = 1.0f / 60.0f;

    core::World worldA;
    core::ParticleSystem systemA;
    core::World worldB;
    core::ParticleSystem systemB;

    const auto runSequence = [&](core::World& world, core::ParticleSystem& system) {
        system.spawn(world, DISPERSED_EFFECT, core::Vector2{1.0f, 2.0f}, core::Vector2{0.3f, -1.0f},
                     core::ParticleKind::LandingDust);
        for (int step = 0; step < 5; ++step) {
            system.update(world, FIXED_DELTA);
        }
        system.spawn(world, DISPERSED_EFFECT, core::Vector2{-2.0f, 0.5f},
                     core::Vector2{-1.0f, 0.0f}, core::ParticleKind::DashTrail);
        for (int step = 0; step < 5; ++step) {
            system.update(world, FIXED_DELTA);
        }
    };
    runSequence(worldA, systemA);
    runSequence(worldB, systemB);

    ASSERT_EQ(systemA.aliveCount(), systemB.aliveCount());
    const std::vector<core::Particle> particlesA = snapshot(worldA);
    const std::vector<core::Particle> particlesB = snapshot(worldB);
    ASSERT_EQ(particlesA.size(), particlesB.size());
    for (std::size_t i = 0; i < particlesA.size(); ++i) {
        EXPECT_FLOAT_EQ(particlesA[i].position.x, particlesB[i].position.x);
        EXPECT_FLOAT_EQ(particlesA[i].position.y, particlesB[i].position.y);
        EXPECT_FLOAT_EQ(particlesA[i].velocity.x, particlesB[i].velocity.x);
        EXPECT_FLOAT_EQ(particlesA[i].velocity.y, particlesB[i].velocity.y);
        EXPECT_FLOAT_EQ(particlesA[i].life, particlesB[i].life);
    }
}

/**
 * @brief Émettre bien au-delà du budget ne dépasse jamais `MAX_PARTICLES`, et recycle les plus
 * anciennes en premier (les origines les plus récemment émises survivent).
 * \castest{<b>Budget : jamais dépassé, recyclage de la plus ancienne.</b><br/>
 * \tcat Unitaire · Émetteur de particules<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Émettre `MAX_PARTICLES + 5` particules individuelles, une origine distincte
 * chacune (x = 0, 1, 2...).<br/>2. Lire le nombre de particules vivantes et leurs origines.<br/>
 * \tattendu `aliveCount() == MAX_PARTICLES` ; les 5 premières origines (x = 0..4) ont disparu, les
 * plus récentes (x = 5..MAX_PARTICLES+4) sont toutes présentes.
 * }
 */
TEST(ParticleSystemTest, BudgetJamaisDepasseRecycleLaPlusAncienne) {
    core::World world;
    core::ParticleSystem system;

    constexpr int EXTRA_EMISSIONS = 5;
    for (int i = 0; i < core::MAX_PARTICLES + EXTRA_EMISSIONS; ++i) {
        system.spawn(world, FIXED_EFFECT, core::Vector2{static_cast<float>(i), 0.0f},
                     core::Vector2{1.0f, 0.0f}, core::ParticleKind::Death);
    }

    EXPECT_EQ(system.aliveCount(), core::MAX_PARTICLES);
    const std::vector<core::Particle> particles = snapshot(world);
    ASSERT_EQ(particles.size(), static_cast<std::size_t>(core::MAX_PARTICLES));
    for (const core::Particle& particle : particles) {
        EXPECT_GE(particle.position.x,
                  static_cast<float>(EXTRA_EMISSIONS));  // les 5 plus vieilles ont disparu
    }
}

/**
 * @brief Une particule disparaît exactement au pas où sa durée de vie atteint zéro, ni avant ni
 * après.
 * \castest{<b>Durée de vie : disparition au pas attendu.</b><br/>
 * \tcat Unitaire · Émetteur de particules<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Émettre une particule de durée de vie fixe 1,0 s.<br/>2. Avancer de 0,25 s par pas
 * (fixedDelta, exactement représentable en flottant), trois fois.<br/>3. Avancer un quatrième
 * pas.<br/>
 * \tattendu Vivante après les trois premiers pas (life = 0,25) ; disparue après le quatrième.
 * }
 */
TEST(ParticleSystemTest, DureeDeVieDisparaitAuPasAttendu) {
    constexpr core::ParticleEffect ONE_SECOND_EFFECT{
        /*count*/ 1,      /*speedMin*/ 2.0f, /*speedMax*/ 2.0f,
        /*lifeMin*/ 1.0f, /*lifeMax*/ 1.0f,  /*spreadRadians*/ 0.0f};
    core::World world;
    core::ParticleSystem system;
    constexpr float FIXED_DELTA = 0.25f;  // exactement representable : aucune derive flottante

    system.spawn(world, ONE_SECOND_EFFECT, core::Vector2{0.0f, 0.0f}, core::Vector2{1.0f, 0.0f},
                 core::ParticleKind::LandingDust);
    ASSERT_EQ(system.aliveCount(), 1);

    for (int step = 0; step < 3; ++step) {
        system.update(world, FIXED_DELTA);
    }
    ASSERT_EQ(system.aliveCount(), 1) << "ne doit pas disparaitre avant le pas attendu";
    const std::vector<core::Particle> stillAlive = snapshot(world);
    ASSERT_EQ(stillAlive.size(), 1u);
    EXPECT_FLOAT_EQ(stillAlive.front().life, 0.25f);

    system.update(world, FIXED_DELTA);
    EXPECT_EQ(system.aliveCount(), 0) << "doit avoir disparu exactement a ce pas";
}

/**
 * @brief Les particules n'ont aucun effet sur une entité de gameplay présente dans le même monde.
 * \castest{<b>Aucun effet sur le gameplay.</b><br/>
 * \tcat Unitaire · Émetteur de particules<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Peupler un `core::Actor` dans le monde.<br/>2. Émettre et simuler des
 * particules plusieurs pas.<br/>
 * \tattendu Les champs du `core::Actor` restent strictement inchangés.
 * }
 */
TEST(ParticleSystemTest, AucunEffetSurLesEntitesDeGameplay) {
    core::World world;
    core::ParticleSystem system;
    const core::Entity player = world.createEntity();
    const core::Actor original{};
    world.addComponent(player, original);

    constexpr core::ParticleEffect BURST{
        /*count*/ 20,     /*speedMin*/ 0.5f, /*speedMax*/ 3.0f,
        /*lifeMin*/ 0.2f, /*lifeMax*/ 0.8f,  /*spreadRadians*/ 3.14159f};
    system.spawn(world, BURST, core::Vector2{0.0f, 0.0f}, core::Vector2{1.0f, 0.0f},
                 core::ParticleKind::Death);
    for (int step = 0; step < 10; ++step) {
        system.update(world, 1.0f / 60.0f);
    }

    const core::Actor& after = world.getComponent<core::Actor>(player);
    EXPECT_FLOAT_EQ(after.facing.x, original.facing.x);
    EXPECT_FLOAT_EQ(after.facing.y, original.facing.y);
    EXPECT_FLOAT_EQ(after.mass, original.mass);
}

/**
 * @brief `clear()` détruit toutes les particules vivantes (rechargement de niveau, TACHE-02).
 * \castest{<b>clear() vide toutes les particules vivantes.</b><br/>
 * \tcat Unitaire · Émetteur de particules<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Émettre plusieurs particules.<br/>2. Appeler `clear()`.<br/>
 * \tattendu `aliveCount() == 0`, et la simulation suivante n'a rien à intégrer.
 * }
 */
TEST(ParticleSystemTest, ClearVideToutesLesParticulesVivantes) {
    core::World world;
    core::ParticleSystem system;
    system.spawn(world, FIXED_EFFECT, core::Vector2{0.0f, 0.0f}, core::Vector2{1.0f, 0.0f},
                 core::ParticleKind::Death);
    ASSERT_EQ(system.aliveCount(), 1);

    system.clear(world);

    EXPECT_EQ(system.aliveCount(), 0);
    EXPECT_TRUE(snapshot(world).empty());
}
