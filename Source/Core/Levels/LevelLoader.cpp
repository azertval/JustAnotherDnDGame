// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Core/Levels/LevelLoader.h"

#include <cmath>
#include <fstream>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <nlohmann/json.hpp>

#include "Core/Levels/CameraFraming.h"
#include "Core/Levels/LevelsLog.h"
#include "Core/Levels/TileMap.h"
#include "Core/Levels/TileType.h"
#include "Core/Levels/TileTypeName.h"

namespace core {

namespace {

// Construit un résultat d'échec avec un message et un code categorise (LOT-15, EX-EDIT-012).
// Journalise systematiquement la raison ici (point unique) : chaque site d'appel n'a pas a le
// refaire, et un echec de chargement reste tracable meme hors du contexte HMI (tests, outillage).
[[nodiscard]] LevelLoadResult failure(std::string message, LevelValidationError code) {
    LEVELS_LOG_WARNING("Echec du chargement : " + message);
    return LevelLoadResult{.level = std::nullopt, .error = std::move(message), .errorCode = code};
}

// Vrai pour les tuiles "déclencheur" liables à une porte (interrupteur ou plaque de pression,
// EX-GP-020/EX-GP-025) : les deux partagent la même règle d'identifiant.
[[nodiscard]] bool isTriggerType(TileType type) {
    return type == TileType::Switch || type == TileType::PressurePlate;
}

// Une porte lue, avec la référence (opensWith) à résoudre en position d'interrupteur.
struct DoorLink {
    GridPosition position;
    std::string opensWith;
};

// Une porte verrouillée lue (EX-GP-023), même forme que DoorLink : la référence (opensWith) est
// résolue en position de clé plutôt que d'interrupteur, mais elle est OBLIGATOIRE ici (contraire
// a une porte classique, ou l'absence de mecanisme est une simple tuile) -- voir la resolution
// dediee plus bas.
struct LockedDoorLink {
    GridPosition position;
    std::string opensWith;
};

// Convertit le champ "depth" d'un plan ("behind"/"front") ; valeur par défaut (Behind) si absent
// ou non reconnu -- même tolérance que parseDecorLayer, et symétrique à
// planeDepthName (LevelWriter.cpp).
[[nodiscard]] PlaneDepth parsePlaneDepth(const nlohmann::json& plane) {
    return plane.value("depth", std::string{"behind"}) == "front" ? PlaneDepth::Front
                                                                  : PlaneDepth::Behind;
}

// Traite le tableau racine optionnel "planes" (EX-DEC-040, LOT-69) : absent = aucun plan
// (rétrocompatibilité, EX-LVL-005). L'ordre du tableau est préservé tel quel (rang =
// superposition, EX-DEC-040), et aucune existence de fichier n'est vérifiée (EX-NFR-011 : Core
// ignore tout du dossier des plans) -- un plan introuvable se replie en damier côté HMI
// (EX-NFR-040).
//
// Les garde-fous de coût (EX-DEC-044) sont appliqués ICI, au chargement, et non laissés à
// l'usage : rien n'empêcherait autrement seize plans à densité native sur un grand niveau, soit
// plusieurs centaines de mégaoctets de texture.
[[nodiscard]] std::optional<LevelLoadResult> parsePlanes(const nlohmann::json& root, int width,
                                                         int height, std::vector<Plane>& planes) {
    if (!root.contains("planes")) {
        return std::nullopt;
    }
    if (!root.at("planes").is_array()) {
        return failure("Le champ 'planes' doit etre une liste", LevelValidationError::ParseError);
    }
    if (root.at("planes").size() > MAX_PLANES_PER_LEVEL) {
        return failure("Trop de plans : " + std::to_string(root.at("planes").size()) +
                           " (maximum " + std::to_string(MAX_PLANES_PER_LEVEL) + ")",
                       LevelValidationError::ParseError);
    }
    for (const nlohmann::json& planeJson : root.at("planes")) {
        Plane plane;
        plane.fileName = planeJson.at("file").get<std::string>();
        if (plane.fileName.empty()) {
            return failure("Le champ 'file' d'un plan ne doit pas etre vide",
                           LevelValidationError::ParseError);
        }
        plane.pixelsPerUnit = planeJson.value("pixelsPerUnit", PLANE_NATIVE_PIXELS_PER_UNIT);
        if (!isValidPlaneDensity(plane.pixelsPerUnit)) {
            return failure("Densite de plan invalide : " + std::to_string(plane.pixelsPerUnit) +
                               " (attendu 4, 8 ou 16)",
                           LevelValidationError::ParseError);
        }
        // Une densite valide peut quand meme depasser ce que le materiel accepte, sur un grand
        // niveau : c'est la combinaison taille x densite qui compte, pas la densite seule.
        if (static_cast<long long>(width) * plane.pixelsPerUnit > MAX_PLANE_TEXTURE_EXTENT ||
            static_cast<long long>(height) * plane.pixelsPerUnit > MAX_PLANE_TEXTURE_EXTENT) {
            return failure("Plan '" + plane.fileName + "' trop grand : " + std::to_string(width) +
                               "x" + std::to_string(height) + " cases a " +
                               std::to_string(plane.pixelsPerUnit) +
                               " px/unite depasse la limite de texture (" +
                               std::to_string(MAX_PLANE_TEXTURE_EXTENT) + " px)",
                           LevelValidationError::ParseError);
        }
        plane.parallaxX = planeJson.value("parallaxX", 1.0F);
        // parallaxY retombe sur parallaxX plutot que sur 1.0 : un plan qui declare un seul facteur
        // veut presque toujours le meme sur les deux axes, et l'ecrire deux fois serait du bruit.
        plane.parallaxY = planeJson.value("parallaxY", plane.parallaxX);
        if (!std::isfinite(plane.parallaxX) || !std::isfinite(plane.parallaxY)) {
            return failure("Facteur de parallaxe non fini pour le plan '" + plane.fileName + "'",
                           LevelValidationError::ParseError);
        }
        plane.opacity = planeJson.value("opacity", 1.0F);
        if (!(plane.opacity >= 0.0F) || !(plane.opacity <= 1.0F)) {
            return failure("Opacite hors de [0,1] pour le plan '" + plane.fileName + "'",
                           LevelValidationError::ParseError);
        }
        plane.depth = parsePlaneDepth(planeJson);
        planes.push_back(std::move(plane));
    }
    return std::nullopt;
}

// Accumulateurs remplis case par case par parseTile() ci-dessous -- toutes des références vers
// les variables locales de LevelLoader::loadFromString, un seul jeu construit pour tout le
// tableau `tiles`.
struct TileParseState {
    TileMap& map;
    GridPosition& entry;
    GridPosition& exit;
    int& entryCount;
    int& exitCount;
    std::set<std::pair<int, int>>& occupiedPositions;
    std::unordered_map<std::string, GridPosition>& switchesById;
    std::vector<DoorLink>& doors;
    std::unordered_map<std::string, GridPosition>& keysById;
    std::vector<LockedDoorLink>& lockedDoors;
    std::vector<TileTextureOverride>& textureOverrides;
};

// Traite UNE entrée du tableau `tiles` : pose la tuile dans la grille et alimente les
// accumulateurs de @p state (portes/dangers à résoudre, décompte entrée/sortie...). Extrait de
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

    // Texture assignee par instance (EX-EDIT-043), independante du type de tuile : pas de liste
    // blanche (usage purement visuel, contrairement aux liens de mecanismes).
    if (tile.contains("texture")) {
        state.textureOverrides.push_back(
            TileTextureOverride{.position = GridPosition{.column = x, .row = y},
                                .assetName = tile.at("texture").get<std::string>()});
    }

    if (*type == TileType::Entry) {
        state.entry = GridPosition{.column = x, .row = y};
        ++state.entryCount;
    } else if (*type == TileType::Exit) {
        state.exit = GridPosition{.column = x, .row = y};
        ++state.exitCount;
    } else if (isTriggerType(*type)) {
        // Interrupteur ou plaque de pression (EX-GP-020/EX-GP-025) : meme regle d'identifiant,
        // partagee avec les portes via 'opensWith'.
        const std::string id = tile.value("id", std::string{});
        if (id.empty()) {
            return failure(
                "Declencheur sans 'id' en (" + std::to_string(x) + ", " + std::to_string(y) + ")",
                LevelValidationError::MissingSwitchId);
        }
        if (!state.switchesById.emplace(id, GridPosition{.column = x, .row = y}).second) {
            return failure("Identifiant de declencheur en double : " + id,
                           LevelValidationError::DuplicateSwitchId);
        }
    } else if (*type == TileType::Door) {
        state.doors.push_back(DoorLink{.position = GridPosition{.column = x, .row = y},
                                       .opensWith = tile.value("opensWith", std::string{})});
    } else if (*type == TileType::Key) {
        // Meme regle d'identifiant que Switch/PressurePlate (EX-GP-023), mais espace de liaison
        // distinct (keysById) : une cle doit obligatoirement etre liee, contrairement a un simple
        // declencheur, verifie plus bas.
        const std::string id = tile.value("id", std::string{});
        if (id.empty()) {
            return failure(
                "Cle sans 'id' en (" + std::to_string(x) + ", " + std::to_string(y) + ")",
                LevelValidationError::MissingSwitchId);
        }
        if (!state.keysById.emplace(id, GridPosition{.column = x, .row = y}).second) {
            return failure("Identifiant de cle en double : " + id,
                           LevelValidationError::DuplicateSwitchId);
        }
    } else if (*type == TileType::LockedDoor) {
        state.lockedDoors.push_back(
            LockedDoorLink{.position = GridPosition{.column = x, .row = y},
                           .opensWith = tile.value("opensWith", std::string{})});
    }
    return std::nullopt;
}

// Le champ racine "decors" (LOT-49) est OBSOLETE depuis le LOT-69, qui remplace les
// decors-sprites par des plans picturaux (EX-DEC-040). Un fichier qui le porte encore reste
// VALIDE : on l'ignore en journalisant, jamais en echouant.
//
// Un champ obsolete n'est pas une donnee *invalide* -- EX-LVL-004 vise la validite -- et le
// rejeter rendrait illisible tout niveau personnel anterieur, a rebours de l'invariant
// EX-LVL-005. Convertir automatiquement un assemblage de sprites en surface peinte serait par
// ailleurs impossible sans rasterisation : mieux vaut ignorer franchement que mentir sur le
// resultat. LevelWriter ne le reemet jamais, donc charger puis enregistrer migre le fichier.
void warnOnObsoleteDecors(const nlohmann::json& root, const std::string& levelName) {
    if (root.contains("decors")) {
        LEVELS_LOG_WARNING("Niveau '" + levelName +
                           "' : champ 'decors' obsolete depuis le LOT-69, ignore. "
                           "Utiliser 'planes' (plans picturaux).");
    }
}

// Traite le champ racine optionnel "cameraFraming" (EX-LVL-006, LOT-64) : absent = aucun cadrage
// declare (@p declared reste vide, la regle de repli s'appliquera). Valide immediatement contre
// les dimensions du niveau (EX-LVL-004) -- le mode inconnu est distingue des autres erreurs de
// validation ici, faute de pouvoir construire un CameraFramingConfig sans mode reconnu. Extrait de
// LevelLoader::loadFromString : std::nullopt en cas de succes, sinon l'echec a renvoyer
// immediatement.
[[nodiscard]] std::optional<LevelLoadResult> parseCameraFraming(
    const nlohmann::json& root, int width, int height,
    std::optional<CameraFramingConfig>& declared) {
    if (!root.contains("cameraFraming")) {
        return std::nullopt;
    }
    const nlohmann::json& framingJson = root.at("cameraFraming");
    const std::string modeName = framingJson.value("mode", std::string{});
    const std::optional<CameraFramingMode> mode = parseCameraFramingMode(modeName);
    if (!mode) {
        return failure("cameraFraming.mode inconnu : " + modeName,
                       LevelValidationError::InvalidCameraFraming);
    }
    CameraFramingConfig config;
    config.mode = *mode;
    if (framingJson.contains("roomWidthTiles")) {
        config.roomWidthTiles = framingJson.at("roomWidthTiles").get<int>();
    }
    if (framingJson.contains("roomHeightTiles")) {
        config.roomHeightTiles = framingJson.at("roomHeightTiles").get<int>();
    }
    // Zones de camera dessinees a la main (mode PerRoom uniquement) : liste d'objets {x, y, width,
    // height}, en cases. Absente = vecteur vide (decoupage automatique en grille inchange).
    if (framingJson.contains("zones")) {
        if (!framingJson.at("zones").is_array()) {
            return failure("cameraFraming.zones doit etre une liste",
                           LevelValidationError::InvalidCameraFraming);
        }
        for (const nlohmann::json& zoneJson : framingJson.at("zones")) {
            config.zones.push_back(CameraZone{.x = zoneJson.value("x", 0),
                                              .y = zoneJson.value("y", 0),
                                              .width = zoneJson.value("width", 1),
                                              .height = zoneJson.value("height", 1)});
        }
    }
    if (const std::optional<std::string> error =
            validateCameraFramingConfig(config, width, height)) {
        return failure(*error, LevelValidationError::InvalidCameraFraming);
    }
    declared = config;
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
    // avertissement (rétrocompatibilité des niveaux antérieurs à ce champ, LOT-44).
    const int version = root.value("version", 0);
    if (version > LEVEL_FORMAT_VERSION) {
        return failure("Version de format non geree : " + std::to_string(version) +
                           " (maximum gere : " + std::to_string(LEVEL_FORMAT_VERSION) + ")",
                       LevelValidationError::UnsupportedFormatVersion);
    }
    return std::nullopt;
}

// Exactement une entrée et une sortie (EX-LVL-004). Extrait de LevelLoader::loadFromString :
// std::nullopt en cas de succès, sinon l'échec à renvoyer immédiatement.
[[nodiscard]] std::optional<LevelLoadResult> validateEntryExitCounts(int entryCount,
                                                                     int exitCount) {
    if (entryCount == 0) {
        return failure("Niveau sans entree (aucune tuile 'entry')",
                       LevelValidationError::InvalidEntryCount);
    }
    if (entryCount > 1) {
        return failure("Plusieurs entrees dans le niveau (une seule attendue)",
                       LevelValidationError::InvalidEntryCount);
    }
    if (exitCount == 0) {
        return failure("Niveau sans sortie (aucune tuile 'exit')",
                       LevelValidationError::InvalidExitCount);
    }
    if (exitCount > 1) {
        return failure("Plusieurs sorties dans le niveau (une seule attendue)",
                       LevelValidationError::InvalidExitCount);
    }
    return std::nullopt;
}

}  // namespace

// Charge un niveau depuis une chaine JSON.
LevelLoadResult LevelLoader::loadFromString(std::string_view json) {
    try {
        const nlohmann::json root = nlohmann::json::parse(json);

        int width = 0;
        int height = 0;
        if (std::optional<LevelLoadResult> headerError = parseHeader(root, width, height)) {
            return std::move(*headerError);
        }

        std::string name = root.value("name", std::string{});
        // Asset de fond et jeu de skins du niveau (EX-REN-044/EX-EDIT-024) : chaines optionnelles,
        // Core ignore tout du dossier d'assets et du mode de rendu (EX-NFR-011).
        std::optional<std::string> background;
        if (root.contains("background")) {
            background = root.at("background").get<std::string>();
        }
        std::optional<std::string> skinSet;
        if (root.contains("skinSet")) {
            skinSet = root.at("skinSet").get<std::string>();
        }
        // Cadrage de camera (EX-LVL-006, LOT-64) : declare (valide contre width/height) ou absent
        // -- resolu plus bas, une fois toutes les tuiles lues (la regle de repli ne depend que des
        // dimensions, deja connues ici, mais resoudre au meme endroit que la construction du
        // niveau garde la regle a un seul site d'appel).
        std::optional<CameraFramingConfig> declaredCameraFraming;
        if (std::optional<LevelLoadResult> framingError =
                parseCameraFraming(root, width, height, declaredCameraFraming)) {
            return std::move(*framingError);
        }
        TileMap map(width, height);

        GridPosition entry{};
        GridPosition exit{};
        int entryCount = 0;
        int exitCount = 0;
        std::set<std::pair<int, int>> occupiedPositions;
        std::unordered_map<std::string, GridPosition> switchesById;
        std::vector<DoorLink> doors;
        std::unordered_map<std::string, GridPosition> keysById;
        std::vector<LockedDoorLink> lockedDoors;
        std::vector<TileTextureOverride> textureOverrides;

        // Chaque objet de 'tiles' place une tuile dans la grille.
        TileParseState tileState{
            map,          entry, exit,     entryCount,  exitCount,       occupiedPositions,
            switchesById, doors, keysById, lockedDoors, textureOverrides};
        for (const nlohmann::json& tile : root.at("tiles")) {
            std::optional<LevelLoadResult> tileError = parseTile(tile, tileState);
            if (tileError) {
                return std::move(*tileError);
            }
        }

        if (std::optional<LevelLoadResult> countError =
                validateEntryExitCounts(entryCount, exitCount)) {
            return std::move(*countError);
        }

        // Résout les liaisons interrupteur↔porte par identifiant. Une porte sans 'opensWith'
        // est une simple tuile (pas de mécanisme).
        std::vector<Mechanism> mechanisms;
        for (const DoorLink& door : doors) {
            if (door.opensWith.empty()) {
                continue;
            }
            const auto found = switchesById.find(door.opensWith);
            if (found == switchesById.end()) {
                return failure("Porte liee a un interrupteur inexistant : " + door.opensWith,
                               LevelValidationError::UnresolvedMechanism);
            }
            mechanisms.push_back(
                Mechanism{.switchPosition = found->second, .doorPosition = door.position});
        }

        // Résout les liaisons clé↔porte verrouillée (EX-GP-023), append à la MÊME liste que
        // ci-dessus (aucune nouvelle notion de liaison) : `MechanismController` distingue leur
        // comportement au type de la tuile déclencheur, pas à leur provenance dans ce vecteur.
        // Contrairement à une porte classique, le lien est OBLIGATOIRE dans les deux sens : une
        // porte verrouillée sans 'opensWith' (ou vers une clé inexistante) et une clé qu'aucune
        // porte ne referme sont toutes deux des niveaux invalides.
        std::set<std::string> usedKeyIds;
        for (const LockedDoorLink& lockedDoor : lockedDoors) {
            if (lockedDoor.opensWith.empty()) {
                return failure("Porte verrouillee sans cle liee en (" +
                                   std::to_string(lockedDoor.position.column) + ", " +
                                   std::to_string(lockedDoor.position.row) + ")",
                               LevelValidationError::UnresolvedMechanism);
            }
            const auto found = keysById.find(lockedDoor.opensWith);
            if (found == keysById.end()) {
                return failure(
                    "Porte verrouillee liee a une cle inexistante : " + lockedDoor.opensWith,
                    LevelValidationError::UnresolvedMechanism);
            }
            usedKeyIds.insert(lockedDoor.opensWith);
            mechanisms.push_back(
                Mechanism{.switchPosition = found->second, .doorPosition = lockedDoor.position});
        }
        for (const auto& keyEntry : keysById) {
            if (usedKeyIds.find(keyEntry.first) == usedKeyIds.end()) {
                return failure("Cle sans porte verrouillee liee : " + keyEntry.first,
                               LevelValidationError::UnresolvedMechanism);
            }
        }

        warnOnObsoleteDecors(root, name);

        std::vector<Plane> planes;
        if (std::optional<LevelLoadResult> planesError = parsePlanes(root, width, height, planes)) {
            return std::move(*planesError);
        }
        // Drapeau de parallaxe (EX-DEC-043) : vrai par defaut, comme le veut la convention « le
        // defaut n'est jamais ecrit » -- un niveau anterieur au LOT-69 se comporte donc comme un
        // niveau qui l'active, ce qui est sans effet tant qu'il n'a aucun plan.
        const bool parallaxEnabled = root.value("parallax", true);

        // Regle de repli (EX-LVL-006) appliquee ici, au point unique de construction du niveau :
        // un champ absent reproduit exactement le comportement historique (core::CameraFraming.h).
        const CameraFramingConfig cameraFraming =
            resolveCameraFraming(declaredCameraFraming, width, height);

        LEVELS_LOG_TRACE("Niveau charge : '" + name + "' (" + std::to_string(width) + "x" +
                         std::to_string(height) + ", " + std::to_string(mechanisms.size()) +
                         " mecanisme(s))");
        return LevelLoadResult{
            .level = Level(LevelData{.name = std::move(name),
                                     .tileMap = std::move(map),
                                     .entry = entry,
                                     .exit = exit,
                                     .mechanisms = std::move(mechanisms),
                                     .background = std::move(background),
                                     .skinSet = std::move(skinSet),
                                     .textureOverrides = std::move(textureOverrides),
                                     .cameraFraming = cameraFraming,
                                     .planes = std::move(planes),
                                     .parallaxEnabled = parallaxEnabled}),
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
