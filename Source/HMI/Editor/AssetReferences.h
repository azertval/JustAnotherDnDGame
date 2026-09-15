// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <string>
#include <vector>

#include "HMI/Graphics/SkinCatalog.h"

/**
 * @file HMI/Editor/AssetReferences.h
 * @brief Détection des références à un fichier d'asset, pour avertir avant renommage/suppression
 *        (`EX-EDIT-026`).
 */

namespace hmi {

/// Une référence nommée à un asset : dans quel jeu de skins, pour quel type de tuile.
struct AssetReference {
    std::string setName;
    /// Nom textuel du type de tuile référençant l'asset (`core::tileTypeName`).
    std::string typeName;

    [[nodiscard]] friend bool operator==(const AssetReference& lhs, const AssetReference& rhs) {
        return lhs.setName == rhs.setName && lhs.typeName == rhs.typeName;
    }
};

/**
 * @brief Repère toutes les entrées de `skins.json` qui citent un fichier donné.
 *
 * Les références aux assets se font par **nom de fichier** : sans cette détection, renommer ou
 * supprimer un asset casse silencieusement les jeux de skins qui le citent (`LOT-43` TACHE-02).
 *
 * **Portée actuelle** : seul `hmi::SkinCatalog` peut référencer un asset aujourd'hui —
 * `core::Level` ne porte encore aucun champ de ce type (le fond de niveau, la désignation d'un jeu
 * de skins par un niveau, etc. sont posés à partir de `LOT-44`). Cette fonction est le point
 * d'extension déjà en place : les lots suivants y ajouteront la détection des niveaux sur disque
 * sans changer sa signature ni celle de `describeReferences`.
 *
 * Logique **pure** (aucune dépendance Qt/GPU/fichier), testable en isolation (`EX-NFR-010`).
 * @param catalog  Catalogue interrogé (tous les jeux, pas seulement le courant).
 * @param fileName Nom de fichier recherché, comparé **exactement** (pas de sous-chaîne : un nom
 *                 proche mais différent ne doit jamais déclencher de faux positif).
 * @return Les références trouvées, dans l'ordre des jeux puis des types du catalogue ; vide si
 *         aucune.
 */
[[nodiscard]] std::vector<AssetReference> findSkinCatalogReferences(const SkinCatalog& catalog,
                                                                    const std::string& fileName);

/**
 * @brief Message d'avertissement listant des références, une par jeu de skins.
 * @param references Références à décrire (typiquement le résultat de `findSkinCatalogReferences`).
 * @return Le message en français, ou une chaîne vide si @p references est vide.
 */
[[nodiscard]] std::string describeReferences(const std::vector<AssetReference>& references);

}  // namespace hmi
