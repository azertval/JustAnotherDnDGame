// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <filesystem>
#include <string>
#include <utility>

/**
 * @file HMI/Editor/FileOperationResult.h
 * @brief Résultat d'une opération de fichier de l'éditeur, **partagé** par les niveaux
 * (`LevelFileOperations`) et les assets (`AssetFileOperations`).
 *
 * En-tête dédié, et non déclaré dans l'un des deux modules : le type appartient aux deux, et le
 * loger chez l'un obligeait l'autre à l'inclure entièrement pour un simple type de retour.
 */

namespace hmi {

/**
 * @brief Succès (avec le chemin résultant) ou échec (avec un message lisible).
 *
 * Ne lève jamais d'exception (`EX-NFR-040`) : l'échec est une valeur, pas un incident. Expose
 * `ok()` en **méthode**, comme `core::LevelLoadResult`, `core::LevelSequenceLoadResult`,
 * `hmi::SoundCatalogResult` — un lecteur qui écrit `result.ok()` ne doit
 * pas avoir à se souvenir duquel des cinq types il parle.
 */
struct FileOperationResult {
    bool succeeded = false;
    std::string error;
    std::filesystem::path path;  ///< Chemin créé/renommé/dupliqué en cas de succès.

    /// @return `true` si l'opération a abouti ; `error` est alors vide et `path` renseigné.
    [[nodiscard]] bool ok() const noexcept {
        return succeeded;
    }

    [[nodiscard]] static FileOperationResult success(std::filesystem::path resultPath) {
        return {true, {}, std::move(resultPath)};
    }
    [[nodiscard]] static FileOperationResult failure(std::string message) {
        return {false, std::move(message), {}};
    }
};

}  // namespace hmi
