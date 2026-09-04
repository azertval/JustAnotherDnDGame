// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Graphics/SkinCatalog.h"

#include <fstream>
#include <sstream>
#include <system_error>
#include <utility>

#include <nlohmann/json.hpp>

#include "Core/Data/JsonDocument.h"
#include "Core/Levels/TileTypeName.h"
#include "HMI/Graphics/GraphicsLog.h"

namespace hmi {

namespace {

// Noms des champs du format. Nommes plutot que repetes en litteraux : le lecteur et l'ecrivain ne
// peuvent pas diverger sur une faute de frappe.
constexpr const char* FIELD_VERSION = "version";
constexpr const char* FIELD_DEFAULT = "defaut";
constexpr const char* FIELD_SETS = "jeux";
constexpr const char* FIELD_ASSET = "asset";
constexpr const char* FIELD_MODE = "mode";

// Construit un resultat d'echec, en journalisant la raison en un point unique (meme patron que
// core::LevelLoader::failure) : chaque site d'appel n'a pas a le refaire.
// Traduction de la categorie partagee (LOT-79) vers celle, documentee, de ce catalogue. Le
// `switch` est exhaustif et sans `default` : ajouter une categorie d'un cote fait echouer la
// compilation plutot que de tomber silencieusement dans un cas par defaut.
[[nodiscard]] SkinCatalogError mapError(core::JsonReadError code) {
    switch (code) {
        case core::JsonReadError::None:
            return SkinCatalogError::None;
        case core::JsonReadError::FileNotFound:
            return SkinCatalogError::FileNotFound;
        case core::JsonReadError::ParseError:
            return SkinCatalogError::ParseError;
        case core::JsonReadError::UnsupportedVersion:
            return SkinCatalogError::UnsupportedVersion;
        case core::JsonReadError::MalformedStructure:
            return SkinCatalogError::MalformedStructure;
    }
    return SkinCatalogError::ParseError;
}

[[nodiscard]] SkinCatalogResult failure(std::string message, SkinCatalogError code) {
    GRAPHICS_LOG_WARNING("skins.json : " + message);
    return SkinCatalogResult{
        .catalog = std::nullopt, .error = std::move(message), .errorCode = code};
}

}  // namespace

const char* skinModeName(SkinMode mode) noexcept {
    // switch exhaustif sans default : un mode ajoute sans nom casse la compilation.
    switch (mode) {
        case SkinMode::Single:
            return "single";
        case SkinMode::Bitmask16:
            return "bitmask16";
    }
    return "single";  // inatteignable : le switch couvre tout l'enum.
}

std::optional<SkinMode> skinModeFromName(std::string_view name) noexcept {
    if (name == "single") {
        return SkinMode::Single;
    }
    if (name == "bitmask16") {
        return SkinMode::Bitmask16;
    }
    return std::nullopt;
}

SkinCatalogResult SkinCatalog::loadFromString(std::string_view json) {
    return fromDocument(core::readJsonObject(json, FORMAT_VERSION, "skins.json", FIELD_VERSION));
}

SkinCatalogResult SkinCatalog::fromDocument(const core::JsonDocument& document) {
    if (!document.ok()) {
        return failure(document.message, mapError(document.error));
    }
    const nlohmann::json& root = document.root;

    SkinCatalog catalog;

    if (root.contains(FIELD_SETS)) {
        if (!root[FIELD_SETS].is_object()) {
            return failure("Le champ « jeux » n'est pas un objet.",
                           SkinCatalogError::MalformedStructure);
        }
        for (const auto& [setName, setJson] : root[FIELD_SETS].items()) {
            if (!setJson.is_object()) {
                return failure("Le jeu « " + setName + " » n'est pas un objet.",
                               SkinCatalogError::MalformedStructure);
            }
            catalog.addSet(setName);
            for (const auto& [typeName, entryJson] : setJson.items()) {
                const std::optional<core::TileType> type = core::parseTileType(typeName);
                if (!type.has_value()) {
                    return failure("Type de tuile inconnu « " + typeName + " » dans le jeu « " +
                                       setName + " ».",
                                   SkinCatalogError::MalformedStructure);
                }
                if (!entryJson.is_object() || !entryJson.contains(FIELD_ASSET) ||
                    !entryJson[FIELD_ASSET].is_string()) {
                    return failure("Entree « " + typeName + " » du jeu « " + setName +
                                       " » sans champ « asset » exploitable.",
                                   SkinCatalogError::MalformedStructure);
                }

                SkinEntry entry;
                entry.asset = entryJson[FIELD_ASSET].get<std::string>();
                // Mode absent : « single », le mode qui convient a tout type dont le voisinage n'a
                // pas de sens. Mode present mais inconnu : refus, jamais un mode devine.
                if (entryJson.contains(FIELD_MODE)) {
                    if (!entryJson[FIELD_MODE].is_string()) {
                        return failure("Le champ « mode » de « " + typeName + " » (jeu « " +
                                           setName + " ») n'est pas une chaine.",
                                       SkinCatalogError::MalformedStructure);
                    }
                    const std::string modeName = entryJson[FIELD_MODE].get<std::string>();
                    const std::optional<SkinMode> mode = skinModeFromName(modeName);
                    if (!mode.has_value()) {
                        return failure("Mode inconnu « " + modeName + " » pour « " + typeName +
                                           " » (jeu « " + setName + " »).",
                                       SkinCatalogError::MalformedStructure);
                    }
                    entry.mode = *mode;
                }
                catalog.assign(setName, *type, std::move(entry));
            }
        }
    }

    if (root.contains(FIELD_DEFAULT)) {
        if (!root[FIELD_DEFAULT].is_string()) {
            return failure("Le champ « defaut » n'est pas une chaine.",
                           SkinCatalogError::MalformedStructure);
        }
        const std::string defaultName = root[FIELD_DEFAULT].get<std::string>();
        if (!catalog.setDefaultSetName(defaultName)) {
            return failure("Le jeu par defaut « " + defaultName + " » n'existe pas.",
                           SkinCatalogError::MalformedStructure);
        }
    }

    return SkinCatalogResult{
        .catalog = std::move(catalog), .error = {}, .errorCode = SkinCatalogError::None};
}

SkinCatalogResult SkinCatalog::loadFromFile(const std::filesystem::path& path) {
    return fromDocument(core::readJsonObjectFromFile(path, FORMAT_VERSION, FIELD_VERSION));
}

std::string SkinCatalog::toJsonString() const {
    // nlohmann::ordered_json : preserve l'ordre d'insertion, donc version/defaut/jeux dans cet
    // ordre. Combine aux std::map internes, le fichier ecrit est reproductible a l'octet pres.
    nlohmann::ordered_json root;
    root[FIELD_VERSION] = FORMAT_VERSION;
    root[FIELD_DEFAULT] = _defaultSet;

    nlohmann::ordered_json sets = nlohmann::ordered_json::object();
    for (const auto& [setName, entries] : _sets) {
        nlohmann::ordered_json set = nlohmann::ordered_json::object();
        for (const auto& [type, entry] : entries) {
            nlohmann::ordered_json entryJson;
            entryJson[FIELD_ASSET] = entry.asset;
            entryJson[FIELD_MODE] = skinModeName(entry.mode);
            set[core::tileTypeName(type)] = std::move(entryJson);
        }
        sets[setName] = std::move(set);
    }
    root[FIELD_SETS] = std::move(sets);

    return root.dump(2) + "\n";
}

bool SkinCatalog::saveToFile(const std::filesystem::path& path) const {
    std::error_code error;
    if (path.has_parent_path()) {
        // create_directories sur un dossier existant n'est pas une erreur : le code d'erreur n'est
        // consulte que pour ne pas lever.
        std::filesystem::create_directories(path.parent_path(), error);
    }
    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    if (!file) {
        GRAPHICS_LOG_WARNING("skins.json : ecriture impossible : " + path.string());
        return false;
    }
    file << toJsonString();
    return file.good();
}

std::optional<SkinEntry> SkinCatalog::resolve(std::string_view setName, core::TileType type) const {
    auto set = _sets.find(setName);
    if (set == _sets.end()) {
        // Jeu inconnu (ou demande explicite du defaut) : repli sur le jeu par defaut plutot qu'un
        // niveau illisible.
        set = _sets.find(_defaultSet);
        if (set == _sets.end()) {
            return std::nullopt;
        }
    }
    const auto entry = set->second.find(type);
    if (entry == set->second.end()) {
        return std::nullopt;  // type non skinne : l'appelant affiche le damier de repli.
    }
    return entry->second;
}

bool SkinCatalog::setDefaultSetName(const std::string& setName) {
    if (!_sets.contains(setName)) {
        return false;
    }
    _defaultSet = setName;
    return true;
}

std::vector<std::string> SkinCatalog::setNames() const {
    std::vector<std::string> names;
    names.reserve(_sets.size());
    for (const auto& [setName, entries] : _sets) {
        (void)entries;
        names.push_back(setName);
    }
    return names;  // deja triees : _sets est un std::map.
}

void SkinCatalog::addSet(const std::string& setName) {
    _sets.try_emplace(setName);
    // Premier jeu cree : il devient le defaut, faute de quoi un catalogue construit par assignation
    // successive n'aurait aucun jeu par defaut et ne resoudrait rien.
    if (_defaultSet.empty()) {
        _defaultSet = setName;
    }
}

void SkinCatalog::assign(const std::string& setName, core::TileType type, SkinEntry entry) {
    addSet(setName);
    _sets[setName][type] = std::move(entry);
}

void SkinCatalog::clearAssignment(const std::string& setName, core::TileType type) {
    const auto set = _sets.find(setName);
    if (set != _sets.end()) {
        set->second.erase(type);
    }
}

std::map<core::TileType, SkinEntry> SkinCatalog::assignments(std::string_view setName) const {
    const auto set = _sets.find(setName);
    if (set == _sets.end()) {
        return {};
    }
    return set->second;
}

}  // namespace hmi
