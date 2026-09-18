// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <array>
#include <optional>

#include "Core/Levels/GridPosition.h"
#include "Core/Math/Rect.h"
#include "Core/Math/Vector2.h"

/**
 * @file Editor/Logic/CanvasPicking.h
 * @brief Le pointage du canevas : d'un point du monde à la case qu'il désigne, dans la vue
 *        isométrique comme dans la vue à plat (`LOT-EDITOR-02`, décision D1, `EX-EDIT-060`).
 *
 * ## Deux vues, un repère de cases
 *
 * La vue **iso** dessine le lieu comme le jeu : `core::IsoProjection`, losanges de rapport 0,62,
 * pièces debout qui montent au-dessus de leur case. La vue **à plat** dessine une case par unité
 * monde, `(colonne, ligne)` = `(x, y)`. Le canevas ne connaît qu'une chose de la vue : ces
 * fonctions ; tout le reste — gestes, outils, brouillon — parle en cases.
 *
 * ## Un relief haut se désigne par son pied
 *
 * On pointe le **losange** d'une case, jamais l'image qui la couvre : un clic sur le haut d'un mur
 * désigne la case derrière le mur, celle dont le losange est sous le pointeur. C'est la règle qui
 * rend le pointage prévisible sous un relief haut ; la transparence des reliefs (phase 3) permet de
 * voir ce qu'on pointe.
 *
 * ## La hauteur, réservée (D11)
 *
 * Le format v4 réservera une `elevation` par case. Le pointage la prend donc en paramètre dès
 * maintenant : une case élevée de `e` niveaux est dessinée `e × ELEVATION_STEP_DIAMONDS` hauteurs
 * de losange plus haut, et se pointe là. Ni le jeu ni l'éditeur ne s'en servent : ils passent 0.
 *
 * Logique pure, sans Qt : testée par `UnitTests` (`Unit/Editor/test_canvas_picking.cpp`).
 */

namespace core {
class IsoProjection;
}

namespace hmi {

/// @brief Les deux vues du canevas.
enum class CanvasView {
    /// Le lieu tel que le jeu le montre (défaut).
    Iso,
    /// Une case par unité, les types en couleurs : pour lire types et collision.
    Flat,
};

/// @brief Hauteur d'un niveau d'élévation, en hauteurs de losange (réserve D11, jamais jouée).
inline constexpr float ELEVATION_STEP_DIAMONDS = 1.0F;

/// @brief Un rectangle de cases, bornes **incluses** ; vide si `lastColumn < firstColumn`.
struct CellRange {
    /// Première colonne couverte.
    int firstColumn = 0;
    /// Première ligne couverte.
    int firstRow = 0;
    /// Dernière colonne couverte, incluse.
    int lastColumn = -1;
    /// Dernière ligne couverte, incluse.
    int lastRow = -1;

    /// @return Vrai si le rectangle ne couvre aucune case.
    [[nodiscard]] bool empty() const noexcept {
        return lastColumn < firstColumn || lastRow < firstRow;
    }
    /// @return Vrai si @p cell est dans le rectangle.
    [[nodiscard]] bool contains(core::GridPosition cell) const noexcept {
        return cell.column >= firstColumn && cell.column <= lastColumn && cell.row >= firstRow &&
               cell.row <= lastRow;
    }
    [[nodiscard]] bool operator==(const CellRange&) const = default;
};

/**
 * @brief Le point de grille **continu** sous un point du monde, en vue iso.
 * @param projection Projection iso du lieu.
 * @param world Point du monde.
 * @param elevation Niveau de la case pointée (0 : le sol).
 */
[[nodiscard]] core::Vector2 isoGridPoint(const core::IsoProjection& projection, core::Vector2 world,
                                         int elevation = 0) noexcept;

/**
 * @brief La case dont le losange contient @p world, en vue iso.
 * @return La case, ou rien hors de la grille.
 */
[[nodiscard]] std::optional<core::GridPosition> pickIsoCell(const core::IsoProjection& projection,
                                                            core::Vector2 world,
                                                            int elevation = 0) noexcept;

/// @brief Comme `pickIsoCell`, ramenée dans la grille : la case la plus proche le long de chaque
///        axe de la grille (glisser un rectangle hors de la carte le borne au bord).
[[nodiscard]] core::GridPosition clampedIsoCell(const core::IsoProjection& projection,
                                                core::Vector2 world, int elevation = 0) noexcept;

/// @return La case sous @p world en vue à plat, ou rien hors d'une grille de @p columns × @p rows.
[[nodiscard]] std::optional<core::GridPosition> pickFlatCell(core::Vector2 world, int columns,
                                                             int rows) noexcept;

/// @return La case sous @p world en vue à plat, ramenée dans la grille.
[[nodiscard]] core::GridPosition clampedFlatCell(core::Vector2 world, int columns,
                                                 int rows) noexcept;

/**
 * @brief Les quatre sommets du losange d'une case, en unités monde : haut, droit, bas, gauche.
 * @param projection Projection iso du lieu.
 * @param cell Case dont on veut le losange.
 * @param elevation Niveau de la case (0 : le sol) ; le losange monte d'autant.
 */
[[nodiscard]] std::array<core::Vector2, 4> isoCellDiamond(const core::IsoProjection& projection,
                                                          core::GridPosition cell,
                                                          int elevation = 0) noexcept;

/**
 * @brief Les cases dont le losange peut couper @p world — celles qu'il faut dessiner pour une vue
 *        cadrée sur ce rectangle. Conservateur : ne manque aucune case visible, peut en compter
 *        quelques-unes de trop dans les coins.
 */
[[nodiscard]] CellRange isoCellsCovering(const core::IsoProjection& projection,
                                         const core::Rect& world) noexcept;

/// @return Les cases qu'un rectangle du monde couvre en vue à plat, bornées à la grille.
[[nodiscard]] CellRange flatCellsCovering(const core::Rect& world, int columns, int rows) noexcept;

}  // namespace hmi
