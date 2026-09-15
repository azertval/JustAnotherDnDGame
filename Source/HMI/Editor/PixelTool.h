// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

/**
 * @file HMI/Editor/PixelTool.h
 * @brief Outil actif du canevas pixel art (`LOT-54` TACHE-03/TACHE-04).
 *
 * Dans son propre en-tête, sans dépendance Qt — comme `hmi::EditorTool` (`HMI/Editor/
 * EditorTool.h`) — pour que la logique pure qui en dépend (`hmi::EditorStatus`, `hmi::PanelFocus`)
 * n'ait pas à inclure `PixelCanvas.h` (widget Qt).
 */

namespace hmi {

/**
 * @brief Outil actif du canevas pixel art — détermine ce que produisent les gestes de souris
 *        (`hmi::PixelCanvas`) et forme un groupe exclusif distinct de `hmi::EditorTool`
 *        (`hmi::EditorActions`, TACHE-04).
 */
enum class PixelTool {
    Brush,
    Eraser,
    Fill,
    Eyedropper,
    /// Sélection rectangulaire (`LOT-54` TACHE-06) : glisser hors d'une sélection existante en
    /// définit une nouvelle ; glisser à l'intérieur la déplace (`hmi::moveRegion`).
    Selection,
};

}  // namespace hmi
