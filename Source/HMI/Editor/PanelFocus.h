// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <array>
#include <cstddef>
#include <optional>

#include "HMI/Editor/EditorTool.h"
#include "HMI/Editor/PixelTool.h"

/**
 * @file HMI/Editor/PanelFocus.h
 * @brief Correspondance outil actif → panneau à mettre en avant (`LOT-57` TACHE-02, `EX-IHM-061`).
 *
 * Logique **pure** (aucune dépendance Qt), testable hors instance d'application (`EX-NFR-010`) —
 * même patron que `HMI/Interface/ActionCatalog.h`. `MainWindow` ne fait que suivre cette table :
 * aucune condition écrite en dur sur un outil particulier.
 */

namespace hmi {

/// Panneau dockable de l'éditeur. Couvre les **neuf** docks depuis le `LOT-69` : la mise en avant
/// automatique (`EX-IHM-061`) n'en concerne toujours qu'une partie, mais la répartition par espace
/// de travail (`EX-IHM-073`) les concerne tous, et une énumération partielle laisserait des
/// panneaux sans espace.
enum class PanelId {
    // Espace d'edition de niveau.
    Palette,
    Levels,
    Links,
    Textures,
    // Espace des plans picturaux (LOT-69).
    Planes,
    // Espace de l'atelier pixel art.
    PixelCanvas,
    PixelHistory,
    PixelPalette,
};

/// Nombre de panneaux, declare au plus pres de l'enumeration qu'il compte. Sert de garde de
/// **completude** a la repartition par espace de travail (`hmi::workspacesForPanel`, `LOT-68`) :
/// un panneau ajoute sans espace resterait affiche dans les deux, ce qu'un test interdit.
inline constexpr std::size_t PANEL_COUNT = 8;

/// Une entrée de la table : l'outil @p tool met en avant le panneau @p panel.
struct PanelFocusEntry {
    EditorTool tool;
    PanelId panel;
};

/// Nombre d'entrées de la table (tous les outils n'ont pas de panneau dédié).
constexpr int PANEL_FOCUS_CATALOG_COUNT = 2;

/// @return La table complète outil → panneau.
[[nodiscard]] const std::array<PanelFocusEntry, PANEL_FOCUS_CATALOG_COUNT>& panelFocusCatalog();

/// @return Le panneau à mettre en avant pour @p tool, ou `std::nullopt` si cet outil n'en a pas
///         (ses contrôles vivent ailleurs, ou son panneau n'est tabifié avec aucun autre -- une
///         mise en avant n'y aurait aucun effet).
[[nodiscard]] std::optional<PanelId> panelForTool(EditorTool tool);

/// Une entrée de la table équivalente pour les outils du canevas pixel art (`LOT-54` TACHE-04) :
/// l'outil @p tool met en avant le panneau @p panel. Table séparée de `panelFocusCatalog` — deux
/// groupes d'outils disjoints (`hmi::EditorTool`/`hmi::PixelTool`), jamais actifs en même temps.
struct PixelPanelFocusEntry {
    PixelTool tool;
    PanelId panel;
};

/// Nombre d'entrées de la table des outils de canevas (les cinq outils du canevas mettent tous
/// en avant le même panneau : on édite toujours en le voyant).
constexpr int PIXEL_PANEL_FOCUS_CATALOG_COUNT = 5;

/// @return La table complète outil de canevas → panneau.
[[nodiscard]] const std::array<PixelPanelFocusEntry, PIXEL_PANEL_FOCUS_CATALOG_COUNT>&
pixelPanelFocusCatalog();

/// @return Le panneau à mettre en avant pour l'outil de canevas @p tool.
[[nodiscard]] std::optional<PanelId> panelForPixelTool(PixelTool tool);

}  // namespace hmi
