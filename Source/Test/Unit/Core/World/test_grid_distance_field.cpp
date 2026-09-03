// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_grid_distance_field.cpp
 * @brief Tests unitaires de core::GridDistanceField (amendement LOT-ANNEXE-08, EX-IA-023).
 */

#include <gtest/gtest.h>

#include "Core/Levels/GridPosition.h"
#include "Core/Levels/TileMap.h"
#include "Core/Levels/TileType.h"
#include "Core/World/GridDistanceField.h"

/**
 * @brief La cible a une distance de zéro à elle-même.
 * \castest{<b>La distance de la cible à elle-même est nulle.</b><br/>
 * \tcat Unitaire · AiSolver Env<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Grille ouverte, `GridDistanceField` construit avec une cible.<br/>
 * \tattendu `distance(cible) == 0`.}
 */
TEST(GridDistanceFieldTest, DistanceDeLaCibleAElleMemeEstNulle) {
    const core::TileMap map(5, 5);
    const core::GridPosition target{2, 2};
    const core::GridDistanceField field(map, target);
    EXPECT_EQ(field.distance(target), 0);
}

/**
 * @brief Sur une grille ouverte, la distance vaut la distance de Manhattan (BFS 4-connexe sans
 * obstacle).
 * \castest{<b>Grille sans mur : distance de grille égale à la distance de Manhattan.</b><br/>
 * \tcat Unitaire · AiSolver Env<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Grille 10x10 vide, cible en `(0,0)`.<br/>2. Lit la distance de plusieurs cases.<br/>
 * \tattendu Chaque distance égale `|colonne| + |ligne|`.}
 */
TEST(GridDistanceFieldTest, GrilleOuverteEgaleDistanceDeManhattan) {
    const core::TileMap map(10, 10);
    const core::GridPosition target{0, 0};
    const core::GridDistanceField field(map, target);

    EXPECT_EQ(field.distance(core::GridPosition{3, 0}), 3);
    EXPECT_EQ(field.distance(core::GridPosition{0, 4}), 4);
    EXPECT_EQ(field.distance(core::GridPosition{3, 4}), 7);
    EXPECT_EQ(field.distance(core::GridPosition{9, 9}), 18);
}

/**
 * @brief Un mur entièrement séparateur rend les cases de l'autre côté inatteignables (distance
 * sentinelle), sans planter.
 * \castest{<b>Mur séparateur complet : cases isolées à la distance sentinelle.</b><br/>
 * \tcat Unitaire · AiSolver Env<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Grille 5x5, colonne 2 entièrement solide (mur de bord en bord).<br/>2. Cible en
 * `(0,0)`, lit la distance d'une case de l'autre côté du mur (`(4,0)`).<br/>
 * \tattendu La distance renvoyée vaut la sentinelle `largeur * hauteur` (25), pas de plantage.}
 */
TEST(GridDistanceFieldTest, MurSeparateurCompletRendInatteignable) {
    core::TileMap map(5, 5);
    for (int row = 0; row < map.height(); ++row) {
        map.setTile(2, row, core::TileType::Solid);
    }
    const core::GridPosition target{0, 0};
    const core::GridDistanceField field(map, target);

    EXPECT_EQ(field.distance(core::GridPosition{4, 0}), map.width() * map.height());
}

/**
 * @brief Un mur partiel force un détour : la distance de grille suit le chemin réel, plus longue
 * que la distance de Manhattan en ligne directe.
 * \castest{<b>Mur partiel (détour) : la distance de grille suit le chemin réel, pas la ligne
 * droite.</b><br/>
 * \tcat Unitaire · AiSolver Env<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Grille 3x3, case `(1,1)` solide (mur direct entre `(1,2)` et la cible `(1,0)`).<br/>
 * \tattendu La distance de `(1,2)` à la cible vaut `4` (détour par une colonne latérale), pas `2`
 * (distance de Manhattan sans obstacle).}
 */
TEST(GridDistanceFieldTest, MurPartielForceUnDetourPlusLongQueLaLigneDirecte) {
    core::TileMap map(3, 3);
    map.setTile(1, 1, core::TileType::Solid);
    const core::GridPosition target{1, 0};
    const core::GridDistanceField field(map, target);

    EXPECT_EQ(field.distance(core::GridPosition{1, 2}), 4);
}

/**
 * @brief Une case hors-grille renvoie la distance sentinelle sans planter.
 * \castest{<b>Case hors-grille : distance sentinelle, pas de plantage.</b><br/>
 * \tcat Unitaire · AiSolver Env<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Grille 5x5, cible valide.<br/>2. Lit la distance d'une case negative et d'une case
 * au-delà des bornes.<br/>
 * \tattendu Les deux renvoient la sentinelle `largeur * hauteur` (25).}
 */
TEST(GridDistanceFieldTest, CaseHorsGrilleRenvoieLaSentinelle) {
    const core::TileMap map(5, 5);
    const core::GridPosition target{2, 2};
    const core::GridDistanceField field(map, target);

    const int sentinel = map.width() * map.height();
    EXPECT_EQ(field.distance(core::GridPosition{-1, 0}), sentinel);
    EXPECT_EQ(field.distance(core::GridPosition{5, 5}), sentinel);
}

/**
 * @brief Le constructeur multi-cibles renvoie la distance à la **plus proche** des cibles fournies
 * (amendement LOT-ANNEXE-21).
 * \castest{<b>Multi-cibles : distance à la plus proche des cibles fournies.</b><br/>
 * \tcat Unitaire · AiSolver Env<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Grille ouverte 10x10, deux cibles `(0,0)` et `(9,9)`.<br/>2. Lit la distance d'une
 * case proche de chaque cible.<br/>
 * \tattendu Chaque case obtient la distance à la cible la plus proche, pas la plus lointaine.}
 */
TEST(GridDistanceFieldTest, MultiCiblesDistanceALaPlusProche) {
    const core::TileMap map(10, 10);
    const std::vector<core::GridPosition> targets{core::GridPosition{0, 0},
                                                  core::GridPosition{9, 9}};
    const core::GridDistanceField field(map, targets);

    EXPECT_EQ(field.distance(core::GridPosition{1, 0}), 1);  // Proche de (0,0).
    EXPECT_EQ(field.distance(core::GridPosition{8, 9}), 1);  // Proche de (9,9).
    EXPECT_EQ(field.distance(core::GridPosition{0, 0}), 0);
    EXPECT_EQ(field.distance(core::GridPosition{9, 9}), 0);
}

/**
 * @brief Une cible solide ou hors-grille dans la liste multi-cibles est ignorée, sans empêcher les
 * autres cibles valides de produire un champ atteignable.
 * \castest{<b>Multi-cibles : une cible invalide n'empêche pas les autres de fonctionner.</b><br/>
 * \tcat Unitaire · AiSolver Env<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Grille 5x5, case `(1,1)` solide utilisée comme première cible, `(4,4)` valide comme
 * seconde.<br/>
 * \tattendu La distance à `(4,4)` reste correcte malgré la cible solide.}
 */
TEST(GridDistanceFieldTest, MultiCiblesIgnoreUneCibleSolideOuHorsGrille) {
    core::TileMap map(5, 5);
    map.setTile(1, 1, core::TileType::Solid);
    const std::vector<core::GridPosition> targets{
        core::GridPosition{1, 1}, core::GridPosition{6, 6}, core::GridPosition{4, 4}};
    const core::GridDistanceField field(map, targets);

    EXPECT_EQ(field.distance(core::GridPosition{4, 4}), 0);
    EXPECT_EQ(field.distance(core::GridPosition{3, 4}), 1);
}
