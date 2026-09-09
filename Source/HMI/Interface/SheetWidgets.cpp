// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Interface/SheetWidgets.h"

#include <QFile>
#include <QFont>
#include <QFontMetrics>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QRectF>
#include <QSvgRenderer>
#include <algorithm>
#include <cmath>
#include <numbers>

#include "HMI/Interface/ApplicationTheme.h"
#include "HMI/Interface/DesignTokens.h"
#include "HMI/Interface/FocusMarker.h"

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

/// Épaisseur d'un encadrement de la maison : celle que la feuille de style donne au panneau et aux
/// onglets d'Options (`identity.frame.thickness`). L'employer ici est ce qui fait que la barre
/// d'onglets de la fiche et celle d'Options portent le MÊME trait.
[[nodiscard]] int framePen() {
    return std::max(1, identityBaseScale().frameThickness * identityScale());
}

[[nodiscard]] QFont sizedFont(const QFont& base, int pixelSize, bool bold) {
    QFont font = base;
    font.setPixelSize(std::max(1, pixelSize));
    font.setBold(bold);
    return font;
}

/// Les trois niveaux de l'echelle typographique de l'identite qu'emploie la planche, deja
/// multiplies par le facteur entier (`LOT-68`).
///
/// ## Pourquoi passer par eux, et jamais par un nombre ecrit ici
///
/// `IdentityBaseScale` n'a que QUATRE niveaux -- 23, 14, 10, 8 -- et tout le reste de l'interface
/// s'y tient : la feuille de style ne connait que `identity.size.*`, et un bouton de menu prend sa
/// hauteur de `sectionTitle`. Les pieces de cette planche ecrivaient 7, 9, 11 et 18 px. Ces
/// tailles ne sont fausses nulle part en particulier ; c'est leur ACCUMULATION qui faisait lire la
/// fiche comme un document importe au milieu d'une interface qui, elle, ne varie que sur quatre
/// crans. Une taille de plus est un cran de plus, et le lecteur n'a plus de repere.
[[nodiscard]] int captionSize() {
    return identityBaseScale().caption * identityScale();
}
[[nodiscard]] int bodySize() {
    return identityBaseScale().body * identityScale();
}
[[nodiscard]] int sectionSize() {
    return identityBaseScale().sectionTitle * identityScale();
}

/// @return Le plus GRAND des trois niveaux dont @p text tient dans @p available pixels de large,
///         ou le plus petit à défaut.
///
/// C'est la seule façon admise de rétrécir un texte ici : on descend d'un CRAN de l'échelle, on
/// n'en sort pas. Une boucle qui décrémente de pixel en pixel finit par écrire une taille que rien
/// d'autre n'emploie — et c'est ainsi que la planche s'était retrouvée avec 7, 9, 11 et 18 px.
///
/// Un texte trop long garde donc une taille de la charte et devient PLUS PETIT plutôt que rogné :
/// « Vitesse » affichait « n » parce que sa valeur, écrite au niveau `sectionTitle`, débordait du
/// médaillon sans que rien ne le dise.
/// @return La largeur qu'il faut à @p caption pour tenir en entier au niveau `caption`.
///
/// Une pièce qui porte un intitulé doit le RÉCLAMER. « Bonus de maîtrise » et « Classe d'armure »
/// sont longs, et une largeur écrite une fois pour toutes les rogne dès qu'on traduit — un intitulé
/// coupé ne se lit pas comme un manque de place, il se lit comme un défaut.
[[nodiscard]] int captionWidth(const QFont& base, const QString& caption) {
    const QFontMetrics metrics(sizedFont(base, captionSize(), true));
    return metrics.horizontalAdvance(caption) + (4 * identityScale());
}

[[nodiscard]] int fittedSize(const QFont& base, const QString& text, int available, bool bold) {
    for (const int level : {sectionSize(), bodySize(), captionSize()}) {
        const QFontMetrics metrics(sizedFont(base, level, bold));
        if (metrics.horizontalAdvance(text) <= available) {
            return level;
        }
    }
    return captionSize();
}

}  // namespace

// ==================================================================================== SheetGauge

SheetGauge::SheetGauge(QWidget* parent) : QWidget(parent) {
    setAttribute(Qt::WA_StyledBackground, false);
}

void SheetGauge::setTone(Tone tone) {
    if (_tone == tone) {
        return;
    }
    _tone = tone;
    update();
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
    // La hauteur suit la TAILLE DU TEXTE, et non l'inverse : ecrite en dur, elle rognerait la
    // valeur au premier changement de l'echelle typographique, sans qu'aucun test ne le dise.
    return {160 * scale, bodySize() + (10 * scale)};
}

QSize SheetGauge::minimumSizeHint() const {
    const int scale = identityScale();
    return {60 * scale, bodySize() + (8 * scale)};
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
    painter.setFont(sizedFont(base, captionSize(), true));
    painter.setPen(toColor(color.frameOrnament));
    painter.drawText(QRect(0, 0, width(), captionHeight), Qt::AlignLeft | Qt::AlignVCenter, _label);

    painter.setFont(sizedFont(base, bodySize(), true));
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
    // La largeur demandée dépend de l'intitulé : sans ce rappel, la pièce garde la taille qu'elle
    // avait avant d'être traduite, et l'intitulé y est rogné dans la langue suivante.
    updateGeometry();
    update();
}

void ShieldValue::setValue(const QString& value) {
    _value = value;
    update();
}

QSize ShieldValue::sizeHint() const {
    const int scale = identityScale();
    // La valeur occupe la bande centrale, haute de 36 % de l'ecu : l'ecu se dimensionne donc SUR
    // elle. Un ecu plus court rognerait la classe d'armure par le haut et par le bas.
    const int shield = std::max(50 * scale, (sectionSize() * 100) / 36);
    return {std::max(44 * scale, captionWidth(font(), _caption)),
            shield + captionSize() + (2 * scale)};
}

QSize ShieldValue::minimumSizeHint() const {
    const int scale = identityScale();
    const int shield = std::max(34 * scale, (sectionSize() * 100) / 36);
    return {std::max(30 * scale, captionWidth(font(), _caption)),
            shield + captionSize() + (2 * scale)};
}

void ShieldValue::paintEvent(QPaintEvent* /*event*/) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const ColorTokens& color = identityTokens().color;
    const int scale = identityScale();
    // L'intitulé vit SOUS l'écu, comme celui des quatre médaillons qui l'entourent. Enfermé dans le
    // blason, « Classe d'armure » n'y tenait à aucun niveau de l'échelle : l'écu est étroit par
    // construction, et un intitulé qui a besoin du triple de sa largeur n'a rien à y faire. Sous
    // lui, le bloc de combat se lit d'une seule façon — un rond, un nom dessous, quatre fois.
    const int captionBand = captionSize() + (2 * scale);
    // L'écu, tracé dans un carré large de tout le widget mais HAUT de ce qui reste : sans quoi il
    // s'élargirait avec l'intitulé qu'il porte au lieu de garder sa silhouette.
    const auto h = static_cast<double>(std::max(1, height() - captionBand));
    const auto w = std::min(static_cast<double>(width()), h * 0.9);
    const double left = (width() - w) / 2.0;

    painter.translate(left, 0.0);

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
    painter.setPen(toColor(color.frameOrnament));
    // L'intitule se met A LA TAILLE de l'ecu, au lieu d'etre rogne par ses flancs. « Classe
    // d'armure » est long, l'ecu est etroit, et un intitule coupe (« asse d'armu ») ne se lit pas
    // comme un manque de place : il se lit comme un defaut.
    painter.setPen(toColor(color.text));
    painter.setFont(
        sizedFont(base, fittedSize(base, _value, static_cast<int>(w * 0.7), true), true));
    painter.drawText(QRectF(0, h * 0.28, w, h * 0.44), Qt::AlignCenter, _value);

    // L'intitulé, sous l'écu et sur toute la largeur du widget : on revient donc au repère de la
    // pièce avant de l'écrire.
    painter.translate(-left, 0.0);
    painter.setPen(toColor(color.frameOrnament));
    painter.setFont(sizedFont(base, captionSize(), true));
    painter.drawText(QRect(0, height() - captionBand, width(), captionBand), Qt::AlignCenter,
                     _caption);
}

// ================================================================================= StatMedallion

StatMedallion::StatMedallion(QWidget* parent) : QWidget(parent) {
    setAttribute(Qt::WA_StyledBackground, false);
}

void StatMedallion::setCaption(const QString& caption) {
    _caption = caption;
    updateGeometry();
    update();
}

void StatMedallion::setValue(const QString& value) {
    _value = value;
    update();
}

QSize StatMedallion::sizeHint() const {
    const int scale = identityScale();
    // Le disque tient la valeur au niveau `sectionTitle`, l'intitule tient dessous au niveau
    // `caption` : les deux niveaux, plus leur air, FONT la taille du medaillon.
    const int disc = (sectionSize() * 2) + (4 * scale);
    return {std::max({52 * scale, disc, captionWidth(font(), _caption)}),
            disc + captionSize() + (2 * scale)};
}

QSize StatMedallion::minimumSizeHint() const {
    const int scale = identityScale();
    const int disc = sectionSize() + (4 * scale);
    return {std::max({34 * scale, disc, captionWidth(font(), _caption)}),
            disc + captionSize() + (2 * scale)};
}

void StatMedallion::paintEvent(QPaintEvent* /*event*/) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const ColorTokens& color = identityTokens().color;
    const int scale = identityScale();
    const QFont base = font();

    const int captionHeight = captionSize() + (2 * scale);
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
    // Un niveau de l'echelle, jamais une fraction du disque : une taille derivee d'une geometrie
    // suit la fenetre au lieu de suivre le facteur entier, et la fiche cesse d'etre agrandie pour
    // devenir etiree. Une valeur trop large descend d'un cran plutot que de deborder.
    const int room = static_cast<int>(discSide - (2.0 * inset));
    painter.setFont(sizedFont(base, fittedSize(base, _value, room, true), true));
    painter.drawText(disc, Qt::AlignCenter, _value);

    painter.setPen(toColor(color.frameOrnament));
    painter.setFont(sizedFont(base, captionSize(), true));
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
    return {140 * scale, bodySize() + (3 * scale)};
}

QSize PipRow::minimumSizeHint() const {
    const int scale = identityScale();
    return {60 * scale, bodySize() + (2 * scale)};
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
    painter.setFont(sizedFont(base, bodySize(), false));
    painter.setPen(toColor(_proficient ? color.text : color.textMuted));
    const QRect labelBox(textLeft, 0, std::max(0, width() - textLeft - valueWidth), height());
    painter.drawText(labelBox, Qt::AlignLeft | Qt::AlignVCenter,
                     painter.fontMetrics().elidedText(_label, Qt::ElideRight, labelBox.width()));

    painter.setFont(sizedFont(base, bodySize(), true));
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

// ====================================================================================== slotIcon

QPixmap slotIcon(const QString& slotName, const QColor& ink, int side) {
    if (slotName.isEmpty() || side <= 0) {
        return {};
    }

    QFile source(QStringLiteral(":/resources/icons/slot/%1.svg").arg(slotName));
    if (!source.open(QIODevice::ReadOnly)) {
        // Un nom inconnu rend un pixmap NUL, et l'appelant laisse alors le medaillon vide. Poser
        // une icone de remplacement ferait passer un emplacement inconnu pour un emplacement
        // connu, ce qui est pire que rien.
        return {};
    }

    // La substitution de `currentColor`, qui est toute la raison d'etre de cette fonction : Qt SVG
    // ne connait pas ce mot-cle et rendrait le trait en noir. La faire sur la SOURCE, avant le
    // rendu, garde l'icone libre de toute teinte sur le disque (EX-IHM-051).
    QByteArray svg = source.readAll();
    svg.replace("currentColor", ink.name(QColor::HexRgb).toLatin1());

    QSvgRenderer renderer(svg);
    if (!renderer.isValid()) {
        return {};
    }

    QPixmap canvas(side, side);
    canvas.fill(Qt::transparent);
    QPainter painter(&canvas);
    painter.setRenderHint(QPainter::Antialiasing, true);
    renderer.render(&painter);
    return canvas;
}

// ============================================================================== EquipmentSlotRow

EquipmentSlotRow::EquipmentSlotRow(QString slotName, bool mirrored, QWidget* parent)
    : QWidget(parent), _slotName(std::move(slotName)), _mirrored(mirrored) {
    setAttribute(Qt::WA_StyledBackground, false);
}

void EquipmentSlotRow::setCaption(const QString& caption) {
    _caption = caption;
    update();
}

void EquipmentSlotRow::setItem(const QString& item) {
    _item = item;
    update();
}

QSize EquipmentSlotRow::sizeHint() const {
    const int scale = identityScale();
    // Deux lignes empilees -- l'intitule au niveau `caption`, l'objet au niveau `body` -- et l'air
    // entre elles. La hauteur suit donc l'echelle typographique, pas un nombre choisi une fois.
    return {150 * scale, captionSize() + bodySize() + (6 * scale)};
}

QSize EquipmentSlotRow::minimumSizeHint() const {
    const int scale = identityScale();
    return {70 * scale, captionSize() + bodySize() + (4 * scale)};
}

void EquipmentSlotRow::paintEvent(QPaintEvent* /*event*/) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const ColorTokens& color = identityTokens().color;
    const int scale = identityScale();
    const QFont base = font();

    // Un emplacement vide porte le tiret cadratin. C'est lui qui decide de l'aspect du medaillon :
    // plein et cercle d'or s'il porte quelque chose, en reserve et cercle de brun sinon. On compte
    // ainsi les emplacements libres sans lire une seule ligne.
    const bool filled = !_item.isEmpty() && _item != QString::fromUtf8("\u2014");

    const int discSide = std::min(height(), captionSize() + bodySize() + (4 * scale));
    const int discX = _mirrored ? (width() - discSide) : 0;
    const QRectF disc(discX, (height() - discSide) / 2.0, discSide, discSide);

    // Le remplissage est pose en DEUX temps, jamais par un ternaire : `couleur ? QColor :
    // Qt::NoBrush` force un type commun, et `Qt::NoBrush` -- qui vaut zero -- devient un QColor
    // NOIR. Le medaillon vide se peignait ainsi en noir plein, ce qui ne ressemblait pas a une
    // erreur de code mais a un choix d'habillage.
    if (filled) {
        painter.setBrush(toColor(color.surfaceAlt));
    } else {
        painter.setBrush(Qt::NoBrush);
    }
    painter.setPen(QPen(toColor(color.frameEdge), inkPen()));
    painter.drawEllipse(disc);
    painter.setBrush(Qt::NoBrush);
    painter.setPen(QPen(toColor(filled ? color.accent : color.frameOrnament), goldPen()));
    const double inset = inkPen() + goldPen();
    painter.drawEllipse(disc.adjusted(inset, inset, -inset, -inset));

    const int glyphSide = static_cast<int>(std::lround(discSide * 0.58));
    const QPixmap glyph =
        slotIcon(_slotName, toColor(filled ? color.text : color.frameOrnament), glyphSide);
    if (!glyph.isNull()) {
        painter.drawPixmap(
            QPointF(disc.center().x() - (glyphSide / 2.0), disc.center().y() - (glyphSide / 2.0)),
            glyph);
    }

    const int gap = 4 * scale;
    const int textWidth = std::max(0, width() - discSide - gap);
    const int textX = _mirrored ? 0 : (discSide + gap);
    const int captionHeight = captionSize() + (2 * scale);
    const Qt::Alignment side = _mirrored ? Qt::AlignRight : Qt::AlignLeft;

    painter.setPen(toColor(color.frameOrnament));
    painter.setFont(sizedFont(base, captionSize(), true));
    painter.drawText(QRect(textX, 0, textWidth, captionHeight), side | Qt::AlignVCenter, _caption);

    painter.setPen(toColor(filled ? color.text : color.textMuted));
    painter.setFont(sizedFont(base, bodySize(), filled));
    const QRect itemBox(textX, captionHeight, textWidth, height() - captionHeight);
    painter.drawText(itemBox, side | Qt::AlignVCenter,
                     painter.fontMetrics().elidedText(_item, Qt::ElideRight, itemBox.width()));
}

// =================================================================================== SheetTabBar

SheetTabBar::SheetTabBar(QWidget* parent) : QWidget(parent) {
    setAttribute(Qt::WA_StyledBackground, false);
    // La barre prend le focus clavier : c'est ce qui permet de changer d'onglet aux fleches, donc
    // a la manette, sans souris (EX-IHM-071).
    setFocusPolicy(Qt::StrongFocus);
}

void SheetTabBar::setTabs(const QStringList& labels) {
    _labels = labels;
    _current = std::clamp(_current, 0, static_cast<int>(_labels.size()) - 1);
    updateGeometry();
    update();
}

void SheetTabBar::setCurrentIndex(int index) {
    if (index < 0 || index >= _labels.size() || index == _current) {
        return;
    }
    _current = index;
    update();
    emit currentChanged(_current);
}

QSize SheetTabBar::sizeHint() const {
    int total = 0;
    for (int index = 0; index < _labels.size(); ++index) {
        total += tabWidth(index);
    }
    const int scale = identityScale();
    // La hauteur est celle d'un onglet d'Options : le corps de texte, plus `small` d'air en haut
    // et en bas, plus les deux traits.
    const int band = bodySize() + (2 * identityBaseScale().spaceSmall * scale) + (2 * framePen());
    return {std::max(total, 60 * scale), band};
}

QSize SheetTabBar::minimumSizeHint() const {
    const int scale = identityScale();
    const int band = bodySize() + (2 * identityBaseScale().spaceSmall * scale) + (2 * framePen());
    return {60 * scale, band};
}

int SheetTabBar::tabWidth(int index) const {
    if (index < 0 || index >= _labels.size()) {
        return 0;
    }
    const int scale = identityScale();
    const QFontMetrics metrics(sizedFont(font(), bodySize(), true));
    // La largeur est TOUJOURS calculee sur la fonte grasse, celle de l'onglet actif. Sinon un
    // onglet s'elargirait en devenant actif, et les autres glisseraient sous le curseur -- une
    // barre dont les reperes bougent quand on la parcourt.
    //
    // La gouttiere est celle des onglets d'Options : `padding: small large` de part et d'autre,
    // soit deux fois `identity.space.large`.
    const int gutter = 2 * identityBaseScale().spaceLarge * scale;
    return metrics.horizontalAdvance(_labels.at(index)) + gutter + (2 * framePen());
}

QRect SheetTabBar::tabRect(int index) const {
    if (index < 0 || index >= _labels.size()) {
        return {};
    }
    // Chaque onglet prend la largeur de son texte, comme sur la planche. Les repartir a parts
    // egales donnerait, a deux onglets, deux moities d'ecran -- et l'onglet actif se lirait comme
    // un panneau plutot que comme un onglet.
    int left = 0;
    for (int before = 0; before < index; ++before) {
        left += tabWidth(before);
    }
    return {left, 0, tabWidth(index), height()};
}

void SheetTabBar::paintEvent(QPaintEvent* /*event*/) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const ColorTokens& color = identityTokens().color;
    const int scale = identityScale();
    const QFont base = font();

    // Anticrenelage DESACTIVE pour les bandes : ce sont des rectangles alignes sur la grille de
    // pixels, comme ceux de `ParchmentPanel`, et une demi-teinte sur leurs aretes ferait une
    // couture claire la ou deux onglets se touchent.
    painter.setRenderHint(QPainter::Antialiasing, false);

    // La recette est celle des onglets d'Options (`#OptionsPage QTabBar::tab` dans la feuille de
    // style), et non une recette propre a cette planche : reserve `surfaceAlt` et texte delave au
    // repos, INVERSION complete -- fond d'accent, texte de la couleur du fond -- pour l'onglet
    // courant. Deux barres d'onglets qui ne se ressemblent pas obligent a reapprendre la meme
    // chose deux fois, et c'est exactement ce qui faisait lire la fiche comme un ecran etranger.
    const int thickness = framePen();
    for (int index = 0; index < _labels.size(); ++index) {
        const QRect box = tabRect(index);
        const bool active = (index == _current);

        painter.setBrush(toColor(active ? color.accent : color.surfaceAlt));
        painter.setPen(QPen(toColor(color.frameEdge), thickness));
        painter.drawRect(box.adjusted(0, 0, -1, -1));

        painter.setFont(sizedFont(base, bodySize(), true));
        painter.setPen(toColor(active ? color.background : color.textMuted));
        const int gutter = identityBaseScale().spaceLarge * scale;
        painter.drawText(box.adjusted(gutter, 0, -gutter, 0), Qt::AlignCenter, _labels.at(index));
    }

    // Pas de filet de fermeture sous la barre : les onglets ne sont PAS accoles au panneau du
    // corps -- un espacement les en separe, comme partout ailleurs dans la page -- et un filet
    // interrompu sous l'onglet courant ne dirait rien qu'on ne voie deja. Il ne se serait de toute
    // facon pas distingue du trait que chaque onglet porte deja sur son propre bord bas.
    const QRect current = tabRect(_current);

    // Le fleuron ne marque plus la SELECTION -- l'inversion de l'onglet courant s'en charge -- mais
    // le FOCUS clavier, qui sans lui ne se voit nulle part : une barre parcourue a la manette doit
    // dire ou l'on est avant de dire ce qui est choisi (EX-IHM-071).
    if (hasFocus()) {
        const int marker = std::min(current.height() / 2, identityBaseScale().body * scale);
        paintFocusFleuron(painter, QRect(current.left() + (2 * scale),
                                         current.center().y() - (marker / 2), marker, marker));
    }
}

void SheetTabBar::focusInEvent(QFocusEvent* event) {
    QWidget::focusInEvent(event);
    update();
}

void SheetTabBar::focusOutEvent(QFocusEvent* event) {
    QWidget::focusOutEvent(event);
    update();
}

void SheetTabBar::mousePressEvent(QMouseEvent* event) {
    for (int index = 0; index < _labels.size(); ++index) {
        if (tabRect(index).contains(event->pos())) {
            setCurrentIndex(index);
            setFocus(Qt::MouseFocusReason);
            return;
        }
    }
    QWidget::mousePressEvent(event);
}

void SheetTabBar::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Left) {
        setCurrentIndex(_current - 1);
        event->accept();
        return;
    }
    if (event->key() == Qt::Key_Right) {
        setCurrentIndex(_current + 1);
        event->accept();
        return;
    }
    QWidget::keyPressEvent(event);
}

// =================================================================================== RoundShield

RoundShield::RoundShield(QWidget* parent) : QWidget(parent) {
    setAttribute(Qt::WA_StyledBackground, false);
}

QSize RoundShield::sizeHint() const {
    const int side = 96 * identityScale();
    return {side, side};
}

QSize RoundShield::minimumSizeHint() const {
    const int side = 56 * identityScale();
    return {side, side};
}

void RoundShield::paintEvent(QPaintEvent* /*event*/) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const ColorTokens& color = identityTokens().color;
    const double side = std::min(width(), height());
    const QPointF center(width() / 2.0, height() / 2.0);
    const double radius = side / 2.0;

    // Quatre anneaux concentriques, du bord vers le centre : le trait d'encre, la bande
    // d'entrelacs, le champ, et l'umbo. C'est ce que le bouclier du corpus montre, ramene a ce qui
    // se lit encore a cinquante pixels -- la tresse tissee, elle, ne se lit plus.
    painter.setBrush(toColor(color.surface));
    painter.setPen(QPen(toColor(color.frameEdge), inkPen()));
    painter.drawEllipse(center, radius, radius);

    // La bande d'entrelacs : un trait ROMPU, large. L'alternance tient lieu de tresse, et reste
    // visible la ou le motif tisse deviendrait une bouillie.
    QPen braid(toColor(color.accent), std::max(2.0, radius * 0.10));
    braid.setDashPattern({2.2, 1.6});
    painter.setBrush(Qt::NoBrush);
    painter.setPen(braid);
    painter.drawEllipse(center, radius * 0.80, radius * 0.80);

    painter.setBrush(toColor(color.surfaceAlt));
    painter.setPen(QPen(toColor(color.frameOrnament), goldPen()));
    painter.drawEllipse(center, radius * 0.62, radius * 0.62);

    // Les rivets du champ, aux quatre points cardinaux, puis l'umbo au centre.
    painter.setBrush(toColor(color.accent));
    painter.setPen(QPen(toColor(color.frameEdge), goldPen()));
    const double rivet = std::max(2.0, radius * 0.07);
    for (const QPointF& offset : {QPointF(0, -radius * 0.42), QPointF(0, radius * 0.42),
                                  QPointF(-radius * 0.42, 0), QPointF(radius * 0.42, 0)}) {
        painter.drawEllipse(center + offset, rivet, rivet);
    }
    painter.drawEllipse(center, radius * 0.17, radius * 0.17);
}

}  // namespace hmi
