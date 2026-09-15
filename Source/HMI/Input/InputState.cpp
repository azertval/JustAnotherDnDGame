// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "HMI/Input/InputState.h"

namespace hmi {

namespace {
// Indice de tableau associé à une touche (son code virtuel Win32).
[[nodiscard]] std::size_t keyIndex(Key key) noexcept {
    return static_cast<std::size_t>(key);
}

// Indice de tableau associé à un bouton de souris.
[[nodiscard]] std::size_t buttonIndex(MouseButton button) noexcept {
    return static_cast<std::size_t>(button);
}

// Indice de tableau associé à un bouton manette (piste brute, GamepadButton).
[[nodiscard]] std::size_t gamepadButtonIndex(GamepadButton button) noexcept {
    return static_cast<std::size_t>(button);
}
}  // namespace

// Ouvre une nouvelle frame : recopie l'état courant vers l'état précédent.
//
// Seul l'état *précédent* est réécrit : l'état courant (touches encore enfoncées, position
// souris) persiste d'une frame à l'autre jusqu'au prochain événement de relâchement.
void InputState::beginFrame() noexcept {
    _keysPrevious = _keysCurrent;
    _gamepadPrevious = _gamepadCurrent;
    _buttonsPrevious = _buttonsCurrent;
    _gamepadButtonsPrevious = _gamepadButtonsCurrent;
    _wheelDelta = 0;
    _typedCharacters.clear();
}

// Relâche toutes les entrées maintenues (courant ET précédent), sans produire de front.
//
// Remettre à zéro les deux états (et pas seulement le courant) évite qu'un front « relâchée »
// n'apparaisse pour chaque touche maintenue au moment de la perte de focus : après cet appel,
// courant == précédent == relâché pour tout, donc aucun front.
void InputState::releaseAll() noexcept {
    _keysCurrent.fill(false);
    _keysPrevious.fill(false);
    _gamepadCurrent.fill(false);
    _gamepadPrevious.fill(false);
    _gamepadButtonsCurrent.fill(false);
    _gamepadButtonsPrevious.fill(false);
    _buttonsCurrent.fill(false);
    _buttonsPrevious.fill(false);
}

// Marque key comme enfoncée dans l'état courant.
//
// Le code hors de la plage suivie est ignoré (garde-fou : les événements proviennent du
// système et pourraient porter un code inattendu).
void InputState::onKeyDown(Key key) noexcept {
    const std::size_t index = keyIndex(key);
    if (index < KEY_COUNT) {
        _keysCurrent[index] = true;
    }
}

void InputState::onKeyUp(Key key) noexcept {
    const std::size_t index = keyIndex(key);
    if (index < KEY_COUNT) {
        _keysCurrent[index] = false;
    }
}

// Marque key comme enfoncée dans l'état courant, source manette (voir en-tête : tableau
// distinct du clavier, jamais fusionné en écriture).
void InputState::onGamepadKeyDown(Key key) noexcept {
    const std::size_t index = keyIndex(key);
    if (index < KEY_COUNT) {
        _gamepadCurrent[index] = true;
    }
}

void InputState::onGamepadKeyUp(Key key) noexcept {
    const std::size_t index = keyIndex(key);
    if (index < KEY_COUNT) {
        _gamepadCurrent[index] = false;
    }
}

// Marque button comme enfoncé dans l'état courant (piste manette brute, voir en-tête).
void InputState::onGamepadButtonDown(GamepadButton button) noexcept {
    _gamepadButtonsCurrent[gamepadButtonIndex(button)] = true;
}

void InputState::onGamepadButtonUp(GamepadButton button) noexcept {
    _gamepadButtonsCurrent[gamepadButtonIndex(button)] = false;
}

// Met à jour la position de la souris.
void InputState::onMouseMove(int x, int y) noexcept {
    _mouseX = x;
    _mouseY = y;
}

// Marque button comme enfoncé dans l'état courant.
void InputState::onMouseButtonDown(MouseButton button) noexcept {
    const std::size_t index = buttonIndex(button);
    if (index < BUTTON_COUNT) {
        _buttonsCurrent[index] = true;
    }
}

// Marque button comme relâché dans l'état courant.
void InputState::onMouseButtonUp(MouseButton button) noexcept {
    const std::size_t index = buttonIndex(button);
    if (index < BUTTON_COUNT) {
        _buttonsCurrent[index] = false;
    }
}

// Accumule un incrément de molette pour la frame courante (plusieurs crans dans la même frame
// s'additionnent, cas normal d'une molette tournée vite).
void InputState::onMouseWheel(int delta) noexcept {
    _wheelDelta += delta;
}

// Enregistre un caractère tapé pour la frame courante, dans l'ordre de saisie.
void InputState::onCharTyped(wchar_t character) {
    _typedCharacters.push_back(character);
}

// Declare l'etat de connexion de la manette pour cette frame.
void InputState::setGamepadConnected(bool connected) noexcept {
    _gamepadConnected = connected;
}

// true si une manette etait connectee a la derniere frame sondee.
bool InputState::gamepadConnected() const noexcept {
    return _gamepadConnected;
}

// true si key est enfoncée à cette frame (maintenue ou vient d'être pressée), clavier OU
// manette (voir en-tête : deux sources indépendantes, combinées ici en lecture).
bool InputState::keyDown(Key key) const noexcept {
    const std::size_t index = keyIndex(key);
    return index < KEY_COUNT && (_keysCurrent[index] || _gamepadCurrent[index]);
}

// Indique si key vient d'être pressée cette frame (front montant), sur l'une ou l'autre source.
bool InputState::keyPressed(Key key) const noexcept {
    const std::size_t index = keyIndex(key);
    if (index >= KEY_COUNT) {
        return false;
    }
    const bool keyboardEdge = _keysCurrent[index] && !_keysPrevious[index];
    const bool gamepadEdge = _gamepadCurrent[index] && !_gamepadPrevious[index];
    return keyboardEdge || gamepadEdge;
}

// Indique si key vient d'être relâchée cette frame (front descendant) : plus enfoncée sur
// AUCUNE source maintenant, mais l'était sur l'une d'elles a la frame precedente.
bool InputState::keyReleased(Key key) const noexcept {
    const std::size_t index = keyIndex(key);
    if (index >= KEY_COUNT) {
        return false;
    }
    const bool downNow = _keysCurrent[index] || _gamepadCurrent[index];
    const bool downBefore = _keysPrevious[index] || _gamepadPrevious[index];
    return !downNow && downBefore;
}

// true si button (piste manette brute) est enfoncé à cette frame.
bool InputState::gamepadButtonDown(GamepadButton button) const noexcept {
    return _gamepadButtonsCurrent[gamepadButtonIndex(button)];
}

// Indique si button (piste manette brute) vient d'être enfoncé cette frame (front montant).
bool InputState::gamepadButtonPressed(GamepadButton button) const noexcept {
    const std::size_t index = gamepadButtonIndex(button);
    return _gamepadButtonsCurrent[index] && !_gamepadButtonsPrevious[index];
}

// Indique si button (piste manette brute) vient d'être relâché cette frame (front descendant).
bool InputState::gamepadButtonReleased(GamepadButton button) const noexcept {
    const std::size_t index = gamepadButtonIndex(button);
    return !_gamepadButtonsCurrent[index] && _gamepadButtonsPrevious[index];
}

// Abscisse de la souris, en pixels de la zone client.
int InputState::mouseX() const noexcept {
    return _mouseX;
}

// Ordonnée de la souris, en pixels de la zone client.
int InputState::mouseY() const noexcept {
    return _mouseY;
}

// true si button est enfoncé à cette frame.
bool InputState::mouseButtonDown(MouseButton button) const noexcept {
    const std::size_t index = buttonIndex(button);
    return index < BUTTON_COUNT && _buttonsCurrent[index];
}

// Indique si button vient d'être cliqué cette frame (front montant).
// true si le bouton est enfoncé maintenant mais ne l'était pas à la frame précédente.
bool InputState::mouseButtonPressed(MouseButton button) const noexcept {
    const std::size_t index = buttonIndex(button);
    return index < BUTTON_COUNT && _buttonsCurrent[index] && !_buttonsPrevious[index];
}

// Indique si button vient d'être relâché cette frame (front descendant).
// true si le bouton n'est plus enfoncé mais l'était à la frame précédente.
bool InputState::mouseButtonReleased(MouseButton button) const noexcept {
    const std::size_t index = buttonIndex(button);
    return index < BUTTON_COUNT && !_buttonsCurrent[index] && _buttonsPrevious[index];
}

// Somme des incréments de molette accumulés depuis le dernier beginFrame().
int InputState::wheelDelta() const noexcept {
    return _wheelDelta;
}

// Caractères tapés depuis le dernier beginFrame(), dans leur ordre de saisie.
const std::vector<wchar_t>& InputState::typedCharacters() const noexcept {
    return _typedCharacters;
}

}  // namespace hmi
