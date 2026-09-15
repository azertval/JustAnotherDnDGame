// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "HMI/Presentation/WorldMapRegions.h"

#include <utility>

#include <nlohmann/json.hpp>

#include "Core/Data/JsonDocument.h"

namespace hmi {
namespace {

constexpr int FORMAT_VERSION = 1;

}  // namespace

RegionAnchors readRegionAnchors(std::string_view json) {
    RegionAnchors result;
    const core::JsonDocument document =
        core::readJsonObject(json, FORMAT_VERSION, "world-map-regions.json");
    if (!document.ok()) {
        result.error = document.message;
        return result;
    }
    const auto regions = document.root.find("regions");
    if (regions == document.root.end() || !regions->is_object()) {
        result.error =
            "world-map-regions.json : le champ « regions » manque ou n'est pas un objet.";
        return result;
    }
    for (const auto& [id, anchor] : regions->items()) {
        const bool pair = anchor.is_array() && anchor.size() == 2 && anchor[0].is_number() &&
                          anchor[1].is_number();
        const double x = pair ? anchor[0].get<double>() : -1.0;
        const double y = pair ? anchor[1].get<double>() : -1.0;
        if (!pair || x < 0.0 || x > 1.0 || y < 0.0 || y > 1.0) {
            result.anchors.clear();
            result.error = "world-map-regions.json : l'ancre de « " + id +
                           " » n'est pas une paire de fractions entre 0 et 1.";
            return result;
        }
        result.anchors.emplace(id, MapAnchor{x, y});
    }
    return result;
}

std::vector<MapRegion> worldMapRegions(const core::Atlas& atlas, const RegionAnchors& anchors,
                                       std::vector<std::string>& missing) {
    std::vector<MapRegion> regions;
    for (const core::Region& region : atlas.regions) {
        const auto anchor = anchors.anchors.find(region.id);
        if (anchor == anchors.anchors.end()) {
            missing.push_back(region.id);
            continue;
        }
        MapRegion shown{.id = region.id,
                        .name = region.name,
                        .x = anchor->second.first,
                        .y = anchor->second.second,
                        .government = region.government,
                        .locations = {}};
        for (const std::string& locationId : region.locations) {
            if (const core::Location* const location = atlas.findLocation(locationId);
                location != nullptr) {
                shown.locations.push_back(location->name);
            }
        }
        regions.push_back(std::move(shown));
    }
    for (const auto& [id, position] : anchors.anchors) {
        if (atlas.findRegion(id) == nullptr) {
            missing.push_back(id);
        }
    }
    return regions;
}

}  // namespace hmi
