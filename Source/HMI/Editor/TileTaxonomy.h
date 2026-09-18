// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <string>
#include <vector>

#include "Core/Levels/TileType.h"

/**
 * @file HMI/Editor/TileTaxonomy.h
 * @brief Organisation des types de tuiles en catégories/sous-groupes pour la palette.
 */

namespace hmi {

/// Une tuile sélectionnable : son type et son libellé affiché.
struct TileEntry {
    core::TileType type;
    std::string label;
};

/// Un sous-groupe : une famille de variantes (formes/tailles/orientations) sous une catégorie.
struct TileSubgroup {
    std::string label;
    std::vector<TileEntry> tiles;
};

/// Une catégorie de premier niveau : ses tuiles directes + ses sous-groupes repliables.
struct TileCategory {
    std::string label;
    std::vector<TileEntry> tiles;
    std::vector<TileSubgroup> subgroups;
};

/**
 * @brief Taxonomie complète des types de tuiles peignables, en catégories/sous-groupes.
 *
 * Logique **pure** (aucune dépendance Qt/GPU), testable (`EX-NFR-010`) : sert de source unique à la
 * palette `QTreeView` de l'éditeur (`EX-EDIT-018`, `EX-IHM-010`). **Chaque** `core::TileType` y
 * figure **exactement une fois**, dans un ordre déterministe (invariant vérifié par les tests).
 * Reprend l'organisation de la palette historique.
 */
[[nodiscard]] std::vector<TileCategory> tileTaxonomy();

}  // namespace hmi
