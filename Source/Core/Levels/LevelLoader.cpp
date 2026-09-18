// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "Core/Levels/LevelLoader.h"

#include <fstream>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <nlohmann/json.hpp>

#include "Core/Data/JsonDocument.h"
#include "Core/Levels/LevelsLog.h"
#include "Core/Levels/MapEntity.h"
#include "Core/Levels/TileLayer.h"
#include "Core/Levels/TileMap.h"
#include "Core/Levels/TileType.h"
#include "Core/Levels/TileTypeName.h"

namespace core {

namespace {

// Construit un résultat d'échec avec un message et un code categorise (EX-EDIT-012).
// Journalise systematiquement la raison ici (point unique) : chaque site d'appel n'a pas a le
// refaire, et un echec de chargement reste tracable meme hors du contexte HMI (tests, outillage).
[[nodiscard]] LevelLoadResult failure(std::string message, LevelValidationError code) {
    LEVELS_LOG_WARNING("Echec du chargement : " + message);
    return LevelLoadResult{.level = std::nullopt, .error = std::move(message), .errorCode = code};
}

// --- Couches et entites (LOT-04, format version 3) -------------------------------------------

// Role d'une couche d'apres son champ "kind". Valeur de conception par defaut (Ground) si absent
// ou non reconnu -- meme tolerance que le reste du format (EX-NFR-040) : une couche au role
// inconnu s'affiche, elle ne fait pas echouer le chargement de la carte entiere.
[[nodiscard]] LayerKind parseLayerKind(const nlohmann::json& layer) {
    const std::string name = layer.value("kind", std::string{"ground"});
    for (int raw = 0; raw < LAYER_KIND_COUNT; ++raw) {
        const auto kind = static_cast<LayerKind>(raw);
        if (layerKindName(kind) == name) {
            return kind;
        }
    }
    return LayerKind::Ground;
}

// Range dans @p properties toute cle de @p object qui n'est pas dans @p known.
//
// C'est LE mecanisme qui evite un version: 4. Une cle inconnue n'est ni rejetee (elle ferait
// echouer un fichier produit par une version ulterieure) ni perdue (l'editeur la ferait
// disparaitre au premier enregistrement) : elle est conservee telle quelle et reemise.
//
// Les valeurs COMPOSITES (objet, tableau) n'entrent pas dans core::PropertyValue et sont ignorees
// -- limite documentee sur core::PropertyMap. Le format n'en produit aucune aujourd'hui.
void collectProperties(const nlohmann::json& object, const std::set<std::string>& known,
                       PropertyMap& properties) {
    for (const auto& [key, value] : object.items()) {
        if (known.contains(key)) {
            continue;
        }
        if (value.is_boolean()) {
            properties[key] = value.get<bool>();
        } else if (value.is_number_integer()) {
            properties[key] = value.get<std::int64_t>();
        } else if (value.is_number_float()) {
            properties[key] = value.get<double>();
        } else if (value.is_string()) {
            properties[key] = value.get<std::string>();
        }
    }
}

// Traite le tableau racine optionnel "layers" (LOT-04). Absent = carte version 2, dont la grille
// unique est promue par l'appelant. Chaque couche porte sa propre grille, aux MEMES dimensions que
// la carte : une couche decalee d'une case rendrait la collision incoherente avec l'affichage,
// d'ou le refus plutot qu'un redimensionnement silencieux.
[[nodiscard]] std::optional<LevelLoadResult> parseLayers(const nlohmann::json& root, int width,
                                                         int height,
                                                         std::vector<TileLayer>& layers) {
    if (!root.contains("layers")) {
        return std::nullopt;
    }
    if (!root.at("layers").is_array()) {
        return failure("Le champ 'layers' doit etre une liste", LevelValidationError::ParseError);
    }
    static const std::set<std::string> known{"name", "kind", "tiles"};
    for (const nlohmann::json& layerJson : root.at("layers")) {
        TileLayer layer{.name = layerJson.value("name", std::string{}),
                        .kind = parseLayerKind(layerJson),
                        .tiles = TileMap(width, height),
                        .properties = {}};
        if (layerJson.contains("tiles")) {
            if (!layerJson.at("tiles").is_array()) {
                return failure("Le champ 'tiles' d'une couche doit etre une liste",
                               LevelValidationError::ParseError);
            }
            for (const nlohmann::json& tile : layerJson.at("tiles")) {
                const int x = tile.at("x").get<int>();
                const int y = tile.at("y").get<int>();
                if (!layer.tiles.inBounds(x, y)) {
                    return failure("Tuile hors bornes dans la couche '" + layer.name + "' en (" +
                                       std::to_string(x) + ", " + std::to_string(y) + ")",
                                   LevelValidationError::OutOfBounds);
                }
                const std::optional<TileType> type =
                    parseTileType(tile.at("type").get<std::string>());
                if (!type) {
                    return failure("Type de tuile inconnu dans la couche '" + layer.name +
                                       "' : " + tile.at("type").get<std::string>(),
                                   LevelValidationError::UnknownTileType);
                }
                layer.tiles.setTile(x, y, *type);
            }
        }
        // Une couche 'collision' DECLAREE est refusee : la collision d'une carte est son tableau
        // racine "tiles", et l'accepter ici creerait une seconde grille a tenir d'accord avec la
        // premiere -- celle ou vit l'entree. Mieux vaut un refus
        // nomme qu'une carte a demi jouable (EX-LVL-016, EX-NFR-040).
        if (layer.kind == LayerKind::Collision) {
            return failure(
                "Couche 'collision' declaree dans 'layers' : la grille de collision "
                "d'une carte est son tableau racine 'tiles'",
                LevelValidationError::ParseError);
        }
        collectProperties(layerJson, known, layer.properties);
        layers.push_back(std::move(layer));
    }
    return std::nullopt;
}

// Traite le tableau racine optionnel "entities" (LOT-04). Core n'attribue AUCUNE semantique au
// champ "type" : c'est le gameplay qui l'interprete, et une entite de type inconnu est une erreur
// de conception toleree plutot qu'une carte invalide (EX-NFR-040).
[[nodiscard]] std::optional<LevelLoadResult> parseEntities(const nlohmann::json& root, int width,
                                                           int height,
                                                           std::vector<MapEntity>& entities) {
    if (!root.contains("entities")) {
        return std::nullopt;
    }
    if (!root.at("entities").is_array()) {
        return failure("Le champ 'entities' doit etre une liste", LevelValidationError::ParseError);
    }
    static const std::set<std::string> known{"type", "x", "y"};
    for (const nlohmann::json& entityJson : root.at("entities")) {
        MapEntity entity{.type = entityJson.value("type", std::string{}),
                         .position = GridPosition{.column = entityJson.value("x", 0),
                                                  .row = entityJson.value("y", 0)},
                         .properties = {}};
        if (entity.position.column < 0 || entity.position.column >= width ||
            entity.position.row < 0 || entity.position.row >= height) {
            return failure("Entite hors bornes en (" + std::to_string(entity.position.column) +
                               ", " + std::to_string(entity.position.row) + ")",
                           LevelValidationError::OutOfBounds);
        }
        collectProperties(entityJson, known, entity.properties);
        entities.push_back(std::move(entity));
    }
    return std::nullopt;
}

// Accumulateurs remplis case par case par parseTile() ci-dessous -- toutes des références vers
// les variables locales de LevelLoader::loadFromString, un seul jeu construit pour tout le
// tableau `tiles`.
struct TileParseState {
    TileMap& map;
    GridPosition& entry;
    int& entryCount;
    std::set<std::pair<int, int>>& occupiedPositions;
    std::vector<TileTextureOverride>& textureOverrides;
};

// Traite UNE entrée du tableau `tiles` : pose la tuile dans la grille et alimente les
// accumulateurs de @p state (décompte des entrées, pièces assignées). Extrait de
// LevelLoader::loadFromString ci-dessous (seule sa taille, pas son comportement) : std::nullopt
// en cas de succès, sinon l'échec à renvoyer IMMÉDIATEMENT -- aucun état partiel n'est jamais
// renvoyé avec succès.
[[nodiscard]] std::optional<LevelLoadResult> parseTile(const nlohmann::json& tile,
                                                       TileParseState& state) {
    const int x = tile.at("x").get<int>();
    const int y = tile.at("y").get<int>();
    const std::string typeName = tile.at("type").get<std::string>();

    const std::optional<TileType> type = parseTileType(typeName);
    if (!type) {
        return failure("Type de tuile inconnu : " + typeName,
                       LevelValidationError::UnknownTileType);
    }
    if (!state.map.inBounds(x, y)) {
        return failure(
            "Tuile hors bornes en (" + std::to_string(x) + ", " + std::to_string(y) + ")",
            LevelValidationError::OutOfBounds);
    }
    if (!state.occupiedPositions.emplace(x, y).second) {
        return failure(
            "Deux tuiles a la meme position (" + std::to_string(x) + ", " + std::to_string(y) + ")",
            LevelValidationError::DuplicatePosition);
    }
    state.map.setTile(x, y, *type);

    // Piece assignee a la case (EX-EDIT-043), independante du type de tuile : pas de liste
    // blanche, l'usage est purement visuel.
    if (tile.contains("texture")) {
        state.textureOverrides.push_back(
            TileTextureOverride{.position = GridPosition{.column = x, .row = y},
                                .assetName = tile.at("texture").get<std::string>()});
    }

    if (*type == TileType::Entry) {
        state.entry = GridPosition{.column = x, .row = y};
        ++state.entryCount;
    }
    return std::nullopt;
}

// Valide les champs d'en-tête obligatoires (width/height/tiles, dimensions strictement positives,
// version de format gérée) et extrait @p width/@p height. Extrait de
// LevelLoader::loadFromString : std::nullopt en cas de succès, sinon l'échec à renvoyer
// immédiatement.
[[nodiscard]] std::optional<LevelLoadResult> parseHeader(const nlohmann::json& root, int& width,
                                                         int& height) {
    if (!root.contains("width") || !root.contains("height") || !root.contains("tiles")) {
        return failure("Champ obligatoire manquant (width, height ou tiles)",
                       LevelValidationError::ParseError);
    }
    if (!root.at("tiles").is_array()) {
        return failure("Le champ 'tiles' doit etre une liste", LevelValidationError::ParseError);
    }

    width = root.at("width").get<int>();
    height = root.at("height").get<int>();
    if (width <= 0 || height <= 0) {
        return failure("Dimensions invalides (width et height doivent etre > 0)",
                       LevelValidationError::ParseError);
    }

    // Version du format (EX-LVL-005) : absente = version initiale (0), sans erreur ni
    // avertissement (rétrocompatibilité des niveaux antérieurs à ce champ).
    const int version = root.value("version", 0);
    if (version > LEVEL_FORMAT_VERSION) {
        return failure("Version de format non geree : " + std::to_string(version) +
                           " (maximum gere : " + std::to_string(LEVEL_FORMAT_VERSION) + ")",
                       LevelValidationError::UnsupportedFormatVersion);
    }
    return std::nullopt;
}

// Exactement une entrée (EX-LVL-004). Extrait de LevelLoader::loadFromString : std::nullopt en cas
// de succès, sinon l'échec à renvoyer immédiatement.
[[nodiscard]] std::optional<LevelLoadResult> validateEntryCount(int entryCount) {
    if (entryCount == 0) {
        return failure("Carte sans entree (aucune tuile 'entry')",
                       LevelValidationError::InvalidEntryCount);
    }
    if (entryCount > 1) {
        return failure("Plusieurs entrees dans la carte (une seule attendue)",
                       LevelValidationError::InvalidEntryCount);
    }
    return std::nullopt;
}

}  // namespace

// Charge un niveau depuis une chaine JSON.
LevelLoadResult LevelLoader::loadFromString(std::string_view json) {
    // Enveloppe commune : brique partagee du LOT-79 (EX-CNT-012). La garde de version est
    // desactivee (0) parce que `parseHeader` porte la sienne, avec sa propre categorie d'echec.
    const JsonDocument document = readJsonObject(json, 0, "niveau");
    if (!document.ok()) {
        return failure(document.message, LevelValidationError::ParseError);
    }
    const nlohmann::json& root = document.root;
    try {
        int width = 0;
        int height = 0;
        if (std::optional<LevelLoadResult> headerError = parseHeader(root, width, height)) {
            return std::move(*headerError);
        }

        std::string name = root.value("name", std::string{});
        TileMap map(width, height);

        GridPosition entry{};
        int entryCount = 0;
        std::set<std::pair<int, int>> occupiedPositions;
        std::vector<TileTextureOverride> textureOverrides;

        // Chaque objet de 'tiles' place une tuile dans la grille.
        TileParseState tileState{.map = map,
                                 .entry = entry,
                                 .entryCount = entryCount,
                                 .occupiedPositions = occupiedPositions,
                                 .textureOverrides = textureOverrides};
        for (const nlohmann::json& tile : root.at("tiles")) {
            std::optional<LevelLoadResult> tileError = parseTile(tile, tileState);
            if (tileError) {
                return std::move(*tileError);
            }
        }

        if (std::optional<LevelLoadResult> countError = validateEntryCount(entryCount)) {
            return std::move(*countError);
        }

        LEVELS_LOG_TRACE("Carte chargee : '" + name + "' (" + std::to_string(width) + "x" +
                         std::to_string(height) + ")");
        // Couches et entites (LOT-04). Le tableau racine "layers" ne porte que les couches
        // VISIBLES (sol, decor) : la grille de collision, elle, EST le tableau racine "tiles" --
        // celui qui porte deja l'entree, et dont dependent l'exploration et la grille de combat
        // tactique. Une seule source de verite, jamais deux grilles a tenir d'accord.
        std::vector<TileLayer> declaredLayers;
        if (std::optional<LevelLoadResult> layersError =
                parseLayers(root, width, height, declaredLayers)) {
            return std::move(*layersError);
        }
        std::vector<MapEntity> entities;
        if (std::optional<LevelLoadResult> entitiesError =
                parseEntities(root, width, height, entities)) {
            return std::move(*entitiesError);
        }

        // La grille racine est PROMUE en couche de tete, pour que tout consommateur boucle sur
        // `layers()` sans cas particulier (EX-LVL-016). Son role dit ce qu'elle vaut : `Collision`
        // quand la carte declare des couches visibles a cote, `Legacy` quand elle n'en declare
        // aucune -- une grille plate de version 2, qui vaut alors a la fois decor et collision.
        std::vector<TileLayer> layers;
        layers.reserve(declaredLayers.size() + 1);
        layers.push_back(
            TileLayer{.name = {},
                      .kind = declaredLayers.empty() ? LayerKind::Legacy : LayerKind::Collision,
                      .tiles = map,
                      .properties = {}});
        for (TileLayer& declared : declaredLayers) {
            layers.push_back(std::move(declared));
        }

        return LevelLoadResult{
            .level = Level(LevelData{.name = std::move(name),
                                     .tileMap = std::move(map),
                                     .layers = std::move(layers),
                                     .entities = std::move(entities),
                                     .entry = entry,
                                     .textureOverrides = std::move(textureOverrides)}),
            .error = {}};
    } catch (const nlohmann::json::exception& error) {
        return failure(std::string("JSON invalide : ") + error.what(),
                       LevelValidationError::ParseError);
    }
}

// Charge un niveau depuis un fichier (lecture binaire puis delegation a loadFromString).
LevelLoadResult LevelLoader::loadFromFile(const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        return failure("Fichier de niveau introuvable : " + path.string(),
                       LevelValidationError::FileNotFound);
    }
    std::ostringstream buffer;
    buffer << file.rdbuf();
    return loadFromString(buffer.str());
}

}  // namespace core
