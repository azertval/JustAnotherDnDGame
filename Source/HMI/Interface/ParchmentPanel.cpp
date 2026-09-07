// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Interface/ParchmentPanel.h"

#include <QColor>
#include <QPainter>
#include <QPoint>
#include <algorithm>
#include <vector>

#include "HMI/Interface/ApplicationTheme.h"
#include "HMI/Interface/DesignTokens.h"
#include "HMI/Interface/OrnamentPainter.h"
#include "HMI/Interface/ParchmentFrame.h"
#include "HMI/Interface/ParchmentOrnaments.h"

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

    // Les cabochons d'angle, par-dessus tout le reste (LOT-76). Le carre dore que les bandes
    // viennent de poser reste le SERTI : l'octogone du cabochon s'y inscrit, et les quatre
    // triangles qui depassent sont de la meme couleur -- le carre ne se voit donc pas, il porte.
    //
    // La variante accentuee garde ses carres nus : son filet passe a la couleur d'accent pour
    // signaler un ecran superpose, et une gemme dessinee par-dessus rendrait ce signal illisible.
    if (_accented) {
        return;
    }
    const int corner = parchmentFrameCorner(width(), height(), identityScale());
    if (corner <= 0) {
        return;
    }
    // La pierre deborde du carre d'angle (PARCHMENT_CABOCHON_FACTOR), mais reste ancree AU COIN et
    // jamais centree dessus : centree, elle sortirait du panneau d'une demi-largeur, et Qt la
    // rognerait -- une pierre coupee en deux a chaque angle, sans qu'aucune erreur ne le dise.
    const int stone = std::min(corner * PARCHMENT_CABOCHON_FACTOR, std::min(width(), height()) / 2);
    const std::vector<OrnamentShape> cabochon = cabochonShapes(stone);
    for (const int x : {0, width() - stone}) {
        for (const int y : {0, height() - stone}) {
            paintOrnaments(painter, cabochon, QPoint(x, y));
        }
    }
}

}  // namespace hmi
