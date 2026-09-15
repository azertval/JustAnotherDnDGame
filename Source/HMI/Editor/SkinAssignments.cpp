// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "HMI/Editor/SkinAssignments.h"

#include <algorithm>
#include <array>
#include <optional>
#include <system_error>

#include "HMI/Editor/TileTaxonomy.h"

namespace hmi {

namespace {

// Extensions d'image acceptees pour un skin. Le decodage (hmi::decodeImageFile) gere davantage de
// formats, mais proposer un fichier qui ne sera pas lu serait pire que de ne pas le proposer.
constexpr std::array<const char*, 2> IMAGE_EXTENSIONS{".png", ".PNG"};

// Ajoute une ligne pour un type, en y reportant son assignation courante si elle existe.
void appendRow(std::vector<SkinRow>& rows, const SkinCatalog& catalog, std::string_view setName,
               const TileEntry& entry) {
    if (entry.type == core::TileType::Empty) {
        return;  // une case vide n'affiche rien : aucune apparence a lui assigner.
    }
    SkinRow row;
    row.type = entry.type;
    row.typeLabel = entry.label;
    if (const std::optional<SkinEntry> assigned = catalog.resolve(setName, entry.type)) {
        row.asset = assigned->asset;
        row.mode = assigned->mode;
    }
    rows.push_back(std::move(row));
}

// Nom du jeu effectivement consulte : le defaut du catalogue si aucun n'est demande.
[[nodiscard]] std::string effectiveSetName(const SkinCatalog& catalog, std::string_view setName) {
    return setName.empty() ? catalog.defaultSetName() : std::string{setName};
}

}  // namespace

std::vector<SkinSection> buildSkinRows(const SkinCatalog& catalog, std::string_view setName) {
    std::vector<SkinSection> sections;
    for (const TileCategory& category : tileTaxonomy()) {
        SkinSection section;
        section.label = category.label;
        for (const TileEntry& entry : category.tiles) {
            appendRow(section.rows, catalog, setName, entry);
        }
        // Sous-groupes aplatis dans leur categorie : le panneau n'a qu'une assignation par type,
        // un troisieme niveau de repli n'apporterait rien.
        for (const TileSubgroup& subgroup : category.subgroups) {
            for (const TileEntry& entry : subgroup.tiles) {
                appendRow(section.rows, catalog, setName, entry);
            }
        }
        if (!section.rows.empty()) {
            sections.push_back(std::move(section));
        }
    }
    return sections;
}

std::vector<std::string> listSkinAssets(const std::filesystem::path& skinsDirectory) {
    std::vector<std::string> assets;
    std::error_code error;
    if (!std::filesystem::is_directory(skinsDirectory, error)) {
        return assets;  // dossier absent : liste vide (meme robustesse que LevelFileOperations).
    }
    for (const std::filesystem::directory_entry& entry :
         std::filesystem::directory_iterator(skinsDirectory, error)) {
        if (!entry.is_regular_file(error)) {
            continue;
        }
        const std::string extension = entry.path().extension().string();
        if (std::ranges::find(IMAGE_EXTENSIONS, extension) == std::end(IMAGE_EXTENSIONS)) {
            continue;
        }
        assets.push_back(entry.path().filename().string());
    }
    std::ranges::sort(assets);
    return assets;
}

void applySkinAssignment(SkinCatalog& catalog, std::string_view setName, core::TileType type,
                         const std::string& asset, SkinMode mode) {
    const std::string set = effectiveSetName(catalog, setName);
    if (asset.empty()) {
        // « Aucun skin » retire l'assignation : associer un fichier de nom vide laisserait une
        // entree que la resolution tenterait de charger a chaque image.
        catalog.clearAssignment(set, type);
        return;
    }
    catalog.assign(set, type, SkinEntry{.asset = asset, .mode = mode});
}

}  // namespace hmi
