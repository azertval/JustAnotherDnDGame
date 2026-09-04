// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Audio/SoundCatalog.h"

#include <utility>

#include <nlohmann/json.hpp>

#include "Core/Data/JsonDocument.h"
#include "HMI/Audio/AudioLog.h"

namespace hmi {

namespace {

// Noms des champs du format. Nommes plutot que repetes en litteraux : le lecteur et l'ecrivain ne
// peuvent pas diverger sur une faute de frappe (meme discipline que hmi::SkinCatalog).
constexpr const char* FIELD_VERSION = "version";
constexpr const char* FIELD_SOUNDS = "sons";

// Traduction de la categorie partagee (LOT-79) vers celle, documentee, de ce catalogue. Les cinq
// categories coincident ; la fonction existe pour que l'ajout d'une categorie d'un cote fasse
// echouer la compilation plutot que de tomber dans un cas par defaut.
[[nodiscard]] SoundCatalogError mapError(core::JsonReadError code) {
    switch (code) {
        case core::JsonReadError::None:
            return SoundCatalogError::None;
        case core::JsonReadError::FileNotFound:
            return SoundCatalogError::FileNotFound;
        case core::JsonReadError::ParseError:
            return SoundCatalogError::ParseError;
        case core::JsonReadError::UnsupportedVersion:
            return SoundCatalogError::UnsupportedVersion;
        case core::JsonReadError::MalformedStructure:
            return SoundCatalogError::MalformedStructure;
    }
    return SoundCatalogError::ParseError;
}

// Construit un resultat d'echec, en journalisant la raison en un point unique (meme patron que
// hmi::SkinCatalog::failure).
[[nodiscard]] SoundCatalogResult failure(std::string message, SoundCatalogError code) {
    AUDIO_LOG_WARNING("sounds.json : " + message);
    return SoundCatalogResult{
        .catalog = std::nullopt, .error = std::move(message), .errorCode = code};
}

}  // namespace

SoundCatalogResult SoundCatalog::loadFromString(std::string_view json) {
    return fromDocument(core::readJsonObject(json, FORMAT_VERSION, "sounds.json", FIELD_VERSION));
}

SoundCatalogResult SoundCatalog::fromDocument(const core::JsonDocument& document) {
    if (!document.ok()) {
        return failure(document.message, mapError(document.error));
    }
    const nlohmann::json& root = document.root;

    SoundCatalog catalog;

    if (root.contains(FIELD_SOUNDS)) {
        if (!root[FIELD_SOUNDS].is_object()) {
            return failure("Le champ « sons » n'est pas un objet.",
                           SoundCatalogError::MalformedStructure);
        }
        for (const auto& [eventId, fileJson] : root[FIELD_SOUNDS].items()) {
            if (!fileJson.is_string() || fileJson.get<std::string>().empty()) {
                return failure("Evenement « " + eventId + " » sans fichier exploitable.",
                               SoundCatalogError::MalformedStructure);
            }
            catalog.assign(eventId, fileJson.get<std::string>());
        }
    }

    return SoundCatalogResult{
        .catalog = std::move(catalog), .error = {}, .errorCode = SoundCatalogError::None};
}

SoundCatalogResult SoundCatalog::loadFromFile(const std::filesystem::path& path) {
    return fromDocument(core::readJsonObjectFromFile(path, FORMAT_VERSION, FIELD_VERSION));
}

std::optional<std::string> SoundCatalog::resolve(std::string_view eventId) const {
    const auto entry = _sounds.find(eventId);
    if (entry == _sounds.end()) {
        return std::nullopt;
    }
    return entry->second;
}

std::vector<std::string> SoundCatalog::eventIds() const {
    std::vector<std::string> ids;
    ids.reserve(_sounds.size());
    for (const auto& entry : _sounds) {
        ids.push_back(entry.first);
    }
    return ids;  // deja triees : _sounds est un std::map.
}

void SoundCatalog::assign(const std::string& eventId, std::string file) {
    _sounds[eventId] = std::move(file);
}

}  // namespace hmi
