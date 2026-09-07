// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Interface/OrnamentPainter.h"

#include <QColor>
#include <QPainter>
#include <QPen>
#include <QPoint>
#include <QPolygonF>
#include <algorithm>

#include "HMI/Interface/ApplicationTheme.h"
#include "HMI/Interface/DesignTokens.h"

namespace hmi {

namespace {

[[nodiscard]] QColor resolve(OrnamentRole role) {
    const ColorTokens& color = identityTokens().color;
    switch (role) {
        case OrnamentRole::Edge:
            return QColor(color.frameEdge.r, color.frameEdge.g, color.frameEdge.b);
        case OrnamentRole::Ornament:
            return QColor(color.frameOrnament.r, color.frameOrnament.g, color.frameOrnament.b);
        case OrnamentRole::OrnamentLight:
            return QColor(color.accentHover.r, color.accentHover.g, color.accentHover.b);
        case OrnamentRole::Gem:
            return QColor(color.gem.r, color.gem.g, color.gem.b);
        case OrnamentRole::GemShadow:
            return QColor(color.gemShadow.r, color.gemShadow.g, color.gemShadow.b);
    }
    return QColor(color.frameOrnament.r, color.frameOrnament.g, color.frameOrnament.b);
}

}  // namespace

void paintOrnaments(QPainter& painter, const std::vector<OrnamentShape>& shapes,
                    const QPoint& origin) {
    if (shapes.empty()) {
        return;
    }
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);

    // Le filet suit le facteur d'agrandissement, comme toutes les longueurs de l'identite
    // (EX-IHM-081) : un contour d'un pixel disparaitrait sur un ecran ou tout le reste a triple.
    const qreal penWidth = static_cast<qreal>(std::max(1, identityScale()));

    for (const OrnamentShape& shape : shapes) {
        if (shape.points.size() < 3) {
            continue;
        }
        QPolygonF polygon;
        polygon.reserve(static_cast<int>(shape.points.size()));
        for (const OrnamentPoint& point : shape.points) {
            polygon << QPointF(origin.x() + point.x, origin.y() + point.y);
        }
        const QColor color = resolve(shape.role);
        if (shape.filled) {
            painter.setPen(Qt::NoPen);
            painter.setBrush(color);
        } else {
            painter.setPen(QPen(color, penWidth, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin));
            painter.setBrush(Qt::NoBrush);
        }
        painter.drawPolygon(polygon);
    }
    painter.restore();
}

}  // namespace hmi
