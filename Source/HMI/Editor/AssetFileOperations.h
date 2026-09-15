// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "HMI/Editor/FileOperationResult.h"
#include "HMI/Graphics/AssetContract.h"

/**
 * @file HMI/Editor/AssetFileOperations.h
 * @brief Opérations fichiers sur les assets d'un dossier (créer/renommer/dupliquer/supprimer),
 *        sans Qt (`EX-EDIT-026`).
 */

namespace hmi {

/**
 * @brief Opérations sur les fichiers d'un dossier d'assets, **sans dépendance Qt/GPU**.
 *
 * Pendant exact de `hmi::LevelFileOperations`, dont elle réutilise le type de résultat
 * (`hmi::FileOperationResult`) : même contrat, jamais d'exception vers l'appelant (`EX-NFR-040`),
 * chaque opération renvoie un résultat exploitable (`EX-NFR-010`).
 *
 * Deux différences avec les niveaux justifient une classe séparée plutôt qu'une réutilisation
 * directe :
 * - un **import** copie un fichier **externe** (hors du dossier géré), après validation de ses
 *   dimensions contre le contrat de sa famille (`EX-REN-007`) — un niveau n'a pas d'équivalent ;
 * - un renommage conserve l'**extension** d'origine (`.png`), alors qu'un niveau n'en a pas côté
 *   utilisateur (`LevelFileOperations` l'ajoute elle-même).
 */
class AssetFileOperations {
public:
    explicit AssetFileOperations(std::filesystem::path assetsDirectory);

    /// @return Les fichiers du dossier, triés par nom (vide si le dossier n'existe pas).
    [[nodiscard]] std::vector<std::filesystem::path> list() const;

    /**
     * @brief Copie un fichier externe dans le dossier géré, après validation de son contrat.
     *
     * **Copie**, jamais déplacement : le fichier source appartient à l'utilisateur (point
     * d'attention `LOT-43` TACHE-02). Les dimensions sont vérifiées **avant** la copie : importer
     * un asset invalide pour le découvrir en jeu serait une régression par rapport à la validation
     * au chargement (`hmi::TextureCache`).
     *
     * Le **décodage** de l'image (`hmi::decodeImageFile`, dépendance Qt) reste à la charge de
     * l'appelant, qui fournit les dimensions déjà lues : cette classe reste ainsi **pure**
     * (`std::filesystem` seul), comme `hmi::LevelFileOperations`, et testable sans Qt
     * (`EX-NFR-010`).
     * @param sourceFile     Fichier externe à copier.
     * @param family         Famille de l'asset, qui fixe les dimensions attendues.
     * @param decodedWidth   Largeur de l'image déjà décodée, en pixels.
     * @param decodedHeight  Hauteur de l'image déjà décodée, en pixels.
     * @return Le résultat, avec le chemin copié en cas de succès.
     */
    [[nodiscard]] FileOperationResult import(const std::filesystem::path& sourceFile,
                                             AssetFamily family, int decodedWidth,
                                             int decodedHeight) const;

    /// Renomme le fichier @p source en @p newStem, en conservant son extension d'origine.
    [[nodiscard]] FileOperationResult rename(const std::filesystem::path& source,
                                             const std::string& newStem) const;

    /// Duplique @p source sous un nom unique (« … (copie) », « … (copie 2) », …).
    [[nodiscard]] FileOperationResult duplicate(const std::filesystem::path& source) const;

    /// Supprime le fichier @p source.
    [[nodiscard]] static FileOperationResult remove(const std::filesystem::path& source);

private:
    /// Chemin d'un nom de fichier (sans extension) dans le dossier géré, avec l'extension donnée.
    [[nodiscard]] std::filesystem::path pathForStem(const std::string& stem,
                                                    const std::filesystem::path& extension) const;

    std::filesystem::path _directory;
};

}  // namespace hmi
