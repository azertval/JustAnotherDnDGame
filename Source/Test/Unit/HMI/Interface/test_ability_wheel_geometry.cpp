// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_ability_wheel_geometry.cpp
 * @brief La roue des caractéristiques de la fiche de personnage.
 *
 * Tests **purs** : aucune instance Qt. Ce qui est couvert ici est ce qu'un coup d'œil ne voit pas —
 * un médaillon qui a quitté son arc, une roue qui déborde de son carré, une ouverture du bas qui
 * s'est refermée sur le nom.
 */

#include <algorithm>
#include <cmath>
#include <vector>

#include <gtest/gtest.h>

#include "HMI/Interface/AbilityWheelGeometry.h"

namespace {

using hmi::ABILITY_SEAT_COUNT;
using hmi::ABILITY_WHEEL_MINIMUM_SIZE;
using hmi::abilityWheelLayout;
using hmi::AbilityWheelLayout;
using hmi::WheelDisc;
using hmi::WheelSeat;
using hmi::wheelSeatAt;

/// Tailles utiles : le plancher, la taille de la maquette, et deux facteurs d'agrandissement.
constexpr int SIZES[] = {ABILITY_WHEEL_MINIMUM_SIZE, 280, 400, 560, 840, 1120};

[[nodiscard]] float distance(const WheelDisc& disc, float x, float y) {
    const float dx = disc.centerX - x;
    const float dy = disc.centerY - y;
    return std::sqrt(dx * dx + dy * dy);
}

// -- Le plancher ------------------------------------------------------------------------------

/**
 * @brief Sous le plancher, la roue ne rend rien -- et le dit.
 *
 * C'est le seul moyen pour l'appelant de savoir qu'il doit se rabattre sur une disposition en
 * lignes. Une roue qui rendrait des médaillons de douze pixels afficherait des valeurs illisibles
 * là où un tableau, lui, resterait lisible.
 */
TEST(AbilityWheelGeometry, SousLePlancherRienNEstAPeindre) {
    for (const int side : {0, 1, 64, ABILITY_WHEEL_MINIMUM_SIZE - 1}) {
        const AbilityWheelLayout layout = abilityWheelLayout(side);
        EXPECT_FALSE(layout.valid) << "cote " << side;
        EXPECT_FLOAT_EQ(layout.arcRadius, 0.0F) << "cote " << side;
        EXPECT_FLOAT_EQ(layout.portrait.radius, 0.0F) << "cote " << side;
    }
    EXPECT_TRUE(abilityWheelLayout(ABILITY_WHEEL_MINIMUM_SIZE).valid);
}

// -- Les six sur leur arc ---------------------------------------------------------------------

/**
 * @brief Les six centres sont sur l'arc, à la même distance du centre de la roue.
 *
 * C'est le défaut qui ne lève aucune erreur : un siège calculé à part dérive de quelques pixels,
 * l'arc passe à côté de son médaillon, et l'écran reste « presque » juste. Une tolérance d'un
 * dixième de pixel suffit à distinguer l'arrondi flottant d'une vraie dérive.
 */
TEST(AbilityWheelGeometry, LesSixSiegesSontSurLArc) {
    for (const int side : SIZES) {
        const AbilityWheelLayout layout = abilityWheelLayout(side);
        ASSERT_TRUE(layout.valid) << "cote " << side;
        for (std::size_t index = 0; index < ABILITY_SEAT_COUNT; ++index) {
            const float radius =
                distance(layout.seats.at(index), layout.portrait.centerX, layout.portrait.centerY);
            EXPECT_NEAR(radius, layout.arcRadius, 0.1F) << "cote " << side << ", siege " << index;
        }
    }
}

/**
 * @brief La roue est symétrique par rapport à sa verticale.
 *
 * Les six sont posés trois à gauche, trois à droite : le premier répond au dernier, le deuxième à
 * l'avant-dernier. Une dissymétrie se verrait à l'œil sur une planche, mais pas sur une capture
 * d'écran de vingt pixels de large -- et c'est là qu'elle se glisse.
 */
TEST(AbilityWheelGeometry, LaRoueEstSymetrique) {
    const AbilityWheelLayout layout = abilityWheelLayout(560);
    ASSERT_TRUE(layout.valid);
    const float axis = layout.portrait.centerX;
    for (std::size_t index = 0; index < ABILITY_SEAT_COUNT / 2; ++index) {
        const WheelDisc& left = layout.seats.at(index);
        const WheelDisc& right = layout.seats.at(ABILITY_SEAT_COUNT - 1 - index);
        EXPECT_NEAR(axis - left.centerX, right.centerX - axis, 0.1F) << "paire " << index;
        EXPECT_NEAR(left.centerY, right.centerY, 0.1F) << "paire " << index;
    }
}

/**
 * @brief L'arc reste OUVERT en bas, et l'ouverture est assez large pour porter le nom.
 *
 * C'est la raison d'être de la composition : le nom du personnage se pose dans cette ouverture.
 * Refermer l'arc rendrait la roue plus régulière et lui retirerait sa fonction -- il n'y aurait
 * plus où écrire qui l'on est.
 */
TEST(AbilityWheelGeometry, LArcResteOuvertEnBas) {
    const AbilityWheelLayout layout = abilityWheelLayout(560);
    ASSERT_TRUE(layout.valid);
    EXPECT_LT(layout.arcSweepDegrees, 360.0F);
    EXPECT_GE(360.0F - layout.arcSweepDegrees, 90.0F) << "ouverture trop etroite pour un nom";

    // Aucun siège sous le centre : l'ouverture est bien en BAS, et non sur un côté.
    const auto lowest = std::max_element(
        layout.seats.begin(), layout.seats.end(),
        [](const WheelDisc& a, const WheelDisc& b) { return a.centerY < b.centerY; });
    EXPECT_LT(lowest->centerY, layout.portrait.centerY + layout.arcRadius)
        << "un siege occupe l'ouverture du bas";
}

// -- Les disques ne se chevauchent pas --------------------------------------------------------

/**
 * @brief Le portrait, son anneau et les médaillons ne se touchent pas.
 *
 * Trois cercles concentriques qui grandissent au même rapport ne peuvent pas se croiser par
 * accident -- sauf si l'un d'eux change de rapport sans que les autres suivent. Le test tient donc
 * l'ORDRE : portrait, puis anneau, puis le bord intérieur des médaillons.
 */
TEST(AbilityWheelGeometry, LesCerclesNeSeChevauchentPas) {
    for (const int side : SIZES) {
        const AbilityWheelLayout layout = abilityWheelLayout(side);
        ASSERT_TRUE(layout.valid) << "cote " << side;
        EXPECT_LT(layout.portrait.radius, layout.ringRadius) << "cote " << side;
        EXPECT_LT(layout.ringRadius, layout.arcRadius - layout.seats.front().radius)
            << "cote " << side << " : l'anneau mord sur les medaillons";
    }
}

/**
 * @brief Deux médaillons voisins ne se recouvrent jamais.
 *
 * Six disques sur un arc de 250 degrés tiennent -- tant que leur rayon suit le même rapport que
 * l'arc. Le jour où l'un des deux rapports bouge seul, les médaillons se chevauchent, et rien ne
 * l'annonce : ils restent tous à l'écran, au bon endroit, de la bonne taille.
 */
TEST(AbilityWheelGeometry, LesMedaillonsVoisinsNeSeRecouvrentPas) {
    for (const int side : SIZES) {
        const AbilityWheelLayout layout = abilityWheelLayout(side);
        ASSERT_TRUE(layout.valid) << "cote " << side;
        for (std::size_t index = 1; index < ABILITY_SEAT_COUNT; ++index) {
            const WheelDisc& previous = layout.seats.at(index - 1);
            const WheelDisc& current = layout.seats.at(index);
            const float gap = distance(current, previous.centerX, previous.centerY);
            EXPECT_GT(gap, previous.radius + current.radius)
                << "cote " << side << ", sieges " << (index - 1) << " et " << index;
        }
    }
}

// -- La roue tient dans son carré ---------------------------------------------------------------

/**
 * @brief Rien ne sort du carré demandé, ni en largeur ni en haut.
 *
 * Un débordement ne lève aucune erreur en Qt : le médaillon est simplement rogné par le parent, et
 * le chiffre qu'il porte disparaît à moitié. Le bas est délibérément libre -- c'est l'ouverture.
 */
TEST(AbilityWheelGeometry, LaRoueTientDansSonCarre) {
    for (const int side : SIZES) {
        const AbilityWheelLayout layout = abilityWheelLayout(side);
        ASSERT_TRUE(layout.valid) << "cote " << side;
        const float extent = static_cast<float>(side);
        for (std::size_t index = 0; index < ABILITY_SEAT_COUNT; ++index) {
            const WheelDisc& seat = layout.seats.at(index);
            EXPECT_GE(seat.centerX - seat.radius, 0.0F) << "cote " << side << ", siege " << index;
            EXPECT_LE(seat.centerX + seat.radius, extent) << "cote " << side << ", siege " << index;
            EXPECT_GE(seat.centerY - seat.radius, 0.0F) << "cote " << side << ", siege " << index;
            EXPECT_LE(seat.centerY + seat.radius, extent) << "cote " << side << ", siege " << index;
        }
    }
}

// -- L'ordre est celui de la planche ------------------------------------------------------------

/**
 * @brief L'ordre des sièges est relevé sur la planche : on part du bas à gauche et on tourne.
 *
 * Force en bas à gauche, Charisme en bas à droite, Constitution et Intelligence en haut. Inverser
 * deux caractéristiques donnerait un écran juste dans sa forme et faux dans son propos, et
 * personne ne s'en apercevrait avant de lire une fiche qu'il connaît.
 */
TEST(AbilityWheelGeometry, LOrdreDesSiegesEstCeluiDeLaPlanche) {
    EXPECT_EQ(wheelSeatAt(0), WheelSeat::Strength);
    EXPECT_EQ(wheelSeatAt(5), WheelSeat::Charisma);

    const AbilityWheelLayout layout = abilityWheelLayout(560);
    ASSERT_TRUE(layout.valid);
    const WheelDisc& strength = layout.seats.at(0);
    const WheelDisc& constitution = layout.seats.at(2);
    const WheelDisc& charisma = layout.seats.at(5);

    EXPECT_LT(strength.centerX, layout.portrait.centerX) << "Force n'est pas a gauche";
    EXPECT_GT(charisma.centerX, layout.portrait.centerX) << "Charisme n'est pas a droite";
    EXPECT_LT(constitution.centerY, strength.centerY) << "Constitution n'est pas au-dessus";
}

// -- L'échelle -----------------------------------------------------------------------------------

/**
 * @brief Doubler le côté double toutes les longueurs, sans exception.
 *
 * L'identité du jeu se met à l'échelle par entiers (`EX-IHM-081`). Un rapport écrit en dur quelque
 * part -- une marge, un rayon minimal -- casserait cette proportionnalité pour lui seul, et la roue
 * se déformerait au lieu de grandir.
 */
TEST(AbilityWheelGeometry, DoublerLeCoteDoubleToutesLesLongueurs) {
    const AbilityWheelLayout simple = abilityWheelLayout(400);
    const AbilityWheelLayout doubled = abilityWheelLayout(800);
    ASSERT_TRUE(simple.valid);
    ASSERT_TRUE(doubled.valid);

    EXPECT_NEAR(doubled.portrait.radius, simple.portrait.radius * 2.0F, 0.01F);
    EXPECT_NEAR(doubled.ringRadius, simple.ringRadius * 2.0F, 0.01F);
    EXPECT_NEAR(doubled.arcRadius, simple.arcRadius * 2.0F, 0.01F);
    for (std::size_t index = 0; index < ABILITY_SEAT_COUNT; ++index) {
        EXPECT_NEAR(doubled.seats.at(index).centerX, simple.seats.at(index).centerX * 2.0F, 0.01F)
            << "siege " << index;
        EXPECT_NEAR(doubled.seats.at(index).centerY, simple.seats.at(index).centerY * 2.0F, 0.01F)
            << "siege " << index;
    }

    // Les angles, eux, ne changent PAS : ce sont les seules valeurs de la table qui ne suivent pas
    // le côté, et les confondre avec des longueurs ferait tourner la roue en grandissant.
    EXPECT_FLOAT_EQ(doubled.arcStartDegrees, simple.arcStartDegrees);
    EXPECT_FLOAT_EQ(doubled.arcSweepDegrees, simple.arcSweepDegrees);
}

}  // namespace
