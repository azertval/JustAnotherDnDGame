// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <array>
#include <cstddef>
#include <optional>

#include "Editor/Logic/EditorTool.h"

/**
 * @file Editor/Logic/PanelFocus.h
 * @brief Correspondance outil actif → panneau à mettre en avant (`EX-IHM-061`).
 *
 * Logique **pure** (aucune dépendance Qt), testable hors instance d'application (`EX-NFR-010`) —
 * même patron que `Editor/Logic/ActionCatalog.h`. `MainWindow` ne fait que suivre cette table :
 * aucune condition écrite en dur sur un outil particulier.
 */

namespace hmi {

/// Panneau dockable de l'éditeur.
enum class PanelId {
    Palette,
    Levels,
    // Couches et entites de la carte (LOT-11).
    Layers,
    Entities,
};

/// Nombre de panneaux, declare au plus pres de l'enumeration qu'il compte.
inline constexpr std::size_t PANEL_COUNT = 4;

/// Une entrée de la table : l'outil @p tool met en avant le panneau @p panel.
struct PanelFocusEntry {
    EditorTool tool;
    PanelId panel;
};

/// Nombre d'entrées de la table (tous les outils n'ont pas de panneau dédié).
constexpr int PANEL_FOCUS_CATALOG_COUNT = 1;

/// @return La table complète outil → panneau.
[[nodiscard]] const std::array<PanelFocusEntry, PANEL_FOCUS_CATALOG_COUNT>& panelFocusCatalog();

/// @return Le panneau que @p tool met en avant, s'il en a un.
[[nodiscard]] std::optional<PanelId> panelForTool(EditorTool tool);

}  // namespace hmi
