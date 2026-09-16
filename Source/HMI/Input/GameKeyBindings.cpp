// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "HMI/Input/GameKeyBindings.h"

#include <fstream>
#include <string>
#include <system_error>

#include <nlohmann/json.hpp>

#include "HMI/HmiLog.h"

namespace hmi {

namespace {

// Nom JSON de chaque action (section "jeu" de keybindings.json) - inverse implicite lors du load.
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

}  // namespace

Key GameKeyBindings::defaultKey(GameAction action) noexcept {
    switch (action) {
        case GameAction::MoveLeft:
            return Key::Left;
        case GameAction::MoveRight:
            return Key::Right;
        case GameAction::AimUp:
            return Key::Up;
        case GameAction::AimDown:
            return Key::Down;
        case GameAction::Jump:
            return Key::Space;
        case GameAction::Dash:
            return Key::Shift;
        case GameAction::Interact:
            return Key::E;
    }
    return Key::Escape;  // inatteignable : switch exhaustif sur GameAction ci-dessus.
}

GameKeyBindings::GameKeyBindings() {
    resetToDefaults();
}

Key GameKeyBindings::key(GameAction action) const noexcept {
    return _keys[static_cast<std::size_t>(action)];
}

// Echange avec l'action qui detenait deja newKey, s'il y en a une : jamais deux actions sur la
// meme touche a l'issue de l'appel.
void GameKeyBindings::setKey(GameAction action, Key newKey) noexcept {
    const auto index = static_cast<std::size_t>(action);
    for (std::size_t other = 0; other < _keys.size(); ++other) {
        if (other != index && _keys[other] == newKey) {
            _keys[other] = _keys[index];
            break;
        }
    }
    _keys[index] = newKey;
}

void GameKeyBindings::resetToDefaults() noexcept {
    for (std::size_t index = 0; index < _keys.size(); ++index) {
        _keys[index] = defaultKey(static_cast<GameAction>(index));
    }
}

// Relit le fichier existant (pour preserver la section "editeur" ecrite par EditorKeyBindings),
// remplace uniquement la section "jeu", puis reecrit. Un fichier absent/corrompu en lecture est
// traite comme vide (on part d'un objet JSON neuf) : jamais bloquant pour la sauvegarde.
bool GameKeyBindings::save(const std::filesystem::path& path) const {
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
    for (std::size_t index = 0; index < _keys.size(); ++index) {
        section[actionName(static_cast<GameAction>(index))] = static_cast<int>(_keys[index]);
    }
    root["jeu"] = std::move(section);

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

GameKeyBindings GameKeyBindings::load(const std::filesystem::path& path) {
    GameKeyBindings bindings;  // valeurs par defaut, ecrasees ci-dessous entree par entree.

    std::ifstream input(path, std::ios::binary);
    if (!input) {
        return bindings;  // fichier absent (premier lancement) : defauts, rien a signaler.
    }

    nlohmann::json root;
    try {
        input >> root;
    } catch (const nlohmann::json::exception& error) {
        HMI_LOG_WARNING(std::string("keybindings.json illisible (touches de jeu) : ") +
                        error.what());
        return bindings;
    }

    const auto section = root.find("jeu");
    if (section == root.end() || !section->is_object()) {
        return bindings;  // pas de section "jeu" : defauts, fichier par ailleurs valide.
    }

    for (std::size_t index = 0; index < bindings._keys.size(); ++index) {
        const auto entry = section->find(actionName(static_cast<GameAction>(index)));
        if (entry == section->end() || !entry->is_number_integer()) {
            continue;  // entree absente/invalide : reste a sa valeur par defaut.
        }
        const int code = entry->get<int>();
        if (code < 0 || code > 0xFF) {
            HMI_LOG_WARNING("Code de touche hors bornes ignore dans keybindings.json (jeu).");
            continue;
        }
        bindings._keys[index] = static_cast<Key>(code);
    }
    return bindings;
}

}  // namespace hmi
