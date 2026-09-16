// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "HMI/Input/KeyName.h"

#include <array>
#include <iomanip>
#include <sstream>

namespace hmi {

namespace {
// Nombre de codes de touche suivis par InputState (KEY_COUNT, InputState.h) : bornes de balayage
// de capturedKey, identiques a celles de l'etat clavier lui-meme.
constexpr int KEY_CODE_COUNT = 256;
}  // namespace

// Nom lisible d'une touche : les enumerateurs nommes de Key en premier (repli generique ensuite,
// meme patron que tileTypeName/screenName ailleurs dans le projet - switch exhaustif sans default,
// suivi d'un repli explicite plutot qu'une branche "unreachable").
std::string keyDisplayName(Key key) {
    switch (key) {
        case Key::Backspace:
            return "Retour arriere";
        case Key::Tab:
            return "Tab";
        case Key::Enter:
            return "Entree";
        case Key::Shift:
            return "Maj";
        case Key::Control:
            return "Ctrl";
        case Key::Escape:
            return "Echap";
        case Key::Space:
            return "Espace";
        case Key::Left:
            return "Fleche gauche";
        case Key::Up:
            return "Fleche haut";
        case Key::Right:
            return "Fleche droite";
        case Key::Down:
            return "Fleche bas";
        case Key::D0:
            return "0";
        case Key::A:
            return "A";
        case Key::C:
            return "C";
        case Key::D:
            return "D";
        case Key::P:
            return "P";
        case Key::Q:
            return "Q";
        case Key::R:
            return "R";
        case Key::S:
            return "S";
        case Key::T:
            return "T";
        case Key::V:
            return "V";
        case Key::W:
            return "W";
        case Key::Y:
            return "Y";
        case Key::Z:
            return "Z";
        case Key::F1:
            return "F1";
        case Key::F2:
            return "F2";
        case Key::F10:
            return "F10";
    }

    const int code = static_cast<int>(key);
    if (code >= '0' && code <= '9') {
        return std::string(1, static_cast<char>(code));
    }
    if (code >= 'A' && code <= 'Z') {
        return std::string(1, static_cast<char>(code));
    }
    std::ostringstream stream;
    stream << "Touche 0x" << std::hex << std::uppercase << std::setw(2) << std::setfill('0')
           << code;
    return stream.str();
}

std::optional<Key> capturedKey(const InputState& input) {
    for (int code = 1; code < KEY_CODE_COUNT; ++code) {
        const Key candidate = static_cast<Key>(code);
        if (candidate == Key::Escape || candidate == Key::Enter) {
            continue;  // reservees a la navigation globale : jamais capturees (LOT-29).
        }
        if (input.keyPressed(candidate)) {
            return candidate;
        }
    }
    return std::nullopt;
}

}  // namespace hmi
