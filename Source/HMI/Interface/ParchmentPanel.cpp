// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Interface/ParchmentPanel.h"

#include <QColor>
#include <QPainter>

#include "HMI/Interface/ApplicationTheme.h"
#include "HMI/Interface/DesignTokens.h"
#include "HMI/Interface/ParchmentFrame.h"

namespace hmi {

namespace {

[[nodiscard]] QColor toQColor(DesignColor color) {
    return QColor(color.r, color.g, color.b, color.a);
}

}  // namespace

ParchmentPanel::ParchmentPanel(QWidget* parent) : QWidget(parent) {
    // L'encadrement est peint ici, pas par la feuille de style : sans fond propre, Qt laisserait
    // transparaitre ce qui se trouve derriere entre les bandes.
    setAttribute(Qt::WA_StyledBackground, false);
}

void ParchmentPanel::setAccented(bool accented) {
    if (_accented == accented) {
        return;
    }
    _accented = accented;
    update();
}

void ParchmentPanel::paintEvent(QPaintEvent* event) {
    QWidget::paintEvent(event);

    // Jetons de la portee IDENTITE, jamais ceux du chassis : ces encadrements n'existent que sur
    // les ecrans du jeu, dont l'apparence ne suit aucun reglage de theme (EX-IHM-054).
    const ColorTokens& color = identityTokens().color;
    QPainter painter(this);
    // Anticrenelage DESACTIVE : toutes les bandes sont des rectangles alignes sur la grille de
    // pixels, et l'anticrenelage y ajouterait une demi-teinte sur chaque arete adjacente -- une
    // couture claire visible la ou deux bandes se touchent. C'est une contrainte de TUILAGE, et
    // non celle du filtrage au plus proche voisin, que la charte parchemin a abandonnee : le
    // fleuron de focus, lui, est bien anticrenele (voir hmi::FocusMarker).
    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.setPen(Qt::NoPen);

    for (const ParchmentStroke& stroke :
         parchmentFrameStrokes(width(), height(), identityScale())) {
        QColor fill;
        switch (stroke.role) {
            case ParchmentRole::Field:
                fill = toQColor(color.surface);
                break;
            case ParchmentRole::Edge:
                fill = toQColor(color.frameEdge);
                break;
            case ParchmentRole::Ornament:
                // Variante accentuee : seul l'ORNEMENT passe a l'accent. Accentuer aussi le trait
                // exterieur ferait disparaitre le contraste entre les deux, et l'encadrement se
                // lirait comme une bordure doree epaisse.
                fill = _accented ? toQColor(color.accent) : toQColor(color.frameOrnament);
                break;
            case ParchmentRole::Shadow:
                fill = toQColor(color.frameShadow);
                break;
        }
        painter.fillRect(stroke.x, stroke.y, stroke.width, stroke.height, fill);
    }
}

}  // namespace hmi
