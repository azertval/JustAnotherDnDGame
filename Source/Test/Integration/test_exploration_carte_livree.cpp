// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

/**
 * @file test_exploration_carte_livree.cpp
 * @brief Test d'intégration : d'un fichier de carte livré jusqu'à l'instantané que le rendu du
 *        lieu dessine — la chaîne que le jeu et l'essai immédiat de l'éditeur partagent
 *        (`hmi::WorldPlay`), sans GPU.
 */

#include <algorithm>
#include <filesystem>
#include <string>

#include <gtest/gtest.h>

#include "Core/World/ExplorationSession.h"
#include "Core/World/WorldTravel.h"
#include "HMI/Game/WorldPlay.h"
#include "HMI/Graphics/WorldSceneComposer.h"

namespace {

[[nodiscard]] hmi::WorldPlay playFromDisk() {
    return hmi::WorldPlay(core::WorldTravel::directoryLoader(JADG_LEVELS_DIR), JADG_ASSETS_DIR);
}

[[nodiscard]] bool hasDrawnFloor(const hmi::WorldSceneSnapshot& snapshot) {
    return std::ranges::any_of(snapshot.floors,
                               [](const std::string& piece) { return !piece.empty(); });
}

}  // namespace

/**
 * @brief Le Colisée livré s'ouvre, nomme son lieu et se compose avec ses planches.
 * \castest{<b>La premiere carte du jeu se charge et se compose.</b><br/>
 * \tcat Integration · Exploration<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Entrer sur la carte livree du Colisee a son entree.<br/>2. Prendre l'instantane
 * de la scene.<br/>
 * \tattendu Le lieu est nomme, la grille a les dimensions de la carte, des pieces de sol sont
 * dessinees et le heros figure parmi les figurines.
 * }
 */
TEST(ExplorationCarteLivreeIntegration, LeColiseeSeChargeEtSeCompose) {
    hmi::WorldPlay play = playFromDisk();
    ASSERT_TRUE(play.enter("coliseum", {}));

    const hmi::WorldSceneSnapshot snapshot = play.snapshot();
    EXPECT_FALSE(snapshot.place.empty());
    EXPECT_EQ(snapshot.columns, play.session().map()->tileMap().width());
    EXPECT_EQ(snapshot.rows, play.session().map()->tileMap().height());
    EXPECT_TRUE(hasDrawnFloor(snapshot));
    ASSERT_FALSE(snapshot.figures.empty());
    EXPECT_EQ(snapshot.figures.back().figure, hmi::WorldPlay::DEFAULT_HERO_FIGURE);
}

/**
 * @brief Le héros marche : un pas d'intention le déplace, et sa bande passe à la marche.
 * \castest{<b>Marcher sur une carte livree deplace le heros et change sa bande.</b><br/>
 * \tcat Integration · Exploration<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Entrer sur Martpart.<br/>2. Avancer d'une seconde, par pas de 1/60 s, dans une
 * direction libre.<br/>
 * \tattendu Le heros a quitte son point d'arrivee et sa figurine joue la bande « walk ».
 * }
 */
TEST(ExplorationCarteLivreeIntegration, LeHerosMarcheSurMartpart) {
    hmi::WorldPlay play = playFromDisk();
    ASSERT_TRUE(play.enter("capital/martpart", {}));
    const core::CellPoint start = play.session().heroPoint();

    // Une direction au moins est libre au point d'arrivée : on essaie les quatre.
    bool moved = false;
    for (const core::Vector2 direction : {core::Vector2{1.0F, 0.0F}, core::Vector2{-1.0F, 0.0F},
                                          core::Vector2{0.0F, 1.0F}, core::Vector2{0.0F, -1.0F}}) {
        for (int step = 0; step < 60; ++step) {
            static_cast<void>(play.step({.move = direction, .interact = false}, 1.0F / 60.0F));
        }
        if (play.session().heroPoint() != start) {
            moved = true;
            break;
        }
    }
    ASSERT_TRUE(moved);
    EXPECT_EQ(play.figures().back().clip, "walk");
}
