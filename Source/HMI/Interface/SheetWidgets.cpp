// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Interface/SheetWidgets.h"

#include <QFont>
#include <QFontMetrics>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QRectF>
#include <algorithm>
#include <cmath>
#include <numbers>

#include "HMI/Interface/ApplicationTheme.h"

namespace hmi {
namespace {

[[nodiscard]] QColor toColor(const DesignColor& value) {
    return {value.r, value.g, value.b};
}

/// Épaisseur du trait d'encre extérieur, en unités d'agrandissement. Deux : sous cela le trait
/// disparaît au premier facteur, et le cadre cesse de se lire comme un cadre.
[[nodiscard]] int inkPen() {
    return 2 * identityScale();
}

/// Épaisseur du filet d'or. Toujours **plus fin** que le trait d'encre : c'est la réserve entre
/// les deux qui fait lire un encadrement, et non leur somme (`LOT-66`).
[[nodiscard]] int goldPen() {
    return std::max(1, identityScale());
}

[[nodiscard]] QFont sizedFont(const QFont& base, int pixelSize, bool bold) {
    QFont font = base;
    font.setPixelSize(std::max(1, pixelSize));
    font.setBold(bold);
    return font;
}

}  // namespace

// ==================================================================================== SheetGauge

SheetGauge::SheetGauge(Tone tone, QWidget* parent) : QWidget(parent), _tone(tone) {
    setAttribute(Qt::WA_StyledBackground, false);
}

void SheetGauge::setLabel(const QString& label) {
    _label = label;
    update();
}

void SheetGauge::setValue(const QString& value) {
    _value = value;
    update();
}

void SheetGauge::setFill(double fill) {
    _fill = std::clamp(fill, 0.0, 1.0);
    update();
}

QSize SheetGauge::sizeHint() const {
    const int scale = identityScale();
    return {160 * scale, 22 * scale};
}

QSize SheetGauge::minimumSizeHint() const {
    return {60 * identityScale(), 18 * identityScale()};
}

void SheetGauge::paintEvent(QPaintEvent* /*event*/) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const ColorTokens& color = identityTokens().color;
    const int scale = identityScale();
    const int barHeight = 6 * scale;
    const int captionHeight = std::max(0, height() - barHeight - (2 * scale));

    // L'intitulé à gauche, la valeur à droite, sur la même ligne de base : c'est ainsi que la
    // planche les pose, et cela évite au nombre de flotter au milieu d'une largeur variable.
    const QFont base = font();
    painter.setFont(sizedFont(base, 9 * scale, true));
    painter.setPen(toColor(color.frameOrnament));
    painter.drawText(QRect(0, 0, width(), captionHeight), Qt::AlignLeft | Qt::AlignVCenter, _label);

    painter.setFont(sizedFont(base, 11 * scale, true));
    painter.setPen(toColor(color.text));
    painter.drawText(QRect(0, 0, width(), captionHeight), Qt::AlignRight | Qt::AlignVCenter,
                     _value);

    const QRect track(0, captionHeight + (2 * scale), width(), barHeight);
    painter.setPen(Qt::NoPen);
    // Le fond de la jauge est le parchemin **assombri**, pas une teinte a part : une couleur de
    // plus a l'ecran pour dire « le vide » n'apprend rien de plus que l'absence de remplissage.
    painter.setBrush(toColor(color.background).darker(108));
    painter.drawRect(track);

    if (_fill > 0.0) {
        QRect filled = track;
        filled.setWidth(static_cast<int>(std::lround(track.width() * _fill)));
        painter.setBrush(_tone == Tone::Vitality ? toColor(color.gem) : toColor(color.accent));
        painter.drawRect(filled);
    }

    painter.setBrush(Qt::NoBrush);
    painter.setPen(QPen(toColor(color.frameEdge), inkPen()));
    painter.drawRect(track.adjusted(0, 0, -1, -1));
}

// =================================================================================== ShieldValue

ShieldValue::ShieldValue(QWidget* parent) : QWidget(parent) {
    setAttribute(Qt::WA_StyledBackground, false);
}

void ShieldValue::setCaption(const QString& caption) {
    _caption = caption;
    update();
}

void ShieldValue::setValue(const QString& value) {
    _value = value;
    update();
}

QSize ShieldValue::sizeHint() const {
    const int scale = identityScale();
    return {44 * scale, 50 * scale};
}

QSize ShieldValue::minimumSizeHint() const {
    return {30 * identityScale(), 34 * identityScale()};
}

void ShieldValue::paintEvent(QPaintEvent* /*event*/) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const ColorTokens& color = identityTokens().color;
    const auto w = static_cast<double>(width());
    const auto h = static_cast<double>(height());

    // L'écu, en proportions du cadre reçu : épaules droites, flancs qui se referment, pointe
    // basse. Le tracer plutôt que l'étirer est ce qui l'empêche de s'ovaliser dans un panneau bas.
    QPainterPath shield;
    shield.moveTo(w * 0.5, h * 0.03);
    shield.lineTo(w * 0.97, h * 0.15);
    shield.lineTo(w * 0.97, h * 0.55);
    shield.cubicTo(w * 0.97, h * 0.82, w * 0.75, h * 0.94, w * 0.5, h * 0.99);
    shield.cubicTo(w * 0.25, h * 0.94, w * 0.03, h * 0.82, w * 0.03, h * 0.55);
    shield.lineTo(w * 0.03, h * 0.15);
    shield.closeSubpath();

    painter.setBrush(toColor(color.surfaceAlt));
    painter.setPen(QPen(toColor(color.frameEdge), inkPen()));
    painter.drawPath(shield);

    // Le filet d'or en RETRAIT, jamais collé au trait d'encre : sans la réserve entre les deux,
    // l'écu se lit comme une bordure épaisse de deux tons.
    QPainterPath inner;
    const double inset = std::max(3.0, std::min(w, h) * 0.09);
    inner.addPath(shield);
    painter.setBrush(Qt::NoBrush);
    painter.setPen(QPen(toColor(color.accent), goldPen()));
    painter.save();
    painter.translate(w * 0.5, h * 0.5);
    painter.scale((w - (2 * inset)) / w, (h - (2 * inset)) / h);
    painter.translate(-w * 0.5, -h * 0.5);
    painter.drawPath(inner);
    painter.restore();

    const QFont base = font();
    const int scale = identityScale();
    painter.setPen(toColor(color.frameOrnament));
    // L'intitule se met A LA TAILLE de l'ecu, au lieu d'etre rogne par ses flancs. « Classe
    // d'armure » est long, l'ecu est etroit, et un intitule coupe (« asse d'armu ») ne se lit pas
    // comme un manque de place : il se lit comme un defaut.
    const QRectF captionBox(w * 0.10, h * 0.20, w * 0.80, h * 0.18);
    int captionSize = 7 * scale;
    for (; captionSize > 2; --captionSize) {
        const QFontMetrics metrics(sizedFont(base, captionSize, true));
        if (metrics.horizontalAdvance(_caption) <= captionBox.width()) {
            break;
        }
    }
    painter.setFont(sizedFont(base, captionSize, true));
    painter.drawText(captionBox, Qt::AlignCenter, _caption);

    painter.setPen(toColor(color.text));
    painter.setFont(sizedFont(base, 18 * scale, true));
    painter.drawText(QRectF(0, h * 0.34, w, h * 0.36), Qt::AlignCenter, _value);
}

// ================================================================================= StatMedallion

StatMedallion::StatMedallion(QWidget* parent) : QWidget(parent) {
    setAttribute(Qt::WA_StyledBackground, false);
}

void StatMedallion::setCaption(const QString& caption) {
    _caption = caption;
    update();
}

void StatMedallion::setValue(const QString& value) {
    _value = value;
    update();
}

QSize StatMedallion::sizeHint() const {
    const int scale = identityScale();
    return {52 * scale, 46 * scale};
}

QSize StatMedallion::minimumSizeHint() const {
    return {34 * identityScale(), 32 * identityScale()};
}

void StatMedallion::paintEvent(QPaintEvent* /*event*/) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const ColorTokens& color = identityTokens().color;
    const int scale = identityScale();
    const QFont base = font();

    const int captionHeight = 8 * scale;
    const int discSide = std::max(1, std::min(width(), height() - captionHeight));
    const QRectF disc((width() - discSide) / 2.0, 0.0, discSide, discSide);

    painter.setBrush(toColor(color.surfaceAlt));
    painter.setPen(QPen(toColor(color.frameEdge), inkPen()));
    painter.drawEllipse(disc);
    painter.setBrush(Qt::NoBrush);
    painter.setPen(QPen(toColor(color.accent), goldPen()));
    const double inset = inkPen() + goldPen();
    painter.drawEllipse(disc.adjusted(inset, inset, -inset, -inset));

    painter.setPen(toColor(color.text));
    painter.setFont(sizedFont(base, static_cast<int>(discSide * 0.42), true));
    painter.drawText(disc, Qt::AlignCenter, _value);

    painter.setPen(toColor(color.frameOrnament));
    painter.setFont(sizedFont(base, 7 * scale, true));
    painter.drawText(QRect(0, height() - captionHeight, width(), captionHeight),
                     Qt::AlignHCenter | Qt::AlignVCenter, _caption);
}

// ======================================================================================== PipRow

PipRow::PipRow(QWidget* parent) : QWidget(parent) {
    setAttribute(Qt::WA_StyledBackground, false);
}

void PipRow::setLabel(const QString& label) {
    _label = label;
    update();
}

void PipRow::setValue(const QString& value) {
    _value = value;
    update();
}

void PipRow::setProficient(bool proficient) {
    _proficient = proficient;
    update();
}

QSize PipRow::sizeHint() const {
    const int scale = identityScale();
    return {140 * scale, 11 * scale};
}

QSize PipRow::minimumSizeHint() const {
    return {60 * identityScale(), 10 * identityScale()};
}

void PipRow::paintEvent(QPaintEvent* /*event*/) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const ColorTokens& color = identityTokens().color;
    const int scale = identityScale();
    const QFont base = font();
    const int pip = 5 * scale;
    const QRectF pipBox(0.0, (height() - pip) / 2.0, pip, pip);

    // Pleine, la pastille est le sceau d'attestation ; creuse, elle reste visible et dit « pas
    // maîtrisée ». La faire disparaître ferait lire la ligne comme incomplète.
    if (_proficient) {
        painter.setBrush(toColor(color.gem));
        painter.setPen(QPen(toColor(color.frameEdge), goldPen()));
    } else {
        painter.setBrush(Qt::NoBrush);
        painter.setPen(QPen(toColor(color.accent), goldPen()));
    }
    painter.drawEllipse(pipBox);

    const int valueWidth = 26 * scale;
    const int textLeft = pip + (4 * scale);
    painter.setFont(sizedFont(base, 9 * scale, false));
    painter.setPen(toColor(_proficient ? color.text : color.textMuted));
    const QRect labelBox(textLeft, 0, std::max(0, width() - textLeft - valueWidth), height());
    painter.drawText(labelBox, Qt::AlignLeft | Qt::AlignVCenter,
                     painter.fontMetrics().elidedText(_label, Qt::ElideRight, labelBox.width()));

    painter.setFont(sizedFont(base, 9 * scale, true));
    painter.setPen(toColor(color.text));
    painter.drawText(QRect(width() - valueWidth, 0, valueWidth, height()),
                     Qt::AlignRight | Qt::AlignVCenter, _value);
}

// ======================================================================================= WaxSeal

WaxSeal::WaxSeal(QWidget* parent) : QWidget(parent) {
    setAttribute(Qt::WA_StyledBackground, false);
}

QSize WaxSeal::sizeHint() const {
    const int side = 30 * identityScale();
    return {side, side};
}

QSize WaxSeal::minimumSizeHint() const {
    const int side = 20 * identityScale();
    return {side, side};
}

void WaxSeal::paintEvent(QPaintEvent* /*event*/) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const ColorTokens& color = identityTokens().color;
    const double side = std::min(width(), height());
    const double radius = side / 2.0;
    const QPointF center(width() / 2.0, height() / 2.0);

    // Le bord festonné : douze pointes alternant deux rayons. C'est ce qui distingue la cire
    // pressée d'un simple disque rouge, et se lit encore à vingt pixels.
    QPainterPath wax;
    constexpr int POINTS = 24;
    for (int index = 0; index < POINTS; ++index) {
        const double angle = (index * 2.0 * std::numbers::pi) / POINTS;
        const double r = radius * ((index % 2 == 0) ? 1.0 : 0.88);
        const QPointF point(center.x() + (r * std::cos(angle)), center.y() + (r * std::sin(angle)));
        if (index == 0) {
            wax.moveTo(point);
        } else {
            wax.lineTo(point);
        }
    }
    wax.closeSubpath();

    painter.setBrush(toColor(color.gem));
    painter.setPen(QPen(toColor(color.gemShadow), goldPen()));
    painter.drawPath(wax);

    // L'empreinte : un anneau et un fleuron creusés, en teinte de profondeur. Sans elle, la cire
    // n'a rien scellé.
    painter.setBrush(Qt::NoBrush);
    QPen stampPen(toColor(color.gemShadow), goldPen());
    painter.setPen(stampPen);
    painter.drawEllipse(center, radius * 0.66, radius * 0.66);

    QPainterPath mark;
    mark.moveTo(center.x(), center.y() - (radius * 0.42));
    mark.lineTo(center.x() + (radius * 0.20), center.y() - (radius * 0.06));
    mark.lineTo(center.x(), center.y() + (radius * 0.42));
    mark.lineTo(center.x() - (radius * 0.20), center.y() - (radius * 0.06));
    mark.closeSubpath();
    painter.setBrush(toColor(color.gemShadow));
    painter.setPen(Qt::NoPen);
    painter.drawPath(mark);
}

}  // namespace hmi
