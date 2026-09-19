// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <QPixmap>
#include <QString>
#include <QWidget>
#include <filesystem>
#include <vector>

#include "Core/Levels/TileType.h"
#include "Editor/Logic/PieceCatalog.h"

/**
 * @file Editor/Ui/PalettePanel.h
 * @brief Panneau « Palette » : les pièces du lieu, les types de tuile, et la gomme.
 */

class QEvent;
class QLineEdit;
class QModelIndex;
class QStandardItemModel;
class QTabWidget;
class QToolButton;
class QTreeView;

namespace hmi {

/**
 * @brief La palette : **la planche du lieu** d'abord, les types en repli (`LOT-EDITOR-03`).
 *
 * Deux onglets et une gomme :
 *
 * - **Pieces** — le catalogue du lieu (`hmi::pieceCatalog`) : vignettes groupées par classe, sous
 *   le nom court que la carte écrit, une recherche, et à part les pièces que la carte cite et que
 *   la planche n'a plus, en damier (`EX-EDIT-063`). Choisir une pièce arme le pinceau de pièce ;
 *   la pièce va d'elle-même sur sa couche.
 * - **Types** — la taxonomie des types de tuile (`hmi::tileTaxonomy`), en arbre : le repli d'une
 *   carte sans lieu, et la collision (`EX-EDIT-018`). Sans lieu, l'onglet des pièces est éteint.
 * - **Eraser** — la gomme de la couche active.
 *
 * Le panneau ne connaît ni brouillon ni canevas : il émet ce qu'on choisit.
 */
class PalettePanel : public QWidget {
    Q_OBJECT

public:
    explicit PalettePanel(QWidget* parent = nullptr);

    /// @return Le type de tuile actuellement sélectionné (`Solid` par défaut).
    [[nodiscard]] core::TileType selectedTile() const noexcept {
        return _selected;
    }

    /**
     * @brief Montre le catalogue d'un lieu. Sans effet si rien n'a changé : le modèle n'est refait
     *        (et la sélection perdue) que quand le lieu ou ses pièces changent.
     * @param catalog        Le catalogue ; vide pour une carte sans lieu.
     * @param placeDirectory Le dossier des images du lieu (`Assets/Scene/<lieu>`).
     */
    void setPieceCatalog(std::vector<PieceCatalogGroup> catalog,
                         const std::filesystem::path& placeDirectory);

signals:
    /// Émis quand l'utilisateur sélectionne une tuile (feuille) dans l'arbre des types.
    void tileSelected(core::TileType type);
    /// Émis quand l'utilisateur choisit une pièce ; @p floor : elle va sur la couche de sol.
    void pieceSelected(const QString& piece, bool floor);
    /// Émis quand l'utilisateur prend la gomme.
    void eraserSelected();

protected:
    /// Régénère les vignettes lors d'un changement d'écran (`QEvent::ScreenChangeInternal`) :
    /// l'échelle d'affichage a pu changer.
    bool event(QEvent* event) override;

private:
    void buildModel();
    void buildPieceModel();
    void onCurrentChanged(const QModelIndex& current);
    void onPieceChanged(const QModelIndex& current);
    /// Relâche la gomme sans rien émettre : un choix de la palette arme son propre pinceau.
    void releaseEraser();
    /// Vignette d'un type : sa couleur dans l'atlas procédural.
    [[nodiscard]] QPixmap thumbnailFor(core::TileType type);
    /// Vignette d'une pièce : son image, réduite dans un carré ; le damier si elle manque.
    [[nodiscard]] QPixmap pieceThumbnail(const PieceCatalogEntry& entry) const;

    QToolButton* _eraser;
    QTabWidget* _tabs;
    QWidget* _piecesPage;
    QLineEdit* _search;
    QTreeView* _pieceTree;
    QStandardItemModel* _pieceModel;
    QTreeView* _tree;
    QStandardItemModel* _model;
    core::TileType _selected = core::TileType::Solid;
    std::vector<PieceCatalogGroup> _catalog;
    std::filesystem::path _placeDirectory;
    /// La pièce choisie, gardée d'une recherche à l'autre.
    QString _selectedPiece;
    bool _selectedPieceFloor = false;
};

}  // namespace hmi
