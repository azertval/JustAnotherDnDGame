// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "HMI/Input/GamepadBindings.h"

#include <fstream>
#include <optional>
#include <string>
#include <system_error>

#include <nlohmann/json.hpp>

#include "HMI/HmiLog.h"

namespace hmi {

namespace {

// Nom JSON de chaque action (section "manette" de keybindings.json) - meme cle que la section
// "jeu" de GameKeyBindings, les deux partagent le meme GameAction.
[[nodiscard]] std::string actionName(GameAction action) {
    switch (action) {
        case GameAction::MoveLeft:
            return "gauche";
        case GameAction::MoveRight:
            return "droite";
        case GameAction::AimUp:
            return "haut";
        case GameAction::AimDown:
            return "bas";
        case GameAction::Jump:
            return "sauter";
        case GameAction::Dash:
            return "dash";
        case GameAction::Interact:
            return "interagir";
    }
    return "";
}

// Nom JSON de chaque bouton (aucun code natif reutilisable, contrairement a Key/VK) - inverse
// implicite au load.
[[nodiscard]] std::string buttonName(GamepadButton button) {
    switch (button) {
        case GamepadButton::Up:
            return "haut";
        case GamepadButton::Down:
            return "bas";
        case GamepadButton::Left:
            return "gauche";
        case GamepadButton::Right:
            return "droite";
        case GamepadButton::A:
            return "a";
        case GamepadButton::B:
            return "b";
        case GamepadButton::X:
            return "x";
        case GamepadButton::Y:
            return "y";
        case GamepadButton::LeftShoulder:
            return "lb";
        case GamepadButton::RightShoulder:
            return "rb";
    }
    return "";
}

// Inverse de buttonName ; nullopt si le nom ne correspond a aucun bouton connu.
[[nodiscard]] std::optional<GamepadButton> parseButtonName(const std::string& name) {
    if (name == "haut") {
        return GamepadButton::Up;
    }
    if (name == "bas") {
        return GamepadButton::Down;
    }
    if (name == "gauche") {
        return GamepadButton::Left;
    }
    if (name == "droite") {
        return GamepadButton::Right;
    }
    if (name == "a") {
        return GamepadButton::A;
    }
    if (name == "b") {
        return GamepadButton::B;
    }
    if (name == "x") {
        return GamepadButton::X;
    }
    if (name == "y") {
        return GamepadButton::Y;
    }
    if (name == "lb") {
        return GamepadButton::LeftShoulder;
    }
    if (name == "rb") {
        return GamepadButton::RightShoulder;
    }
    return std::nullopt;
}

}  // namespace

GamepadButton GamepadBindings::defaultButton(GameAction action) noexcept {
    switch (action) {
        case GameAction::MoveLeft:
            return GamepadButton::Left;
        case GameAction::MoveRight:
            return GamepadButton::Right;
        case GameAction::AimUp:
            return GamepadButton::Up;
        case GameAction::AimDown:
            return GamepadButton::Down;
        case GameAction::Jump:
            return GamepadButton::A;
        case GameAction::Dash:
            return GamepadButton::RightShoulder;
        case GameAction::Interact:
            return GamepadButton::X;
    }
    return GamepadButton::A;  // inatteignable : switch exhaustif sur GameAction ci-dessus.
}

GamepadBindings::GamepadBindings() {
    resetToDefaults();
}

GamepadButton GamepadBindings::button(GameAction action) const noexcept {
    return _buttons[static_cast<std::size_t>(action)];
}

// Echange avec l'action qui detenait deja newButton, s'il y en a une : jamais deux actions sur le
// meme bouton a l'issue de l'appel.
void GamepadBindings::setKey(GameAction action, GamepadButton newButton) noexcept {
    const auto index = static_cast<std::size_t>(action);
    for (std::size_t other = 0; other < _buttons.size(); ++other) {
        if (other != index && _buttons[other] == newButton) {
            _buttons[other] = _buttons[index];
            break;
        }
    }
    _buttons[index] = newButton;
}

void GamepadBindings::resetToDefaults() noexcept {
    for (std::size_t index = 0; index < _buttons.size(); ++index) {
        _buttons[index] = defaultButton(static_cast<GameAction>(index));
    }
}

// Relit le fichier existant (pour preserver les sections "jeu"/"editeur"), remplace uniquement la
// section "manette", puis reecrit. Un fichier absent/corrompu en lecture est traite comme vide.
bool GamepadBindings::save(const std::filesystem::path& path) const {
    nlohmann::json root = nlohmann::json::object();
    std::ifstream input(path, std::ios::binary);
    if (input) {
        try {
            input >> root;
        } catch (const nlohmann::json::exception&) {
            root = nlohmann::json::object();
        }
        if (!root.is_object()) {
            root = nlohmann::json::object();
        }
    }

    nlohmann::json section = nlohmann::json::object();
    for (std::size_t index = 0; index < _buttons.size(); ++index) {
        section[actionName(static_cast<GameAction>(index))] = buttonName(_buttons[index]);
    }
    root["manette"] = std::move(section);

    std::error_code errorCode;
    std::filesystem::create_directories(path.parent_path(), errorCode);
    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    if (!output) {
        return false;
    }
    const std::string dumped = root.dump(2);
    output.write(dumped.data(), static_cast<std::streamsize>(dumped.size()));
    return output.good();
}

GamepadBindings GamepadBindings::load(const std::filesystem::path& path) {
    GamepadBindings bindings;  // valeurs par defaut, ecrasees ci-dessous entree par entree.

    std::ifstream input(path, std::ios::binary);
    if (!input) {
        return bindings;  // fichier absent (premier lancement) : defauts, rien a signaler.
    }

    nlohmann::json root;
    try {
        input >> root;
    } catch (const nlohmann::json::exception& error) {
        HMI_LOG_WARNING(std::string("keybindings.json illisible (manette) : ") + error.what());
        return bindings;
    }

    const auto section = root.find("manette");
    if (section == root.end() || !section->is_object()) {
        return bindings;  // pas de section "manette" : defauts, fichier par ailleurs valide.
    }

    for (std::size_t index = 0; index < bindings._buttons.size(); ++index) {
        const auto entry = section->find(actionName(static_cast<GameAction>(index)));
        if (entry == section->end() || !entry->is_string()) {
            continue;  // entree absente/invalide : reste a sa valeur par defaut.
        }
        const std::optional<GamepadButton> parsed = parseButtonName(entry->get<std::string>());
        if (!parsed) {
            HMI_LOG_WARNING("Nom de bouton manette inconnu ignore dans keybindings.json.");
            continue;
        }
        bindings._buttons[index] = *parsed;
    }
    return bindings;
}

}  // namespace hmi
