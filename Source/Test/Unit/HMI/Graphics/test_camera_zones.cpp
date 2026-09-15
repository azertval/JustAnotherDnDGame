// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

/**
 * @file test_camera_zones.cpp
 * @brief Tests unitaires de la résolution de zone de caméra active (mode *par salle*, zones
 *        dessinées à la main, `EX-LVL-007`).
 */

#include <gtest/gtest.h>

#include "HMI/Graphics/CameraZones.h"

namespace {
using core::CameraZone;
using core::GridPosition;
}  // namespace

/**
 * @brief Une position à l'intérieur d'une zone unique renvoie l'indice de cette zone.
 * \castest{<b>Une position dans une zone unique renvoie son indice.</b><br/>
 * \tcat Unitaire · Camera Zones<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Construire une liste avec une seule zone.<br/>2. Résoudre une position à
 * l'intérieur.<br/>
 * \tattendu L'indice `0` est renvoyé.
 * }
 */
TEST(CameraZonesTest, PositionDansUneZoneUniqueRenvoieSonIndice) {
    const std::vector<CameraZone> zones = {CameraZone{.x = 0, .y = 0, .width = 10, .height = 10}};
    const std::optional<std::size_t> result = hmi::activeCameraZoneIndex(zones, GridPosition{5, 5});
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, 0u);
}

/**
 * @brief Une position hors de toutes les zones ne renvoie aucun indice (repli niveau entier côté
 * appelant).
 * \castest{<b>Une position hors de toutes les zones ne renvoie aucun indice.</b><br/>
 * \tcat Unitaire · Camera Zones<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Construire une liste de zones qui ne couvre pas toute la grille.<br/>2. Résoudre une
 * position dans le trou.<br/>
 * \tattendu `std::nullopt` est renvoyé.
 * }
 */
TEST(CameraZonesTest, PositionHorsDeToutesLesZonesNeRenvoieRien) {
    const std::vector<CameraZone> zones = {CameraZone{.x = 0, .y = 0, .width = 5, .height = 5}};
    const std::optional<std::size_t> result =
        hmi::activeCameraZoneIndex(zones, GridPosition{20, 20});
    EXPECT_FALSE(result.has_value());
}

/**
 * @brief Les bornes d'une zone sont inclusives côté haut-gauche et exclusives côté bas-droit,
 * comme `core::Rect::contains` — une grille de zones jointives se partitionne donc sans
 * chevauchement ni trou artificiel à la frontière.
 * \castest{<b>Les bornes d'une zone sont inclusives haut-gauche, exclusives bas-droit.</b><br/>
 * \tcat Unitaire · Camera Zones<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire une zone de 5x5 à l'origine.<br/>2. Résoudre sa case d'origine, puis la
 * case juste après son bord droit/bas.<br/>
 * \tattendu L'origine résout dans la zone ; la case juste hors du bord ne résout dans aucune zone.
 * }
 */
TEST(CameraZonesTest, BornesInclusivesHautGaucheExclusivesBasDroit) {
    const std::vector<CameraZone> zones = {CameraZone{.x = 0, .y = 0, .width = 5, .height = 5}};
    EXPECT_TRUE(hmi::activeCameraZoneIndex(zones, GridPosition{0, 0}).has_value());
    EXPECT_TRUE(hmi::activeCameraZoneIndex(zones, GridPosition{4, 4}).has_value());
    EXPECT_FALSE(hmi::activeCameraZoneIndex(zones, GridPosition{5, 4}).has_value());
    EXPECT_FALSE(hmi::activeCameraZoneIndex(zones, GridPosition{4, 5}).has_value());
}

/**
 * @brief Deux zones chevauchantes : la **première** de la liste qui contient la position est
 * retenue, l'ordre porte donc la priorité.
 * \castest{<b>Deux zones chevauchantes : la première zone de la liste gagne.</b><br/>
 * \tcat Unitaire · Camera Zones<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire deux zones qui se chevauchent.<br/>2. Résoudre une position dans le
 * chevauchement.<br/>3. Inverser l'ordre des deux zones et répéter.<br/>
 * \tattendu La zone retenue est toujours la première de la liste, quel que soit l'ordre.
 * }
 */
TEST(CameraZonesTest, ZonesChevauchantesLaPremiereDeLaListeGagne) {
    const CameraZone first{.x = 0, .y = 0, .width = 10, .height = 10};
    const CameraZone second{.x = 5, .y = 5, .width = 10, .height = 10};
    const GridPosition overlap{7, 7};  // dans les deux zones.

    const std::vector<CameraZone> firstThenSecond = {first, second};
    const std::optional<std::size_t> resultA = hmi::activeCameraZoneIndex(firstThenSecond, overlap);
    ASSERT_TRUE(resultA.has_value());
    EXPECT_EQ(*resultA, 0u);

    const std::vector<CameraZone> secondThenFirst = {second, first};
    const std::optional<std::size_t> resultB = hmi::activeCameraZoneIndex(secondThenFirst, overlap);
    ASSERT_TRUE(resultB.has_value());
    EXPECT_EQ(*resultB, 0u);
}

/**
 * @brief Une liste de zones vide ne renvoie jamais d'indice.
 * \castest{<b>Une liste de zones vide ne renvoie jamais d'indice.</b><br/>
 * \tcat Unitaire · Camera Zones<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Résoudre une position avec une liste de zones vide.<br/>
 * \tattendu `std::nullopt` est renvoyé.
 * }
 */
TEST(CameraZonesTest, ListeDeZonesVideNeRenvoieRien) {
    EXPECT_FALSE(hmi::activeCameraZoneIndex({}, GridPosition{0, 0}).has_value());
}
