// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "HMI/Editor/AssetLibrary.h"

#include <algorithm>
#include <cctype>
#include <system_error>

namespace hmi {

namespace {

// Minuscules ASCII, pour une comparaison insensible a la casse (extension et filtre).
[[nodiscard]] std::string toLower(const std::string& text) {
    std::string lowered = text;
    std::ranges::transform(lowered, lowered.begin(),
                           [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return lowered;
}

// Seule extension acceptee a ce jour : le decodage (hmi::decodeImageFile) gere davantage de
// formats, mais lister un fichier qui ne sera pas lu serait pire que de ne pas le lister.
[[nodiscard]] bool hasImageExtension(const std::filesystem::path& path) {
    return toLower(path.extension().string()) == ".png";
}

}  // namespace

std::vector<std::string> listAssetFiles(const std::filesystem::path& directory,
                                        const std::string& filterText) {
    std::vector<std::string> assets;
    std::error_code error;
    if (!std::filesystem::is_directory(directory, error)) {
        return assets;  // dossier absent : liste vide, etat de depart legitime.
    }

    const std::string needle = toLower(filterText);
    for (const std::filesystem::directory_entry& entry :
         std::filesystem::directory_iterator(directory, error)) {
        if (!entry.is_regular_file(error) || !hasImageExtension(entry.path())) {
            continue;
        }
        std::string name = entry.path().filename().string();
        if (!needle.empty() && toLower(name).find(needle) == std::string::npos) {
            continue;
        }
        assets.push_back(std::move(name));
    }
    std::ranges::sort(assets);
    return assets;
}

}  // namespace hmi
