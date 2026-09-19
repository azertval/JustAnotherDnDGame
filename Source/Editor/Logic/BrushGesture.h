// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <string>
#include <vector>

#include "Core/Levels/GridPosition.h"
#include "Core/Levels/TileType.h"
#include "Editor/Logic/LayerView.h"

/**
 * @file Editor/Logic/BrushGesture.h
 * @brief Ce que fait un coup de pinceau sur le brouillon : un type, une pièce ou la gomme, sur
 *        une case ou un rectangle (`LOT-EDITOR-03`, `EX-EDIT-064`, `EX-EDIT-065`).
 *
 * Fonctions pures : le canevas les appelle au clic et au glisser, et l'éditeur sans fenêtre
 * (`LOT-EDITOR-13`, `hmi::applyGestureScript`) les appelle telles quelles (règle 4 de la feuille
 * de route). Chaque appel est **un** pas d'annulation, ou rien.
 *
 * ## Où va le geste
 *
 * - Un **type** se peint sur la couche active (la collision ou une couche visuelle).
 * - Une **pièce** va sur **sa** couche, quelle que soit la couche active : un sol sur la première
 *   couche de sol, le reste sur la première de décor (`hmi::pieceTargetLayer`). Elle écrit sa
 *   pièce, le type de sa case et la collision de son emprise.
 * - La **gomme** vise la couche active : sur une couche visuelle, elle retire la pièce entière qui
 *   couvre la case, ou son type ; sur la collision, elle **libère** les cases forcées, qui
 *   reprennent la déduction.
 *
 * Une couche verrouillée refuse tout geste (`EX-EDIT-061`).
 */

namespace core {
class LevelDraft;
}

namespace hmi {

/// @brief Ce que le pinceau pose.
enum class BrushKind {
    /// Un type de tuile (la palette des types).
    Type,
    /// Une pièce de la planche du lieu.
    Piece,
    /// La gomme.
    Eraser,
};

/// @brief Le pinceau armé.
struct CanvasBrush {
    BrushKind kind = BrushKind::Type;
    /// Le type peint (`Type`), ou celui qu'écrit la case d'ancrage d'une pièce (`Piece`,
    /// `hmi::pieceCellType`).
    core::TileType type = core::TileType::Solid;
    /// La pièce posée (`Piece`).
    std::string piece;
    /// La pièce est un sol : elle va sur la couche de sol.
    bool floor = false;

    [[nodiscard]] bool operator==(const CanvasBrush&) const = default;
};

/// @brief Ce qu'un geste a fait.
struct BrushResult {
    /// Le brouillon a changé (un pas d'annulation de plus).
    bool changed = false;
    /// Le geste est refusé, et pourquoi (en anglais, pour la barre d'état) ; vide sinon.
    std::string refusal;

    [[nodiscard]] bool operator==(const BrushResult&) const = default;
};

/// @return Ce que la barre d'état dit du pinceau : la pièce, le type, ou `Eraser`.
[[nodiscard]] std::string brushLabel(const CanvasBrush& brush);

/**
 * @brief Applique @p brush au rectangle [@p first, @p last] (bornes incluses, dans n'importe quel
 *        ordre).
 * @param draft      Le brouillon.
 * @param brush      Le pinceau.
 * @param active     La couche active.
 * @param view       Les réglages des couches (verrous).
 * @param first      Un coin du rectangle.
 * @param last       Le coin opposé (égal à @p first pour une seule case).
 * @param continuing Le geste prolonge un glisser : une pièce ne se repose pas sur une case que la
 *                   même pièce couvre déjà, sans quoi glisser un étal 2 × 1 le décalerait d'une
 * case à chaque pas.
 */
BrushResult applyBrush(core::LevelDraft& draft, const CanvasBrush& brush, LayerSlot active,
                       const LayerViewState& view, core::GridPosition first,
                       core::GridPosition last, bool continuing = false);

/**
 * @brief Peint un bloc de types (`[ligne][colonne]`) à partir de @p origin sur la couche active,
 *        en un pas : le rectangle du pinceau et le collage.
 */
BrushResult paintTypeBlock(core::LevelDraft& draft, LayerSlot active, const LayerViewState& view,
                           core::GridPosition origin,
                           const std::vector<std::vector<core::TileType>>& block);

}  // namespace hmi
