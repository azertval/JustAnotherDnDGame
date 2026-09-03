// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_aabb_vs_aabb.cpp
 * @brief Tests unitaires du balayage AABB continu contre une boîte fixe unique
 *        (`sweepAabbVsAabb`, `EX-GP-005`) — collision des blocs à taille réduite.
 */

#include <gtest/gtest.h>

#include "Core/Math/Vector2.h"
#include "Core/Physics/Aabb.h"
#include "Core/Physics/AabbVsAabb.h"

namespace {

// Boîte 1×1 dont le coin haut-gauche est en (x, y).
core::Aabb unitBox(float x, float y) {
    return core::Aabb::fromTopLeftSize(core::Vector2{x, y}, core::Vector2{1.0f, 1.0f});
}

// Boîte réduite (0,5×0,5) centrée dans la case (2, 1) — un corps plus petit qu'une case.
core::Aabb reducedObstacle() {
    return core::Aabb::fromTopLeftSize(core::Vector2{2.25f, 1.25f}, core::Vector2{0.5f, 0.5f});
}

}  // namespace

/**
 * @brief Sans chevauchement possible, la boîte parcourt tout son déplacement, sans contact.
 * \castest{<b>Balayage boîte-boîte : trajet libre, loin de l'obstacle</b><br/>
 * \tcat Unitaire · Physique · Balayage AABB vs AABB<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Placer un obstacle réduit en (2, 1).<br/>2. Balayer une boîte 1×1 partie de loin,
 * dont le trajet ne le croise pas.<br/>
 * \tattendu Aucun contact ; la boîte atteint exactement la position visée.
 * }
 */
TEST(AabbVsAabbTest, TrajetLibreLoinDeLObstacle) {
    const core::SweepResult r =
        core::sweepAabbVsAabb(unitBox(10.0f, 10.0f), core::Vector2{1.0f, 1.0f}, reducedObstacle());

    EXPECT_FALSE(r.hit);
    EXPECT_FLOAT_EQ(r.position.x, 11.0f);
    EXPECT_FLOAT_EQ(r.position.y, 11.0f);
}

/**
 * @brief Un obstacle réduit stoppe le mouvement horizontal exactement à son bord, sans pénétration.
 * \castest{<b>Balayage boîte-boîte : butée horizontale contre un obstacle réduit</b><br/>
 * \tcat Unitaire · Physique · Balayage AABB vs AABB<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Placer un obstacle réduit (0,5×0,5) centré en (2, 1).<br/>2. Balayer une boîte 1×1
 * vers la droite, alignée verticalement avec l'obstacle.<br/>
 * \tattendu La boîte s'arrête au ras du bord de l'obstacle (bord droit = bord gauche de
 * l'obstacle = 2,25), jamais au-delà ; normale horizontale.
 * }
 */
TEST(AabbVsAabbTest, ButeeHorizontaleContreUnObstacleReduit) {
    const core::Aabb obstacle = reducedObstacle();  // [2.25, 2.75] x [1.25, 1.75]
    // Boîte alignée sur la même bande Y que l'obstacle (partiellement, suffisant pour chevaucher).
    const core::SweepResult r =
        core::sweepAabbVsAabb(unitBox(0.5f, 1.0f), core::Vector2{5.0f, 0.0f}, obstacle);

    EXPECT_TRUE(r.hit);
    EXPECT_FLOAT_EQ(r.position.x, 1.25f);  // bord droit = 2.25 = bord gauche de l'obstacle
    EXPECT_FLOAT_EQ(r.position.y, 1.0f);
    EXPECT_FLOAT_EQ(r.normal.x, -1.0f);
}

/**
 * @brief Un obstacle réduit stoppe la chute exactement à son sommet, sans pénétration.
 * \castest{<b>Balayage boîte-boîte : butée verticale (dessus d'un obstacle réduit)</b><br/>
 * \tcat Unitaire · Physique · Balayage AABB vs AABB<br/>
 * \tcrit Bloquant<br/>
 * \tetapes 1. Placer un obstacle réduit centré en (2, 1).<br/>2. Balayer une boîte 1×1 en chute
 * verticale au-dessus.<br/>
 * \tattendu La boîte se pose exactement sur le dessus de l'obstacle (bord bas = bord haut de
 * l'obstacle = 1,25) ; normale vers le haut (appui).
 * }
 */
TEST(AabbVsAabbTest, ButeeVerticaleSurUnObstacleReduit) {
    const core::Aabb obstacle = reducedObstacle();
    const core::SweepResult r =
        core::sweepAabbVsAabb(unitBox(2.0f, -5.0f), core::Vector2{0.0f, 8.0f}, obstacle);

    EXPECT_TRUE(r.hit);
    EXPECT_FLOAT_EQ(r.position.y, 0.25f);  // bord bas = 1.25 = bord haut de l'obstacle
    EXPECT_FLOAT_EQ(r.normal.y, -1.0f);
}

/**
 * @brief Le personnage peut passer dans l'espace laissé libre autour d'un obstacle réduit, sans
 * être bloqué par la case entière qui le contient (`EX-GP-005`).
 * \castest{<b>Balayage boîte-boîte : le vide autour d'un obstacle réduit reste
 * franchissable</b><br/> \tcat Unitaire · Physique · Balayage AABB vs AABB<br/> \tcrit Majeur<br/>
 * \tetapes 1. Placer un obstacle réduit centré en (2, 1) (marge de 0,25 de chaque côté).<br/>2.
 * Balayer une boîte étroite qui passe dans cette marge, sans jamais chevaucher l'obstacle.<br/>
 * \tattendu Aucun contact : la marge autour de l'obstacle réduit est bien franchissable.
 * }
 */
TEST(AabbVsAabbTest, EspaceAutourDeLObstacleReduitEstFranchissable) {
    const core::Aabb obstacle = reducedObstacle();  // [2.25, 2.75] x [1.25, 1.75]
    // Boîte fine (0,2 de large) rasant le bord HAUT de la case (y = 1.0 a 1.05), qui reste hors de
    // la bande [1.25, 1.75] occupee par l'obstacle reduit : jamais de chevauchement vertical.
    const core::Aabb narrowBox =
        core::Aabb::fromTopLeftSize(core::Vector2{1.5f, 1.0f}, core::Vector2{0.2f, 0.05f});
    const core::SweepResult r =
        core::sweepAabbVsAabb(narrowBox, core::Vector2{2.0f, 0.0f}, obstacle);

    EXPECT_FALSE(r.hit);
    EXPECT_FLOAT_EQ(r.position.x, 3.5f);
}

/**
 * @brief Glissement le long d'un obstacle réduit : bloqué en X, le mouvement vertical continue
 * normalement (même ressenti qu'un mur classique, `sweepAabb`).
 * \castest{<b>Balayage boîte-boîte : glissement diagonal contre un obstacle réduit</b><br/>
 * \tcat Unitaire · Physique · Balayage AABB vs AABB<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Placer un obstacle réduit centré en (2, 1).<br/>2. Balayer une boîte en diagonale
 * (droite + bas), bloquée en X par l'obstacle mais pas en Y.<br/>
 * \tattendu Composante X arrêtée au bord de l'obstacle ; composante Y appliquée intégralement (pas
 * de blocage croisé entre les deux axes).
 * }
 */
TEST(AabbVsAabbTest, GlissementDiagonalLeLongDUnObstacleReduit) {
    const core::Aabb obstacle = reducedObstacle();  // [2.25, 2.75] x [1.25, 1.75]
    // Boîte fine en Y (0,2) positionnee pour chevaucher l'obstacle en Y au moment du contact X,
    // mais dont le trajet vertical propre ne rencontre pas l'obstacle (deplacement Y libre).
    const core::Aabb box =
        core::Aabb::fromTopLeftSize(core::Vector2{0.5f, 1.3f}, core::Vector2{1.0f, 0.2f});
    const core::SweepResult r = core::sweepAabbVsAabb(box, core::Vector2{5.0f, 3.0f}, obstacle);

    EXPECT_FLOAT_EQ(r.normal.x, -1.0f);
    EXPECT_FLOAT_EQ(r.position.x, 1.25f);  // bord droit = 2.25 = bord gauche de l'obstacle
    EXPECT_FLOAT_EQ(r.normal.y, 0.0f);     // axe Y non bloqué : glissement libre
    EXPECT_FLOAT_EQ(r.position.y, 4.3f);   // 1.3 + 3.0, deplacement Y complet
}

/**
 * @brief Sans déplacement sur un axe, la position de cet axe reste inchangée (pas d'effet de bord).
 * \castest{<b>Balayage boîte-boîte : déplacement nul sur un axe le laisse inchangé</b><br/>
 * \tcat Unitaire · Physique · Balayage AABB vs AABB<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Placer un obstacle réduit.<br/>2. Balayer avec un déplacement nul sur les deux
 * axes.<br/>
 * \tattendu Position inchangée, aucun contact signalé.
 * }
 */
TEST(AabbVsAabbTest, DeplacementNulNeBougeRien) {
    const core::Aabb obstacle = reducedObstacle();
    const core::Aabb box = unitBox(0.0f, 0.0f);
    const core::SweepResult r = core::sweepAabbVsAabb(box, core::Vector2{0.0f, 0.0f}, obstacle);

    EXPECT_FALSE(r.hit);
    EXPECT_FLOAT_EQ(r.position.x, 0.0f);
    EXPECT_FLOAT_EQ(r.position.y, 0.0f);
}
