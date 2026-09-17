// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "HMI/Graphics/AssetPaths.h"

#include <system_error>

namespace hmi {

// Construit un résolveur pour un dossier d'assets donné.
AssetPaths::AssetPaths(std::filesystem::path directory) : _directory(std::move(directory)) {}

// Résout un nom de fichier logique vers un chemin existant dans le dossier d'assets.
std::optional<std::filesystem::path> AssetPaths::resolve(const std::string& fileName) const {
    std::filesystem::path candidate = _directory / fileName;
    std::error_code error;
    if (!std::filesystem::is_regular_file(candidate, error) || error) {
        return std::nullopt;
    }
    return candidate;
}

}  // namespace hmi
