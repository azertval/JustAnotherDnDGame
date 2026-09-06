// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Interface/FocusMarker.h"

#include <QPainter>
#include <QPolygon>
#include <QRect>
#include <algorithm>
#include <vector>

#include "HMI/Interface/ApplicationTheme.h"
#include "HMI/Interface/DesignTokens.h"
#include "HMI/Interface/ParchmentFrame.h"

namespace hmi {

namespace {

/// Cote du carre englobant de la marque, a l'echelle courante -- meme grandeur que celle employee
/// par `MenuEntryButton`, pour que la marque soit la meme d'un ecran a l'autre.
[[nodiscard]] int markerSide() {
    return identityBaseScale().sectionTitle * identityScale();
}

/// Ecart entre la marque et le controle qu'elle designe.
[[nodiscard]] int markerGap() {
    return identityBaseScale().spaceSmall * identityScale();
}

}  // namespace

void paintFocusFleuron(QPainter& painter, const QRect& box) {
    // Un rectangle non carre est ramene a son plus petit cote puis centre : le fleuron etire ne se
    // reconnait plus, et une marque qu'on ne reconnait pas ne designe rien.
    const int side = std::min(box.width(), box.height());
    const std::vector<ParchmentPoint> points = focusFleuronPoints(side);
    if (points.empty()) {
        return;
    }
    const int originX = box.x() + ((box.width() - side) / 2);
    const int originY = box.y() + ((box.height() - side) / 2);

    QPolygon polygone;
    polygone.reserve(static_cast<int>(points.size()));
    for (const ParchmentPoint& point : points) {
        polygone << QPoint(originX + point.x, originY + point.y);
    }

    const DesignColor accent = identityTokens().color.accent;
    // Anticrenelage ACTIF, contrairement aux bandes de l'encadrement : les aretes obliques du
    // fleuron sont les seules du jeu a ne pas suivre la grille de pixels, et la charte parchemin
    // n'impose plus le filtrage au plus proche voisin qui l'interdisait (LOT-66). Sans lui, la
    // pointe redeviendrait l'escalier du pixel art.
    const QPainter::RenderHints hints = painter.renderHints();
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(accent.r, accent.g, accent.b));
    painter.drawPolygon(polygone);
    painter.setBrush(Qt::NoBrush);
    painter.setRenderHints(hints);
}

FocusMarker::FocusMarker(QWidget* host) : QWidget(host) {
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setFocusPolicy(Qt::NoFocus);
    hide();
}

void FocusMarker::follow(QWidget* focused) {
    QWidget* const host = parentWidget();
    // Focus sorti de l'ecran hote (ou pose sur l'hote lui-meme, qui n'est pas un controle) : la
    // marque n'a plus rien a designer.
    if (host == nullptr || focused == nullptr || focused == host || !host->isAncestorOf(focused) ||
        !focused->isVisible()) {
        hide();
        return;
    }

    const int side = markerSide();
    if (focusFleuronPoints(side).empty()) {
        hide();  // echelle trop petite pour un fleuron reconnaissable (voir focusFleuronPoints).
        return;
    }

    // Position du controle dans le repere de l'hote : `focused` peut etre a plusieurs niveaux de
    // profondeur (onglet, disposition imbriquee), d'ou le passage par les coordonnees globales.
    const QPoint topLeft = host->mapFromGlobal(focused->mapToGlobal(QPoint(0, 0)));
    const int x = topLeft.x() - side - markerGap();
    const int y = topLeft.y() + ((focused->height() - side) / 2);

    // Un controle colle au bord gauche ne laisse pas la place : la marque est alors inutilisable,
    // et la masquer vaut mieux que la peindre par-dessus un voisin.
    if (x < 0) {
        hide();
        return;
    }

    setGeometry(x, y, side, side);
    raise();
    show();
    update();
}

void FocusMarker::paintEvent(QPaintEvent* event) {
    (void)event;
    QPainter painter(this);
    paintFocusFleuron(painter, rect());
}

}  // namespace hmi
