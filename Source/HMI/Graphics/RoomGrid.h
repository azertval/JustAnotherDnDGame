// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include "Core/Levels/GridPosition.h"

/**
 * @file HMI/Graphics/RoomGrid.h
 * @brief Partition d'un niveau en salles de taille fixe, pour le cadrage caméra (LOT-32).
 */

namespace hmi {

/// Rectangle (en cases) d'une salle, borné aux dimensions réelles du niveau.
struct RoomBounds {
    int column = 0;
    int row = 0;
    int width = 0;
    int height = 0;
};

/**
 * @brief Partitionne une grille de niveau en un tableau de **salles** de taille fixe, façon
 *        *Celeste* (`LOT-32`).
 *
 * Le niveau reste une grille de tuiles **unique** (`core::TileMap`, `LOT-07`/`16`, sans limite de
 * taille) ; `RoomGrid` n'en est qu'une **projection géométrique** dérivée, utilisée par la caméra
 * du jeu (cadrage sur la salle courante) et le repère visuel de l'éditeur — aucune donnée de
 * niveau n'est dupliquée ni modifiée.
 *
 * La taille de salle est **réglable par niveau** depuis le `LOT-64` (`core::CameraFramingConfig`,
 * mode *par salle*) ; `ROOM_WIDTH_TILES`/`ROOM_HEIGHT_TILES` n'en restent que la **valeur par
 * défaut**, plus la seule vérité. Si les dimensions du niveau ne sont pas un multiple exact de la
 * taille de salle retenue, la dernière colonne/ligne de salles est **rognée** à la taille réelle
 * restante — jamais agrandie au-delà des bornes du niveau.
 *
 * Logique **pure**, sans dépendance rendu — testable sans GPU (`EX-NFR-010`).
 */
class RoomGrid {
public:
    /// Largeur de salle par défaut, en cases (`EX-REN-015`) — mêmes valeurs que
    /// `core::DEFAULT_ROOM_WIDTH_TILES` (`Core`, LOT-64) : deux constantes distinctes plutôt qu'une
    /// dépendance de `HMI` vers `Core` pour une seule valeur (`RoomGrid` précède `Core::Levels`
    /// historiquement) ; `Source/Test/Unit/Core/Levels/test_camera_framing.cpp` vérifie qu'elles
    /// restent égales.
    static constexpr int ROOM_WIDTH_TILES = 24;
    /// Hauteur de salle par défaut, en cases (`EX-REN-015`).
    static constexpr int ROOM_HEIGHT_TILES = 14;

    /**
     * @brief Construit la partition pour un niveau de @p levelWidth × @p levelHeight cases.
     * @param levelWidth   Largeur du niveau, en cases (> 0).
     * @param levelHeight  Hauteur du niveau, en cases (> 0).
     * @param roomWidthTiles  Largeur de salle à utiliser, en cases (> 0) ; `ROOM_WIDTH_TILES` par
     *                     défaut (`LOT-64` : taille personnalisable par niveau).
     * @param roomHeightTiles Hauteur de salle à utiliser, en cases (> 0) ; `ROOM_HEIGHT_TILES` par
     *                     défaut.
     */
    RoomGrid(int levelWidth, int levelHeight, int roomWidthTiles = ROOM_WIDTH_TILES,
             int roomHeightTiles = ROOM_HEIGHT_TILES);

    /// @return La largeur de salle utilisée par cette partition, en cases.
    [[nodiscard]] int roomWidthTiles() const noexcept {
        return _roomWidthTiles;
    }

    /// @return La hauteur de salle utilisée par cette partition, en cases.
    [[nodiscard]] int roomHeightTiles() const noexcept {
        return _roomHeightTiles;
    }

    /// @return Le nombre de salles sur l'axe horizontal (au moins 1).
    [[nodiscard]] int columns() const noexcept {
        return _columns;
    }

    /// @return Le nombre de salles sur l'axe vertical (au moins 1).
    [[nodiscard]] int rows() const noexcept {
        return _rows;
    }

    /**
     * @brief Indice (colonne, ligne) de la salle contenant @p tile.
     *
     * Une position hors des bornes du niveau est **bornée** à la salle la plus proche (jamais de
     * comportement indéfini) : colonnes/lignes négatives ramenées à 0, au-delà de la dernière
     * salle ramenées à `columns() - 1`/`rows() - 1`.
     * @param tile Position de case (colonne, ligne).
     * @return L'indice de la salle contenant (ou la plus proche de) @p tile.
     */
    [[nodiscard]] core::GridPosition roomIndexAt(core::GridPosition tile) const noexcept;

    /**
     * @brief Rectangle (en cases) de la salle d'indice @p roomIndex.
     *
     * @pre `roomIndex` est dans `[0, columns())` × `[0, rows())`.
     * @param roomIndex Indice de salle (colonne, ligne), tel que renvoyé par `roomIndexAt`.
     * @return Le rectangle de la salle, rogné aux bornes du niveau sur la dernière colonne/ligne.
     */
    [[nodiscard]] RoomBounds roomBounds(core::GridPosition roomIndex) const noexcept;

private:
    int _levelWidth;
    int _levelHeight;
    int _roomWidthTiles;
    int _roomHeightTiles;
    int _columns;
    int _rows;
};

}  // namespace hmi
