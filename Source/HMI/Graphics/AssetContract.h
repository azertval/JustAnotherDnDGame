// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <string>

/**
 * @file HMI/Graphics/AssetContract.h
 * @brief Verdict de validation d'un asset graphique contre ses dimensions décodées
 *        (`EX-REN-007`).
 */

namespace hmi {

/**
 * @brief Verdict de validation d'un asset, avec son message exploitable.
 *
 * Le message est vide quand l'asset est conforme. Sinon il nomme le **fichier**, la dimension
 * **trouvée** et la dimension **attendue** : c'est ce que l'auteur de l'asset lit pour savoir quoi
 * corriger (`EX-REN-007`). Rendu par `hmi::AnimationCatalog::validateAgainstTexture`, qui confronte
 * une description d'animation au PNG décodé.
 */
struct AssetValidation {
    /// `true` si l'asset respecte le contrat de sa famille.
    bool valid = false;
    /// Message d'erreur en français, vide si `valid`.
    std::string message;
};

}  // namespace hmi
