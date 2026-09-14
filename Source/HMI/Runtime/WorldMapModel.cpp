// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Runtime/WorldMapModel.h"

#include <QStringList>
#include <QVariantMap>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "Core/World/Atlas.h"
#include "HMI/HmiLog.h"
#include "HMI/Platform/ExecutableDirectory.h"
#include "HMI/Presentation/WorldMapRegions.h"

namespace hmi {

WorldMapModel::WorldMapModel(QObject* parent) : QObject(parent) {}

void WorldMapModel::load() {
    const std::filesystem::path root = executableDirectory();
    const core::Atlas atlas = core::loadAtlas(root / "World");
    for (const std::string& error : atlas.errors) {
        HMI_LOG_WARNING("Atlas : " + error);
    }

    std::ifstream file(root / "Maps" / "world-map-regions.json", std::ios::binary);
    std::stringstream buffer;
    buffer << file.rdbuf();
    const RegionAnchors anchors = readRegionAnchors(buffer.str());
    if (!anchors.ok()) {
        HMI_LOG_ERROR("Carte du monde : " + anchors.error);
    }

    std::vector<std::string> missing;
    const std::vector<MapRegion> regions = worldMapRegions(atlas, anchors, missing);
    for (const std::string& id : missing) {
        // Une region sans ancre manque sur la carte ; une ancre sans region ne designe rien. Les
        // deux se corrigent dans les donnees, et doivent donc se voir (EX-CNT-010).
        HMI_LOG_WARNING("Carte du monde : region ou ancre sans correspondance '" + id + "'.");
    }

    _regions.clear();
    _locationCount = 0;
    for (const MapRegion& region : regions) {
        QStringList locations;
        for (const std::string& location : region.locations) {
            locations.append(QString::fromStdString(location));
        }
        _locationCount += static_cast<int>(region.locations.size());
        _regions.append(QVariantMap{
            {QStringLiteral("regionId"), QString::fromStdString(region.id)},
            {QStringLiteral("name"), QString::fromStdString(region.name)},
            {QStringLiteral("x"), region.x},
            {QStringLiteral("y"), region.y},
            {QStringLiteral("government"), QString::fromStdString(region.government)},
            {QStringLiteral("locations"), locations.join(QLatin1Char('\n'))},
            {QStringLiteral("locationCount"), static_cast<int>(region.locations.size())}});
    }
    emit changed();
}

}  // namespace hmi
