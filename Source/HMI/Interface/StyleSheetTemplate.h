// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>
#include <unordered_map>

/**
 * @file HMI/Interface/StyleSheetTemplate.h
 * @brief Substitution de marqueurs dollar-accolade dans un modèle de feuille de style (`LOT-56`
 *        TACHE-02).
 *
 * Logique **pure** (aucune dépendance Qt/GPU), testable hors instance d'application
 * (`EX-NFR-010`) — compilée à la fois dans `JustAnotherDnDGame` et directement dans `UnitTests`,
 * comme `HMI/Interface/DesignTokens.cpp`.
 */

namespace hmi {

/// Résultat d'une substitution : succès et texte produit, ou échec et message d'erreur — jamais un
/// résultat produit silencieusement avec un trou (marqueur inconnu non substitué).
struct StyleSheetSubstitutionResult {
    bool ok = false;
    std::string text;
    std::string error;
};

/**
 * @brief Remplace chaque marqueur dollar-accolade de @p templateText par sa valeur dans @p values.
 *
 * Fonction **pure** : mêmes entrées, même sortie, aucun effet de bord, aucune dépendance à un
 * fichier ou une instance d'application.
 * @param templateText Texte du modèle, contenant zéro ou plusieurs marqueurs.
 * @param values        Table nom de marqueur -> valeur de substitution.
 * @return `ok=true` avec le texte substitué ; `ok=false` avec un message nommant le premier
 *         marqueur absent de @p values — jamais un résultat partiellement substitué.
 */
[[nodiscard]] StyleSheetSubstitutionResult substituteStyleSheetTemplate(
    const std::string& templateText, const std::unordered_map<std::string, std::string>& values);

}  // namespace hmi
