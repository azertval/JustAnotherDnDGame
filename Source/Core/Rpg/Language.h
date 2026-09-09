// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

/**
 * @file Core/Rpg/Language.h
 * @brief Le catalogue des seize langues (`EX-RPG-042`).
 */

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace core {

/**
 * @brief Une langue, et si elle est exotique.
 *
 * L'espèce et le bloc de créature ne portent que des **identifiants** de langue — `common`,
 * `elvish`. Le nom lisible vit ici, et nulle part ailleurs : affiché tel quel, l'identifiant se
 * lirait comme une donnée alors que c'est une clé, et « common, elvish » au milieu d'un écran
 * français est exactement le défaut que ce catalogue supprime.
 */
struct LanguageDefinition {
    std::string id;
    std::string name;
    /// Vrai pour les huit langues exotiques, qui ne s'obtiennent pas librement à la création. Le
    /// champ est **requis** par le schéma : une langue dont on ignore si elle est exotique ne peut
    /// pas être proposée à la création, et deviner « non » ouvrirait l'infernal à tout le monde.
    bool exotic = false;
};

/// @brief Les langues chargées, et ce qui n'a pas pu l'être.
struct LanguageCatalog {
    std::vector<LanguageDefinition> languages;
    std::vector<std::string> errors;

    /// @brief La langue portant cet identifiant, ou `nullptr`.
    [[nodiscard]] const LanguageDefinition* find(std::string_view id) const;
};

/**
 * @brief Charge le catalogue des langues depuis son dossier.
 *
 * @param directory `Source/Elements/Rpg/languages`.
 * @return Le catalogue et la liste des échecs. Ne lève jamais (`EX-NFR-040`).
 */
[[nodiscard]] LanguageCatalog loadLanguages(const std::filesystem::path& directory);

}  // namespace core
