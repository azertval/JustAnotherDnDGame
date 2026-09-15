// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

#include "Core/Levels/TileType.h"
#include "HMI/Graphics/SkinCatalog.h"

/**
 * @file HMI/Editor/SkinAssignments.h
 * @brief Logique pure alimentant le panneau « Textures » (`EX-EDIT-042`, `EX-EDIT-024`).
 */

namespace hmi {

/// Une ligne du panneau : un type de tuile et l'apparence qui lui est assignée, s'il y en a une.
struct SkinRow {
    core::TileType type;
    /// Libellé du type, repris de la taxonomie de la palette (`hmi::tileTaxonomy`).
    std::string typeLabel;
    /// Fichier assigné, **vide** si le type n'est pas skinné (il s'affiche alors en damier).
    std::string asset;
    /// Mode de découpage. N'a de sens que si `asset` est renseigné.
    SkinMode mode = SkinMode::Single;

    [[nodiscard]] friend bool operator==(const SkinRow& lhs, const SkinRow& rhs) {
        return lhs.type == rhs.type && lhs.typeLabel == rhs.typeLabel && lhs.asset == rhs.asset &&
               lhs.mode == rhs.mode;
    }
};

/// Un groupe de lignes, correspondant à une catégorie de la palette.
struct SkinSection {
    std::string label;
    std::vector<SkinRow> rows;
};

/**
 * @brief Construit les lignes du panneau pour un jeu de skins donné.
 *
 * Réutilise la **taxonomie de la palette** (`hmi::tileTaxonomy`) plutôt que d'énumérer les trente
 * types à plat : une liste plate serait inutilisable, et deux organisations différentes entre la
 * palette et ce panneau obligeraient le level designer à réapprendre où trouver chaque tuile.
 *
 * Les sous-groupes de la taxonomie sont **aplatis** dans leur catégorie : le panneau n'a pas de
 * sélection à opérer, seulement une assignation par type, et un troisième niveau de repli
 * n'apporterait rien.
 *
 * `core::TileType::Empty` est exclu : une case vide n'affiche rien, elle n'a pas d'apparence à
 * assigner.
 *
 * Logique **pure** (aucune dépendance Qt/GPU), testable en isolation (`EX-NFR-010`).
 * @param catalog Catalogue interrogé pour l'assignation courante de chaque type.
 * @param setName Nom du jeu de skins consulté ; vide pour le jeu par défaut du catalogue.
 * @return Les sections, dans l'ordre de la taxonomie.
 */
[[nodiscard]] std::vector<SkinSection> buildSkinRows(const SkinCatalog& catalog,
                                                     std::string_view setName);

/**
 * @brief Liste les fichiers de skins disponibles, par balayage d'un dossier.
 *
 * Le level designer choisit dans cette liste : aucune saisie de chemin, qui serait à la fois
 * pénible et source de configurations invalides.
 * @param skinsDirectory Dossier balayé (typiquement `executableDirectory()/Assets/Skins`).
 * @return Les noms de fichiers image, triés par ordre alphabétique ; vide si le dossier est
 *         absent — un état de départ légitime, pas une erreur.
 */
[[nodiscard]] std::vector<std::string> listSkinAssets(const std::filesystem::path& skinsDirectory);

/**
 * @brief Applique une assignation à un catalogue, ou la retire.
 *
 * Point unique de la modification, pour que le panneau n'ait pas à décider lui-même ce que
 * signifie « aucun skin » — un `asset` vide **retire** l'assignation, et le type redevient non
 * skinné plutôt que d'être associé à un fichier de nom vide.
 * @param catalog Catalogue modifié.
 * @param setName Jeu concerné ; vide pour le jeu par défaut du catalogue.
 * @param type    Type de tuile.
 * @param asset   Fichier à assigner, ou chaîne vide pour retirer l'assignation.
 * @param mode    Mode de découpage appliqué si @p asset est renseigné.
 */
void applySkinAssignment(SkinCatalog& catalog, std::string_view setName, core::TileType type,
                         const std::string& asset, SkinMode mode);

}  // namespace hmi
