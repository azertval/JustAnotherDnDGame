// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Interface/MenuEntryButton.h"

#include <QPainter>
#include <QRect>
#include <algorithm>

#include "HMI/Interface/ApplicationTheme.h"
#include "HMI/Interface/DesignTokens.h"
#include "HMI/Interface/FocusMarker.h"

namespace hmi {

MenuEntryButton::MenuEntryButton(QWidget* parent) : QPushButton(parent) {
    setFlat(true);
    setCursor(Qt::PointingHandCursor);
}

void MenuEntryButton::enterEvent(QEnterEvent* event) {
    QPushButton::enterEvent(event);
    update();
}

void MenuEntryButton::leaveEvent(QEvent* event) {
    QPushButton::leaveEvent(event);
    update();
}

void MenuEntryButton::paintEvent(QPaintEvent* event) {
    // Le bouton se peint d'abord normalement : couleur, police, marges et etat desactive viennent
    // tous de theme-identity.qss. On n'ajoute que la marque, par-dessus.
    QPushButton::paintEvent(event);

    // Un bouton indisponible ne porte jamais la marque, meme survole : elle indiquerait une action
    // qui n'aura pas lieu.
    if (!isEnabled() || (!hasFocus() && !underMouse())) {
        return;
    }

    const int scale = identityScale();
    // Gouttiere reservee a la marque : exactement la marge gauche que theme-identity.qss donne a
    // ces boutons (identity.space.medium), de sorte que le fleuron se loge devant le texte sans le
    // decaler.
    const int gutter = identityBaseScale().spaceMedium * scale;
    // Le fleuron tient DANS la gouttiere, jamais l'inverse : a la taille d'un intertitre il
    // deborderait sur le texte, qui se decalerait d'une entree a l'autre selon qu'elle porte ou
    // non la marque -- un menu dont les lignes bougent quand on les parcourt.
    const int side = std::min(gutter, identityBaseScale().sectionTitle * scale);
    // Centre dans la gouttiere, et verticalement dans le bouton. Une marque calee en haut
    // « flotterait » au-dessus du texte des que la hauteur de ligne grandit.
    const QRect box((gutter - side) / 2, (height() - side) / 2, side, side);

    QPainter painter(this);
    paintFocusFleuron(painter, box);
}

}  // namespace hmi
