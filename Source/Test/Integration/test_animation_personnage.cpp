// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_animation_personnage.cpp
 * @brief Tests d'intégration de l'animation du personnage : ECS + système d'animation assemblés.
 *
 * `AnimationSystem` dérive le clip (idle/run) et l'image courante de la **vitesse simulée** d'un
 * `core::Actor`. Depuis le `LOT-06`, il n'y a plus ni sol à quitter ni mur où glisser : les clips
 * aériens du platformer restent déclarés dans `core::playerClipSet()` — poses procédurales et
 * spritesheets externes s'y accrochent — mais plus rien ne les sélectionne, et c'est le
 * vocabulaire de sprites RPG du `LOT-08` qui refera ce jeu de clips.
 */

#include <gtest/gtest.h>

#include "Core/Ecs/Components/Actor.h"
#include "Core/Ecs/Components/Animation.h"
#include "Core/Ecs/Components/Velocity.h"
#include "Core/Ecs/Systems/AnimationSystem.h"
#include "Core/Ecs/World.h"
#include "Core/Math/Vector2.h"

namespace {
constexpr float STEP = 1.0f / 60.0f;

core::Entity spawnCharacter(core::World& world, core::Vector2 velocity) {
    const core::Entity entity = world.createEntity();
    world.addComponent(entity, core::Actor{});
    world.addComponent(entity, core::Velocity{velocity});
    core::Animation animation;
    animation.clips = core::playerClipSet();
    animation.clipIndex = core::PLAYER_CLIP_IDLE;
    world.addComponent(entity, animation);
    return entity;
}
}  // namespace

/**
 * @brief À l'arrêt, le clip est Idle et l'image alterne 0/1 après chaque durée d'image.
 * \castest{<b>A l'arret, le clip est Idle et l'image alterne 0/1 apres chaque duree d'image.</b>
 * <br/>
 * \tcat Integration · Animation Personnage<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Placer un acteur de vitesse nulle.<br/>2. Avancer l'animation d'une duree
 * d'image.<br/>
 * \tattendu Le clip est Idle, et l'image alterne 0 puis 1.
 * }
 */
TEST(AnimationPersonnageIntegration, ALArretEstEnRepos) {
    core::World world;
    const core::Entity entity = spawnCharacter(world, core::Vector2{0.0f, 0.0f});

    core::AnimationSystem system;
    system.update(world, STEP);

    const core::Animation& animation = world.getComponent<core::Animation>(entity);
    EXPECT_EQ(animation.clipIndex, core::PLAYER_CLIP_IDLE);
    EXPECT_EQ(animation.frameIndex, 0);

    // 0,5 s à 60 Hz = 30 pas : l'image de repos doit avoir bouclé au moins une fois (2 images).
    for (int i = 0; i < 31; ++i) {
        system.update(world, STEP);
    }
    EXPECT_EQ(animation.clipIndex, core::PLAYER_CLIP_IDLE);
    EXPECT_EQ(animation.frameIndex, 1);
}

/**
 * @brief En mouvement horizontal, le clip est Run et l'image boucle sur les 4 images dans l'ordre.
 * \castest{<b>En mouvement, le clip est Run et l'image boucle sur les 4 images dans l'ordre.</b>
 * <br/>
 * \tcat Integration · Animation Personnage<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Placer un acteur en mouvement horizontal.<br/>2. Avancer l'animation de quatre
 * durees d'image.<br/>
 * \tattendu Le clip est Run, et l'image passe 0, 1, 2, 3 puis reboucle sur 0.
 * }
 */
TEST(AnimationPersonnageIntegration, EnMouvementCourt) {
    core::World world;
    const core::Entity entity = spawnCharacter(world, core::Vector2{3.0f, 0.0f});

    core::AnimationSystem system;
    system.update(world,
                  STEP);  // Idle -> Run : consomme le pas de transition (pas d'accumulation).

    const core::Animation& animation = world.getComponent<core::Animation>(entity);
    EXPECT_EQ(animation.clipIndex, core::PLAYER_CLIP_RUN);
    EXPECT_EQ(animation.frameIndex, 0);

    // Durée d'une image de course : 0,1 s (6 pas à 60 Hz). Vérifie 0 -> 1 -> 2 -> 3 -> 0.
    for (int expectedFrame : {1, 2, 3, 0}) {
        for (int i = 0; i < 6; ++i) {
            system.update(world, STEP);
        }
        EXPECT_EQ(animation.clipIndex, core::PLAYER_CLIP_RUN);
        EXPECT_EQ(animation.frameIndex, expectedFrame);
    }
}

/**
 * @brief Marcher **vers le haut** anime autant que marcher vers la droite : les deux axes comptent
 * pareil (`EX-EXP-001`).
 *
 * Le platformer ne regardait que la composante horizontale — la verticale y voulait dire « saut »
 * ou « chute ». En vue de dessus, une marche verticale est une marche.
 * \castest{<b>Une marche purement verticale anime comme une marche horizontale.</b><br/>
 * \tcat Integration · Animation Personnage<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Placer un acteur dont la vitesse est purement verticale.<br/>2. Avancer d'un
 * pas.<br/>
 * \tattendu Le clip resolu est Run, comme pour une marche horizontale.
 * }
 */
TEST(AnimationPersonnageIntegration, MarcheVerticaleCourtAussi) {
    core::World world;
    const core::Entity entity = spawnCharacter(world, core::Vector2{0.0f, -3.0f});

    core::AnimationSystem system;
    system.update(world, STEP);

    EXPECT_EQ(world.getComponent<core::Animation>(entity).clipIndex, core::PLAYER_CLIP_RUN);
}

/**
 * @brief Un changement de clip réinitialise immédiatement l'image et le chronomètre à zéro.
 * \castest{<b>Un changement de clip reinitialise immediatement l'image et le chronometre.</b><br/>
 * \tcat Integration · Animation Personnage<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Faire courir un acteur jusqu'a une image differente de 0.<br/>2. Annuler sa
 * vitesse.<br/>
 * \tattendu Le clip repasse a Idle, image 0, chronometre a zero.
 * }
 */
TEST(AnimationPersonnageIntegration, ChangementDeClipReinitialiseLImage) {
    core::World world;
    const core::Entity entity = spawnCharacter(world, core::Vector2{3.0f, 0.0f});
    core::Velocity& velocity = world.getComponent<core::Velocity>(entity);

    core::AnimationSystem system;
    system.update(world,
                  STEP);  // Idle -> Run : consomme le pas de transition (pas d'accumulation).
    for (int i = 0; i < 6; ++i) {
        system.update(world, STEP);
    }
    const core::Animation& animation = world.getComponent<core::Animation>(entity);
    ASSERT_EQ(animation.clipIndex, core::PLAYER_CLIP_RUN);
    ASSERT_EQ(animation.frameIndex, 1);

    velocity.value = core::Vector2{};  // arrêt net
    system.update(world, STEP);

    EXPECT_EQ(animation.clipIndex, core::PLAYER_CLIP_IDLE);
    EXPECT_EQ(animation.frameIndex, 0);
    EXPECT_FLOAT_EQ(animation.elapsed, 0.0f);
}

/**
 * @brief Une entité sans composant Animation n'est pas affectée par le système.
 * \castest{<b>Une entite sans composant Animation n'est pas affectee par le systeme.</b><br/>
 * \tcat Integration · Animation Personnage<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Creer un acteur sans composant Animation.<br/>2. Avancer le systeme dix pas.<br/>
 * \tattendu Aucun composant Animation n'apparait.
 * }
 */
TEST(AnimationPersonnageIntegration, EntiteSansAnimationIgnoree) {
    core::World world;
    const core::Entity entity = world.createEntity();
    world.addComponent(entity, core::Actor{});
    world.addComponent(entity, core::Velocity{});

    core::AnimationSystem system;
    for (int i = 0; i < 10; ++i) {
        system.update(world, STEP);
    }

    EXPECT_FALSE(world.hasComponent<core::Animation>(entity));
}

/**
 * @brief Une entité portant Animation mais sans jeu de clips assigné n'est pas affectée
 *        (`EX-NFR-040`) : aucune donnée à progresser, ni plantage.
 * \castest{<b>Une entite Animation sans jeu de clips assigne reste inerte, sans planter.</b><br/>
 * \tcat Integration · Animation Personnage<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Creer un acteur dont l'Animation n'a aucun jeu de clips.<br/>2. Avancer le systeme
 * dix pas.<br/>
 * \tattendu L'animation reste a ses valeurs par defaut, sans exception.
 * }
 */
TEST(AnimationPersonnageIntegration, AnimationSansJeuDeClipsResteInerte) {
    core::World world;
    const core::Entity entity = world.createEntity();
    world.addComponent(entity, core::Actor{});
    world.addComponent(entity, core::Velocity{});
    world.addComponent(entity, core::Animation{});  // clips == nullptr

    core::AnimationSystem system;
    for (int i = 0; i < 10; ++i) {
        system.update(world, STEP);
    }

    const core::Animation& animation = world.getComponent<core::Animation>(entity);
    EXPECT_EQ(animation.clipIndex, 0);
    EXPECT_EQ(animation.frameIndex, 0);
}
