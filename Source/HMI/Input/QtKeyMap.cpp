// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "HMI/Input/QtKeyMap.h"

#include <QtGlobal>

#include <qnamespace.h>

namespace hmi {

std::optional<hmi::Key> qtKeyToHmiKey(int qtKey) {
    switch (qtKey) {
        case Qt::Key_Escape:
            return hmi::Key::Escape;
        case Qt::Key_Tab:
        case Qt::Key_Backtab:
            return hmi::Key::Tab;
        case Qt::Key_Return:
        case Qt::Key_Enter:
            return hmi::Key::Enter;
        case Qt::Key_Backspace:
            return hmi::Key::Backspace;
        case Qt::Key_Shift:
            return hmi::Key::Shift;
        case Qt::Key_Control:
            return hmi::Key::Control;
        case Qt::Key_Space:
            return hmi::Key::Space;
        case Qt::Key_Left:
            return hmi::Key::Left;
        case Qt::Key_Up:
            return hmi::Key::Up;
        case Qt::Key_Right:
            return hmi::Key::Right;
        case Qt::Key_Down:
            return hmi::Key::Down;
        case Qt::Key_F1:
            return hmi::Key::F1;
        case Qt::Key_F2:
            return hmi::Key::F2;
        case Qt::Key_F10:
            return hmi::Key::F10;
        default:
            break;
    }
    // Lettres A–Z et chiffres 0–9 : Qt reprend les valeurs ASCII majuscules, identiques aux codes
    // virtuels Win32 — conversion directe.
    if ((qtKey >= Qt::Key_0 && qtKey <= Qt::Key_9) || (qtKey >= Qt::Key_A && qtKey <= Qt::Key_Z)) {
        return static_cast<hmi::Key>(qtKey);
    }
    return std::nullopt;
}

int hmiKeyToQtKey(hmi::Key key) {
    switch (key) {
        case hmi::Key::Escape:
            return Qt::Key_Escape;
        case hmi::Key::Tab:
            return Qt::Key_Tab;
        case hmi::Key::Enter:
            return Qt::Key_Return;
        case hmi::Key::Backspace:
            return Qt::Key_Backspace;
        case hmi::Key::Shift:
            return Qt::Key_Shift;
        case hmi::Key::Control:
            return Qt::Key_Control;
        case hmi::Key::Space:
            return Qt::Key_Space;
        case hmi::Key::Left:
            return Qt::Key_Left;
        case hmi::Key::Up:
            return Qt::Key_Up;
        case hmi::Key::Right:
            return Qt::Key_Right;
        case hmi::Key::Down:
            return Qt::Key_Down;
        case hmi::Key::F1:
            return Qt::Key_F1;
        case hmi::Key::F2:
            return Qt::Key_F2;
        case hmi::Key::F10:
            return Qt::Key_F10;
        default:
            // Lettres A-Z, chiffres 0-9 : meme valeur Win32 <-> Qt (cf. qtKeyToHmiKey ci-dessus).
            return static_cast<int>(key);
    }
}

}  // namespace hmi
