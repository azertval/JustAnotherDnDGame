// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <utility>
#include <vector>

#include "HMI/Graphics/AssetContract.h"

/**
 * @file HMI/Editor/PixelAssetIO.h
 * @brief Tailles d'asset proposées à la création dans l'atelier pixel art (`LOT-54` TACHE-05,
 *        `EX-REN-007`).
 */

namespace hmi {

/**
 * @brief Tailles proposées à la création d'un asset de la famille donnée.
 *
 * Fonction **pure** (`EX-NFR-010`) : dérivée de `hmi::assetDimensionContract`, jamais une seconde
 * description des mêmes règles — une création à une taille non conforme ne doit pas être possible
 * du tout, plutôt que refusée après coup au chargement (`hmi::validateAsset`).
 *
 * Chaque taille renvoyée passe `hmi::validateAsset` pour sa famille. Une famille à **dimensions
 * libres** (fond, décor, police) renvoie une liste **vide** : il n'existe alors aucune progression
 * de tailles pertinente, l'appelant doit permettre une saisie libre (strictement positive).
 * @param family Famille de l'asset à créer.
 * @return Une courte progression de tailles conformes (croissante), ou vide si les dimensions sont
 *         libres.
 */
[[nodiscard]] std::vector<std::pair<int, int>> validAssetSizes(AssetFamily family);

}  // namespace hmi
