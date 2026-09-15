// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

/**
 * @file test_swept_collision.cpp
 * @brief Tests unitaires du balayage AABB continu (`sweepAabb`) contre une grille de tuiles.
 */

#include <gtest/gtest.h>

#include "Core/Levels/TileMap.h"
#include "Core/Levels/TileType.h"
#include "Core/Math/Vector2.h"
#include "Core/Physics/Aabb.h"
#include "Core/Physics/SweptCollision.h"

namespace {

// Grille sans tuile solide, de dimensions données (toutes les cases vides par défaut).
core::TileMap emptyMap(int width, int height) {
    return core::TileMap(width, height);
}

// Boîte 1×1 dont le coin haut-gauche est en (x, y).
core::Aabb unitBox(float x, float y) {
    return core::Aabb::fromTopLeftSize(core::Vector2{x, y}, core::Vector2{1.0f, 1.0f});
}

}  // namespace

/**
 * @brief Sans obstacle, la boîte parcourt tout son déplacement, sans contact.
 * \castest{<b>Balayage : trajet libre</b><br/>
 * \tcat Unitaire · Physique · Balayage AABB<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Grille sans tuile solide.<br/>2. Balayer une boîte 1×1 d'un déplacement (3, 2).<br/>
 * \tattendu Aucun contact ; la boîte atteint exactement la position visée.}
 */
TEST(SweptCollisionTest, TrajetLibre) {
    const core::TileMap tiles = emptyMap(10, 10);
    const core::SweepResult r =
        core::sweepAabb(unitBox(0.0f, 0.0f), core::Vector2{3.0f, 2.0f}, tiles);

    EXPECT_FALSE(r.hit);
    EXPECT_FLOAT_EQ(r.position.x, 3.0f);
    EXPECT_FLOAT_EQ(r.position.y, 2.0f);
    EXPECT_FLOAT_EQ(r.normal.x, 0.0f);
    EXPECT_FLOAT_EQ(r.normal.y, 0.0f);
}

/**
 * @brief Un mur à droite stoppe le mouvement horizontal, la boîte s'arrête au ras du mur.
 * \castest{<b>Balayage : butée horizontale (mur à droite)</b><br/>
 * \tcat Unitaire · Physique · Balayage AABB<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Placer un mur solide à droite.<br/>2. Balayer la boîte vers la droite.<br/>
 * \tattendu La boîte s'arrête au ras du mur (bord droit = bord du mur) ; normale horizontale.}
 */
TEST(SweptCollisionTest, ButeeHorizontale) {
    core::TileMap tiles = emptyMap(10, 3);
    tiles.setTile(3, 1, core::TileType::Solid);
    const core::SweepResult r =
        core::sweepAabb(unitBox(0.0f, 1.0f), core::Vector2{5.0f, 0.0f}, tiles);

    EXPECT_TRUE(r.hit);
    EXPECT_FLOAT_EQ(r.position.x, 2.0f);  // bord droit = 3.0 = bord gauche du mur
    EXPECT_FLOAT_EQ(r.position.y, 1.0f);
    EXPECT_FLOAT_EQ(r.normal.x, -1.0f);
}

/**
 * @brief Un sol stoppe la chute : la boîte se pose dessus, normale vers le haut (appui au sol).
 * \castest{<b>Balayage : butée verticale (sol)</b><br/>
 * \tcat Unitaire · Physique · Balayage AABB<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Placer un sol solide en dessous.<br/>2. Balayer la boîte vers le bas.<br/>
 * \tattendu La boîte se pose sur le sol (bord bas = haut du sol) ; normale vers le haut.}
 */
TEST(SweptCollisionTest, ButeeVerticale) {
    core::TileMap tiles = emptyMap(3, 10);
    tiles.setTile(1, 5, core::TileType::Solid);
    const core::SweepResult r =
        core::sweepAabb(unitBox(1.0f, 0.0f), core::Vector2{0.0f, 10.0f}, tiles);

    EXPECT_TRUE(r.hit);
    EXPECT_FLOAT_EQ(r.position.y, 4.0f);  // bord bas = 5.0 = haut du sol
    EXPECT_FLOAT_EQ(r.position.x, 1.0f);
    EXPECT_FLOAT_EQ(r.normal.y, -1.0f);
}

/**
 * @brief Cœur de la décision de cadrage : un pas plus grand qu'une tuile ne traverse pas le mur.
 * \castest{<b>Balayage : non-tunneling à vitesse élevée</b><br/>
 * \tcat Unitaire · Physique · Balayage AABB<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Mur à 5 tuiles.<br/>2. Balayer d'un déplacement de 10 (≫ une tuile) vers le mur.<br/>
 * \tattendu La boîte s'arrête au ras du mur ; elle ne le traverse pas (garantie continue).}
 */
TEST(SweptCollisionTest, NonTunneling) {
    core::TileMap tiles = emptyMap(8, 2);
    tiles.setTile(5, 0, core::TileType::Solid);
    const core::SweepResult r =
        core::sweepAabb(unitBox(0.0f, 0.0f), core::Vector2{10.0f, 0.0f}, tiles);

    EXPECT_TRUE(r.hit);
    EXPECT_FLOAT_EQ(r.position.x, 4.0f);  // stoppé au ras du mur malgré delta = 10
    EXPECT_FLOAT_EQ(r.normal.x, -1.0f);
}

/**
 * @brief Glissement : en butant contre un mur vertical, la boîte continue de descendre le long du
 * mur.
 * \castest{<b>Balayage : glissement le long d'un mur</b><br/>
 * \tcat Unitaire · Physique · Balayage AABB<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Mur vertical à droite.<br/>2. Balayer d'un déplacement diagonal (droite + bas).<br/>
 * \tattendu Bloquée en X (au ras du mur), mais la descente complète en Y a lieu (glissement).}
 */
TEST(SweptCollisionTest, GlissementLeLongDuMur) {
    core::TileMap tiles = emptyMap(6, 4);
    tiles.setTile(3, 0, core::TileType::Solid);
    tiles.setTile(3, 1, core::TileType::Solid);
    tiles.setTile(3, 2, core::TileType::Solid);
    const core::SweepResult r =
        core::sweepAabb(unitBox(0.0f, 0.0f), core::Vector2{5.0f, 2.0f}, tiles);

    EXPECT_TRUE(r.hit);
    EXPECT_FLOAT_EQ(r.position.x, 2.0f);  // bloqué au ras du mur (bord droit = 3.0)
    EXPECT_FLOAT_EQ(r.position.y, 2.0f);  // mais la descente complète (glissement) a eu lieu
    EXPECT_FLOAT_EQ(r.normal.x, -1.0f);
}

/**
 * @brief Un mur à gauche stoppe le déplacement vers la gauche, bord gauche collé au mur.
 * \castest{<b>Balayage : butée horizontale (mur à gauche)</b><br/>
 * \tcat Unitaire · Physique · Balayage AABB<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mur solide à gauche.<br/>2. Balayer la boîte vers la gauche.<br/>
 * \tattendu La boîte s'arrête au ras du mur (bord gauche = bord droit du mur) ; normale opposée.}
 */
TEST(SweptCollisionTest, ButeeGauche) {
    core::TileMap tiles = emptyMap(10, 3);
    tiles.setTile(2, 1, core::TileType::Solid);
    const core::SweepResult r =
        core::sweepAabb(unitBox(6.0f, 1.0f), core::Vector2{-5.0f, 0.0f}, tiles);

    EXPECT_TRUE(r.hit);
    EXPECT_FLOAT_EQ(r.position.x, 3.0f);  // bord gauche = 3.0 = bord droit du mur (column 2)
    EXPECT_FLOAT_EQ(r.normal.x, 1.0f);
}

/**
 * @brief Un plafond stoppe la montée, bord haut collé sous le plafond.
 * \castest{<b>Balayage : butée verticale (plafond)</b><br/>
 * \tcat Unitaire · Physique · Balayage AABB<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Plafond solide au-dessus.<br/>2. Balayer la boîte vers le haut.<br/>
 * \tattendu La boîte s'arrête sous le plafond (bord haut = bas du plafond) ; normale vers le bas.}
 */
TEST(SweptCollisionTest, ButeePlafond) {
    core::TileMap tiles = emptyMap(3, 10);
    tiles.setTile(1, 2, core::TileType::Solid);
    const core::SweepResult r =
        core::sweepAabb(unitBox(1.0f, 6.0f), core::Vector2{0.0f, -5.0f}, tiles);

    EXPECT_TRUE(r.hit);
    EXPECT_FLOAT_EQ(r.position.y, 3.0f);  // bord haut = 3.0 = bas du plafond (ligne 2)
    EXPECT_FLOAT_EQ(r.normal.y, 1.0f);
}

/**
 * @brief En marchant le long d'un sol, la boîte n'est pas bloquée horizontalement par ce sol
 * (peau).
 * \castest{<b>Balayage : marcher sur un sol sans blocage horizontal</b><br/>
 * \tcat Unitaire · Physique · Balayage AABB<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Sol continu ; boîte posée dessus.<br/>2. Balayer horizontalement le long du sol.<br/>
 * \tattendu Aucun blocage : la « peau » perpendiculaire évite de confondre marcher sur / buter
 * contre.}
 */
TEST(SweptCollisionTest, MarcheSurLeSolSansBlocageHorizontal) {
    core::TileMap tiles = emptyMap(10, 3);
    for (int column = 0; column < 10; ++column) {
        tiles.setTile(column, 2, core::TileType::Solid);  // sol continu sur la ligne 2
    }
    // La boîte repose sur le sol (bord bas = 2.0) et avance à droite.
    const core::SweepResult r =
        core::sweepAabb(unitBox(0.0f, 1.0f), core::Vector2{3.0f, 0.0f}, tiles);

    EXPECT_FALSE(r.hit);
    EXPECT_FLOAT_EQ(r.position.x, 3.0f);  // avance librement, le sol dessous ne bloque pas
}
