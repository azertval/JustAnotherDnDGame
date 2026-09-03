// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_niveau_ecs.cpp
 * @brief Test d'intégration : d'un fichier de niveau (JSON) jusqu'aux entités de l'ECS.
 *
 * Assemble le chargement (`LevelLoader`), le modèle (`Level`/`TileMap`) et l'ECS (`World`) via
 * `buildLevelScene` : le niveau lu depuis le JSON doit peupler le monde d'exactement une entité
 * par tuile non vide, à la bonne position et avec la région attendue.
 */

#include <cstddef>
#include <filesystem>
#include <map>
#include <utility>

#include <gtest/gtest.h>

#include "Core/Ecs/Components/Sprite.h"
#include "Core/Ecs/Components/Transform.h"
#include "Core/Ecs/World.h"
#include "Core/Levels/Level.h"
#include "Core/Levels/LevelLoader.h"
#include "Core/Levels/LevelScene.h"
#include "Core/Levels/TileMap.h"
#include "Core/Levels/TileType.h"

namespace {

// Correspondance de test : la région encode le type de tuile dans son champ x (vérifiable).
core::AtlasRegion regionEncodingType(core::TileType type) {
    return core::AtlasRegion{static_cast<int>(type), 0, 1, 1};
}

// Niveau 2x2 : entrée (0,0), solide (1,0), sortie (1,1) ; (0,1) reste vide.
constexpr const char* LEVEL = R"({
  "name": "mini",
  "width": 2,
  "height": 2,
  "tiles": [
    { "x": 0, "y": 0, "type": "entry" },
    { "x": 1, "y": 0, "type": "solid" },
    { "x": 1, "y": 1, "type": "exit" }
  ]
})";

}  // namespace

/**
 * @brief Le niveau chargé peuple le monde d'une entité par tuile non vide, bien placée et typée.
 * \castest{<b>Le niveau chargé peuple le monde d'une entité par tuile non vide, bien placée et
 * typée.</b><br/>
 * \tcat Integration · Niveau Ecs<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Le niveau chargé peuple le monde d'une entité par tuile non vide, bien placée et typée.
 * }
 */
TEST(NiveauEcsIntegration, DuJsonAuxEntites) {
    const core::LevelLoadResult result = core::LevelLoader::loadFromString(LEVEL);
    ASSERT_TRUE(result.ok()) << result.error;

    core::World world;
    core::buildLevelScene(world, *result.level, regionEncodingType);

    // Relève chaque entité (position -> type encodé dans la région).
    std::map<std::pair<int, int>, int> tilesByPosition;
    world.view<core::Transform, core::Sprite>().each(
        [&](core::Entity, core::Transform& transform, core::Sprite& sprite) {
            const int column = static_cast<int>(transform.position.x);
            const int row = static_cast<int>(transform.position.y);
            tilesByPosition[{column, row}] = sprite.region.x;
        });

    // Trois tuiles non vides, la case (0,1) restant vide.
    EXPECT_EQ(tilesByPosition.size(), 3u);
    EXPECT_EQ(tilesByPosition.count({0, 1}), 0u);

    EXPECT_EQ(tilesByPosition.at({0, 0}), static_cast<int>(core::TileType::Entry));
    EXPECT_EQ(tilesByPosition.at({1, 0}), static_cast<int>(core::TileType::Solid));
    EXPECT_EQ(tilesByPosition.at({1, 1}), static_cast<int>(core::TileType::Exit));
}
