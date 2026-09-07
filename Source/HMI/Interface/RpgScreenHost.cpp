// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Interface/RpgScreenHost.h"

#include <QStackedWidget>
#include <QVBoxLayout>

#include "HMI/Interface/CharacterSheetPage.h"
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
        // Un écran qui déclare une PLANCHE (LOT-38) reçoit sa mise en page Qt Designer ; les
        // autres gardent le rendu générique de leur ossature (LOT-68). C'est la TABLE qui le dit,
        // pas ce code : il ne connaît toujours aucun écran par son nom.
        //
        // Une seule planche existe à ce jour, celle de la fiche : `_sheetPage` la retient pour
        // pouvoir lui passer ses valeurs. La deuxième demandera de retenir laquelle est laquelle,
        // et ce sera le moment de le faire -- pas avant.
        QWidget* planche = nullptr;
        if (descriptor.rendering == RpgRendering::Plate) {
            _sheetPage = new CharacterSheetPage();
            planche = _sheetPage;
        }
        auto* const frame = planche != nullptr ? new RpgScreenFrame(descriptor, planche, _stack)
                                               : new RpgScreenFrame(descriptor, _stack);
        connect(frame, &RpgScreenFrame::closeRequested, this, &RpgScreenHost::closeRequested);
        connect(frame, &RpgScreenFrame::nextScreenRequested, this, &RpgScreenHost::showNextScreen);
        connect(frame, &RpgScreenFrame::previousScreenRequested, this,
                &RpgScreenHost::showPreviousScreen);
        _frames.insert(static_cast<int>(descriptor.id), frame);
        _stack->addWidget(frame);
    }
    _current = rpgScreens().front().id;
}

void RpgScreenHost::retranslateUi(const Localization& loc) {
    for (RpgScreenFrame* const frame : _frames) {
        frame->retranslateUi(loc);
    }
    if (_sheetPage != nullptr) {
        _sheetPage->retranslateUi(loc);
        // La planche repasse ses valeurs après la langue : `retranslateUi` réécrit les intitulés,
        // et rejouer les valeurs évite qu'un changement de langue vide une fiche remplie.
        _sheetPage->applyValues(_sheetValues, loc.text("rpg.empty"));
    }
}

void RpgScreenHost::setValues(RpgScreenId screen,
                              const std::map<std::string, std::string>& values) {
    if (RpgScreenFrame* const frame = _frames.value(static_cast<int>(screen), nullptr);
        frame != nullptr) {
        frame->setValues(values);
    }
    if (screen == RpgScreenId::CharacterSheet) {
        // Retenues, parce qu'un changement de langue les rejoue (`retranslateUi`).
        _sheetValues = values;
        if (_sheetPage != nullptr) {
            _sheetPage->applyValues(values, "—");
        }
    }
}

void RpgScreenHost::showScreen(RpgScreenId screen) {
    RpgScreenFrame* const frame = _frames.value(static_cast<int>(screen), nullptr);
    if (frame == nullptr) {
        return;
    }
    _current = screen;
    _stack->setCurrentWidget(frame);
    frame->focusDefaultAction();
    emit screenChanged(screen);
}

void RpgScreenHost::showNextScreen() {
    showScreen(nextRpgScreen(_current));
}

void RpgScreenHost::showPreviousScreen() {
    showScreen(previousRpgScreen(_current));
}

void RpgScreenHost::focusDefaultAction() {
    if (RpgScreenFrame* const frame = _frames.value(static_cast<int>(_current), nullptr);
        frame != nullptr) {
        frame->focusDefaultAction();
    }
}

}  // namespace hmi
