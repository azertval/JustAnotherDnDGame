// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "HMI/Interface/ThemeIcons.h"

#include <QColor>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <algorithm>

#include "HMI/Interface/DesignTokens.h"

namespace hmi {

namespace {

[[nodiscard]] QColor toQColor(DesignColor color) {
    return {color.r, color.g, color.b, color.a};
}

[[nodiscard]] QColor resolveColor(IconColorRole role, const DesignTokens& tokens) {
    switch (role) {
        case IconColorRole::Foreground:
            return toQColor(tokens.color.text);
        case IconColorRole::Accent:
            return toQColor(tokens.color.accent);
    }
    return toQColor(tokens.color.text);
}

[[nodiscard]] QPainterPath strokePath(const IconStroke& stroke, int pixelSize) {
    QPainterPath path;
    if (stroke.points.empty()) {
        return path;
    }
    const auto toPixel = [pixelSize](IconPoint p) {
        return QPointF(p.x * static_cast<float>(pixelSize), p.y * static_cast<float>(pixelSize));
    };
    path.moveTo(toPixel(stroke.points.front()));
    for (std::size_t i = 1; i < stroke.points.size(); ++i) {
        path.lineTo(toPixel(stroke.points[i]));
    }
    if (stroke.closed) {
        path.closeSubpath();
    }
    return path;
}

}  // namespace

QIcon themeIcon(IconId id, int pixelSize, const DesignTokens& tokens) {
    const int size = std::max(1, pixelSize);
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);
    const qreal penWidth = std::max(1.0, static_cast<double>(size) / 12.0);

    for (const IconStroke& stroke : iconGeometry(id).strokes) {
        const QColor color = resolveColor(stroke.color, tokens);
        const QPainterPath path = strokePath(stroke, size);
        if (stroke.filled) {
            painter.setPen(Qt::NoPen);
            painter.setBrush(color);
        } else {
            painter.setPen(QPen(color, penWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            painter.setBrush(Qt::NoBrush);
        }
        painter.drawPath(path);
    }

    return QIcon(pixmap);
}

}  // namespace hmi
