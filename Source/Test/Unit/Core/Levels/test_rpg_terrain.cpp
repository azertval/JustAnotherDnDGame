// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_rpg_terrain.cpp
 * @brief Tests unitaires du vocabulaire de terrain du RPG (`EX-EXP-005`, `LOT-08`) :
 * franchissabilité, aller-retour de format, et chaîne complète jusqu'aux libellés livrés.
 */

#include <algorithm>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "Core/Levels/LevelLoader.h"
#include "Core/Levels/LevelWriter.h"
#include "Core/Levels/TileType.h"
#include "Core/Levels/TileTypeName.h"

namespace {

// Les neuf types introduits par le LOT-08, dans l'ordre de l'enumeration.
const std::vector<core::TileType> RPG_TERRAIN{
    core::TileType::Grass, core::TileType::Dirt,      core::TileType::Sand,
    core::TileType::Water, core::TileType::DeepWater, core::TileType::Wall,
    core::TileType::Cliff, core::TileType::Bridge,    core::TileType::Stairs};

// Carte minimale valide portant une tuile du type demande en (2, 0).
std::string mapWith(core::TileType type) {
    return R"({
      "version": 3,
      "name": "Terrain",
      "width": 4,
      "height": 2,
      "tiles": [
        { "x": 0, "y": 0, "type": "entry" },
        { "x": 3, "y": 1, "type": "exit" },
        { "x": 2, "y": 0, "type": ")" +
           core::tileTypeName(type) + R"(" }
      ]
    })";
}

}  // namespace

/**
 * @brief Chaque type de terrain survit à l'aller-retour chargement → écriture → rechargement
 * (`EX-LVL-003`).
 * \castest{<b>Chaque type de terrain survit a l'aller-retour de format.</b><br/>
 * \tcat Unitaire · Terrain RPG<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Pour chacun des neuf types, charger une carte qui le porte.<br/>2. La reserialiser
 * puis la recharger.<br/>
 * \tattendu La case porte toujours le meme type.
 * }
 */
TEST(TerrainRpgTest, AllerRetourSurChaqueTypeDeTerrain) {
    for (const core::TileType type : RPG_TERRAIN) {
        const core::LevelLoadResult loaded = core::LevelLoader::loadFromString(mapWith(type));
        ASSERT_TRUE(loaded.ok()) << core::tileTypeName(type) << " : " << loaded.error;
        ASSERT_EQ(loaded.level->tileMap().tile(2, 0), type) << core::tileTypeName(type);

        const std::string json = core::LevelWriter::toJsonString(*loaded.level);
        const core::LevelLoadResult reloaded = core::LevelLoader::loadFromString(json);
        ASSERT_TRUE(reloaded.ok()) << core::tileTypeName(type) << " : " << reloaded.error;
        EXPECT_EQ(reloaded.level->tileMap().tile(2, 0), type) << core::tileTypeName(type);
    }
}

/**
 * @brief Ce qui se marche se marche, ce qui arrête arrête : la franchissabilité de chaque type de
 * terrain est celle que son nom promet.
 *
 * L'eau **profonde** bloque alors qu'elle n'est pas de la matière : rien ne permet encore de la
 * franchir, et `core::isSolid` est le seul endroit d'où la règle de nage la sortira le jour venu.
 * \castest{<b>La franchissabilite de chaque terrain est celle que son nom promet.</b><br/>
 * \tcat Unitaire · Terrain RPG<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Interroger isSolid sur les neuf types.<br/>
 * \tattendu Mur, falaise et eau profonde bloquent ; herbe, terre, sable, eau, pont et escalier
 * laissent passer.
 * }
 */
TEST(TerrainRpgTest, FranchissabiliteDeChaqueTerrain) {
    EXPECT_TRUE(core::isSolid(core::TileType::Wall));
    EXPECT_TRUE(core::isSolid(core::TileType::Cliff));
    EXPECT_TRUE(core::isSolid(core::TileType::DeepWater));

    EXPECT_FALSE(core::isSolid(core::TileType::Grass));
    EXPECT_FALSE(core::isSolid(core::TileType::Dirt));
    EXPECT_FALSE(core::isSolid(core::TileType::Sand));
    EXPECT_FALSE(core::isSolid(core::TileType::Water));
    EXPECT_FALSE(core::isSolid(core::TileType::Bridge));
    EXPECT_FALSE(core::isSolid(core::TileType::Stairs));
}

/**
 * @brief Une rive existe : l'eau peu profonde se traverse, l'eau profonde non — c'est la seule
 * distinction qui rende une côte jouable.
 * \castest{<b>L'eau peu profonde se traverse, l'eau profonde non.</b><br/>
 * \tcat Unitaire · Terrain RPG<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Comparer la franchissabilite de Water et DeepWater.<br/>
 * \tattendu Les deux different.
 * }
 */
TEST(TerrainRpgTest, LaRiveDistingueLesDeuxEaux) {
    EXPECT_NE(core::isSolid(core::TileType::Water), core::isSolid(core::TileType::DeepWater));
}

/**
 * @brief La borne de l'énumération reste **dérivée** du dernier type : aucune valeur recopiée à la
 * main ne peut se désynchroniser.
 * \castest{<b>La borne de l'enumeration reste derivee du dernier type.</b><br/>
 * \tcat Unitaire · Terrain RPG<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Comparer TILE_TYPE_COUNT au dernier enumerateur.<br/>2. Verifier que chaque valeur
 * jusqu'a cette borne porte un nom.<br/>
 * \tattendu La borne vaut le dernier type plus un, et les vingt noms sont non vides et uniques.
 * }
 */
TEST(TerrainRpgTest, BorneDeLEnumerationDerivee) {
    EXPECT_EQ(core::TILE_TYPE_COUNT, static_cast<int>(core::TileType::Stairs) + 1);
    EXPECT_EQ(core::TILE_TYPE_COUNT, 20);

    std::vector<std::string> names;
    for (int raw = 0; raw < core::TILE_TYPE_COUNT; ++raw) {
        const std::string name = core::tileTypeName(static_cast<core::TileType>(raw));
        EXPECT_FALSE(name.empty()) << raw;
        names.push_back(name);
    }
    const std::vector<std::string> sorted = [names]() mutable {
        std::sort(names.begin(), names.end());
        return names;
    }();
    EXPECT_EQ(std::adjacent_find(sorted.begin(), sorted.end()), sorted.end())
        << "deux types partagent un nom";
}
