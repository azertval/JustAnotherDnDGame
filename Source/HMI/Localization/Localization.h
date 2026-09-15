// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <filesystem>
#include <string>
#include <string_view>
#include <unordered_map>

/**
 * @file HMI/Localization/Localization.h
 * @brief Catalogue de traduction : résolution des textes d'interface par clé et par langue.
 */

namespace hmi {

/**
 * @brief Catalogue de traduction (i18n) : associe des **clés** stables à des chaînes selon
 *        la **langue active**, avec repli sur la langue par défaut.
 *
 * Tous les textes affichés passent par une clé (`EX-REN-033`) : le code ne contient aucun
 * libellé en dur. Les traductions sont chargées depuis un **fichier par langue**
 * (`<langue>.lang`, format `clé = valeur`, UTF-8), ce qui rend l'ajout d'une langue trivial.
 * La résolution suit un **repli déterministe** : langue active, puis langue par défaut, puis
 * la clé elle-même — jamais de plantage (`EX-NFR-040`). Le catalogue est une logique pure,
 * sans fenêtre ni GPU, donc testable en isolation (`EX-NFR-010`).
 */
class Localization {
public:
    /**
     * @brief Construit un catalogue vide.
     * @param directory Dossier contenant les fichiers `<langue>.lang` (pour les chargements
     *                  par fichier). Optionnel : les tests peuvent injecter des tables en mémoire.
     */
    explicit Localization(std::filesystem::path directory = {});

    /**
     * @brief Analyse un contenu `clé = valeur` en table de traductions.
     * @param content Texte source (UTF-8). Les lignes vides et les lignes débutant par `#`
     *                (commentaires) sont ignorées ; les espaces autour de `=` sont retirés ;
     *                un `=` présent dans la valeur est conservé (seul le premier sépare).
     * @return La table clé → valeur.
     */
    [[nodiscard]] static std::unordered_map<std::string, std::string> parseCatalog(
        std::string_view content);

    /**
     * @brief Définit la langue **par défaut** (source de repli) et l'active dessus.
     * @param language Identifiant de langue (ex. « fr »).
     * @param strings  Table de traductions déjà analysée.
     */
    void setDefaultCatalog(std::string language,
                           std::unordered_map<std::string, std::string> strings);

    /**
     * @brief Définit la langue **active** à partir d'une table déjà analysée.
     * @param language Identifiant de langue.
     * @param strings  Table de traductions déjà analysée.
     */
    void setActiveCatalog(std::string language,
                          std::unordered_map<std::string, std::string> strings);

    /**
     * @brief Charge la langue **par défaut** depuis `<directory>/<language>.lang` et l'active.
     * @param language Identifiant de langue (nom de fichier sans extension).
     * @return true si le fichier a été lu ; false (récupérable) s'il est absent ou illisible.
     */
    [[nodiscard]] bool loadDefaultLanguage(const std::string& language);

    /**
     * @brief Charge la langue **active** depuis `<directory>/<language>.lang`.
     * @param language Identifiant de langue (nom de fichier sans extension).
     * @return true si le fichier a été lu ; false (récupérable) s'il est absent ou illisible
     *         — la langue active précédente est alors conservée.
     */
    [[nodiscard]] bool loadLanguage(const std::string& language);

    /**
     * @brief Résout le texte associé à @p key.
     * @param key Clé de traduction (ex. « menu.quitter »).
     * @return La chaîne de la langue active, sinon celle de la langue par défaut, sinon la clé.
     */
    [[nodiscard]] std::string text(std::string_view key) const;

    /// @return L'identifiant de la langue active.
    [[nodiscard]] const std::string& activeLanguage() const noexcept {
        return _activeLanguage;
    }

private:
    /// Lit tout le contenu d'un fichier texte. @return true et remplit @p out en cas de succès.
    [[nodiscard]] static bool readFile(const std::filesystem::path& path, std::string& out);

    std::filesystem::path _directory;
    std::string _defaultLanguage;
    std::string _activeLanguage;
    std::unordered_map<std::string, std::string> _defaultStrings;
    std::unordered_map<std::string, std::string> _activeStrings;
};

}  // namespace hmi
