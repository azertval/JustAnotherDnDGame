// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Graphics/ArenaAppearanceCatalog.h"

#include <utility>

#include <nlohmann/json.hpp>

#include "Core/Data/JsonDocument.h"
#include "HMI/Graphics/GraphicsLog.h"

namespace hmi {

namespace {

constexpr const char* FIELD_HEROES = "heroes";
constexpr const char* FIELD_GLADIATORS = "gladiators";
constexpr const char* FIELD_PALE_SLABS = "paleSlabs";
constexpr const char* FIELD_HERO_FRAMES = "heroFrames";
constexpr const char* FIELD_ENEMY_FRAMES = "enemyFrames";

// Meme patron que hmi::SkinCatalog : une traduction exhaustive, sans default, pour qu'une
// categorie ajoutee d'un cote fasse echouer la compilation de l'autre plutot que de se perdre.
[[nodiscard]] ArenaAppearanceError mapError(core::JsonReadError code) {
    switch (code) {
        case core::JsonReadError::None:
            return ArenaAppearanceError::None;
        case core::JsonReadError::FileNotFound:
            return ArenaAppearanceError::FileNotFound;
        case core::JsonReadError::ParseError:
            return ArenaAppearanceError::ParseError;
        case core::JsonReadError::UnsupportedVersion:
            return ArenaAppearanceError::UnsupportedVersion;
        case core::JsonReadError::MalformedStructure:
            return ArenaAppearanceError::MalformedStructure;
    }
    return ArenaAppearanceError::ParseError;
}

[[nodiscard]] ArenaAppearanceCatalogResult failure(std::string message, ArenaAppearanceError code) {
    GRAPHICS_LOG_WARNING("arena_appearance : " + message);
    return ArenaAppearanceCatalogResult{
        .catalog = std::nullopt, .error = std::move(message), .errorCode = code};
}

/// Lit un tableau de chaines obligatoire et non vide : une liste de figurines vide laisserait
/// `figureFor` sans rien a choisir, ce que le format refuse plutot que de le decouvrir au rendu.
[[nodiscard]] bool readStringArray(const nlohmann::json& root, const char* field,
                                   std::vector<std::string>& out, std::string& error) {
    if (!root.contains(field) || !root[field].is_array() || root[field].empty()) {
        error = std::string("Le champ « ") + field + " » est absent ou n'est pas un tableau non vide.";
        return false;
    }
    for (const nlohmann::json& entry : root[field]) {
        if (!entry.is_string()) {
            error = std::string("Une entree du champ « ") + field + " » n'est pas une chaine.";
            return false;
        }
        out.push_back(entry.get<std::string>());
    }
    return true;
}

[[nodiscard]] bool readPositiveInt(const nlohmann::json& root, const char* field, int& out,
                                   std::string& error) {
    if (!root.contains(field) || !root[field].is_number_integer() || root[field].get<int>() <= 0) {
        error = std::string("Le champ « ") + field + " » est absent ou n'est pas un entier positif.";
        return false;
    }
    out = root[field].get<int>();
    return true;
}

}  // namespace

ArenaAppearanceCatalogResult ArenaAppearanceCatalog::loadFromString(std::string_view json) {
    return fromDocument(core::readJsonObject(json, FORMAT_VERSION, "arena_appearance"));
}

ArenaAppearanceCatalogResult ArenaAppearanceCatalog::fromDocument(const core::JsonDocument& document) {
    if (!document.ok()) {
        return failure(document.message, mapError(document.error));
    }
    const nlohmann::json& root = document.root;

    ArenaAppearanceCatalog catalog;
    std::string error;

    if (!readStringArray(root, FIELD_HEROES, catalog._heroes, error) ||
        !readStringArray(root, FIELD_GLADIATORS, catalog._gladiators, error) ||
        !readStringArray(root, FIELD_PALE_SLABS, catalog._paleSlabs, error)) {
        return failure(error, ArenaAppearanceError::MalformedStructure);
    }
    if (!readPositiveInt(root, FIELD_HERO_FRAMES, catalog._heroFrames, error) ||
        !readPositiveInt(root, FIELD_ENEMY_FRAMES, catalog._enemyFrames, error)) {
        return failure(error, ArenaAppearanceError::MalformedStructure);
    }

    return ArenaAppearanceCatalogResult{
        .catalog = std::move(catalog), .error = {}, .errorCode = ArenaAppearanceError::None};
}

ArenaAppearanceCatalogResult ArenaAppearanceCatalog::loadFromFile(const std::filesystem::path& path) {
    return fromDocument(core::readJsonObjectFromFile(path, FORMAT_VERSION));
}

ArenaTileAppearance ArenaAppearanceCatalog::tileAppearance(core::GridPosition cell, int columns,
                                                            int rows, bool wall) const {
    // Transcription de ArenaTile.ui.qml:57-84 (LOT-50) : les booleens y sont independants, portes
    // ici comme un role unique pour un mur (mutuellement exclusifs par construction : bannerSpot
    // exige d'etre sur un bord haut/bas, torchSpot sur un bord gauche/droit).
    ArenaTileAppearance appearance;
    appearance.wall = wall;

    const bool corner =
        (cell.column == 0 || cell.column == columns - 1) && (cell.row == 0 || cell.row == rows - 1);
    const bool topOrBottom = cell.row == 0 || cell.row == rows - 1;

    if (wall) {
        if (corner) {
            appearance.wallFeature = WallFeature::Corner;
        } else if (topOrBottom && cell.column % 5 == 0) {
            appearance.wallFeature = WallFeature::BannerSpot;
        } else if (!topOrBottom && cell.row % 4 == 2) {
            appearance.wallFeature = WallFeature::TorchSpot;
        } else {
            appearance.wallFeature = WallFeature::Plain;
        }
        return appearance;
    }

    appearance.gateSpot =
        cell.column == 0 || cell.column == columns - 1 || cell.row == 0 || cell.row == rows - 1;

    if (!_paleSlabs.empty() &&
        (cell.column * 3 + cell.row * 5 + cell.column * cell.row) % 7 == 0) {
        appearance.slab = true;
        appearance.slabVariant =
            (cell.column * 3 + cell.row * 5) % static_cast<int>(_paleSlabs.size());
    }

    return appearance;
}

FigureAppearance ArenaAppearanceCatalog::figureFor(std::string_view name,
                                                    core::CombatSide side) const {
    const std::vector<std::string>& roster =
        side == core::CombatSide::Allies ? _heroes : _gladiators;
    const int frames = side == core::CombatSide::Allies ? _heroFrames : _enemyFrames;
    if (roster.empty()) {
        return {};
    }
    // Transcription de ArenaTile.ui.qml:73-74 : le nom choisit la figurine, pour qu'elle reste la
    // meme d'un tour a l'autre sans etat a tenir. `roster.size()` remplace le `4` litteral du QML :
    // le nombre de figurines n'est plus une constante dupliquee.
    const int index = name.empty()
                          ? 0
                          : (static_cast<int>(name.size()) * 7 +
                             static_cast<unsigned char>(name.front())) %
                                static_cast<int>(roster.size());
    return {roster[static_cast<std::size_t>(index)], frames};
}

}  // namespace hmi
