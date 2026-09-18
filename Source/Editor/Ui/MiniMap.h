// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <QColor>
#include <QImage>
#include <QPolygonF>
#include <QWidget>
#include <array>
#include <functional>

#include "Core/Levels/TileType.h"
#include "Core/Math/Vector2.h"

/**
 * @file Editor/Ui/MiniMap.h
 * @brief La mini-carte du canevas : toute la carte à plat, un pixel par case, et le cadre de ce que
 *        la vue montre (`LOT-EDITOR-02`, phase 3, `EX-EDIT-061`).
 *
 * Martpart fait 48 × 40 cases : dans la vue iso agrandie, on ne voit qu'un quartier. La mini-carte
 * dit où l'on est et ramène la vue d'un clic. Elle montre la **matière** de chaque case (le type de
 * la première couche de sol, à défaut la grille racine) et fonce ce qui arrête le pas ; le cadre
 * est un losange quand la vue est iso, puisque l'écran y est tourné par rapport à la grille.
 */

namespace core {
class LevelDraft;
}

namespace hmi {

/// @brief La mini-carte : l'image de la carte, le cadre de la vue, et le clic qui la déplace.
class MiniMap : public QWidget {
    Q_OBJECT

public:
    /// La couleur d'un type de tuile (celle de la vue à plat).
    using TileColor = std::function<QColor(core::TileType)>;

    explicit MiniMap(TileColor tileColor, QWidget* parent = nullptr);

    /// Refait l'image de la carte.
    void setDraft(const core::LevelDraft& draft);
    /// Le cadre de la vue : quatre points de grille continus (colonne, ligne).
    void setVisibleCorners(const std::array<core::Vector2, 4>& corners);

    [[nodiscard]] QSize sizeHint() const override;

signals:
    /// Un clic demande de centrer la vue sur ce point de grille (colonne, ligne).
    void centerRequested(core::Vector2 gridPoint);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;

private:
    /// Le rectangle du widget où l'image est posée, proportions gardées.
    [[nodiscard]] QRectF imageRect() const;
    void requestCenter(const QPointF& position);

    TileColor _tileColor;
    QImage _image;
    QPolygonF _frame;
};

}  // namespace hmi
