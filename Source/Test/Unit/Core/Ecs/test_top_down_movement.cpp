// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_top_down_movement.cpp
 * @brief Tests unitaires du déplacement libre en 8 directions (`EX-EXP-001` à `EX-EXP-004`,
 *        LOT-06). Logique pure : ni fenêtre, ni GPU (`EX-ARCH-001`).
 */

#include <cmath>
#include <vector>

#include <gtest/gtest.h>

#include "Core/Ecs/Components/Actor.h"
#include "Core/Ecs/Components/Collider.h"
#include "Core/Ecs/Components/Transform.h"
#include "Core/Ecs/Components/Velocity.h"
#include "Core/Ecs/Systems/TopDownMovementSystem.h"
#include "Core/Ecs/World.h"
#include "Core/Levels/TileMap.h"
#include "Core/Levels/TileType.h"
#include "Core/Math/Vector2.h"
#include "Core/Physics/PlayerInput.h"
#include "Core/Physics/TopDownConfig.h"

namespace {

constexpr float STEP = 1.0f / 60.0f;
// Boite d'acteur volontairement plus petite qu'une case : deux cases libres cote a cote laissent
// alors passer, ce qui rend les scenarios de glissement lisibles.
constexpr float ACTOR_SIZE = 0.4f;

// Salle de 10x10 entierement libre, entouree de murs solides : de quoi marcher longtemps sans
// sortir de la grille.
core::TileMap openRoom() {
    core::TileMap map(10, 10);
    for (int i = 0; i < 10; ++i) {
        map.setTile(i, 0, core::TileType::Solid);
        map.setTile(i, 9, core::TileType::Solid);
        map.setTile(0, i, core::TileType::Solid);
        map.setTile(9, i, core::TileType::Solid);
    }
    return map;
}

core::Entity spawnActor(core::World& world, core::Vector2 position) {
    const core::Entity entity = world.createEntity();
    const core::Vector2 size{ACTOR_SIZE, ACTOR_SIZE};
    world.addComponent(entity, core::Transform{position, size, 0.0f});
    world.addComponent(entity, core::Velocity{});
    world.addComponent(entity, core::Collider{size});
    world.addComponent(entity, core::Actor{});
    return entity;
}

core::PlayerInput intent(float moveX, float moveY) {
    core::PlayerInput input;
    input.moveX = moveX;
    input.moveY = moveY;
    return input;
}

// Avance @p steps pas fixes avec la meme intention, et rend la distance parcourue.
float walk(core::World& world, core::Entity actor, const core::TileMap& map,
           const core::PlayerInput& input, int steps) {
    const core::TopDownConfig config;
    const core::Vector2 start = world.getComponent<core::Transform>(actor).position;
    for (int step = 0; step < steps; ++step) {
        core::updateTopDownMovement(world, actor, input, map, config, STEP);
    }
    return (world.getComponent<core::Transform>(actor).position - start).length();
}

}  // namespace

/**
 * @brief La diagonale n'est **pas** plus rapide que la marche cardinale (`EX-EXP-001`).
 *
 * Sans normalisation de l'intention, aller en diagonale donnerait `√2 ≈ 1,41` fois la vitesse
 * cardinale : le défaut le plus courant du genre, et le plus visible en jeu.
 * \castest{<b>La diagonale n'est pas plus rapide que la marche cardinale.</b><br/>
 * \tcat Unitaire · Deplacement top-down<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Faire marcher un acteur vers la droite pendant 60 pas.<br/>2. Faire marcher un
 * second acteur en diagonale bas-droite pendant 60 pas.<br/>
 * \tattendu Les deux distances parcourues sont egales.
 * }
 */
TEST(DeplacementTopDownTest, LaDiagonaleNEstPasPlusRapide) {
    const core::TileMap map = openRoom();

    core::World cardinal;
    const core::Entity droite = spawnActor(cardinal, core::Vector2{2.0f, 2.0f});
    const float distanceCardinale = walk(cardinal, droite, map, intent(1.0f, 0.0f), 60);

    core::World diagonale;
    const core::Entity biais = spawnActor(diagonale, core::Vector2{2.0f, 2.0f});
    const float distanceDiagonale = walk(diagonale, biais, map, intent(1.0f, 1.0f), 60);

    ASSERT_GT(distanceCardinale, 0.0f);
    EXPECT_NEAR(distanceDiagonale, distanceCardinale, 1e-4f);
}

/**
 * @brief Marcher vers le haut va aussi vite que marcher vers la droite : aucun axe n'est
 * privilégié, faute de gravité (`EX-EXP-001`).
 * \castest{<b>Aucun axe n'est privilegie : la marche est isotrope.</b><br/>
 * \tcat Unitaire · Deplacement top-down<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Faire marcher un acteur vers chacune des quatre directions cardinales, 30 pas.<br/>
 * \tattendu Les quatre distances parcourues sont egales.
 * }
 */
TEST(DeplacementTopDownTest, LesQuatreDirectionsCardinalesSontEquivalentes) {
    const core::TileMap map = openRoom();
    std::vector<float> distances;
    for (const core::PlayerInput& input :
         {intent(1.0f, 0.0f), intent(-1.0f, 0.0f), intent(0.0f, 1.0f), intent(0.0f, -1.0f)}) {
        core::World world;
        const core::Entity actor = spawnActor(world, core::Vector2{4.0f, 4.0f});
        distances.push_back(walk(world, actor, map, input, 30));
    }

    ASSERT_GT(distances.front(), 0.0f);
    for (const float distance : distances) {
        EXPECT_NEAR(distance, distances.front(), 1e-4f);
    }
}

/**
 * @brief Le personnage ne traverse **aucun** mur, même lancé à une vitesse absurde : le balayage
 * est continu (`EX-EXP-002`).
 * \castest{<b>Un acteur lance a vitesse absurde ne traverse pas un mur.</b><br/>
 * \tcat Unitaire · Deplacement top-down<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Poser un acteur pres du mur de droite, avec une vitesse de 1000 unites/s.<br/>2.
 * Avancer d'un pas.<br/>
 * \tattendu L'acteur s'arrete contre le mur, du bon cote.
 * }
 */
TEST(DeplacementTopDownTest, AucuneTraverseeDeMurAVitesseAbsurde) {
    const core::TileMap map = openRoom();
    core::World world;
    const core::Entity actor = spawnActor(world, core::Vector2{2.0f, 4.0f});
    world.getComponent<core::Velocity>(actor).value = core::Vector2{1000.0f, 0.0f};

    const core::TopDownConfig config;
    core::updateTopDownMovement(world, actor, intent(1.0f, 0.0f), map, config, STEP);

    // Le mur de droite occupe la colonne 9 : le bord droit de la boite ne peut pas la depasser.
    const core::Vector2 position = world.getComponent<core::Transform>(actor).position;
    EXPECT_LE(position.x + ACTOR_SIZE, 9.0f);
    EXPECT_GT(position.x, 2.0f);  // il a bien avance jusqu'au mur
}

/**
 * @brief Un mur pris en biais laisse **glisser** le long de sa surface : l'axe libre continue
 * d'avancer (`EX-EXP-003`).
 * \castest{<b>Un mur pris en biais laisse glisser le long de sa surface.</b><br/>
 * \tcat Unitaire · Deplacement top-down<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Placer un acteur contre le mur du haut.<br/>2. Marcher en diagonale haut-droite
 * pendant 60 pas.<br/>
 * \tattendu L'acteur n'a pas traverse le mur, mais a bien avance vers la droite.
 * }
 */
TEST(DeplacementTopDownTest, GlissementLeLongDUnMurPrisEnBiais) {
    const core::TileMap map = openRoom();
    core::World world;
    const core::Entity actor = spawnActor(world, core::Vector2{2.0f, 1.05f});

    walk(world, actor, map, intent(1.0f, -1.0f), 60);

    const core::Vector2 position = world.getComponent<core::Transform>(actor).position;
    EXPECT_GE(position.y, 1.0f);  // le mur du haut (ligne 0) n'a pas ete franchi
    EXPECT_GT(position.x, 2.5f);  // et le deplacement horizontal a continue malgre le contact
}

/**
 * @brief Pousser contre un mur n'accumule aucun élan : la vitesse de l'axe bloqué est annulée
 * (`EX-EXP-003`).
 * \castest{<b>Pousser contre un mur n'accumule aucun elan.</b><br/>
 * \tcat Unitaire · Deplacement top-down<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Pousser un acteur contre le mur de droite pendant 120 pas.<br/>
 * \tattendu Sa vitesse horizontale est nulle, et sa position n'a pas depasse le mur.
 * }
 */
TEST(DeplacementTopDownTest, PousserContreUnMurNAccumuleAucunElan) {
    const core::TileMap map = openRoom();
    core::World world;
    const core::Entity actor = spawnActor(world, core::Vector2{8.0f, 4.0f});

    walk(world, actor, map, intent(1.0f, 0.0f), 120);

    EXPECT_FLOAT_EQ(world.getComponent<core::Velocity>(actor).value.x, 0.0f);
    EXPECT_LE(world.getComponent<core::Transform>(actor).position.x + ACTOR_SIZE, 9.0f);
}

/**
 * @brief L'orientation suit la marche et **survit à l'arrêt** : un personnage immobile regarde là
 * où il allait (`EX-EXP-004`).
 * \castest{<b>L'orientation suit la marche et survit a l'arret.</b><br/>
 * \tcat Unitaire · Deplacement top-down<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Marcher vers le bas, puis relacher l'intention pendant 60 pas.<br/>
 * \tattendu L'orientation vaut toujours (0, 1) apres l'arret.
 * }
 */
TEST(DeplacementTopDownTest, OrientationConserveeALArret) {
    const core::TileMap map = openRoom();
    core::World world;
    const core::Entity actor = spawnActor(world, core::Vector2{4.0f, 4.0f});

    walk(world, actor, map, intent(0.0f, 1.0f), 10);
    const core::Vector2 facingEnMarche = world.getComponent<core::Actor>(actor).facing;
    EXPECT_NEAR(facingEnMarche.x, 0.0f, 1e-5f);
    EXPECT_NEAR(facingEnMarche.y, 1.0f, 1e-5f);

    walk(world, actor, map, intent(0.0f, 0.0f), 60);
    const core::Vector2 facingALArret = world.getComponent<core::Actor>(actor).facing;
    EXPECT_NEAR(facingALArret.x, 0.0f, 1e-5f);
    EXPECT_NEAR(facingALArret.y, 1.0f, 1e-5f);
}

/**
 * @brief Intention relâchée : le personnage s'arrête **net**, sans dérive résiduelle
 * (`EX-EXP-001`).
 * \castest{<b>Intention relachee, l'acteur s'arrete net.</b><br/>
 * \tcat Unitaire · Deplacement top-down<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Lancer un acteur a pleine vitesse.<br/>2. Relacher l'intention pendant 60 pas.<br/>
 * \tattendu Sa vitesse est exactement nulle, et sa position ne bouge plus.
 * }
 */
TEST(DeplacementTopDownTest, IntentionRelacheeArreteNet) {
    const core::TileMap map = openRoom();
    core::World world;
    const core::Entity actor = spawnActor(world, core::Vector2{4.0f, 4.0f});

    walk(world, actor, map, intent(1.0f, 0.0f), 30);
    walk(world, actor, map, intent(0.0f, 0.0f), 60);

    EXPECT_FLOAT_EQ(world.getComponent<core::Velocity>(actor).value.x, 0.0f);
    EXPECT_FLOAT_EQ(world.getComponent<core::Velocity>(actor).value.y, 0.0f);

    const core::Vector2 arret = world.getComponent<core::Transform>(actor).position;
    walk(world, actor, map, intent(0.0f, 0.0f), 10);
    EXPECT_FLOAT_EQ(world.getComponent<core::Transform>(actor).position.x, arret.x);
    EXPECT_FLOAT_EQ(world.getComponent<core::Transform>(actor).position.y, arret.y);
}

/**
 * @brief Déterminisme : mêmes entrées, mêmes positions, sur 600 pas (`EX-NFR-002`).
 * \castest{<b>Memes entrees, memes positions sur 600 pas.</b><br/>
 * \tcat Unitaire · Deplacement top-down<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Rejouer deux fois la meme sequence de 600 intentions variables.<br/>2. Comparer les
 * trajectoires pas a pas.<br/>
 * \tattendu Les deux trajectoires coincident exactement, au bit pres.
 * }
 */
TEST(DeplacementTopDownTest, MemesEntreesMemesPositionsSur600Pas) {
    const core::TileMap map = openRoom();
    const core::TopDownConfig config;

    const auto rejouer = [&map, &config] {
        core::World world;
        const core::Entity actor = spawnActor(world, core::Vector2{4.0f, 4.0f});
        std::vector<core::Vector2> trace;
        trace.reserve(600);
        for (int step = 0; step < 600; ++step) {
            // Intention variable et sans aleatoire : un tour de cadran toutes les 40 images, de
            // quoi croiser les murs, les diagonales et les arrets.
            const int phase = (step / 40) % 4;
            const core::PlayerInput input = intent(phase == 0 ? 1.0f : (phase == 2 ? -1.0f : 0.0f),
                                                   phase == 1 ? 1.0f : (phase == 3 ? -1.0f : 0.0f));
            core::updateTopDownMovement(world, actor, input, map, config, STEP);
            trace.push_back(world.getComponent<core::Transform>(actor).position);
        }
        return trace;
    };

    const std::vector<core::Vector2> premier = rejouer();
    const std::vector<core::Vector2> second = rejouer();

    ASSERT_EQ(premier.size(), second.size());
    for (std::size_t step = 0; step < premier.size(); ++step) {
        ASSERT_FLOAT_EQ(premier[step].x, second[step].x) << "pas " << step;
        ASSERT_FLOAT_EQ(premier[step].y, second[step].y) << "pas " << step;
    }
}

/**
 * @brief Une entité incomplète (sans `Actor`) est ignorée sans plantage (`EX-NFR-040`).
 * \castest{<b>Une entite sans composant Actor est ignoree sans planter.</b><br/>
 * \tcat Unitaire · Deplacement top-down<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Creer une entite portant Transform, Velocity et Collider, mais pas Actor.<br/>2.
 * Lui appliquer un pas de deplacement.<br/>
 * \tattendu Sa position n'a pas bouge, et rien n'a plante.
 * }
 */
TEST(DeplacementTopDownTest, EntiteSansActeurIgnoree) {
    const core::TileMap map = openRoom();
    core::World world;
    const core::Entity entity = world.createEntity();
    const core::Vector2 size{ACTOR_SIZE, ACTOR_SIZE};
    world.addComponent(entity, core::Transform{core::Vector2{4.0f, 4.0f}, size, 0.0f});
    world.addComponent(entity, core::Velocity{});
    world.addComponent(entity, core::Collider{size});

    const core::TopDownConfig config;
    core::updateTopDownMovement(world, entity, intent(1.0f, 0.0f), map, config, STEP);

    EXPECT_FLOAT_EQ(world.getComponent<core::Transform>(entity).position.x, 4.0f);
}
