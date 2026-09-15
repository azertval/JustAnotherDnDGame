// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <cstddef>

#include "HMI/Editor/EditorTool.h"
#include "HMI/Editor/PanelFocus.h"
#include "HMI/Editor/PixelTool.h"

/**
 * @file HMI/Interface/EditorWorkspace.h
 * @brief Espaces de travail exclusifs de l'éditeur (`LOT-68`, `EX-IHM-073`).
 *
 * Logique **pure** (aucune dépendance Qt), testable hors instance d'application (`EX-NFR-010`) —
 * même patron que `hmi::dressingFor` (`ScreenFlow.h`) et `hmi::panelForTool` (`PanelFocus.h`).
 * `MainWindow` ne fait que suivre ces tables : aucune condition écrite en dur sur un dock ou une
 * barre d'outils en particulier.
 *
 * L'éditeur affichait jusqu'ici ses **neuf** docks et ses **deux** barres d'outils simultanément,
 * soit une trentaine de contrôles permanents, alors que l'édition de niveau et l'atelier pixel art
 * ne se pratiquent jamais en même temps : les deux tiers de l'interface étaient hors sujet à tout
 * instant.
 *
 * À ne pas confondre avec la **mise en avant** de `EX-IHM-061`, qui est une suggestion et ne masque
 * jamais rien. Changer d'espace est un acte explicite de l'utilisateur, et c'est ce qui rend le
 * masquage légitime ici.
 */

namespace hmi {

/// Espace de travail actif. Ils s'excluent : aucun état intermédiaire n'existe.
enum class EditorWorkspace {
    Level,     ///< Édition de niveau.
    Planes,    ///< Mode création : peinture des plans picturaux (`LOT-69`, `EX-EDIT-046`).
    PixelArt,  ///< Atelier pixel art.
};

/// Nombre d'espaces, déclaré au plus près de l'énumération qu'il compte.
inline constexpr std::size_t EDITOR_WORKSPACE_COUNT = 3;

/// Ensemble d'espaces, un bit par valeur de `hmi::EditorWorkspace`.
using EditorWorkspaceMask = unsigned;

/// @return Le masque ne contenant que @p workspace.
[[nodiscard]] constexpr EditorWorkspaceMask workspaceBit(EditorWorkspace workspace) noexcept {
    return 1u << static_cast<unsigned>(workspace);
}

/// @return `true` si @p mask contient @p workspace.
[[nodiscard]] constexpr bool workspaceMaskContains(EditorWorkspaceMask mask,
                                                   EditorWorkspace workspace) noexcept {
    return (mask & workspaceBit(workspace)) != 0u;
}

/// Ce qu'un espace affiche, hors panneaux (voir `workspacesForPanel`). Même patron que
/// `hmi::ScreenDressing` : une table décide, la fenêtre applique.
struct WorkspaceDressing {
    bool levelToolBarVisible = false;
    bool pixelToolBarVisible = false;
    bool workshopMenuVisible = false;

    [[nodiscard]] friend bool operator==(const WorkspaceDressing&,
                                         const WorkspaceDressing&) noexcept = default;
};

/// @return Les barres et menus visibles dans @p workspace. Jamais les deux barres d'outils à la
/// fois : c'est la propriété que le lot existe pour établir.
[[nodiscard]] WorkspaceDressing dressingForWorkspace(EditorWorkspace workspace) noexcept;

/**
 * @brief Espaces dans lesquels @p panel est affiché.
 *
 * **Un masque, pas une valeur unique** (`LOT-69` TACHE-08) : un panneau peut servir à plusieurs
 * espaces à la fois — le canevas, l'historique et la palette appartiennent aussi bien à l'atelier
 * pixel art qu'au mode création. Les dupliquer par espace coûterait deux canevas, deux
 * historiques et deux états à tenir synchronisés.
 *
 * GARDE DE COMPLÉTUDE : le masque d'un panneau n'est **jamais vide**. Un panneau sans espace
 * resterait affiché partout, ce qui viderait la séparation de son sens — et ne se verrait qu'à
 * l'écran, jamais en relecture.
 * @param panel Panneau interrogé.
 * @return Le masque des espaces qui l'affichent ; jamais vide.
 */
[[nodiscard]] EditorWorkspaceMask workspacesForPanel(PanelId panel) noexcept;

/// @return L'espace auquel appartient @p tool — toujours `Level`, les deux groupes d'outils étant
/// disjoints par construction. Existe pour que `MainWindow` bascule d'espace en suivant une table
/// plutôt qu'une condition écrite en dur.
[[nodiscard]] EditorWorkspace workspaceForTool(EditorTool tool) noexcept;

/// @return L'espace auquel appartient @p tool — toujours `PixelArt`.
[[nodiscard]] EditorWorkspace workspaceForPixelTool(PixelTool tool) noexcept;

}  // namespace hmi
