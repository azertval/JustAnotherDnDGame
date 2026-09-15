// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <optional>

#include "HMI/Editor/PixelOperations.h"
#include "HMI/Graphics/TileAutotile.h"

/**
 * @file HMI/Editor/PixelAutotilePreview.h
 * @brief Mode planche à raccords de l'atelier pixel art : détection, case survolée et aperçu
 *        d'assemblage (`LOT-54` TACHE-08, `EX-EDIT-025`).
 *
 * Fonctions **pures** (aucune dépendance Qt/GPU, `EX-NFR-010`), qui n'utilisent jamais qu'une
 * seule table de raccords : celle de `HMI/Graphics/TileAutotile.h` (`LOT-42`). Une planche
 * bitmask16 fait exactement `AUTOTILE_SHEET_SIDE` × `AUTOTILE_SHEET_SIDE` cases — c'est ce qui
 * distingue une planche à raccords d'un autre asset de la même famille (`EX-REN-007` autorise des
 * planches plus grandes).
 */

namespace hmi {

/**
 * @brief `true` si @p image a exactement les dimensions d'une planche à raccords bitmask16.
 * @param image    Image consultée.
 * @param tileSize Côté d'une case, en pixels (`TextureAtlas::TILE_SIZE`).
 */
[[nodiscard]] bool isBitmask16Candidate(const DecodedImage& image, int tileSize) noexcept;

/**
 * @brief Case de la planche sous un pixel donné.
 * @param image    Image consultée.
 * @param tileSize Côté d'une case, en pixels.
 * @param x        Colonne du pixel.
 * @param y        Ligne du pixel.
 * @return La case, ou `std::nullopt` si @p image n'est pas une planche bitmask16
 *         (`isBitmask16Candidate`) ou si `(x, y)` est hors bornes.
 */
[[nodiscard]] std::optional<AutotileCell> bitmaskCellAtPixel(const DecodedImage& image,
                                                             int tileSize, int x, int y) noexcept;

/**
 * @brief Construit l'aperçu d'assemblage 3×3 démonstratif à partir de la planche en cours.
 *
 * Les repères disent à quoi *sert* chaque case ; seul cet assemblage dit si le résultat *tient*
 * (epic.md) — composé avec `hmi::autotileAssemblyMasks`/`hmi::autotileCell`, la **même** table que
 * le rendu du niveau, jamais une seconde.
 * @param sheet    Planche en cours d'édition.
 * @param tileSize Côté d'une case, en pixels.
 * @return Une image de `3 * tileSize` de côté ; une case qui déborderait de @p sheet (planche plus
 *         petite que prévu) reste transparente plutôt que de lire hors bornes.
 */
[[nodiscard]] DecodedImage buildAutotileAssemblyPreview(const DecodedImage& sheet, int tileSize);

}  // namespace hmi
