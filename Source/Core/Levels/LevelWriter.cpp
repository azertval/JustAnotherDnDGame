// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Core/Levels/LevelWriter.h"

#include <fstream>
#include <map>
#include <string>
#include <utility>

#include <nlohmann/json.hpp>

#include "Core/Levels/LevelLoader.h"
#include "Core/Levels/TileMap.h"
#include "Core/Levels/TileType.h"
#include "Core/Levels/TileTypeName.h"

namespace core {

namespace {

// Vrai pour les tuiles "déclencheur" liables à une cible (interrupteur, plaque de pression ou
// clé, EX-GP-020/EX-GP-025/EX-GP-023) : toutes partagent la même règle d'identifiant
// (LevelLoader.cpp).
[[nodiscard]] bool isTriggerType(TileType type) {
    return type == TileType::Switch || type == TileType::PressurePlate || type == TileType::Key;
}

// Vrai pour les tuiles "cible" d'une liaison de mécanisme (porte classique ou porte verrouillée) :
// les deux se résolvent depuis le même vecteur `mechanisms` (LevelLoader.cpp), écrivent le même
// champ 'opensWith'.
[[nodiscard]] bool isDoorLikeType(TileType type) {
    return type == TileType::Door || type == TileType::LockedDoor;
}

// Nom JSON d'un PlaneDepth (LOT-69, EX-DEC-042), symétrique à parsePlaneDepth (LevelLoader.cpp).
// La profondeur par défaut (Behind) n'est jamais écrite (voir buildJson) : ce nom ne sert donc en
// pratique que pour Front, mais le switch reste exhaustif pour que l'ajout d'une valeur soit
// signalé par le compilateur.
[[nodiscard]] const char* planeDepthName(PlaneDepth depth) {
    switch (depth) {
        case PlaneDepth::Behind:
            return "behind";
        case PlaneDepth::Front:
            return "front";
    }
    return "behind";
}

}  // namespace

std::string LevelWriter::toJsonString(const Level& level) {
    return buildJson(level.name(), level.tileMap(), level.mechanisms(), level.background(),
                     level.skinSet(), level.textureOverrides(), level.cameraFraming(),
                     level.planes(), level.parallaxEnabled());
}

bool LevelWriter::saveToFile(const Level& level, const std::filesystem::path& path) {
    std::ofstream file(path, std::ios::binary);
    if (!file) {
        return false;
    }
    const std::string json = toJsonString(level);
    file.write(json.data(), static_cast<std::streamsize>(json.size()));
    return file.good();
}

std::string LevelWriter::buildJson(
    const std::string& name, const TileMap& tileMap, const std::vector<Mechanism>& mechanisms,

    const std::optional<std::string>& background, const std::optional<std::string>& skinSet,
    const std::vector<TileTextureOverride>& textureOverrides,
    const CameraFramingConfig& cameraFraming, const std::vector<Plane>& planes,
    bool parallaxEnabled) {
    nlohmann::json root;
    root["version"] = LEVEL_FORMAT_VERSION;
    root["name"] = name;
    root["width"] = tileMap.width();
    root["height"] = tileMap.height();
    // Capacites du tableau (EX-GP-055), a distinguer des budgets ci-dessus : omises quand le
    // niveau s'en remet aux reglages du moteur.
    if (background) {
        root["background"] = *background;
    }
    if (skinSet) {
        root["skinSet"] = *skinSet;
    }
    // Cadrage de camera (EX-LVL-006, LOT-64) : omis quand il coincide avec ce que la regle de
    // repli recalculerait pour ces dimensions -- meme convention "omis si defaut" que
    // background/skinSet ci-dessus, condition necessaire pour qu'un niveau jamais
    // retouche sur ce point reste sans le champ apres un aller-retour editeur.
    if (cameraFraming != resolveCameraFraming(std::nullopt, tileMap.width(), tileMap.height())) {
        nlohmann::json framingJson;
        framingJson["mode"] = std::string(cameraFramingModeName(cameraFraming.mode));
        if (cameraFraming.roomWidthTiles) {
            framingJson["roomWidthTiles"] = *cameraFraming.roomWidthTiles;
        }
        if (cameraFraming.roomHeightTiles) {
            framingJson["roomHeightTiles"] = *cameraFraming.roomHeightTiles;
        }
        if (!cameraFraming.zones.empty()) {
            nlohmann::json zonesJson = nlohmann::json::array();
            for (const CameraZone& zone : cameraFraming.zones) {
                nlohmann::json zoneJson;
                zoneJson["x"] = zone.x;
                zoneJson["y"] = zone.y;
                zoneJson["width"] = zone.width;
                zoneJson["height"] = zone.height;
                zonesJson.push_back(std::move(zoneJson));
            }
            framingJson["zones"] = std::move(zonesJson);
        }
        root["cameraFraming"] = std::move(framingJson);
    }

    // Identifiants de déclencheurs (interrupteur ou plaque de pression) régénérés de façon
    // déterministe (balayage ligne par ligne) : ni Level ni LevelDraft ne conservent les
    // identifiants du fichier d'origine.
    std::map<std::pair<int, int>, std::string> switchIds;
    int nextSwitchId = 0;
    for (int row = 0; row < tileMap.height(); ++row) {
        for (int column = 0; column < tileMap.width(); ++column) {
            if (isTriggerType(tileMap.tile(column, row))) {
                switchIds.emplace(std::make_pair(column, row),
                                  "s" + std::to_string(nextSwitchId++));
            }
        }
    }

    // Position de porte -> identifiant de l'interrupteur qui l'ouvre, d'après les mécanismes.
    std::map<std::pair<int, int>, std::string> doorOpensWith;
    for (const Mechanism& mechanism : mechanisms) {
        const auto found = switchIds.find(
            std::make_pair(mechanism.switchPosition.column, mechanism.switchPosition.row));
        if (found != switchIds.end()) {
            doorOpensWith.emplace(
                std::make_pair(mechanism.doorPosition.column, mechanism.doorPosition.row),
                found->second);
        }
    }

    // Position -> nom d'asset de la texture assignee par instance (EX-EDIT-043), independamment
    // du type de la tuile a cette position.
    std::map<std::pair<int, int>, std::string> textureOverrideByPosition;
    for (const TileTextureOverride& override : textureOverrides) {
        textureOverrideByPosition.emplace(
            std::make_pair(override.position.column, override.position.row), override.assetName);
    }

    nlohmann::json tiles = nlohmann::json::array();
    for (int row = 0; row < tileMap.height(); ++row) {
        for (int column = 0; column < tileMap.width(); ++column) {
            const TileType type = tileMap.tile(column, row);
            if (type == TileType::Empty) {
                continue;
            }
            nlohmann::json tile;
            tile["x"] = column;
            tile["y"] = row;
            tile["type"] = tileTypeName(type);
            if (isTriggerType(type)) {
                const auto found = switchIds.find(std::make_pair(column, row));
                if (found != switchIds.end()) {
                    tile["id"] = found->second;
                }
            } else if (isDoorLikeType(type)) {
                const auto found = doorOpensWith.find(std::make_pair(column, row));
                if (found != doorOpensWith.end()) {
                    tile["opensWith"] = found->second;
                }
            }
            // Texture assignee par instance (EX-EDIT-043) : independante du type, peut
            // accompagner n'importe quel champ ci-dessus.
            const auto overrideFound = textureOverrideByPosition.find(std::make_pair(column, row));
            if (overrideFound != textureOverrideByPosition.end()) {
                tile["texture"] = overrideFound->second;
            }
            tiles.push_back(std::move(tile));
        }
    }
    root["tiles"] = std::move(tiles);

    // Tableau racine optionnel "planes" (EX-DEC-040, LOT-69), omis si vide : l'ordre du vecteur
    // est preserve (rang = superposition). Chaque champ a sa valeur par defaut est omis, comme
    // partout ailleurs dans ce format (convention du LOT-67) -- un plan solidaire du niveau a
    // densite native ne produit donc qu'un seul champ, "file".
    if (!planes.empty()) {
        nlohmann::json planesJson = nlohmann::json::array();
        for (const Plane& plane : planes) {
            nlohmann::json entry;
            entry["file"] = plane.fileName;
            if (plane.pixelsPerUnit != PLANE_NATIVE_PIXELS_PER_UNIT) {
                entry["pixelsPerUnit"] = plane.pixelsPerUnit;
            }
            if (plane.parallaxX != 1.0F) {
                entry["parallaxX"] = plane.parallaxX;
            }
            // parallaxY n'est ecrit que s'il differe de parallaxX : le chargeur le fait retomber
            // sur ce dernier, ecrire les deux quand ils sont egaux serait du bruit.
            if (plane.parallaxY != plane.parallaxX) {
                entry["parallaxY"] = plane.parallaxY;
            }
            if (plane.opacity != 1.0F) {
                entry["opacity"] = plane.opacity;
            }
            if (plane.depth != PlaneDepth::Behind) {
                entry["depth"] = planeDepthName(plane.depth);
            }
            planesJson.push_back(std::move(entry));
        }
        root["planes"] = std::move(planesJson);
    }
    // Drapeau de parallaxe (EX-DEC-043) : omis a sa valeur par defaut (true).
    if (!parallaxEnabled) {
        root["parallax"] = false;
    }

    // Indentation a deux espaces et saut de ligne final, comme tout ecrivain JSON du projet
    // (rejeu, configuration d'entrainement, progression, raccourcis) : un niveau est un fichier
    // versionne, dont la relecture en revue de code suppose un diff ligne a ligne.
    return root.dump(2) + "\n";
}

}  // namespace core
