// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Interface/RpgScreenHost.h"

#include <QStackedWidget>
#include <QVBoxLayout>

#include "HMI/Interface/RpgCharacterSheetPlate.h"
#include "HMI/Interface/RpgScreenFrame.h"
#include "HMI/Localization/Localization.h"

namespace hmi {

RpgScreenHost::RpgScreenHost(QWidget* parent) : QWidget(parent) {
    setObjectName(QStringLiteral("RpgScreenHost"));
    setAttribute(Qt::WA_StyledBackground, true);

    auto* const layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    _stack = new QStackedWidget(this);
    layout->addWidget(_stack);

    // La table décide, pas ce code : aucun écran n'est nommé ici, et un neuvième descripteur
    // suffirait à le voir apparaître dans la pile et dans le cycle (EX-IHM-090).
    for (const RpgScreenDescriptor& descriptor : rpgScreens()) {
        // Le genre de rendu est lu dans la TABLE, et le branchement se fait ici -- une seule fois.
        // Le reste de cet hote ne manipule que des `RpgScreenSurface` : sans cela, il faudrait
        // brancher sur le genre a la traduction, aux valeurs, au focus et a l'affichage, et le
        // cinquieme endroit serait oublie le jour d'une seconde planche.
        RpgScreenSurface* surface = nullptr;
        if (descriptor.rendering == RpgRendering::DesignerPlate) {
            auto* const plate = new RpgCharacterSheetPlate(descriptor, _stack);
            connect(plate, &RpgCharacterSheetPlate::closeRequested, this,
                    &RpgScreenHost::closeRequested);
            surface = plate;
        } else {
            auto* const frame = new RpgScreenFrame(descriptor, _stack);
            connect(frame, &RpgScreenFrame::closeRequested, this, &RpgScreenHost::closeRequested);
            connect(frame, &RpgScreenFrame::nextScreenRequested, this,
                    &RpgScreenHost::showNextScreen);
            connect(frame, &RpgScreenFrame::previousScreenRequested, this,
                    &RpgScreenHost::showPreviousScreen);
            surface = frame;
        }
        _frames.insert(static_cast<int>(descriptor.id), surface);
        _stack->addWidget(surface->widget());
    }
    _current = rpgScreens().front().id;
}

void RpgScreenHost::retranslateUi(const Localization& loc) {
    for (RpgScreenSurface* const surface : _frames) {
        surface->retranslateUi(loc);
    }
    // Les valeurs sont reposées après la langue : `retranslateUi` remet tout au tiret cadratin,
    // et sans ce rejeu un changement de langue viderait une fiche remplie.
    for (auto entree = _screenValues.constBegin(); entree != _screenValues.constEnd(); ++entree) {
        if (RpgScreenSurface* const surface = _frames.value(entree.key(), nullptr);
            surface != nullptr) {
            surface->setValues(entree.value());
        }
    }
}

void RpgScreenHost::setValues(RpgScreenId screen,
                              const std::map<std::string, std::string>& values) {
    if (RpgScreenSurface* const surface = _frames.value(static_cast<int>(screen), nullptr);
        surface != nullptr) {
        surface->setValues(values);
    }
    // Retenues, parce qu'un changement de langue les rejoue (`retranslateUi`).
    _screenValues.insert(static_cast<int>(screen), values);
}

void RpgScreenHost::showScreen(RpgScreenId screen) {
    RpgScreenSurface* const surface = _frames.value(static_cast<int>(screen), nullptr);
    if (surface == nullptr) {
        return;
    }
    _current = screen;
    _stack->setCurrentWidget(surface->widget());
    surface->focusDefaultAction();
    emit screenChanged(screen);
}

void RpgScreenHost::showNextScreen() {
    showScreen(nextRpgScreen(_current));
}

void RpgScreenHost::showPreviousScreen() {
    showScreen(previousRpgScreen(_current));
}

void RpgScreenHost::focusDefaultAction() {
    if (RpgScreenSurface* const surface = _frames.value(static_cast<int>(_current), nullptr);
        surface != nullptr) {
        surface->focusDefaultAction();
    }
}

}  // namespace hmi
