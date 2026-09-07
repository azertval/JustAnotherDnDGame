// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Interface/TitleBanner.h"

#include <QFontMetrics>
#include <QPainter>
#include <QPoint>
#include <QResizeEvent>
#include <algorithm>
#include <vector>

#include "HMI/Interface/ApplicationTheme.h"
#include "HMI/Interface/OrnamentPainter.h"
#include "HMI/Interface/ParchmentOrnaments.h"

namespace hmi {

namespace {

/// Marge entre le texte et le chevron de la plaque, en unites d'agrandissement. Deux unites : sous
/// cela, la premiere lettre touche la pointe du chevron, qui la fait lire comme tronquee.
constexpr int TEXT_PADDING_UNITS = 2;

[[nodiscard]] int textPadding() {
    return TEXT_PADDING_UNITS * identityScale();
}

}  // namespace

TitleBanner::TitleBanner(QWidget* parent) : QLabel(parent) {
    setAlignment(Qt::AlignCenter);
    // Le bandeau est peint ICI, pas par la feuille de style : un QLabel sans fond propre laisserait
    // transparaitre le parchemin de l'ecran entre les ailes et la plaque.
    setAttribute(Qt::WA_StyledBackground, false);
    updateMargins();
}

QSize TitleBanner::sizeHint() const {
    const QSize text = fontMetrics().size(Qt::TextSingleLine, this->text());
    // La hauteur commande TOUT le reste : l'envergure des ailes en depend, donc la largeur aussi.
    // La calculer depuis la largeur du texte serait circulaire.
    const int bannerHeight = text.height() + (4 * identityScale());
    const int bannerWidth = text.width() + (2 * bannerWingSpan(bannerHeight)) + (2 * textPadding());
    return {bannerWidth, bannerHeight};
}

QSize TitleBanner::minimumSizeHint() const {
    return sizeHint();
}

void TitleBanner::resizeEvent(QResizeEvent* event) {
    QLabel::resizeEvent(event);
    updateMargins();
}

void TitleBanner::updateMargins() {
    // Les marges de contenu, et non un decalage au moment de peindre : c'est la disposition de Qt
    // qui doit savoir ou le texte tient, sinon `wordWrap` et l'elision decident sur la mauvaise
    // largeur -- et le mot coupe n'apparait que sur le titre le plus long.
    const int span = bannerWingSpan(height()) + textPadding();
    setContentsMargins(span, 0, span, 0);
}

void TitleBanner::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    const std::vector<OrnamentShape> shapes = titleBannerShapes(width(), height());
    paintOrnaments(painter, shapes, QPoint(0, 0));
    painter.end();
    // Le texte APRES l'ornement, et par la peinture normale du QLabel : c'est elle qui applique la
    // police et la couleur de la feuille de style (EX-IHM-051), qu'un dessin a la main ici
    // court-circuiterait.
    QLabel::paintEvent(event);
}

}  // namespace hmi
