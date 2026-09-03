// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>

/**
 * @file HMI/Interface/FontResolution.h
 * @brief Résolution de la famille de police de l'IHM (`LOT-56` TACHE-03, `EX-IHM-052`).
 *
 * Logique **pure** (aucune dépendance Qt/GPU), testable hors instance d'application
 * (`EX-NFR-010`) — compilée à la fois dans `JustAnotherDnDGame` et directement dans `UnitTests`, comme
 * `HMI/Interface/DesignTokens.cpp`.
 */

namespace hmi {

/// Résultat de la résolution : la police embarquée si elle a pu être enregistrée auprès de Qt,
/// sinon aucun nom de famille -- jamais un second nom codé en dur. Dans ce second cas, l'appelant
/// (Qt) demande une famille **générique** via `QFont::StyleHint`, jamais un nom de repli littéral.
struct FontFamilyResolution {
    bool useEmbeddedFamily = false;
    std::string embeddedFamily;

    [[nodiscard]] friend bool operator==(const FontFamilyResolution&,
                                         const FontFamilyResolution&) noexcept = default;
};

/**
 * @brief Choisit la famille de police à employer pour le corps de l'IHM.
 * @param fontRegistered     `true` si le fichier de police embarquée a pu être enregistré auprès
 *                            de `QFontDatabase` (les deux graisses, régulière et grasse).
 * @param embeddedFamilyName Nom de famille rapporté par Qt pour la police embarquée (ex. "Inter").
 * @return La police embarquée si @p fontRegistered vaut `true` ; sinon un résultat sans nom de
 *         famille, signalant à l'appelant de retomber sur une famille générique.
 */
[[nodiscard]] FontFamilyResolution resolveFontFamily(bool fontRegistered,
                                                     const std::string& embeddedFamilyName);

}  // namespace hmi
