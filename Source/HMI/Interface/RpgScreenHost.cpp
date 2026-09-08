// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Interface/RpgScreenHost.h"

#include <QStackedWidget>
#include <QVBoxLayout>

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
        auto* const frame = new RpgScreenFrame(descriptor, _stack);
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
    // Les valeurs sont reposées après la langue : `retranslateUi` remet tout au tiret cadratin,
    // et sans ce rejeu un changement de langue viderait une fiche remplie.
    for (auto entree = _screenValues.constBegin(); entree != _screenValues.constEnd(); ++entree) {
        if (RpgScreenFrame* const frame = _frames.value(entree.key(), nullptr); frame != nullptr) {
            frame->setValues(entree.value());
        }
    }
}

void RpgScreenHost::setValues(RpgScreenId screen,
                              const std::map<std::string, std::string>& values) {
    if (RpgScreenFrame* const frame = _frames.value(static_cast<int>(screen), nullptr);
        frame != nullptr) {
        frame->setValues(values);
    }
    // Retenues, parce qu'un changement de langue les rejoue (`retranslateUi`).
    _screenValues.insert(static_cast<int>(screen), values);
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
