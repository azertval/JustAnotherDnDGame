// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

/**
 * @file Editor/Logic/ThemeResolution.h
 * @brief Résolution du thème effectif du châssis d'édition (`EX-IHM-054`).
 *
 * Logique **pure** (aucune dépendance Qt/GPU), testable hors instance d'application
 * (`EX-NFR-010`) — compilée à la fois dans `JustAnotherRpgGame` et directement dans `UnitTests`.
 */

namespace hmi {

/// Réglage choisi par l'utilisateur (menu Affichage de l'éditeur, persisté entre sessions).
enum class EditorThemeSetting { System, Light, Dark };

/// Thème effectivement appliqué au châssis d'édition.
enum class EditorThemeMode { Light, Dark };

/**
 * @brief Résout le thème effectif à appliquer.
 *
 * Fonction **pure** : `System` + système clair -> `Light` ; `System` + système sombre -> `Dark` ;
 * `Light`/`Dark` forcé -> ignore le réglage système.
 * @param setting           Réglage utilisateur courant.
 * @param systemPrefersDark Le système d'exploitation préfère-t-il un thème sombre.
 * @return Le thème à appliquer au châssis d'édition (jamais à l'identité du jeu, invariante).
 */
[[nodiscard]] EditorThemeMode resolveEffectiveEditorTheme(EditorThemeSetting setting,
                                                          bool systemPrefersDark) noexcept;

}  // namespace hmi
