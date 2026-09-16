// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

/**
 * @file test_world_map_regions.cpp
 * @brief Tests unitaires des régions posées sur la carte du monde (LOT-87, T3.6).
 */

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "Core/World/Atlas.h"
#include "HMI/Presentation/WorldMapRegions.h"

namespace {

[[nodiscard]] std::string readFile(const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary);
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

}  // namespace

/**
 * @brief Des ancres valides se lisent ; une ancre hors de la carte fait échouer la lecture.
 * \castest{<b>Les ancres des régions se lisent, et une ancre hors de la carte est refusée.</b><br/>
 * \tcat Unitaire · Carte du monde<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Lire deux ancres valides.<br/>
 * 2. Lire une ancre dont l'abscisse vaut 1,2.<br/>
 * \tattendu Deux ancres aux valeurs lues ; puis un échec qui nomme la région, sans ancre rendue.
 * }
 */
TEST(WorldMapRegionsTest, AncresLuesEtBornees) {
    const hmi::RegionAnchors valid =
        hmi::readRegionAnchors(R"({"regions":{"a":[0.25,0.5],"b":[1,0]}})");
    ASSERT_TRUE(valid.ok()) << valid.error;
    ASSERT_EQ(valid.anchors.size(), 2U);
    EXPECT_DOUBLE_EQ(valid.anchors.at("a").first, 0.25);
    EXPECT_DOUBLE_EQ(valid.anchors.at("a").second, 0.5);

    const hmi::RegionAnchors outside = hmi::readRegionAnchors(R"({"regions":{"c":[1.2,0.5]}})");
    EXPECT_FALSE(outside.ok());
    EXPECT_TRUE(outside.anchors.empty());
    EXPECT_NE(outside.error.find("c"), std::string::npos);
}

/**
 * @brief Chaque région de l'atlas livré a une ancre, et chaque ancre désigne une région.
 * \castest{<b>Les treize régions de l'atlas sont posées sur la carte du monde.</b><br/>
 * \tcat Unitaire · Carte du monde<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Charger l'atlas livré et `Source/Elements/Maps/world-map-regions.json`.<br/>
 * 2. Joindre les deux.<br/>
 * \tattendu Aucune région sans ancre, aucune ancre sans région ; autant de régions posées que
 * l'atlas en compte, et les lieux de l'atlas répartis dans leurs régions (107).
 * }
 */
TEST(WorldMapRegionsTest, AtlasLivreEntierementPose) {
    const core::Atlas atlas = core::loadAtlas(JADG_WORLD_DIR);
    ASSERT_TRUE(atlas.errors.empty());
    const hmi::RegionAnchors anchors = hmi::readRegionAnchors(readFile(JADG_WORLD_MAP_REGIONS));
    ASSERT_TRUE(anchors.ok()) << anchors.error;

    std::vector<std::string> missing;
    const std::vector<hmi::MapRegion> regions = hmi::worldMapRegions(atlas, anchors, missing);
    EXPECT_TRUE(missing.empty()) << (missing.empty() ? "" : missing.front());
    EXPECT_EQ(regions.size(), atlas.regions.size());

    std::size_t locations = 0;
    for (const hmi::MapRegion& region : regions) {
        locations += region.locations.size();
    }
    EXPECT_EQ(locations, atlas.locations.size());
}
