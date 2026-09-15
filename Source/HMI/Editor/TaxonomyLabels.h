// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <string>

/**
 * @file HMI/Editor/TaxonomyLabels.h
 * @brief Traduction des libellés de la taxonomie de tuiles (`EX-REN-033`).
 */

namespace hmi {

class Localization;

/**
 * @brief Clé de traduction d'un libellé de la taxonomie.
 *
 * `hmi::tileTaxonomy` reste une logique **pure**, sans dépendance i18n : elle porte des libellés
 * français, qui servent aussi de clés de correspondance ici. Cette table est le point **unique**
 * de la conversion, partagé par tous les panneaux qui affichent des types de tuiles — la palette
 * (`LOT-35`) et le panneau « Textures » (`LOT-42`). Deux copies divergeraient au premier libellé
 * ajouté, et un panneau afficherait alors du français au milieu d'une interface anglaise.
 * @param label Libellé issu de la taxonomie.
 * @return La clé de traduction, ou une chaîne vide si le libellé n'en a pas.
 */
[[nodiscard]] std::string taxonomyLabelKey(const std::string& label);

/**
 * @brief Libellé de taxonomie traduit dans la langue active.
 * @param loc   Catalogue de traduction, ou `nullptr` avant la première retraduction.
 * @param label Libellé issu de la taxonomie.
 * @return Le libellé traduit, ou le libellé source à défaut de catalogue ou de clé.
 */
[[nodiscard]] std::string localizedTaxonomyLabel(const Localization* loc, const std::string& label);

}  // namespace hmi
