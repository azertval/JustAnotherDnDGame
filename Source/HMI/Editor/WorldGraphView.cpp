// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "HMI/Editor/WorldGraphView.h"

#include <QColor>
#include <QEvent>
#include <QFont>
#include <QFontMetrics>
#include <QHelpEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPen>
#include <QPolygonF>
#include <QStringList>
#include <QToolTip>
#include <algorithm>
#include <cmath>
#include <utility>

#include "HMI/Interface/ApplicationTheme.h"
#include "HMI/Localization/Localization.h"

namespace hmi {

namespace {

/// Marge autour du cercle réservée aux étiquettes et aux boucles, en pixels logiques.
constexpr qreal LABEL_MARGIN = 90.0;
/// Tolérance de désignation d'une flèche, en pixels du widget.
constexpr qreal EDGE_PICK_TOLERANCE = 5.0;
/// Pointe de flèche : longueur et demi-ouverture (radians).
constexpr qreal ARROW_LENGTH = 10.0;
constexpr qreal ARROW_ANGLE = 0.45;
/// Légende : hauteur d'une ligne, marge et longueur de l'échantillon.
constexpr int LEGEND_ROW = 18;
constexpr int LEGEND_PADDING = 8;
constexpr int LEGEND_SAMPLE = 26;
constexpr int LEGEND_ROWS = 5;

[[nodiscard]] QColor toQColor(DesignColor color) {
    return {color.r, color.g, color.b, color.a};
}

[[nodiscard]] QPointF toPoint(core::Vector2 vector) {
    return {static_cast<qreal>(vector.x), static_cast<qreal>(vector.y)};
}

// Pointe pleine d'une flèche arrivant en @p tip depuis @p from.
[[nodiscard]] QPolygonF arrowHead(QPointF from, QPointF tip) {
    const QPointF direction = tip - from;
    const qreal angle = std::atan2(direction.y(), direction.x());
    const auto wing = [&](qreal side) {
        return tip - QPointF(std::cos(angle + side) * ARROW_LENGTH,
                             std::sin(angle + side) * ARROW_LENGTH);
    };
    return QPolygonF({tip, wing(ARROW_ANGLE), wing(-ARROW_ANGLE)});
}

}  // namespace

WorldGraphView::WorldGraphView(QWidget* parent) : QWidget(parent) {
    setMouseTracking(true);
    setMinimumSize(160, 160);
}

QSize WorldGraphView::sizeHint() const {
    return {320, 320};
}

void WorldGraphView::setGraph(core::WorldGraph graph, std::filesystem::path levelsDir) {
    _graph = std::move(graph);
    _dir = std::move(levelsDir);
    _layout = layoutWorldGraph(_graph);
    _hoveredNode.reset();
    _hoveredEdge.reset();
    update();
}

void WorldGraphView::retranslateUi(const Localization& loc) {
    _loc = &loc;
    update();
}

QString WorldGraphView::localized(const char* key) const {
    return _loc != nullptr ? QString::fromStdString(_loc->text(key)) : QString::fromLatin1(key);
}

qreal WorldGraphView::scale() const {
    const qreal extent = 2.0 * (static_cast<qreal>(_layout.circleRadius) + LABEL_MARGIN);
    const qreal available = std::min<qreal>(width(), height() - LEGEND_ROWS * LEGEND_ROW);
    return std::clamp(available / extent, 0.05, 1.0);
}

core::Vector2 WorldGraphView::toLayout(QPointF widgetPoint) const {
    const QPointF origin(width() / 2.0, (height() - LEGEND_ROWS * LEGEND_ROW) / 2.0);
    const QPointF local = (widgetPoint - origin) / scale();
    return {static_cast<float>(local.x()), static_cast<float>(local.y())};
}

QString WorldGraphView::nodeLabel(std::size_t node) const {
    const WorldGraphLayoutNode& n = _layout.nodes.at(node);
    if (n.ghost && n.mapId.empty()) {
        return localized("world_graph.ghost.no_target");
    }
    return QString::fromStdString(n.name.empty() ? n.mapId : n.name);
}

QString WorldGraphView::statusText(core::PortalLinkStatus status) const {
    switch (status) {
        case core::PortalLinkStatus::Resolved:
            return localized("world_graph.status.resolved");
        case core::PortalLinkStatus::MissingTarget:
            return localized("world_graph.status.missing_target");
        case core::PortalLinkStatus::UnknownMap:
            return localized("world_graph.status.unknown_map");
        case core::PortalLinkStatus::MissingArrival:
            return localized("world_graph.status.missing_arrival");
        case core::PortalLinkStatus::UnknownArrival:
            return localized("world_graph.status.unknown_arrival");
        case core::PortalLinkStatus::TargetUnreadable:
            return localized("world_graph.status.target_unreadable");
    }
    return {};
}

QString WorldGraphView::nodeToolTip(std::size_t node) const {
    const WorldGraphLayoutNode& n = _layout.nodes.at(node);
    if (n.ghost) {
        return n.mapId.empty()
                   ? localized("world_graph.tooltip.ghost_no_target")
                   : localized("world_graph.tooltip.ghost").arg(QString::fromStdString(n.mapId));
    }
    QStringList lines{
        localized("world_graph.tooltip.map").arg(nodeLabel(node), QString::fromStdString(n.mapId))};
    if (n.unreadable) {
        lines
            << localized("world_graph.tooltip.unreadable").arg(QString::fromStdString(n.loadError));
    } else {
        lines << localized("world_graph.tooltip.open");
    }
    return lines.join(QLatin1Char('\n'));
}

QString WorldGraphView::edgeToolTip(std::size_t edge) const {
    const WorldGraphLayoutEdge& e = _layout.edges.at(edge);
    QStringList lines{
        localized("world_graph.tooltip.edge").arg(nodeLabel(e.from), nodeLabel(e.to))};
    for (const std::size_t index : e.portals) {
        const core::WorldPortalLink& portal = _graph.portals.at(index);
        const QString arrival = portal.arrival.empty() ? localized("world_graph.tooltip.no_arrival")
                                                       : QString::fromStdString(portal.arrival);
        lines << localized("world_graph.tooltip.portal")
                     .arg(QString::number(portal.position.column),
                          QString::number(portal.position.row), arrival, statusText(portal.status));
    }
    return lines.join(QLatin1Char('\n'));
}

bool WorldGraphView::event(QEvent* event) {
    if (event->type() == QEvent::ToolTip) {
        const auto* help = static_cast<QHelpEvent*>(event);
        const core::Vector2 point = toLayout(help->pos());
        if (const auto node = nodeAt(_layout, point, WORLD_GRAPH_NODE_RADIUS)) {
            QToolTip::showText(help->globalPos(), nodeToolTip(*node), this);
        } else if (const auto edge =
                       edgeAt(_layout, point, static_cast<float>(EDGE_PICK_TOLERANCE / scale()))) {
            QToolTip::showText(help->globalPos(), edgeToolTip(*edge), this);
        } else {
            QToolTip::hideText();
            event->ignore();
        }
        return true;
    }
    return QWidget::event(event);
}

void WorldGraphView::mouseMoveEvent(QMouseEvent* event) {
    const core::Vector2 point = toLayout(event->position());
    std::optional<std::size_t> node = nodeAt(_layout, point, WORLD_GRAPH_NODE_RADIUS);
    std::optional<std::size_t> edge;
    if (!node) {
        edge = edgeAt(_layout, point, static_cast<float>(EDGE_PICK_TOLERANCE / scale()));
    }
    if (node != _hoveredNode || edge != _hoveredEdge) {
        _hoveredNode = node;
        _hoveredEdge = edge;
        const WorldGraphLayoutNode* hovered = node ? &_layout.nodes[*node] : nullptr;
        const bool openable = hovered != nullptr && !hovered->ghost && !hovered->unreadable;
        setCursor(openable ? Qt::PointingHandCursor : Qt::ArrowCursor);
        update();
    }
    QWidget::mouseMoveEvent(event);
}

void WorldGraphView::leaveEvent(QEvent* event) {
    _hoveredNode.reset();
    _hoveredEdge.reset();
    update();
    QWidget::leaveEvent(event);
}

void WorldGraphView::mouseDoubleClickEvent(QMouseEvent* event) {
    const auto node = nodeAt(_layout, toLayout(event->position()), WORLD_GRAPH_NODE_RADIUS);
    if (node && event->button() == Qt::LeftButton) {
        const WorldGraphLayoutNode& n = _layout.nodes[*node];
        if (!n.ghost && !n.unreadable) {
            const std::filesystem::path path = _dir / (n.mapId + ".json");
            emit levelOpenRequested(QString::fromStdString(path.string()));
            return;
        }
    }
    QWidget::mouseDoubleClickEvent(event);
}

void WorldGraphView::paintEvent(QPaintEvent* /*event*/) {
    const ColorTokens& colors = currentEditorTokens().color;
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), toQColor(colors.background));

    if (_layout.nodes.empty()) {
        painter.setPen(toQColor(colors.textMuted));
        painter.drawText(rect(), Qt::AlignCenter | Qt::TextWordWrap,
                         localized("world_graph.empty"));
        return;
    }

    painter.save();
    painter.translate(width() / 2.0, (height() - LEGEND_ROWS * LEGEND_ROW) / 2.0);
    painter.scale(scale(), scale());
    paintEdges(painter);
    paintNodes(painter);
    painter.restore();

    paintLegend(painter);
}

void WorldGraphView::paintEdges(QPainter& painter) const {
    const ColorTokens& colors = currentEditorTokens().color;
    for (std::size_t i = 0; i < _layout.edges.size(); ++i) {
        const WorldGraphLayoutEdge& edge = _layout.edges[i];
        const WorldGraphEdgeGeometry geometry = worldGraphEdgeGeometry(_layout, i);
        const QColor color = toQColor(edge.broken ? colors.error : colors.textMuted);
        const bool hovered = _hoveredEdge == i;
        QPen pen(color, hovered ? 3.0 : 1.6, edge.broken ? Qt::DashLine : Qt::SolidLine);
        painter.setPen(pen);
        painter.setBrush(Qt::NoBrush);

        if (edge.selfLoop) {
            painter.drawEllipse(toPoint(geometry.loopCenter), geometry.loopRadius,
                                geometry.loopRadius);
        } else {
            const QPointF start = toPoint(geometry.start);
            const QPointF end = toPoint(geometry.end);
            painter.drawLine(start, end);
            painter.setPen(Qt::NoPen);
            painter.setBrush(color);
            painter.drawPolygon(arrowHead(start, end));
        }

        if (edge.count() > 1) {
            const QString text = QString::number(edge.count());
            const QFontMetrics metrics(painter.font());
            const qreal diameter = std::max<qreal>(16.0, metrics.horizontalAdvance(text) + 8.0);
            const QRectF badge(toPoint(geometry.badge) - QPointF(diameter / 2.0, 8.0),
                               QSizeF(diameter, 16.0));
            painter.setPen(Qt::NoPen);
            painter.setBrush(color);
            painter.drawRoundedRect(badge, 8.0, 8.0);
            painter.setPen(toQColor(colors.background));
            painter.drawText(badge, Qt::AlignCenter, text);
        }
    }
}

void WorldGraphView::paintNodes(QPainter& painter) const {
    const ColorTokens& colors = currentEditorTokens().color;
    const QFont baseFont = painter.font();
    QFont nameFont = baseFont;
    nameFont.setBold(true);
    QFont idFont = baseFont;
    idFont.setPointSizeF(baseFont.pointSizeF() * 0.85);

    for (std::size_t i = 0; i < _layout.nodes.size(); ++i) {
        const WorldGraphLayoutNode& node = _layout.nodes[i];
        const QPointF center = toPoint(node.center);
        const bool hovered = _hoveredNode == i;
        const qreal penWidth = hovered ? 3.0 : 1.8;

        if (node.ghost) {
            painter.setPen(QPen(toQColor(colors.textMuted), penWidth, Qt::DashLine));
            painter.setBrush(Qt::NoBrush);
        } else if (node.unreadable) {
            painter.setPen(QPen(toQColor(colors.error), penWidth, Qt::DotLine));
            painter.setBrush(toQColor(colors.surfaceAlt));
        } else {
            painter.setPen(QPen(toQColor(colors.accent), penWidth));
            painter.setBrush(toQColor(colors.surface));
        }
        painter.drawEllipse(center, WORLD_GRAPH_NODE_RADIUS, WORLD_GRAPH_NODE_RADIUS);

        // Marque au cœur du disque : « ? » pour un fantôme, « ! » pour une carte illisible.
        if (node.ghost || node.unreadable) {
            painter.setFont(nameFont);
            painter.setPen(toQColor(node.ghost ? colors.textMuted : colors.error));
            const QRectF heart(
                center - QPointF(WORLD_GRAPH_NODE_RADIUS, WORLD_GRAPH_NODE_RADIUS),
                QSizeF(2.0 * WORLD_GRAPH_NODE_RADIUS, 2.0 * WORLD_GRAPH_NODE_RADIUS));
            painter.drawText(heart, Qt::AlignCenter,
                             node.ghost ? QStringLiteral("?") : QStringLiteral("!"));
        }

        const qreal labelWidth = static_cast<qreal>(WORLD_GRAPH_NODE_SPACING) - 8.0;
        const QRectF nameRect(center.x() - labelWidth / 2.0,
                              center.y() + WORLD_GRAPH_NODE_RADIUS + 2.0, labelWidth, 18.0);
        const QRectF idRect = nameRect.translated(0.0, 16.0);

        QFont labelFont = nameFont;
        labelFont.setItalic(node.ghost);
        painter.setFont(labelFont);
        painter.setPen(toQColor(node.ghost ? colors.textMuted : colors.text));
        const QString name = nodeLabel(i);
        painter.drawText(
            nameRect, Qt::AlignHCenter | Qt::AlignTop,
            QFontMetrics(labelFont).elidedText(name, Qt::ElideRight, static_cast<int>(labelWidth)));

        // Seconde ligne : l'identifiant d'une carte nommée, ou l'état d'une carte à problème.
        QString detail;
        if (node.unreadable) {
            detail = localized("world_graph.node.unreadable");
        } else if (node.ghost) {
            detail = localized("world_graph.node.ghost");
        } else if (!node.name.empty() && node.name != node.mapId) {
            detail = QString::fromStdString(node.mapId);
        }
        if (!detail.isEmpty()) {
            painter.setFont(idFont);
            painter.setPen(toQColor(node.unreadable ? colors.error : colors.textMuted));
            painter.drawText(idRect, Qt::AlignHCenter | Qt::AlignTop,
                             QFontMetrics(idFont).elidedText(detail, Qt::ElideRight,
                                                             static_cast<int>(labelWidth)));
        }
    }
    painter.setFont(baseFont);
}

void WorldGraphView::paintLegend(QPainter& painter) const {
    const ColorTokens& colors = currentEditorTokens().color;
    const qreal radius = 6.0;
    int y = height() - LEGEND_ROWS * LEGEND_ROW + LEGEND_ROW / 2 - LEGEND_PADDING / 2;
    const int x = LEGEND_PADDING;
    const int textX = x + LEGEND_SAMPLE + LEGEND_PADDING;

    const auto label = [&](const char* key) {
        painter.setPen(toQColor(colors.textMuted));
        painter.drawText(QRectF(textX, y - LEGEND_ROW / 2, width() - textX, LEGEND_ROW),
                         Qt::AlignLeft | Qt::AlignVCenter, localized(key));
        y += LEGEND_ROW;
    };
    const QPointF sample(x + LEGEND_SAMPLE / 2.0, 0.0);

    painter.setPen(QPen(toQColor(colors.accent), 1.5));
    painter.setBrush(toQColor(colors.surface));
    painter.drawEllipse(sample + QPointF(0.0, y), radius, radius);
    label("world_graph.legend.map");

    painter.setPen(QPen(toQColor(colors.error), 1.5, Qt::DotLine));
    painter.setBrush(toQColor(colors.surfaceAlt));
    painter.drawEllipse(sample + QPointF(0.0, y), radius, radius);
    label("world_graph.legend.unreadable");

    painter.setPen(QPen(toQColor(colors.textMuted), 1.5, Qt::DashLine));
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(sample + QPointF(0.0, y), radius, radius);
    label("world_graph.legend.ghost");

    painter.setPen(QPen(toQColor(colors.textMuted), 1.6));
    painter.drawLine(QPointF(x, y), QPointF(x + LEGEND_SAMPLE, y));
    label("world_graph.legend.portal");

    painter.setPen(QPen(toQColor(colors.error), 1.6, Qt::DashLine));
    painter.drawLine(QPointF(x, y), QPointF(x + LEGEND_SAMPLE, y));
    label("world_graph.legend.broken");
}

}  // namespace hmi
