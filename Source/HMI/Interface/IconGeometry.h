// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <vector>

/**
 * @file HMI/Interface/IconGeometry.h
 * @brief Géométrie des icônes de l'IHM, dessinées par code (`LOT-56` TACHE-04, `EX-IHM-055`).
 *
 * Logique **pure** (aucune dépendance Qt/GPU), testable hors instance d'application
 * (`EX-NFR-010`) — compilée à la fois dans `JustAnotherDnDGame` et directement dans `UnitTests`,
 * comme `HMI/Interface/DesignTokens.cpp`. Même découpage que `hmi::gameHudLines` : cette fonction
 * décide *quoi* dessiner, `hmi::themeIcon` (Qt, `ThemeIcons.h`) décide *comment* le peindre.
 */

namespace hmi {

/// Identifiant d'une icône : un par outil d'édition et par commande principale.
enum class IconId {
    ToolPaint,
    ToolRectangle,
    ToolSelection,
    ToolLink,
    ToolTextureAssign,
    ToolCameraZone,
    Save,
    Playtest,
    Undo,
    Redo,
    ToggleGrid,
    ResetCamera,
    ToggleRenderMode,
    Copy,
    Paste,
    Rename,
    ShortcutsOverview,
    // Outils du canevas pixel art (LOT-54 TACHE-04) : groupe exclusif distinct des outils de
    // niveau ci-dessus (`EditorActionGroup::PixelTools`, `HMI/Interface/ActionCatalog.h`).
    PixelBrush,
    PixelEraser,
    PixelFill,
    PixelEyedropper,
    // Commandes de fichier de l'atelier pixel art (LOT-54 TACHE-05) : ouvrir/creer/enregistrer/
    // enregistrer sous, groupe `EditorActionGroup::PixelCommands`.
    PixelOpen,
    PixelCreate,
    PixelSave,
    PixelSaveAs,
    // Sélection et transformations de région (LOT-54 TACHE-06).
    PixelSelectionTool,
    PixelFlipHorizontal,
    PixelFlipVertical,
    PixelRotateClockwise,
    PixelRotateCounterClockwise,
};

/// Rôle de couleur d'un trait, résolu depuis les jetons de design au moment du rendu (recoloration
/// automatique avec le thème, y compris après une bascule TACHE-06).
enum class IconColorRole { Foreground, Accent };

/// Un point en espace normalisé [0,1] x [0,1] (origine en haut à gauche).
struct IconPoint {
    float x = 0.0f;
    float y = 0.0f;
};

/// Un trait de l'icône : polyligne ouverte, ou polygone fermé (éventuellement rempli), d'une seule
/// couleur.
struct IconStroke {
    std::vector<IconPoint> points;
    bool closed = false;
    bool filled = false;
    IconColorRole color = IconColorRole::Foreground;
};

/// Géométrie complète d'une icône : liste ordonnée de traits.
struct IconGeometry {
    std::vector<IconStroke> strokes;
};

/**
 * @brief Décrit la géométrie d'une icône, en espace normalisé.
 *
 * Fonction **pure** : mêmes entrées, même sortie, aucune dépendance à Qt ni au GPU. Les formes
 * visent des silhouettes distinctes plutôt qu'un détail fin : elles doivent rester lisibles à la
 * plus petite taille d'icône des jetons (`SizeTokens::iconSmall`).
 */
[[nodiscard]] IconGeometry iconGeometry(IconId id);

}  // namespace hmi
