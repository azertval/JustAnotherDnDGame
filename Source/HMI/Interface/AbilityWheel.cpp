// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Interface/AbilityWheel.h"

#include <QFont>
#include <QFontMetrics>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QRectF>
#include <algorithm>
#include <cmath>

#include "HMI/Interface/ApplicationTheme.h"

namespace hmi {
namespace {

/// Le tiret cadratin : ce qu'affiche un champ que rien n'alimente. Jamais un zéro, qui se lirait
/// comme un état du jeu (`LOT-38`).
const QString EMPTY_VALUE = QString::fromUtf8("—");

/// Rapports de texte, **au rayon du médaillon** et non au côté de la roue : un chiffre inscrit
/// dans un cercle doit grandir avec ce cercle, sans quoi il en déborde ou y flotte.
constexpr float SCORE_TEXT_RATIO = 0.80F;
constexpr float MODIFIER_TEXT_RATIO = 0.52F;
constexpr float LABEL_TEXT_RATIO = 0.30F;

/// Le cabochon du modificateur, posé à cheval sur le bord du médaillon.
constexpr float MODIFIER_DISC_RATIO = 0.46F;

/// Épaisseurs de trait, au rayon du médaillon. Le filet intérieur est plus fin que le trait
/// d'encre : c'est la réserve entre les deux qui fait lire un cadre (`LOT-66`).
constexpr float EDGE_PEN_RATIO = 0.055F;
constexpr float INNER_PEN_RATIO = 0.040F;

[[nodiscard]] QColor toColor(const DesignColor& value) {
    return {value.r, value.g, value.b};
}

/// Qt compte les angles depuis trois heures, en sens inverse, et en seizièmes de degré. La table
/// les compte depuis le haut, en sens horaire. La conversion est ici, et une seule fois.
[[nodiscard]] int toQtAngle(float degreesFromTop) {
    return static_cast<int>(std::lround((90.0F - degreesFromTop) * 16.0F));
}

[[nodiscard]] QRectF discRect(const WheelDisc& disc) {
    return {disc.centerX - disc.radius, disc.centerY - disc.radius, disc.radius * 2.0F,
            disc.radius * 2.0F};
}

[[nodiscard]] QFont scaledFont(const QFont& base, float pixelSize, bool bold) {
    QFont font = base;
    font.setPixelSize(std::max(1, static_cast<int>(std::lround(pixelSize))));
    font.setBold(bold);
    return font;
}

}  // namespace

AbilityWheel::AbilityWheel(QWidget* parent) : QWidget(parent) {
    // Le parchemin de l'écran se peint au travers : la roue n'a pas de fond propre, seulement des
    // formes. Un fond opaque ici découperait un carré clair au milieu de la feuille.
    setAttribute(Qt::WA_StyledBackground, false);
    for (SeatText& seat : _seats) {
        seat.score = EMPTY_VALUE;
        seat.modifier = EMPTY_VALUE;
    }
}

void AbilityWheel::setSeat(WheelSeat seat, const QString& label, const QString& score,
                           const QString& modifier) {
    SeatText& text = _seats.at(static_cast<std::size_t>(seat));
    text.label = label;
    text.score = score.isEmpty() ? EMPTY_VALUE : score;
    text.modifier = modifier.isEmpty() ? EMPTY_VALUE : modifier;
    update();
}

void AbilityWheel::setIdentity(const QString& name, const QString& subtitle) {
    _name = name;
    _subtitle = subtitle;
    update();
}

void AbilityWheel::setPortrait(const QPixmap& portrait) {
    _portrait = portrait;
    update();
}

QSize AbilityWheel::sizeHint() const {
    // Deux fois le plancher : c'est la taille à laquelle la roue est confortable, et la valeur que
    // la disposition doit préférer quand la place le permet.
    const int side = ABILITY_WHEEL_MINIMUM_SIZE * 2;
    return {side, side};
}

QSize AbilityWheel::minimumSizeHint() const {
    // Le plancher est tenu ICI, et non par un repli dans le peintre : c'est la disposition de Qt
    // qui doit garantir que la roue ne descend jamais sous sa taille lisible.
    return {ABILITY_WHEEL_MINIMUM_SIZE, ABILITY_WHEEL_MINIMUM_SIZE};
}

bool AbilityWheel::hasHeightForWidth() const {
    return true;
}

int AbilityWheel::heightForWidth(int width) const {
    return width;  // La roue est carrée : son arc n'a de sens que dans un carré.
}

void AbilityWheel::paintEvent(QPaintEvent* /*event*/) {
    const int side = std::min(width(), height());
    const AbilityWheelLayout layout = abilityWheelLayout(side);
    if (!layout.valid) {
        return;  // Sous le plancher : rien n'est lisible, donc rien n'est peint.
    }

    QPainter painter(this);
    // Anticrénelage actif : tout est courbe ici, et sans lui la roue redeviendrait l'escalier du
    // pixel art dont la charte est sortie (`LOT-66`).
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter.translate((width() - side) / 2, (height() - side) / 2);

    const ColorTokens& color = identityTokens().color;
    const QColor edge = toColor(color.frameEdge);
    const QColor ornament = toColor(color.frameOrnament);
    const QColor accent = toColor(color.accent);
    const QColor surfaceAlt = toColor(color.surfaceAlt);
    const QColor ink = toColor(color.text);
    const QColor muted = toColor(color.textMuted);

    const float seatRadius = layout.seats.front().radius;
    const float edgePen = std::max(1.0F, seatRadius * EDGE_PEN_RATIO);
    const float innerPen = std::max(1.0F, seatRadius * INNER_PEN_RATIO);

    // --- L'arc qui relie les six ---------------------------------------------------------------
    const QRectF arcBox(layout.portrait.centerX - layout.arcRadius,
                        layout.portrait.centerY - layout.arcRadius, layout.arcRadius * 2.0F,
                        layout.arcRadius * 2.0F);
    painter.setBrush(Qt::NoBrush);
    painter.setPen(QPen(accent, innerPen));
    // L'ouverture du bas n'est pas peinte : c'est elle qui reçoit le nom, et un arc fermé la
    // barrerait.
    painter.drawArc(arcBox, toQtAngle(layout.arcStartDegrees + layout.arcSweepDegrees),
                    static_cast<int>(std::lround(layout.arcSweepDegrees * 16.0F)));

    // --- Les fleurons intercalaires ------------------------------------------------------------
    painter.setBrush(accent);
    painter.setPen(QPen(edge, innerPen));
    for (const WheelDisc& fleuron : layout.fleurons) {
        // Un losange, et non un disque : posé sur l'arc, un disque se confondrait avec un médaillon
        // rétréci, et le joueur y chercherait une valeur.
        QPainterPath diamond;
        diamond.moveTo(fleuron.centerX, fleuron.centerY - fleuron.radius);
        diamond.lineTo(fleuron.centerX + fleuron.radius, fleuron.centerY);
        diamond.lineTo(fleuron.centerX, fleuron.centerY + fleuron.radius);
        diamond.lineTo(fleuron.centerX - fleuron.radius, fleuron.centerY);
        diamond.closeSubpath();
        painter.drawPath(diamond);
    }

    // --- L'anneau d'entrelacs autour du portrait -----------------------------------------------
    const QRectF ringBox(layout.portrait.centerX - layout.ringRadius,
                         layout.portrait.centerY - layout.ringRadius, layout.ringRadius * 2.0F,
                         layout.ringRadius * 2.0F);
    painter.setBrush(Qt::NoBrush);
    QPen ringPen(accent, innerPen * 2.0F);
    // Le trait rompu tient lieu d'entrelacs : la tresse du livre ne se lit plus sous cinquante
    // pixels, alors que l'alternance, elle, reste visible.
    ringPen.setDashPattern({4.0, 3.0});
    painter.setPen(ringPen);
    painter.drawEllipse(ringBox);

    // --- Le portrait ---------------------------------------------------------------------------
    const QRectF portraitBox = discRect(layout.portrait);
    painter.save();
    QPainterPath clip;
    clip.addEllipse(portraitBox);
    painter.setClipPath(clip);
    if (_portrait.isNull()) {
        // Le marqueur du `LOT-39` : deux diagonales sur un aplat. Un rond vide se lirait comme un
        // défaut de rendu ; celles-ci disent « pas encore livrée ».
        painter.fillRect(portraitBox, toColor(color.background));
        painter.setPen(QPen(ornament, innerPen));
        painter.drawLine(portraitBox.topLeft(), portraitBox.bottomRight());
        painter.drawLine(portraitBox.topRight(), portraitBox.bottomLeft());
    } else {
        painter.drawPixmap(portraitBox.toRect(), _portrait);
    }
    painter.restore();
    painter.setBrush(Qt::NoBrush);
    painter.setPen(QPen(edge, edgePen));
    painter.drawEllipse(portraitBox);

    // --- Les six médaillons --------------------------------------------------------------------
    const QFont base = font();
    const QFont scoreFont = scaledFont(base, seatRadius * SCORE_TEXT_RATIO, true);
    const QFont modifierFont = scaledFont(base, seatRadius * MODIFIER_TEXT_RATIO, true);
    const QFont labelFont = scaledFont(base, seatRadius * LABEL_TEXT_RATIO, true);

    for (std::size_t index = 0; index < ABILITY_SEAT_COUNT; ++index) {
        const WheelDisc& seat = layout.seats.at(index);
        const SeatText& text = _seats.at(index);
        const QRectF seatBox = discRect(seat);

        painter.setBrush(surfaceAlt);
        painter.setPen(QPen(edge, edgePen));
        painter.drawEllipse(seatBox);
        painter.setBrush(Qt::NoBrush);
        painter.setPen(QPen(accent, innerPen));
        painter.drawEllipse(
            seatBox.adjusted(edgePen * 2.0, edgePen * 2.0, -edgePen * 2.0, -edgePen * 2.0));

        painter.setPen(ink);
        painter.setFont(scoreFont);
        painter.drawText(seatBox, Qt::AlignCenter, text.score);

        // Le cabochon du modificateur, à cheval sur le bord : deux nombres posés côte à côte dans
        // le même cercle se confondraient, et c'est exactement ce que la planche évite en sortant
        // le second de son médaillon.
        const float modifierRadius = seat.radius * MODIFIER_DISC_RATIO;
        const float offset = seat.radius * 0.72F;
        const bool onTheLeft = seat.centerX > layout.portrait.centerX;
        const QRectF modifierBox(seat.centerX + (onTheLeft ? -offset : offset) - modifierRadius,
                                 seat.centerY - offset - modifierRadius, modifierRadius * 2.0F,
                                 modifierRadius * 2.0F);
        painter.setBrush(accent);
        painter.setPen(QPen(edge, innerPen));
        painter.drawEllipse(modifierBox);
        painter.setPen(ink);
        painter.setFont(modifierFont);
        painter.drawText(modifierBox, Qt::AlignCenter, text.modifier);

        // La plaque du nom, sous le médaillon : de l'encre pleine, texte en réserve. C'est le
        // bandeau sombre de la planche, ramené à sa fonction.
        if (!text.label.isEmpty()) {
            const QFontMetrics metrics(labelFont);
            const int labelWidth = metrics.horizontalAdvance(text.label) +
                                   static_cast<int>(std::lround(seat.radius * 0.36F));
            const int labelHeight =
                metrics.height() + static_cast<int>(std::lround(seat.radius * 0.12F));
            const QRectF labelBox(seat.centerX - (labelWidth / 2.0),
                                  seat.centerY + seat.radius - (labelHeight / 3.0), labelWidth,
                                  labelHeight);
            painter.setBrush(edge);
            painter.setPen(Qt::NoPen);
            painter.drawRect(labelBox);
            painter.setPen(surfaceAlt);
            painter.setFont(labelFont);
            painter.drawText(labelBox, Qt::AlignCenter, text.label);
        }
    }

    // --- Le nom, dans l'ouverture de l'arc -----------------------------------------------------
    if (!_name.isEmpty() || !_subtitle.isEmpty()) {
        const auto lowest = std::max_element(
            layout.seats.begin(), layout.seats.end(),
            [](const WheelDisc& a, const WheelDisc& b) { return a.centerY < b.centerY; });
        const float top = lowest->centerY + lowest->radius + (seatRadius * 0.55F);
        const QRectF band(0.0, top, static_cast<double>(side), static_cast<double>(side) - top);
        if (band.height() > 0.0) {
            const QFont nameFont = scaledFont(base, seatRadius * 0.74F, true);
            const QFont subtitleFont = scaledFont(base, seatRadius * 0.40F, false);
            const QFontMetrics nameMetrics(nameFont);

            painter.setFont(nameFont);
            painter.setPen(ink);
            const QRectF nameBox(band.left(), band.top(), band.width(), nameMetrics.height());
            painter.drawText(nameBox, Qt::AlignHCenter | Qt::AlignTop, _name);

            painter.setFont(subtitleFont);
            painter.setPen(muted);
            const QRectF subtitleBox(band.left(), nameBox.bottom(), band.width(),
                                     band.bottom() - nameBox.bottom());
            painter.drawText(subtitleBox, Qt::AlignHCenter | Qt::AlignTop, _subtitle);
        }
    }
}

}  // namespace hmi
