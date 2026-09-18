// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "Core/Levels/LevelWriter.h"

#include <cstdint>
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
        // Alternative par alternative plutot que std::visit : l'analyseur statique ne suit pas la
        // table de saut de visit et croit la valeur non initialisee.
        if (const auto* const flag = std::get_if<bool>(&value)) {
            object[key] = *flag;
        } else if (const auto* const integer = std::get_if<std::int64_t>(&value)) {
            object[key] = *integer;
        } else if (const auto* const real = std::get_if<double>(&value)) {
            object[key] = *real;
        } else if (const auto* const text = std::get_if<std::string>(&value)) {
            object[key] = *text;
        }
    }
}

// Tuiles non vides d'une grille, au format {x, y, type}. Les cases vides sont omises (EX-LVL-003),
// et une couche ne porte pas le champ specifique du tableau racine (texture) : les pieces
// assignees par case restent attachees a la grille racine.
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

// Position (colonne, ligne) -> chaine associee (piece assignee).
using PositionMap = std::map<std::pair<int, int>, std::string>;

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

// Tableau racine "tiles" : grille de collision et pieces assignees par case.
[[nodiscard]] nlohmann::json rootTilesJson(const TileMap& tileMap,
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
            // Piece assignee a la case (EX-EDIT-043), independante du type.
            writeIfFound(textureOverrideByPosition, std::make_pair(column, row), "texture", tile);
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

}  // namespace

std::string LevelWriter::toJsonString(const Level& level) {
    // Recompose l'agregat a partir des accesseurs : Level ne conserve pas de LevelData, et le
    // cout (une copie de la grille et des vecteurs) est celui d'un enregistrement de fichier, pas
    // d'une boucle de jeu. L'entree est volontairement omise -- buildJson la relit de la grille,
    // jamais du champ.
    return buildJson(LevelData{.name = level.name(),
                               .tileMap = level.tileMap(),
                               .layers = level.layers(),
                               .entities = level.entities(),
                               .textureOverrides = level.textureOverrides()});
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
    const TileMap& tileMap = data.tileMap;

    nlohmann::json root;
    root["version"] = LEVEL_FORMAT_VERSION;
    root["name"] = data.name;
    root["width"] = tileMap.width();
    root["height"] = tileMap.height();
    root["tiles"] = rootTilesJson(tileMap, textureOverridesByPosition(data.textureOverrides));

    nlohmann::json layersJson = visibleLayersJson(data.layers);
    if (!layersJson.empty()) {
        root["layers"] = std::move(layersJson);
    }

    // Tableau racine optionnel "entities" (EX-LVL-017, LOT-04), omis si vide.
    if (!data.entities.empty()) {
        root["entities"] = entitiesJson(data.entities);
    }

    // Indentation a deux espaces et saut de ligne final, comme tout ecrivain JSON du projet : une
    // carte est un fichier versionne, dont la relecture en revue de code suppose un diff ligne a
    // ligne.
    return root.dump(2) + "\n";
}

}  // namespace core
