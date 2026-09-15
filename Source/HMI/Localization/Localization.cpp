// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "HMI/Localization/Localization.h"

#include <fstream>
#include <sstream>
#include <utility>

namespace hmi {

namespace {
// text sans les espaces et tabulations de début et de fin.
[[nodiscard]] std::string_view trim(std::string_view text) noexcept {
    const auto isSpace = [](char character) {
        return character == ' ' || character == '\t' || character == '\r' || character == '\n';
    };
    std::size_t begin = 0;
    std::size_t end = text.size();
    while (begin < end && isSpace(text[begin])) {
        ++begin;
    }
    while (end > begin && isSpace(text[end - 1])) {
        --end;
    }
    return text.substr(begin, end - begin);
}
}  // namespace

// Construit un catalogue vide.
Localization::Localization(std::filesystem::path directory) : _directory(std::move(directory)) {}

// Analyse un contenu `clé = valeur` en table de traductions.
//
// Chaque ligne est traitée indépendamment : une ligne vide ou commençant (après espaces) par
// `#` est ignorée ; sinon, le **premier** `=` sépare la clé de la valeur, toutes deux
// débarrassées de leurs espaces de bordure. Un `=` ultérieur appartient donc à la valeur.
std::unordered_map<std::string, std::string> Localization::parseCatalog(std::string_view content) {
    std::unordered_map<std::string, std::string> strings;
    std::size_t position = 0;

    while (position <= content.size()) {
        const std::size_t newline = content.find('\n', position);
        const std::size_t lineEnd = newline == std::string_view::npos ? content.size() : newline;
        const std::string_view line = content.substr(position, lineEnd - position);
        position = lineEnd + 1;

        const std::string_view trimmed = trim(line);
        if (trimmed.empty() || trimmed.front() == '#') {
            if (newline == std::string_view::npos) {
                break;
            }
            continue;  // ligne vide ou commentaire
        }

        const std::size_t separator = trimmed.find('=');
        if (separator != std::string_view::npos) {
            const std::string_view key = trim(trimmed.substr(0, separator));
            const std::string_view value = trim(trimmed.substr(separator + 1));
            if (!key.empty()) {
                strings.insert_or_assign(std::string(key), std::string(value));
            }
        }

        if (newline == std::string_view::npos) {
            break;
        }
    }
    return strings;
}

// Définit la langue par défaut (source de repli) et l'active dessus.
void Localization::setDefaultCatalog(std::string language,
                                     std::unordered_map<std::string, std::string> strings) {
    _defaultLanguage = language;
    _defaultStrings = std::move(strings);
    // La langue active démarre sur la langue par défaut tant qu'aucune autre n'est chargée.
    _activeLanguage = std::move(language);
    _activeStrings = _defaultStrings;
}

// Définit la langue active à partir d'une table déjà analysée.
void Localization::setActiveCatalog(std::string language,
                                    std::unordered_map<std::string, std::string> strings) {
    _activeLanguage = std::move(language);
    _activeStrings = std::move(strings);
}

// Charge la langue par défaut depuis `<directory>/<language>.lang` et l'active.
bool Localization::loadDefaultLanguage(const std::string& language) {
    std::string content;
    if (!readFile(_directory / (language + ".lang"), content)) {
        return false;
    }
    setDefaultCatalog(language, parseCatalog(content));
    return true;
}

// Charge la langue active depuis `<directory>/<language>.lang`.
bool Localization::loadLanguage(const std::string& language) {
    std::string content;
    if (!readFile(_directory / (language + ".lang"), content)) {
        return false;  // langue absente : on garde la langue active en cours
    }
    setActiveCatalog(language, parseCatalog(content));
    return true;
}

// Résout le texte associé à une clé.
//
// Le repli garantit un affichage toujours défini : une clé oubliée dans une traduction
// réapparaît dans la langue par défaut, et une clé inconnue s'affiche telle quelle (utile
// pour repérer un oubli sans planter).
std::string Localization::text(std::string_view key) const {
    const std::string lookup(key);
    if (const auto active = _activeStrings.find(lookup); active != _activeStrings.end()) {
        return active->second;
    }
    if (const auto fallback = _defaultStrings.find(lookup); fallback != _defaultStrings.end()) {
        return fallback->second;
    }
    return lookup;
}

// Lit tout le contenu d'un fichier texte (true et remplit out en cas de succès).
bool Localization::readFile(const std::filesystem::path& path, std::string& out) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        return false;
    }
    std::ostringstream buffer;
    buffer << file.rdbuf();
    out = buffer.str();
    return true;
}

}  // namespace hmi
