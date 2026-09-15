// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <filesystem>
#include <optional>
#include <string>

/**
 * @file HMI/Graphics/AssetPaths.h
 * @brief Résolution de chemins d'assets graphiques dans un dossier donné.
 */

namespace hmi {

/**
 * @brief Résout des noms d'assets logiques vers des chemins de fichiers, dans un dossier donné.
 *
 * Logique **pure** (aucune dépendance fenêtre/GPU/Qt), testable en isolation (`EX-NFR-010`) : le
 * dossier est injecté au constructeur (comme `hmi::Localization`), jamais résolu en dur ici — les
 * appelants combinent typiquement `hmi::executableDirectory() / "Assets"` (patron `Levels`/
 * `Localization`). Un asset absent ou illisible est un cas **attendu** (`EX-NFR-040`) : `resolve`
 * renvoie `std::nullopt` plutôt que de planter, pour permettre un repli (ex. atlas procédural).
 */
class AssetPaths {
public:
    /**
     * @brief Construit un résolveur pour un dossier d'assets donné.
     * @param directory Dossier racine des assets (n'a pas besoin d'exister à la construction).
     */
    explicit AssetPaths(std::filesystem::path directory);

    /**
     * @brief Résout un nom de fichier logique vers un chemin existant dans le dossier d'assets.
     * @param fileName Nom du fichier, relatif au dossier d'assets (ex. « atlas.png »).
     * @return Le chemin complet si le fichier existe et est un fichier régulier ; `std::nullopt`
     *         sinon (dossier ou fichier absent, illisible, ou chemin désignant autre chose).
     */
    [[nodiscard]] std::optional<std::filesystem::path> resolve(const std::string& fileName) const;

    /// @return Le dossier d'assets configuré.
    [[nodiscard]] const std::filesystem::path& directory() const noexcept {
        return _directory;
    }

private:
    std::filesystem::path _directory;
};

}  // namespace hmi
