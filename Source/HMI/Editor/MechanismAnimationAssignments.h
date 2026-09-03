// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

#include "Core/Levels/TileType.h"
#include "HMI/Graphics/SkinCatalog.h"

/**
 * @file HMI/Editor/MechanismAnimationAssignments.h
 * @brief Logique pure de la section « Animations » du panneau « Textures » (`LOT-47` TACHE-04).
 */

namespace hmi {

/// Les familles de mécanismes à état dont l'apparence est pilotée par un clip (`LOT-47`), dans
/// l'ordre où elles sont proposées par la section « Animations ».
inline constexpr core::TileType MECHANISM_ANIMATION_TYPES[] = {
    core::TileType::Door, core::TileType::Switch,     core::TileType::PressurePlate,
    core::TileType::Key,  core::TileType::LockedDoor,
};

/// Une ligne de la section « Animations » : une famille de mécanisme, l'asset qui lui sert
/// d'apparence par défaut, et le diagnostic de clips manquants pour cet asset.
struct MechanismAnimationRow {
    core::TileType type{};
    /// Libellé du type, repris de la taxonomie de la palette (`hmi::tileTaxonomy`), même patron
    /// que `hmi::SkinRow::typeLabel`.
    std::string typeLabel;
    /// Fichier assigné (relatif à `Assets/Skins/`), **vide** si la famille n'a pas d'asset par
    /// défaut — dans ce cas `missingClips` reste vide : il n'y a rien à diagnostiquer.
    std::string asset;
    /// Clips attendus (`hmi::mechanismExpectedClips`) que l'asset assigné ne fournit pas — utile à
    /// l'auteur pour savoir ce qu'il lui reste à dessiner, sans lancer le jeu. Vide si `asset` est
    /// vide (rien à diagnostiquer), ou si tous les clips attendus sont présents. Un asset assigné
    /// sans fichier `.anim.json` du tout liste **tous** les clips attendus comme manquants — c'est
    /// une image fixe, pas encore d'animation, information que le diagnostic doit montrer même si
    /// le moteur (`hmi::GameSession`) la traite en silence à l'exécution (cas légitime, `LOT-46`).
    std::vector<std::string> missingClips;
};

/**
 * @brief Construit les lignes de la section « Animations », diagnostic de clips manquants compris.
 *
 * Logique **pure** (`std::filesystem` + `hmi::AnimationCatalog`, aucune dépendance Qt/GPU),
 * testable en isolation (`EX-NFR-010`). Stockage identique à un skin de type (`hmi::SkinCatalog`,
 * `LOT-42`) : ce n'est pas une seconde configuration, seulement une vue dédiée à six types
 * particuliers, avec un diagnostic que la section « Skins » n'offre pas.
 * @param catalog        Catalogue interrogé pour l'assignation courante de chaque famille.
 * @param setName        Jeu de skins consulté ; vide pour le jeu par défaut du catalogue.
 * @param skinsDirectory Dossier des skins, pour lire le descripteur d'animation de l'asset assigné
 *                       (`<asset>.anim.json`).
 * @return Une ligne par famille de `MECHANISM_ANIMATION_TYPES`, dans cet ordre.
 */
[[nodiscard]] std::vector<MechanismAnimationRow> buildMechanismAnimationRows(
    const SkinCatalog& catalog, std::string_view setName,
    const std::filesystem::path& skinsDirectory);

}  // namespace hmi
