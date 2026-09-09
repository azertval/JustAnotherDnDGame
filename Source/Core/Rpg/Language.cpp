// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Core/Rpg/Language.h"

#include <algorithm>
#include <system_error>

#include "Core/Data/JsonDocument.h"

namespace core {

namespace {

// Les entrees de catalogue ne portent pas de champ `version` : ce sont des donnees, pas des
// documents de format (meme choix qu'au LOT-33, au LOT-36 et pour les competences).
constexpr int SANS_GARDE_DE_VERSION = 0;

}  // namespace

const LanguageDefinition* LanguageCatalog::find(std::string_view id) const {
    const auto trouve = std::ranges::find(languages, id, &LanguageDefinition::id);
    return trouve == languages.end() ? nullptr : &*trouve;
}

LanguageCatalog loadLanguages(const std::filesystem::path& directory) {
    LanguageCatalog catalogue;

    std::error_code code;
    if (!std::filesystem::is_directory(directory, code)) {
        // Un dossier absent n'est PAS un catalogue vide : sans langue, chaque espece afficherait
        // ses identifiants bruts, et un dialogue refuse faute de langue commune deviendrait
        // impossible a expliquer au joueur.
        catalogue.errors.push_back(directory.string() + " : dossier de langues absent.");
        return catalogue;
    }

    std::vector<std::filesystem::path> fichiers;
    for (const auto& entree : std::filesystem::directory_iterator(directory, code)) {
        if (entree.is_regular_file(code) && entree.path().extension() == ".json") {
            fichiers.push_back(entree.path());
        }
    }
    std::ranges::sort(fichiers);

    for (const std::filesystem::path& chemin : fichiers) {
        const JsonDocument document = readJsonObjectFromFile(chemin, SANS_GARDE_DE_VERSION);
        if (!document.ok()) {
            // Le message porte deja le fichier et la ligne (EX-CNT-010).
            catalogue.errors.push_back(document.message);
            continue;
        }
        const std::string nom = chemin.filename().string();
        const auto identifiant = document.root.find("id");
        const auto libelle = document.root.find("name");
        const auto exotique = document.root.find("exotic");
        if (identifiant == document.root.end() || !identifiant->is_string() ||
            libelle == document.root.end() || !libelle->is_string() ||
            exotique == document.root.end() || !exotique->is_boolean()) {
            // `exotic` est exige au meme titre que le nom : le deviner reviendrait a decider seul
            // quelles langues un personnage peut prendre a la creation.
            catalogue.errors.push_back(nom + " : champs 'id', 'name' ou 'exotic' absents.");
            continue;
        }
        catalogue.languages.push_back(
            {identifiant->get<std::string>(), libelle->get<std::string>(), exotique->get<bool>()});
    }
    std::ranges::sort(catalogue.languages, {}, &LanguageDefinition::id);
    return catalogue;
}

}  // namespace core
