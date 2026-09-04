// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Core/Levels/LevelSequence.h"

#include <fstream>
#include <sstream>
#include <utility>

#include <nlohmann/json.hpp>

#include "Core/Data/JsonDocument.h"
#include "Core/Levels/LevelsLog.h"

namespace core {

namespace {

// Construit un résultat d'échec avec un message et un code catégorisé -- même patron que
// LevelLoader.cpp::failure (point unique de journalisation).
[[nodiscard]] LevelSequenceLoadResult failure(std::string message, LevelSequenceError code) {
    LEVELS_LOG_WARNING("Echec du chargement de la sequence : " + message);
    return LevelSequenceLoadResult{
        .sequence = std::nullopt, .error = std::move(message), .errorCode = code};
}

}  // namespace

LevelSequenceLoadResult LevelSequenceLoader::loadFromString(std::string_view json) {
    // Enveloppe commune (JSON bien forme, racine objet, garde de version) : brique partagee du
    // LOT-79 (EX-CNT-012). Elle situe en outre l'erreur de syntaxe a la ligne, ce que ce lecteur
    // ne savait pas faire.
    const JsonDocument document =
        readJsonObject(json, LEVEL_SEQUENCE_FORMAT_VERSION, "sequence de niveaux");
    if (!document.ok()) {
        return failure(document.message, document.error == JsonReadError::UnsupportedVersion
                                             ? LevelSequenceError::UnsupportedFormatVersion
                                             : LevelSequenceError::ParseError);
    }
    const nlohmann::json& root = document.root;
    try {
        if (!root.contains("levels")) {
            return failure("Champ obligatoire manquant ('levels')", LevelSequenceError::ParseError);
        }
        if (!root.at("levels").is_array()) {
            return failure("Le champ 'levels' doit etre une liste", LevelSequenceError::ParseError);
        }

        LevelSequence result;
        result.titleKey = root.value("titleKey", std::string{});
        for (const nlohmann::json& entry : root.at("levels")) {
            result.levels.push_back(entry.get<std::string>());
        }
        if (result.levels.empty()) {
            return failure("Sequence sans aucun niveau", LevelSequenceError::EmptySequence);
        }

        return LevelSequenceLoadResult{.sequence = std::move(result), .error = {}};
    } catch (const nlohmann::json::exception& error) {
        return failure(std::string("JSON invalide : ") + error.what(),
                       LevelSequenceError::ParseError);
    }
}

LevelSequenceLoadResult LevelSequenceLoader::loadFromFile(const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        return failure("Fichier de sequence introuvable : " + path.string(),
                       LevelSequenceError::FileNotFound);
    }
    std::ostringstream buffer;
    buffer << file.rdbuf();
    LevelSequenceLoadResult result = loadFromString(buffer.str());
    if (!result.ok()) {
        return result;
    }

    // Chaque niveau référencé doit exister, à côté du fichier de séquence lui-même (c'est là que
    // vivent les niveaux, Source/Elements/Levels) -- une entrée fautive est nommée, pas un
    // plantage différé au premier chargement de niveau (EX-NFR-040).
    const std::filesystem::path directory = path.parent_path();
    for (const std::string& levelName : result.sequence->levels) {
        if (!std::filesystem::exists(directory / levelName)) {
            return failure("Niveau reference introuvable : " + levelName,
                           LevelSequenceError::MissingLevelFile);
        }
    }
    return result;
}

}  // namespace core
