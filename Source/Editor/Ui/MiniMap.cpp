// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "Editor/Ui/MiniMap.h"

#include <QMouseEvent>
#include <QPainter>
#include <algorithm>
#include <utility>

#include "Core/Levels/LevelDraft.h"
#include "Core/Levels/TileLayer.h"
#include "Core/Levels/TileMap.h"

namespace hmi {

MiniMap::MiniMap(TileColor tileColor, QWidget* parent)
    : QWidget(parent), _tileColor(std::move(tileColor)) {
    setMinimumSize(120, 100);
    setCursor(Qt::CrossCursor);
    setToolTip(QStringLiteral("Click or drag to move the view"));
}

QSize MiniMap::sizeHint() const {
    return {220, 190};
}

void MiniMap::setDraft(const core::LevelDraft& draft) {
    const core::TileMap& root = draft.tileMap();
    // La matière : la première couche de sol, à défaut la grille racine (celle que le jeu lit).
    const core::TileMap* ground = &root;
    for (const core::TileLayer& layer : draft.layers()) {
        if (layer.kind == core::LayerKind::Ground) {
            ground = &layer.tiles;
            break;
        }
    }
    QImage image(std::max(1, root.width()), std::max(1, root.height()),
                 QImage::Format_ARGB32_Premultiplied);
    image.fill(QColor(0x1e, 0x22, 0x2b));
    for (int row = 0; row < root.height(); ++row) {
        for (int column = 0; column < root.width(); ++column) {
            const core::TileType type =
                ground->inBounds(column, row) ? ground->tile(column, row) : core::TileType::Empty;
            QColor color =
                type == core::TileType::Empty ? QColor(0x1e, 0x22, 0x2b) : _tileColor(type);
            // Ce qui arrête le pas se lit d'un coup d'œil : foncé.
            if (core::isSolid(root.tile(column, row))) {
                color = color.darker(260);
            }
            image.setPixelColor(column, row, color);
        }
    }
    _image = std::move(image);
    update();
}

void MiniMap::setVisibleCorners(const std::array<core::Vector2, 4>& corners) {
    QPolygonF frame;
    for (const core::Vector2& corner : corners) {
        frame << QPointF(corner.x, corner.y);
    }
    if (frame != _frame) {
        _frame = std::move(frame);
        update();
    }
}

QRectF MiniMap::imageRect() const {
    if (_image.isNull()) {
        return {};
    }
    const double scale = std::min(static_cast<double>(width()) / _image.width(),
                                  static_cast<double>(height()) / _image.height());
    const double w = _image.width() * scale;
    const double h = _image.height() * scale;
    return {(width() - w) / 2.0, (height() - h) / 2.0, w, h};
}

void MiniMap::paintEvent(QPaintEvent* /*event*/) {
    QPainter painter(this);
    painter.fillRect(rect(), palette().window());
    const QRectF target = imageRect();
    if (target.isEmpty()) {
        return;
    }
    painter.drawImage(target, _image);
    // Le cadre de la vue, en cases : une case = (target / taille de l'image).
    painter.save();
    painter.setClipRect(target);
    painter.translate(target.topLeft());
    painter.scale(target.width() / _image.width(), target.height() / _image.height());
    QPen pen(QColor(255, 236, 140), 2.0);
    pen.setCosmetic(true);
    painter.setPen(pen);
    painter.setBrush(QColor(255, 236, 140, 40));
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.drawPolygon(_frame);
    painter.restore();
}

void MiniMap::requestCenter(const QPointF& position) {
    const QRectF target = imageRect();
    if (target.isEmpty()) {
        return;
    }
    const auto column =
        static_cast<float>((position.x() - target.left()) / target.width() * _image.width());
    const auto row =
        static_cast<float>((position.y() - target.top()) / target.height() * _image.height());
    emit centerRequested({column, row});
}

void MiniMap::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        requestCenter(event->position());
    }
}

void MiniMap::mouseMoveEvent(QMouseEvent* event) {
    if (event->buttons().testFlag(Qt::LeftButton)) {
        requestCenter(event->position());
    }
}

}  // namespace hmi
