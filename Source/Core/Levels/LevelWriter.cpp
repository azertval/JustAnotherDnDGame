// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "Core/Levels/LevelWriter.h"

#include <fstream>
#include <map>
#include <string>
#include <utility>
#include <variant>

#include <nlohmann/json.hpp>

#include "Core/Levels/LevelLoader.h"
#include "Core/Levels/LevelProperties.h"
#include "Core/Levels/MapEntity.h"
#include "Core/Levels/TileLayer.h"
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

// --- Couches et entites (LOT-04, format version 3) -------------------------------------------

// Reemet les proprietes libres d'une couche ou d'une entite A PLAT dans son objet JSON, a cote de
// ses champs connus (EX-LVL-018). C'est la moitie ecriture du mecanisme qui evite un version: 4 :
// le chargeur range dans cette table toute cle qu'il ne reconnait pas, et on la lui rend telle
// quelle. Une cle homonyme d'un champ connu ne peut pas s'y trouver -- collectProperties l'aurait
// ecartee -- et un ecrasement est donc impossible.
//
// core::PropertyMap est ordonnee : l'ecriture est deterministe, deux enregistrements du meme
// niveau produisent le meme fichier.
void writeProperties(const PropertyMap& properties, nlohmann::json& object) {
    for (const auto& [key, value] : properties) {
        std::visit([&object, &key](const auto& raw) { object[key] = raw; }, value);
    }
}

// Tuiles non vides d'une grille, au format {x, y, type}. Les cases vides sont omises (EX-LVL-003),
// et une couche ne porte AUCUN des champs specifiques du tableau racine (id, opensWith, texture) :
// les mecanismes et les textures par instance restent attaches a la grille racine, seule source de
// verite des liaisons.
[[nodiscard]] nlohmann::json layerTilesJson(const TileMap& tiles) {
    nlohmann::json array = nlohmann::json::array();
    for (int row = 0; row < tiles.height(); ++row) {
        for (int column = 0; column < tiles.width(); ++column) {
            const TileType type = tiles.tile(column, row);
            if (type == TileType::Empty) {
                continue;
            }
            nlohmann::json tile;
            tile["x"] = column;
            tile["y"] = row;
            tile["type"] = tileTypeName(type);
            array.push_back(std::move(tile));
        }
    }
    return array;
}

// Vrai pour la couche que le chargeur PROMEUT depuis la grille racine (Collision, ou Legacy pour
// une carte sans couche declaree). Elle n'est jamais reecrite dans "layers" : elle est deja le
// tableau racine "tiles". L'ecrire dupliquerait la grille dans le fichier, et une carte version 2
// ressortirait convertie en version 3 dans le dos de son auteur.
[[nodiscard]] bool isPromotedRootLayer(const TileLayer& layer) {
    return layer.kind == LayerKind::Collision || layer.kind == LayerKind::Legacy;
}

// Position (colonne, ligne) -> chaine associee (identifiant, texture...).
using PositionMap = std::map<std::pair<int, int>, std::string>;

// Cadrage de camera (EX-LVL-006, LOT-64) au format JSON : seuls le mode et les champs renseignes.
[[nodiscard]] nlohmann::json cameraFramingJson(const CameraFramingConfig& cameraFraming) {
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
    return framingJson;
}

// Identifiants de déclencheurs (interrupteur ou plaque de pression) régénérés de façon
// déterministe (balayage ligne par ligne) : ni Level ni LevelDraft ne conservent les
// identifiants du fichier d'origine.
[[nodiscard]] PositionMap triggerIds(const TileMap& tileMap) {
    PositionMap switchIds;
    int nextSwitchId = 0;
    for (int row = 0; row < tileMap.height(); ++row) {
        for (int column = 0; column < tileMap.width(); ++column) {
            if (isTriggerType(tileMap.tile(column, row))) {
                switchIds.emplace(std::make_pair(column, row),
                                  "s" + std::to_string(nextSwitchId++));
            }
        }
    }
    return switchIds;
}

// Position de porte -> identifiant de l'interrupteur qui l'ouvre, d'après les mécanismes.
[[nodiscard]] PositionMap doorOpensWithIds(const std::vector<Mechanism>& mechanisms,
                                           const PositionMap& switchIds) {
    PositionMap doorOpensWith;
    for (const Mechanism& mechanism : mechanisms) {
        const auto found = switchIds.find(
            std::make_pair(mechanism.switchPosition.column, mechanism.switchPosition.row));
        if (found != switchIds.end()) {
            doorOpensWith.emplace(
                std::make_pair(mechanism.doorPosition.column, mechanism.doorPosition.row),
                found->second);
        }
    }
    return doorOpensWith;
}

// Position -> nom d'asset de la texture assignee par instance (EX-EDIT-043), independamment
// du type de la tuile a cette position.
[[nodiscard]] PositionMap textureOverridesByPosition(
    const std::vector<TileTextureOverride>& textureOverrides) {
    PositionMap textureOverrideByPosition;
    for (const TileTextureOverride& override : textureOverrides) {
        textureOverrideByPosition.emplace(
            std::make_pair(override.position.column, override.position.row), override.assetName);
    }
    return textureOverrideByPosition;
}

// Valeur associee a @p key dans @p values, ecrite sous @p field de @p object si elle existe.
void writeIfFound(const PositionMap& values, const std::pair<int, int>& key, const char* field,
                  nlohmann::json& object) {
    const auto found = values.find(key);
    if (found != values.end()) {
        object[field] = found->second;
    }
}

// Tableau racine "tiles" : grille de collision, identifiants de declencheurs, liaisons de portes et
// textures par instance.
[[nodiscard]] nlohmann::json rootTilesJson(const TileMap& tileMap, const PositionMap& switchIds,
                                           const PositionMap& doorOpensWith,
                                           const PositionMap& textureOverrideByPosition) {
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
            const std::pair<int, int> position = std::make_pair(column, row);
            if (isTriggerType(type)) {
                writeIfFound(switchIds, position, "id", tile);
            } else if (isDoorLikeType(type)) {
                writeIfFound(doorOpensWith, position, "opensWith", tile);
            }
            // Texture assignee par instance (EX-EDIT-043) : independante du type, peut
            // accompagner n'importe quel champ ci-dessus.
            writeIfFound(textureOverrideByPosition, position, "texture", tile);
            tiles.push_back(std::move(tile));
        }
    }
    return tiles;
}

// Tableau racine optionnel "layers" (EX-LVL-016, LOT-04) : les couches VISIBLES uniquement,
// dans leur ordre de superposition, du sol vers le decor. La grille racine, promue en couche
// au chargement, en est exclue -- elle est deja "tiles". Rien a ecrire, pas de
// champ : une carte plate reste une carte plate.
[[nodiscard]] nlohmann::json visibleLayersJson(const std::vector<TileLayer>& layers) {
    nlohmann::json layersJson = nlohmann::json::array();
    for (const TileLayer& layer : layers) {
        if (!isPromotedRootLayer(layer)) {
            nlohmann::json layerJson;
            // Le nom est libre et facultatif : omis quand il est vide, comme tout champ a sa
            // valeur par defaut. Le role, lui, est toujours ecrit -- c'est la raison d'etre de la
            // couche, jamais du bruit.
            if (!layer.name.empty()) {
                layerJson["name"] = layer.name;
            }
            layerJson["kind"] = layerKindName(layer.kind);
            layerJson["tiles"] = layerTilesJson(layer.tiles);
            writeProperties(layer.properties, layerJson);
            layersJson.push_back(std::move(layerJson));
        }
    }
    return layersJson;
}

// Tableau racine "entities" (EX-LVL-017, LOT-04). Le type est ecrit meme vide : une entite sans
// type est une donnee fautive qu'il vaut mieux voir dans le fichier que faire disparaitre a
// l'enregistrement.
[[nodiscard]] nlohmann::json entitiesJson(const std::vector<MapEntity>& entities) {
    nlohmann::json array = nlohmann::json::array();
    for (const MapEntity& entity : entities) {
        nlohmann::json entityJson;
        entityJson["type"] = entity.type;
        entityJson["x"] = entity.position.column;
        entityJson["y"] = entity.position.row;
        writeProperties(entity.properties, entityJson);
        array.push_back(std::move(entityJson));
    }
    return array;
}

// Un plan (EX-DEC-040, LOT-69). Chaque champ a sa valeur par defaut est omis, comme partout
// ailleurs dans ce format (convention du LOT-67) -- un plan solidaire du niveau a densite native
// ne produit donc qu'un seul champ, "file".
[[nodiscard]] nlohmann::json planeJson(const Plane& plane) {
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
    return entry;
}

}  // namespace

std::string LevelWriter::toJsonString(const Level& level) {
    // Recompose l'agregat a partir des accesseurs : Level ne conserve pas de LevelData, et le
    // cout (une copie de la grille et des vecteurs) est celui d'un enregistrement de fichier, pas
    // d'une boucle de jeu. entry/exit sont volontairement omis -- buildJson les relit de la
    // grille, jamais du champ.
    return buildJson(LevelData{.name = level.name(),
                               .tileMap = level.tileMap(),
                               .layers = level.layers(),
                               .entities = level.entities(),
                               .mechanisms = level.mechanisms(),
                               .background = level.background(),
                               .skinSet = level.skinSet(),
                               .textureOverrides = level.textureOverrides(),
                               .cameraFraming = level.cameraFraming(),
                               .planes = level.planes(),
                               .parallaxEnabled = level.parallaxEnabled()});
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

std::string LevelWriter::buildJson(const LevelData& data) {
    // Composantes lues sous leur nom court. Des references, jamais des copies : le seul but est
    // que le corps ci-dessous se lise sans un "data." sur chaque ligne.
    const std::string& name = data.name;
    const TileMap& tileMap = data.tileMap;
    const std::optional<std::string>& background = data.background;
    const std::optional<std::string>& skinSet = data.skinSet;
    const CameraFramingConfig& cameraFraming = data.cameraFraming;
    const std::vector<Plane>& planes = data.planes;
    const bool parallaxEnabled = data.parallaxEnabled;

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
        root["cameraFraming"] = cameraFramingJson(cameraFraming);
    }

    const PositionMap switchIds = triggerIds(tileMap);
    root["tiles"] = rootTilesJson(tileMap, switchIds, doorOpensWithIds(data.mechanisms, switchIds),
                                  textureOverridesByPosition(data.textureOverrides));

    nlohmann::json layersJson = visibleLayersJson(data.layers);
    if (!layersJson.empty()) {
        root["layers"] = std::move(layersJson);
    }

    // Tableau racine optionnel "entities" (EX-LVL-017, LOT-04), omis si vide.
    if (!data.entities.empty()) {
        root["entities"] = entitiesJson(data.entities);
    }

    // Tableau racine optionnel "planes" (EX-DEC-040, LOT-69), omis si vide : l'ordre du vecteur
    // est preserve (rang = superposition).
    if (!planes.empty()) {
        nlohmann::json planesJson = nlohmann::json::array();
        for (const Plane& plane : planes) {
            planesJson.push_back(planeJson(plane));
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
