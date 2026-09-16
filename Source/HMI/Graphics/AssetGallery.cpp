// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "HMI/Graphics/AssetGallery.h"

#include <algorithm>
#include <cmath>
#include <map>
#include <system_error>
#include <utility>

#include <nlohmann/json.hpp>

#include "Core/Data/JsonDocument.h"
#include "HMI/Graphics/AnimationCatalog.h"

namespace hmi {

namespace {

/// Version la plus élevée des manifestes lus ; absente, elle vaut 1 (`core::readJsonObject`).
constexpr int MANIFEST_VERSION = 1;

using json = nlohmann::json;

[[nodiscard]] std::string stemOf(const std::string& fileName) {
    const std::size_t dot = fileName.find('.');
    return dot == std::string::npos ? fileName : fileName.substr(0, dot);
}

[[nodiscard]] std::vector<std::string> stringList(const json& root, const char* key) {
    std::vector<std::string> values;
    const auto found = root.find(key);
    if (found == root.end() || !found->is_array()) {
        return values;
    }
    for (const json& value : *found) {
        if (value.is_string()) {
            values.push_back(value.get<std::string>());
        }
    }
    return values;
}

[[nodiscard]] std::pair<int, int> intPair(const json& object, const char* key, int fallbackX,
                                          int fallbackY) {
    const auto found = object.find(key);
    if (found == object.end() || !found->is_array() || found->size() != 2 ||
        !(*found)[0].is_number_integer() || !(*found)[1].is_number_integer()) {
        return {fallbackX, fallbackY};
    }
    return {(*found)[0].get<int>(), (*found)[1].get<int>()};
}

/**
 * @brief Une forme animée, d'après son `.anim.json` : le clip du nom du fichier s'il existe, le
 *        premier sinon.
 * @return Faux si le fichier est absent ; une erreur est ajoutée s'il est illisible.
 */
bool readAnimatedEntry(AssetGalleryEntry& entry, const std::filesystem::path& descriptor,
                       std::vector<std::string>& errors) {
    std::error_code ignored;
    if (!std::filesystem::is_regular_file(descriptor, ignored)) {
        return false;
    }
    const AnimationDescriptionResult result = AnimationCatalog::loadFromFile(descriptor);
    if (!result.ok() || result.description->clips.clipCount() == 0) {
        errors.push_back(descriptor.generic_string() + " : " +
                         (result.ok() ? std::string("aucun clip") : result.error));
        return false;
    }
    const AnimationDescription& description = *result.description;
    const int named = description.clips.indexOf(stemOf(descriptor.filename().string()));
    const core::AnimationClip& clip = description.clips.clipAt(named >= 0 ? named : 0);
    entry.frameWidth = description.frameWidth;
    entry.frameHeight = description.frameHeight;
    entry.frames = clip.frames;
    entry.frameDuration = clip.frameDuration;
    entry.loop = clip.endMode == core::ClipEndMode::Loop;
    return true;
}

/// Un manifeste lu ; absent : rien, sans erreur. Illisible : une erreur nommée.
[[nodiscard]] core::JsonDocument readManifest(const std::filesystem::path& path,
                                              std::vector<std::string>& errors) {
    core::JsonDocument document = core::readJsonObjectFromFile(path, MANIFEST_VERSION);
    if (!document.ok() && document.error != core::JsonReadError::FileNotFound) {
        errors.push_back(path.generic_string() + " : " + document.message);
    }
    return document;
}

void readNpcs(const std::filesystem::path& root, AssetGalleryCatalog& catalog) {
    const core::JsonDocument document =
        readManifest(root / "Npc" / "manifest.json", catalog.errors);
    if (!document.ok()) {
        return;
    }
    AssetGalleryFamily family{.title = "PNJ", .directory = "Npc", .entries = {}};
    // Tous les dossiers de PNJ, pas seulement la liste `npcs` : celle-ci ne nomme que les PNJ
    // retenus pour le jeu, et la galerie sert justement à voir les autres.
    std::vector<std::string> npcs;
    std::error_code error;
    for (const auto& item : std::filesystem::directory_iterator(root / "Npc", error)) {
        if (item.is_directory()) {
            npcs.push_back(item.path().filename().string());
        }
    }
    std::sort(npcs.begin(), npcs.end());
    const std::vector<std::string> animations = stringList(document.root, "animations");
    for (const std::string& npc : npcs) {
        for (const std::string& animation : animations) {
            AssetGalleryEntry entry{.family = family.title,
                                    .model = npc,
                                    .form = animation,
                                    .path = "Npc/" + npc + "/" + animation + ".png"};
            if (readAnimatedEntry(entry, root / "Npc" / npc / (animation + ".anim.json"),
                                  catalog.errors)) {
                family.entries.push_back(std::move(entry));
            }
        }
    }
    if (!family.entries.empty()) {
        catalog.families.push_back(std::move(family));
    }
}

void readColiseum(const std::filesystem::path& root, AssetGalleryCatalog& catalog) {
    const core::JsonDocument document =
        readManifest(root / "Coliseum" / "manifest.json", catalog.errors);
    if (!document.ok()) {
        return;
    }
    const auto figures = [&](const char* title, const char* key, const char* directory,
                             const std::vector<std::string>& animations) {
        AssetGalleryFamily family{.title = title, .directory = "Coliseum", .entries = {}};
        for (const std::string& name : stringList(document.root, key)) {
            for (const std::string& animation : animations) {
                const std::string folder = std::string(directory) + "/" + name + "/";
                AssetGalleryEntry entry{.family = family.title,
                                        .model = name,
                                        .form = animation,
                                        .path = "Coliseum/" + folder + animation + ".png"};
                if (readAnimatedEntry(
                        entry, root / "Coliseum" / directory / name / (animation + ".anim.json"),
                        catalog.errors)) {
                    family.entries.push_back(std::move(entry));
                }
            }
        }
        if (!family.entries.empty()) {
            catalog.families.push_back(std::move(family));
        }
    };
    const std::vector<std::string> animations = stringList(document.root, "animations");
    figures("Colisée · héros", "heroes", "characters", animations);
    figures("Colisée · gladiateurs", "gladiators", "enemies", animations);

    const auto files = document.root.find("files");
    if (files == document.root.end() || !files->is_object()) {
        return;
    }
    AssetGalleryFamily pieces{.title = "Colisée · pièces", .directory = "Coliseum", .entries = {}};
    for (const auto& [key, value] : files->items()) {
        const std::size_t slash = key.find('/');
        if (slash == std::string::npos || !value.is_object()) {
            continue;
        }
        const std::string folder = key.substr(0, slash);
        if (folder == "characters" || folder == "enemies") {
            continue;
        }
        const auto [width, height] = intPair(value, "size", 0, 0);
        pieces.entries.push_back(AssetGalleryEntry{.family = pieces.title,
                                                   .model = folder,
                                                   .form = stemOf(key.substr(slash + 1)),
                                                   .path = "Coliseum/" + key,
                                                   .frameWidth = width,
                                                   .frameHeight = height});
    }
    // `nlohmann::json` range ses clés par ordre alphabétique : les dossiers se suivent déjà.
    if (!pieces.entries.empty()) {
        catalog.families.push_back(std::move(pieces));
    }
}

[[nodiscard]] int classRank(const std::string& textureClass) {
    if (textureClass == "floor") {
        return 0;
    }
    if (textureClass == "tall") {
        return 1;
    }
    return textureClass == "wide" ? 2 : 3;
}

void readScenes(const std::filesystem::path& root, AssetGalleryCatalog& catalog) {
    std::vector<std::filesystem::path> dispositions;
    std::error_code error;
    for (const auto& item : std::filesystem::directory_iterator(root / "Scene", error)) {
        if (item.is_directory()) {
            dispositions.push_back(item.path());
        }
    }
    std::sort(dispositions.begin(), dispositions.end());
    for (const std::filesystem::path& directory : dispositions) {
        const core::JsonDocument document =
            readManifest(directory / "manifest.json", catalog.errors);
        if (!document.ok()) {
            continue;
        }
        const auto textures = document.root.find("textures");
        if (textures == document.root.end() || !textures->is_object()) {
            continue;
        }
        const std::string name = directory.filename().string();
        AssetGalleryFamily family{
            .title = "Scène · " + name, .directory = "Scene/" + name, .entries = {}};
        for (const auto& [key, value] : textures->items()) {
            if (!value.is_object() || !value.contains("file") || !value["file"].is_string()) {
                continue;
            }
            const auto [width, height] = intPair(value, "size", 0, 0);
            const auto [columns, rows] = intPair(value, "footprint", 1, 1);
            const auto [anchorX, anchorY] = intPair(value, "anchor", -1, -1);
            const std::size_t slash = key.rfind('/');
            family.entries.push_back(
                AssetGalleryEntry{.family = family.title,
                                  .model = value.value("class", std::string("autre")),
                                  .form = slash == std::string::npos ? key : key.substr(slash + 1),
                                  .path = family.directory + "/" + value["file"].get<std::string>(),
                                  .frameWidth = width,
                                  .frameHeight = height,
                                  .footprintColumns = std::max(1, columns),
                                  .footprintRows = std::max(1, rows),
                                  .anchorX = anchorX,
                                  .anchorY = anchorY});
        }
        std::stable_sort(family.entries.begin(), family.entries.end(),
                         [](const AssetGalleryEntry& left, const AssetGalleryEntry& right) {
                             return classRank(left.model) < classRank(right.model);
                         });
        if (!family.entries.empty()) {
            catalog.families.push_back(std::move(family));
        }
    }
}

void readSkins(const std::filesystem::path& root, AssetGalleryCatalog& catalog) {
    std::vector<std::filesystem::path> descriptors;
    std::error_code error;
    for (const auto& item : std::filesystem::directory_iterator(root / "Skins", error)) {
        const std::string fileName = item.path().filename().string();
        if (item.is_regular_file() && fileName.ends_with(".anim.json")) {
            descriptors.push_back(item.path());
        }
    }
    std::sort(descriptors.begin(), descriptors.end());
    AssetGalleryFamily family{.title = "Skins", .directory = "Skins", .entries = {}};
    for (const std::filesystem::path& descriptor : descriptors) {
        const std::string stem = stemOf(descriptor.filename().string());
        std::error_code ignored;
        if (!std::filesystem::is_regular_file(descriptor.parent_path() / (stem + ".png"),
                                              ignored)) {
            continue;
        }
        AssetGalleryEntry entry{.family = family.title,
                                .model = "skins",
                                .form = stem,
                                .path = "Skins/" + stem + ".png"};
        if (readAnimatedEntry(entry, descriptor, catalog.errors)) {
            family.entries.push_back(std::move(entry));
        }
    }
    if (!family.entries.empty()) {
        catalog.families.push_back(std::move(family));
    }
}

[[nodiscard]] int ceilCells(int pixels) {
    return pixels <= 0 ? 0 : (pixels + ASSET_GALLERY_CELL_PIXELS - 1) / ASSET_GALLERY_CELL_PIXELS;
}

}  // namespace

AssetGalleryCatalog AssetGalleryCatalog::load(const std::filesystem::path& assetsRoot) {
    AssetGalleryCatalog catalog;
    readNpcs(assetsRoot, catalog);
    readColiseum(assetsRoot, catalog);
    readScenes(assetsRoot, catalog);
    readSkins(assetsRoot, catalog);
    return catalog;
}

std::size_t AssetGalleryCatalog::entryCount() const noexcept {
    std::size_t count = 0;
    for (const AssetGalleryFamily& family : families) {
        count += family.entries.size();
    }
    return count;
}

AssetGalleryBloc assetGalleryBlocShape(const AssetGalleryEntry& entry) {
    const int footprintColumns = std::max(1, entry.footprintColumns);
    const int footprintRows = std::max(1, entry.footprintRows);
    // Le dessin monte au-dessus du bas de l'emprise ; en largeur, il est centré sur elle.
    const int inner = std::max(footprintColumns, ceilCells(entry.frameWidth));
    const int above = std::max(footprintRows, ceilCells(entry.frameHeight));
    AssetGalleryBloc bloc;
    bloc.columns = inner + 2;
    bloc.rows = above + 2;
    bloc.footprintColumn = (bloc.columns - footprintColumns) / 2;
    bloc.footprintRow = bloc.rows - 1 - footprintRows;
    return bloc;
}

AssetGalleryLayout layoutAssetGallery(const AssetGalleryCatalog& catalog, int maximumColumns) {
    AssetGalleryLayout layout;
    int row = 0;

    // Une ligne visuelle : ses blocs sont posés sur le même bas, une fois sa hauteur connue.
    std::vector<AssetGalleryBloc> line;
    int lineColumns = 0;
    int lineRows = 0;
    const auto flush = [&] {
        for (AssetGalleryBloc& bloc : line) {
            bloc.row = row + lineRows - bloc.rows;
            layout.blocs.push_back(bloc);
        }
        layout.columns = std::max(layout.columns, lineColumns);
        row += lineRows;
        line.clear();
        lineColumns = 0;
        lineRows = 0;
    };

    for (int familyIndex = 0; familyIndex < static_cast<int>(catalog.families.size());
         ++familyIndex) {
        const AssetGalleryFamily& family = catalog.families[static_cast<std::size_t>(familyIndex)];
        layout.bands.push_back(AssetGalleryBand{.family = familyIndex, .row = row});
        ++row;

        // Les modèles dans l'ordre de leur première forme.
        std::vector<std::string> models;
        std::map<std::string, std::vector<int>> byModel;
        for (int index = 0; index < static_cast<int>(family.entries.size()); ++index) {
            const std::string& model = family.entries[static_cast<std::size_t>(index)].model;
            if (!byModel.contains(model)) {
                models.push_back(model);
            }
            byModel[model].push_back(index);
        }
        for (const std::string& model : models) {
            for (const int index : byModel[model]) {
                AssetGalleryBloc bloc =
                    assetGalleryBlocShape(family.entries[static_cast<std::size_t>(index)]);
                if (lineColumns > 0 && lineColumns + bloc.columns > maximumColumns) {
                    flush();
                }
                bloc.family = familyIndex;
                bloc.entry = index;
                bloc.column = lineColumns;
                lineColumns += bloc.columns;
                lineRows = std::max(lineRows, bloc.rows);
                line.push_back(bloc);
            }
            flush();
        }
    }
    layout.rows = row;
    return layout;
}

AssetGalleryVisibility assetGalleryVisibility(const AssetGalleryBloc& bloc,
                                              const AssetGalleryView& view,
                                              double ringCells) noexcept {
    const auto overlaps = [&](double margin) {
        return bloc.column < view.column + view.columns + margin &&
               bloc.column + bloc.columns > view.column - margin &&
               bloc.row < view.row + view.rows + margin && bloc.row + bloc.rows > view.row - margin;
    };
    if (overlaps(0.0)) {
        return AssetGalleryVisibility::Drawn;
    }
    return overlaps(ringCells) ? AssetGalleryVisibility::Preloaded
                               : AssetGalleryVisibility::Unloaded;
}

int assetGalleryFrameRank(const AssetGalleryEntry& entry, double seconds) noexcept {
    const int count = entry.frameCount();
    if (count <= 1 || entry.frameDuration <= 0.0 || seconds <= 0.0) {
        return 0;
    }
    const auto step = static_cast<long long>(std::floor(seconds / entry.frameDuration));
    if (entry.loop) {
        return static_cast<int>(step % count);
    }
    const auto hold = static_cast<long long>(
        std::ceil(ASSET_GALLERY_ONE_SHOT_HOLD_SECONDS / entry.frameDuration));
    return static_cast<int>(std::min<long long>(step % (count + hold), count - 1));
}

}  // namespace hmi
