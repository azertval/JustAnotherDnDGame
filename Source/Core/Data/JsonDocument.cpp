// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Core/Data/JsonDocument.h"

#include <fstream>
#include <sstream>
#include <utility>

namespace core {

namespace {

/// @brief Construit un échec, en un point unique pour que tous se ressemblent.
[[nodiscard]] JsonDocument failure(JsonReadError code, std::string message,
                                   TextPosition position = {}) {
    return JsonDocument{.root = nlohmann::json::object(),
                        .error = code,
                        .message = std::move(message),
                        .position = position,
                        .version = 0};
}

/// @brief Préfixe « fichier:ligne:colonne : » d'un message, réduit à ce qui est connu.
[[nodiscard]] std::string prefix(std::string_view origin, TextPosition position) {
    std::string out;
    if (!origin.empty()) {
        out += std::string(origin);
    }
    if (position.line > 0) {
        if (!out.empty()) {
            out += ':';
        }
        out += std::to_string(position.line) + ':' + std::to_string(position.column);
    }
    if (!out.empty()) {
        out += " : ";
    }
    return out;
}

}  // namespace

TextPosition positionOf(std::string_view text, std::size_t byteOffset) {
    if (byteOffset == 0 || byteOffset > text.size()) {
        return {};
    }
    // `byte` de nlohmann est 1-indexé et désigne l'octet **fautif** : on compte ce qui précède.
    const std::size_t limit = byteOffset - 1;
    int line = 1;
    int column = 1;
    for (std::size_t i = 0; i < limit; ++i) {
        if (text[i] == '\n') {
            ++line;
            column = 1;
        } else {
            ++column;
        }
    }
    return {line, column};
}

JsonDocument readJsonObject(std::string_view json, int supportedVersion, std::string_view origin,
                            std::string_view versionField) {
    // `parse` en mode non-lançant renverrait `discarded` sans dire **où**. On laisse donc
    // l'exception se produire ici, à l'intérieur de la brique, pour en extraire la position — et
    // aucune ne franchit cette frontière (EX-NFR-040).
    nlohmann::json root;
    try {
        root = nlohmann::json::parse(json);
    } catch (const nlohmann::json::parse_error& e) {
        const TextPosition position = positionOf(json, e.byte);
        return failure(JsonReadError::ParseError,
                       prefix(origin, position) + "JSON malforme : " + e.what(), position);
    }

    if (!root.is_object()) {
        return failure(JsonReadError::ParseError,
                       prefix(origin, {}) + "La racine du document n'est pas un objet.");
    }

    // Version absente : vaut 1. Un catalogue écrit avant que le format ne se versionne reste
    // lisible, ce qui évite d'avoir à réécrire les fichiers existants au premier versionnage.
    int version = 1;
    const std::string field(versionField);
    if (root.contains(field)) {
        if (!root[field].is_number_integer()) {
            return failure(JsonReadError::MalformedStructure,
                           prefix(origin, {}) + "Le champ « " + field + " » n'est pas un entier.");
        }
        version = root[field].get<int>();
    }
    if (supportedVersion > 0 && version > supportedVersion) {
        return failure(JsonReadError::UnsupportedVersion,
                       prefix(origin, {}) + "Version de format " + std::to_string(version) +
                           " non geree (cette version du jeu lit jusqu'a " +
                           std::to_string(supportedVersion) + ").");
    }

    return JsonDocument{.root = std::move(root),
                        .error = JsonReadError::None,
                        .message = {},
                        .position = {},
                        .version = version};
}

JsonDocument readJsonObjectFromFile(const std::filesystem::path& path, int supportedVersion,
                                    std::string_view versionField) {
    std::ifstream file(path);
    if (!file) {
        return failure(JsonReadError::FileNotFound,
                       "Fichier introuvable ou illisible : " + path.string());
    }
    std::ostringstream contents;
    contents << file.rdbuf();
    const std::string text = contents.str();
    // Le nom du fichier entre dans les messages : « sounds.json:12:5 : … » se corrige, « JSON
    // malforme » ne se corrige pas.
    return readJsonObject(text, supportedVersion, path.filename().string(), versionField);
}

}  // namespace core
