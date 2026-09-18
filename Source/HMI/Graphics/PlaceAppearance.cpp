// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "HMI/Graphics/PlaceAppearance.h"

#include <algorithm>
#include <optional>
#include <set>
#include <utility>

#include "Core/Data/JsonDocument.h"
#include "Core/Levels/TileTypeName.h"

namespace hmi {

namespace {

constexpr std::string_view FIELD_PLACE = "place";
constexpr std::string_view FIELD_FLOORS = "floors";
constexpr std::string_view FIELD_RELIEF = "relief";

[[nodiscard]] PlaceAppearanceError mapError(core::JsonReadError error) {
    switch (error) {
        case core::JsonReadError::None:
            return PlaceAppearanceError::None;
        case core::JsonReadError::FileNotFound:
            return PlaceAppearanceError::FileNotFound;
        case core::JsonReadError::ParseError:
            return PlaceAppearanceError::ParseError;
        case core::JsonReadError::UnsupportedVersion:
            return PlaceAppearanceError::UnsupportedVersion;
        case core::JsonReadError::MalformedStructure:
            return PlaceAppearanceError::MalformedStructure;
    }
    return PlaceAppearanceError::MalformedStructure;
}

[[nodiscard]] PlaceAppearanceResult failure(std::string message, PlaceAppearanceError error) {
    return PlaceAppearanceResult{.appearance = {}, .error = error, .message = std::move(message)};
}

// Lit un objet « type de tuile -> liste de pieces ». Un type inconnu est une donnee fautive, pas
// un type a deviner : le signaler vaut mieux que dessiner du sable sous un mur.
[[nodiscard]] bool readTable(const nlohmann::json& root, std::string_view field,
                             std::map<core::TileType, std::vector<std::string>>& table,
                             std::string& error) {
    const auto found = root.find(field);
    if (found == root.end()) {
        return true;  // Une table sans sol, ou sans relief, est legitime.
    }
    if (!found->is_object()) {
        error = std::string{field} + " doit etre un objet";
        return false;
    }
    for (const auto& [name, pieces] : found->items()) {
        const std::optional<core::TileType> type = core::parseTileType(name);
        if (!type.has_value()) {
            error = std::string{field} + " : type de tuile inconnu « " + name + " »";
            return false;
        }
        if (!pieces.is_array() || pieces.empty()) {
            error = std::string{field} + " / " + name + " doit etre une liste non vide";
            return false;
        }
        std::vector<std::string> noms;
        for (const nlohmann::json& piece : pieces) {
            if (!piece.is_string() || piece.get<std::string>().empty()) {
                error = std::string{field} + " / " + name + " : piece vide";
                return false;
            }
            noms.push_back(piece.get<std::string>());
        }
        table.emplace(*type, std::move(noms));
    }
    return true;
}

// La variante d'une case : toujours la meme pour la meme case, et sans rapport avec l'ordre de
// parcours. Les deux facteurs sont premiers entre eux et avec les petits nombres de variantes, si
// bien que les voisines ne tombent pas toutes sur la meme.
[[nodiscard]] std::size_t variantOf(core::GridPosition cell, std::size_t count) {
    const long long melange =
        (static_cast<long long>(cell.column) * 7) + (static_cast<long long>(cell.row) * 13);
    const long long positif = melange < 0 ? -melange : melange;
    return count == 0 ? 0 : static_cast<std::size_t>(positif) % count;
}

[[nodiscard]] std::string_view pieceOf(
    const std::map<core::TileType, std::vector<std::string>>& table, core::TileType type,
    core::GridPosition cell) {
    const auto found = table.find(type);
    if (found == table.end() || found->second.empty()) {
        return {};
    }
    return found->second[variantOf(cell, found->second.size())];
}

}  // namespace

PlaceAppearanceResult PlaceAppearance::loadFromString(std::string_view json) {
    return fromDocument(core::readJsonObject(json, FORMAT_VERSION, "appearance.json"));
}

PlaceAppearanceResult PlaceAppearance::loadFromFile(const std::filesystem::path& path) {
    return fromDocument(core::readJsonObjectFromFile(path, FORMAT_VERSION));
}

PlaceAppearanceResult PlaceAppearance::fromDocument(const core::JsonDocument& document) {
    if (!document.ok()) {
        return failure(document.message, mapError(document.error));
    }
    PlaceAppearance appearance;
    const auto place = document.root.find(FIELD_PLACE);
    if (place == document.root.end() || !place->is_string()) {
        return failure("place manquant ou non textuel", PlaceAppearanceError::MalformedStructure);
    }
    appearance._place = place->get<std::string>();

    std::string error;
    if (!readTable(document.root, FIELD_FLOORS, appearance._floors, error) ||
        !readTable(document.root, FIELD_RELIEF, appearance._relief, error)) {
        return failure(std::move(error), PlaceAppearanceError::MalformedStructure);
    }
    return {
        .appearance = std::move(appearance), .error = PlaceAppearanceError::None, .message = {}};
}

std::string_view PlaceAppearance::floorPiece(core::TileType type, core::GridPosition cell) const {
    return pieceOf(_floors, type, cell);
}

std::string_view PlaceAppearance::reliefPiece(core::TileType type, core::GridPosition cell) const {
    return pieceOf(_relief, type, cell);
}

std::vector<std::string> PlaceAppearance::pieces() const {
    std::set<std::string> uniques;
    for (const auto* table : {&_floors, &_relief}) {
        for (const auto& [type, noms] : *table) {
            uniques.insert(noms.begin(), noms.end());
        }
    }
    return {uniques.begin(), uniques.end()};
}

}  // namespace hmi
