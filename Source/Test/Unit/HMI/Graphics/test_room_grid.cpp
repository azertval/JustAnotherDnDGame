// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

/**
 * @file test_room_grid.cpp
 * @brief Tests unitaires de la partition d'un niveau en salles (`RoomGrid`, LOT-32).
 */

#include <gtest/gtest.h>

#include "Core/Levels/GridPosition.h"
#include "HMI/Graphics/RoomGrid.h"

/**
 * @brief Un niveau plus petit qu'une salle produit une seule salle couvrant le niveau entier.
 * \castest{<b>Un niveau plus petit qu'une salle produit une seule salle couvrant le niveau
 * entier.</b><br/>
 * \tcat Unitaire · RoomGrid<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire un `RoomGrid` pour un niveau plus petit que la taille de salle sur les
 * deux axes.<br/>2. Vérifier `columns()`/`rows()` et le rectangle de l'unique salle.<br/>
 * \tattendu Une seule salle (`columns() == rows() == 1`), dont le rectangle couvre exactement le
 * niveau entier — condition de non-régression exploitée par le cadrage caméra (`LOT-16`).
 * }
 */
TEST(RoomGridTest, NiveauPlusPetitQuUneSalleProduitUneSeuleSalle) {
    const hmi::RoomGrid grid(10, 6);

    EXPECT_EQ(grid.columns(), 1);
    EXPECT_EQ(grid.rows(), 1);

    const hmi::RoomBounds bounds = grid.roomBounds(core::GridPosition{0, 0});
    EXPECT_EQ(bounds.column, 0);
    EXPECT_EQ(bounds.row, 0);
    EXPECT_EQ(bounds.width, 10);
    EXPECT_EQ(bounds.height, 6);
}

/**
 * @brief Un niveau exactement multiple de la taille de salle ne rogne aucune salle.
 * \castest{<b>Un niveau exactement multiple de la taille de salle ne rogne aucune salle.</b><br/>
 * \tcat Unitaire · RoomGrid<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire un `RoomGrid` pour un niveau dont chaque dimension est un multiple exact
 * de la taille de salle.<br/>2. Vérifier `columns()`/`rows()` et le rectangle de chaque
 * salle.<br/>
 * \tattendu Le nombre de salles correspond exactement au ratio ; chaque salle a la taille pleine
 * (aucun rectangle rogné), y compris la dernière colonne/ligne.
 * }
 */
TEST(RoomGridTest, NiveauMultipleExactNeRogneAucuneSalle) {
    const int width = hmi::RoomGrid::ROOM_WIDTH_TILES * 3;
    const int height = hmi::RoomGrid::ROOM_HEIGHT_TILES * 2;
    const hmi::RoomGrid grid(width, height);

    EXPECT_EQ(grid.columns(), 3);
    EXPECT_EQ(grid.rows(), 2);

    const hmi::RoomBounds last = grid.roomBounds(core::GridPosition{2, 1});
    EXPECT_EQ(last.column, hmi::RoomGrid::ROOM_WIDTH_TILES * 2);
    EXPECT_EQ(last.row, hmi::RoomGrid::ROOM_HEIGHT_TILES * 1);
    EXPECT_EQ(last.width, hmi::RoomGrid::ROOM_WIDTH_TILES);
    EXPECT_EQ(last.height, hmi::RoomGrid::ROOM_HEIGHT_TILES);
}

/**
 * @brief Un niveau non multiple de la taille de salle rogne la dernière colonne/ligne.
 * \castest{<b>Un niveau non multiple de la taille de salle rogne la dernière colonne/ligne.</b>
 * <br/>
 * \tcat Unitaire · RoomGrid<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire un `RoomGrid` pour un niveau dépassant de quelques cases un multiple de
 * la taille de salle.<br/>2. Vérifier le rectangle de la dernière salle sur chaque axe.<br/>
 * \tattendu La dernière colonne/ligne de salles est rognée à la taille réellement restante,
 * jamais agrandie au-delà des bornes du niveau.
 * }
 */
TEST(RoomGridTest, NiveauNonMultipleRogneLaDerniereSalle) {
    const int extraWidth = 5;
    const int extraHeight = 3;
    const int width = hmi::RoomGrid::ROOM_WIDTH_TILES + extraWidth;
    const int height = hmi::RoomGrid::ROOM_HEIGHT_TILES + extraHeight;
    const hmi::RoomGrid grid(width, height);

    EXPECT_EQ(grid.columns(), 2);
    EXPECT_EQ(grid.rows(), 2);

    const hmi::RoomBounds last = grid.roomBounds(core::GridPosition{1, 1});
    EXPECT_EQ(last.column, hmi::RoomGrid::ROOM_WIDTH_TILES);
    EXPECT_EQ(last.row, hmi::RoomGrid::ROOM_HEIGHT_TILES);
    EXPECT_EQ(last.width, extraWidth);
    EXPECT_EQ(last.height, extraHeight);
}

/**
 * @brief `roomIndexAt` renvoie l'indice de salle correct aux quatre coins et au centre.
 * \castest{<b>`roomIndexAt` renvoie l'indice de salle correct aux quatre coins et au centre.</b>
 * <br/>
 * \tcat Unitaire · RoomGrid<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire un `RoomGrid` de 2x2 salles.<br/>2. Interroger `roomIndexAt` à des
 * positions choisies dans chacune des quatre salles.<br/>
 * \tattendu Chaque position renvoie l'indice de la salle qui la contient réellement.
 * }
 */
TEST(RoomGridTest, RoomIndexAtQuatreCoins) {
    const int width = hmi::RoomGrid::ROOM_WIDTH_TILES * 2;
    const int height = hmi::RoomGrid::ROOM_HEIGHT_TILES * 2;
    const hmi::RoomGrid grid(width, height);

    EXPECT_EQ(grid.roomIndexAt(core::GridPosition{0, 0}), (core::GridPosition{0, 0}));
    EXPECT_EQ(grid.roomIndexAt(core::GridPosition{width - 1, 0}), (core::GridPosition{1, 0}));
    EXPECT_EQ(grid.roomIndexAt(core::GridPosition{0, height - 1}), (core::GridPosition{0, 1}));
    EXPECT_EQ(grid.roomIndexAt(core::GridPosition{width - 1, height - 1}),
              (core::GridPosition{1, 1}));
}

/**
 * @brief Une position hors des bornes du niveau est bornée à la salle la plus proche.
 * \castest{<b>Une position hors des bornes du niveau est bornée à la salle la plus proche.</b>
 * <br/>
 * \tcat Unitaire · RoomGrid<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Construire un `RoomGrid` pour un petit niveau.<br/>2. Interroger `roomIndexAt` avec
 * des coordonnées négatives puis très supérieures aux bornes.<br/>
 * \tattendu Aucun comportement indéfini : l'indice renvoyé reste dans `[0, columns())` ×
 * `[0, rows())`, borné au bord correspondant du niveau.
 * }
 */
TEST(RoomGridTest, PositionHorsBornesEstBornee) {
    const hmi::RoomGrid grid(10, 6);

    EXPECT_EQ(grid.roomIndexAt(core::GridPosition{-100, -100}), (core::GridPosition{0, 0}));
    EXPECT_EQ(grid.roomIndexAt(core::GridPosition{1000, 1000}), (core::GridPosition{0, 0}));
}

/**
 * @brief Construit sans taille explicite, `RoomGrid` utilise `ROOM_WIDTH_TILES`/`ROOM_HEIGHT_TILES`
 * comme valeurs par défaut ; une taille personnalisée (`LOT-64`) partitionne selon celle-ci, avec
 * la même règle de troncature au bord du niveau.
 * \castest{<b>Une taille de salle personnalisée partitionne selon cette taille.</b><br/>
 * \tcat Unitaire · RoomGrid<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire un `RoomGrid` sans taille explicite, vérifier `roomWidthTiles()`/
 * `roomHeightTiles()`.<br/>2. Construire un `RoomGrid` avec une taille personnalisée non multiple
 * du niveau.<br/>3. Vérifier le nombre de salles et la troncature de la dernière.<br/>
 * \tattendu Sans taille explicite, la partition utilise `ROOM_WIDTH_TILES`/`ROOM_HEIGHT_TILES` ;
 * avec une taille personnalisée, le nombre de salles et leur rognage suivent cette taille, pas la
 * valeur par défaut.
 * }
 */
TEST(RoomGridTest, TailleDeSallePersonnaliseePartitionneSelonCetteTaille) {
    const hmi::RoomGrid defaultSized(10, 6);
    EXPECT_EQ(defaultSized.roomWidthTiles(), hmi::RoomGrid::ROOM_WIDTH_TILES);
    EXPECT_EQ(defaultSized.roomHeightTiles(), hmi::RoomGrid::ROOM_HEIGHT_TILES);

    constexpr int customWidth = 10;
    constexpr int customHeight = 8;
    const hmi::RoomGrid custom(24, 20, customWidth, customHeight);
    EXPECT_EQ(custom.roomWidthTiles(), customWidth);
    EXPECT_EQ(custom.roomHeightTiles(), customHeight);
    EXPECT_EQ(custom.columns(), 3);  // ceil(24 / 10)
    EXPECT_EQ(custom.rows(), 3);     // ceil(20 / 8)

    const hmi::RoomBounds last = custom.roomBounds(core::GridPosition{2, 2});
    EXPECT_EQ(last.column, customWidth * 2);
    EXPECT_EQ(last.row, customHeight * 2);
    EXPECT_EQ(last.width, 24 - customWidth * 2);    // rognee a la largeur restante
    EXPECT_EQ(last.height, 20 - customHeight * 2);  // rognee a la hauteur restante
}
