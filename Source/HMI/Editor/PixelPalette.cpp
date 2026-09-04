// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Editor/PixelPalette.h"

#include <array>
#include <cstdio>
#include <fstream>
#include <optional>
#include <sstream>
#include <system_error>
#include <unordered_map>
#include <utility>

#include <nlohmann/json.hpp>

#include "Core/Data/JsonDocument.h"
#include "HMI/HmiLog.h"

namespace hmi {

namespace {

constexpr const char* FIELD_VERSION = "version";
constexpr const char* FIELD_COLORS = "colors";
constexpr const char* FIELD_NAME = "name";
constexpr const char* FIELD_COLOR = "color";

// Couleur R8G8B8A8_UNORM (ordre memoire R,G,B,A, LOT-54 TACHE-01) -> chaine hexadecimale
// "#rrggbbaa", meme convention que hmi::formatColorHex (EditorStatus.cpp) : trop petite pour
// justifier une fonction partagee entre deux fichiers independants.
std::string formatColorHex(std::uint32_t color) {
    std::array<char, 10> buffer{};
    std::snprintf(buffer.data(), buffer.size(), "#%02x%02x%02x%02x",
                  static_cast<unsigned>(color & 0xFFU), static_cast<unsigned>((color >> 8) & 0xFFU),
                  static_cast<unsigned>((color >> 16) & 0xFFU),
                  static_cast<unsigned>((color >> 24) & 0xFFU));
    return std::string(buffer.data());
}

// Chiffre hexadecimal -> valeur, sans lever (contrairement a std::stoul) : aucune exception ne
// doit franchir la lecture d'un fichier utilisateur (EX-NFR-040).
std::optional<int> hexNibble(char c) noexcept {
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    }
    if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    }
    return std::nullopt;
}

std::optional<std::uint8_t> hexByte(char high, char low) noexcept {
    const std::optional<int> h = hexNibble(high);
    const std::optional<int> l = hexNibble(low);
    if (!h || !l) {
        return std::nullopt;
    }
    return static_cast<std::uint8_t>((*h << 4) | *l);
}

// "#rrggbb" (alpha implicite 255) ou "#rrggbbaa" -> couleur R8G8B8A8_UNORM.
std::optional<std::uint32_t> parseColorHex(const std::string& text) {
    if (text.size() != 7 && text.size() != 9) {
        return std::nullopt;
    }
    if (text[0] != '#') {
        return std::nullopt;
    }
    const std::optional<std::uint8_t> r = hexByte(text[1], text[2]);
    const std::optional<std::uint8_t> g = hexByte(text[3], text[4]);
    const std::optional<std::uint8_t> b = hexByte(text[5], text[6]);
    if (!r || !g || !b) {
        return std::nullopt;
    }
    std::uint8_t a = 0xFFU;
    if (text.size() == 9) {
        const std::optional<std::uint8_t> parsedAlpha = hexByte(text[7], text[8]);
        if (!parsedAlpha) {
            return std::nullopt;
        }
        a = *parsedAlpha;
    }
    return static_cast<std::uint32_t>(*r) | (static_cast<std::uint32_t>(*g) << 8) |
           (static_cast<std::uint32_t>(*b) << 16) | (static_cast<std::uint32_t>(a) << 24);
}

// Une entree individuelle de `colors` -> entree de palette, ou nullopt si malformee (forme
// inattendue, ou couleur hexadecimale invalide) -- journalise la raison, ne leve jamais (EX-NFR-
// 040 : une entree fautive n'invalide pas le reste de la palette).
std::optional<PixelPaletteEntry> parseColorEntry(const nlohmann::json& entryJson) {
    if (!entryJson.is_object() || !entryJson.contains(FIELD_NAME) ||
        !entryJson.contains(FIELD_COLOR) || !entryJson[FIELD_NAME].is_string() ||
        !entryJson[FIELD_COLOR].is_string()) {
        HMI_LOG_WARNING("palettes.json : entree malformee, ignoree.");
        return std::nullopt;
    }
    const std::optional<std::uint32_t> color =
        parseColorHex(entryJson[FIELD_COLOR].get<std::string>());
    if (!color) {
        HMI_LOG_WARNING("palettes.json : couleur invalide, entree ignoree.");
        return std::nullopt;
    }
    return PixelPaletteEntry{.name = entryJson[FIELD_NAME].get<std::string>(), .color = *color};
}

}  // namespace

PixelPalette PixelPalette::loadFromString(std::string_view json) {
    PixelPalette palette;
    // Enveloppe commune : brique partagee du LOT-79 (EX-CNT-012). Ce lecteur n'a pas de champ de
    // version, d'ou la garde desactivee (0).
    const core::JsonDocument document = core::readJsonObject(json, 0, "palettes.json");
    if (!document.ok()) {
        HMI_LOG_WARNING(document.message + " Palette vide.");
        return palette;
    }
    const nlohmann::json& root = document.root;
    if (!root.contains(FIELD_COLORS) || !root[FIELD_COLORS].is_array()) {
        HMI_LOG_WARNING("palettes.json : structure inattendue, palette vide.");
        return palette;
    }

    for (const nlohmann::json& entryJson : root[FIELD_COLORS]) {
        std::optional<PixelPaletteEntry> entry = parseColorEntry(entryJson);
        if (entry) {
            palette._entries.push_back(std::move(*entry));
        }
    }
    return palette;
}

PixelPalette PixelPalette::loadFromFile(const std::filesystem::path& path) {
    std::ifstream file(path);
    if (!file) {
        // Absent ou illisible : etat de depart legitime (palette vide), pas une erreur -- a la
        // difference de skins.json (LOT-43), un projet neuf n'a simplement encore rien defini.
        return PixelPalette{};
    }
    std::ostringstream contents;
    contents << file.rdbuf();
    return loadFromString(contents.str());
}

std::string PixelPalette::toJsonString() const {
    nlohmann::ordered_json root;
    root[FIELD_VERSION] = FORMAT_VERSION;
    nlohmann::ordered_json colors = nlohmann::ordered_json::array();
    for (const PixelPaletteEntry& entry : _entries) {
        nlohmann::ordered_json entryJson;
        entryJson[FIELD_NAME] = entry.name;
        entryJson[FIELD_COLOR] = formatColorHex(entry.color);
        colors.push_back(std::move(entryJson));
    }
    root[FIELD_COLORS] = std::move(colors);
    return root.dump(2) + "\n";
}

bool PixelPalette::saveToFile(const std::filesystem::path& path) const {
    std::error_code error;
    if (path.has_parent_path()) {
        std::filesystem::create_directories(path.parent_path(), error);
    }
    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    if (!file) {
        HMI_LOG_WARNING("palettes.json : ecriture impossible : " + path.string());
        return false;
    }
    file << toJsonString();
    return file.good();
}

void PixelPalette::add(std::string name, std::uint32_t color) {
    _entries.push_back(PixelPaletteEntry{.name = std::move(name), .color = color});
}

bool PixelPalette::removeAt(std::size_t index) {
    if (index >= _entries.size()) {
        return false;
    }
    _entries.erase(_entries.begin() + static_cast<std::ptrdiff_t>(index));
    return true;
}

bool PixelPalette::renameAt(std::size_t index, std::string newName) {
    if (index >= _entries.size()) {
        return false;
    }
    _entries[index].name = std::move(newName);
    return true;
}

bool PixelPalette::moveEntry(std::size_t from, std::size_t to) {
    if (from >= _entries.size() || to >= _entries.size()) {
        return false;
    }
    if (from == to) {
        return true;
    }
    PixelPaletteEntry entry = _entries[from];
    _entries.erase(_entries.begin() + static_cast<std::ptrdiff_t>(from));
    _entries.insert(_entries.begin() + static_cast<std::ptrdiff_t>(to), std::move(entry));
    return true;
}

std::vector<PixelPaletteExtractionEntry> extractPalette(const DecodedImage& image) {
    std::vector<PixelPaletteExtractionEntry> result;
    std::unordered_map<std::uint32_t, std::size_t> indexOf;
    for (const std::uint32_t pixel : image.pixels) {
        if ((pixel >> 24) == 0) {
            continue;  // alpha nul : la gomme, jamais une couleur (meme regle que la contrainte).
        }
        const auto found = indexOf.find(pixel);
        if (found == indexOf.end()) {
            indexOf.emplace(pixel, result.size());
            result.push_back(PixelPaletteExtractionEntry{.color = pixel, .count = 1});
        } else {
            ++result[found->second].count;
        }
    }
    return result;
}

std::uint32_t nearestPaletteColor(std::uint32_t color,
                                  const std::vector<std::uint32_t>& palette) noexcept {
    const std::uint32_t alpha = color & 0xFF000000U;
    if (alpha == 0 || palette.empty()) {
        return color;  // gomme, ou palette vide : couleur inchangee.
    }

    const int r = static_cast<int>(color & 0xFFU);
    const int g = static_cast<int>((color >> 8) & 0xFFU);
    const int b = static_cast<int>((color >> 16) & 0xFFU);

    std::uint32_t nearestRgb = palette.front() & 0x00FFFFFFU;
    long long bestDistance = -1;
    for (const std::uint32_t entry : palette) {
        const int entryR = static_cast<int>(entry & 0xFFU);
        const int entryG = static_cast<int>((entry >> 8) & 0xFFU);
        const int entryB = static_cast<int>((entry >> 16) & 0xFFU);
        const long long dr = r - entryR;
        const long long dg = g - entryG;
        const long long db = b - entryB;
        const long long distance = (dr * dr) + (dg * dg) + (db * db);
        // '<' strict, jamais '<=' : a distance egale, la PREMIERE entree rencontree l'emporte --
        // departage stable et deterministe (meme geste, meme resultat, a chaque execution).
        if (bestDistance < 0 || distance < bestDistance) {
            bestDistance = distance;
            nearestRgb = entry & 0x00FFFFFFU;
        }
    }
    return nearestRgb | alpha;
}

}  // namespace hmi
