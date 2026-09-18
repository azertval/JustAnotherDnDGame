// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "HMI/Input/EditorKeyBindings.h"

#include <fstream>
#include <string>
#include <system_error>

#include <nlohmann/json.hpp>

#include "HMI/HmiLog.h"

namespace hmi {

namespace {

// Nom JSON de chaque action (section "editeur" de keybindings.json) - inverse implicite au load.
[[nodiscard]] std::string actionName(EditorAction action) {
    switch (action) {
        case EditorAction::Save:
            return "sauvegarder";
        case EditorAction::Undo:
            return "annuler";
        case EditorAction::Redo:
            return "refaire";
        case EditorAction::Copy:
            return "copier";
        case EditorAction::Paste:
            return "coller";
        case EditorAction::Playtest:
            return "testRapide";
        case EditorAction::ToggleGrid:
            return "grille";
        case EditorAction::ToggleHelp:
            return "aide";
        case EditorAction::Rename:
            return "renommer";
    }
    return "";
}

}  // namespace

Key EditorKeyBindings::defaultKey(EditorAction action) noexcept {
    switch (action) {
        case EditorAction::Save:
            return Key::S;
        case EditorAction::Undo:
            return Key::Z;
        case EditorAction::Redo:
            return Key::Y;
        case EditorAction::Copy:
            return Key::C;
        case EditorAction::Paste:
            return Key::V;
        case EditorAction::Playtest:
            return Key::P;
        case EditorAction::ToggleGrid:
            return Key::F10;
        case EditorAction::ToggleHelp:
            return Key::F1;
        case EditorAction::Rename:
            return Key::F2;
    }
    return Key::Escape;  // inatteignable : switch exhaustif sur EditorAction ci-dessus.
}

EditorKeyBindings::EditorKeyBindings() {
    resetToDefaults();
}

Key EditorKeyBindings::key(EditorAction action) const noexcept {
    return _keys[static_cast<std::size_t>(action)];
}

// Echange avec l'action qui detenait deja newKey, s'il y en a une : jamais deux actions sur la
// meme touche a l'issue de l'appel.
void EditorKeyBindings::setKey(EditorAction action, Key newKey) noexcept {
    const auto index = static_cast<std::size_t>(action);
    for (std::size_t other = 0; other < _keys.size(); ++other) {
        if (other != index && _keys[other] == newKey) {
            _keys[other] = _keys[index];
            break;
        }
    }
    _keys[index] = newKey;
}

void EditorKeyBindings::resetToDefaults() noexcept {
    for (std::size_t index = 0; index < _keys.size(); ++index) {
        _keys[index] = defaultKey(static_cast<EditorAction>(index));
    }
}

// Relit le fichier existant (pour preserver la section "jeu" ecrite par GameKeyBindings), remplace
// uniquement la section "editeur", puis reecrit. Un fichier absent/corrompu en lecture est traite
// comme vide (on part d'un objet JSON neuf) : jamais bloquant pour la sauvegarde.
bool EditorKeyBindings::save(const std::filesystem::path& path) const {
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
        section[actionName(static_cast<EditorAction>(index))] = static_cast<int>(_keys[index]);
    }
    root["editeur"] = std::move(section);

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

EditorKeyBindings EditorKeyBindings::load(const std::filesystem::path& path) {
    EditorKeyBindings bindings;  // valeurs par defaut, ecrasees ci-dessous entree par entree.

    std::ifstream input(path, std::ios::binary);
    if (!input) {
        return bindings;  // fichier absent (premier lancement) : defauts, rien a signaler.
    }

    nlohmann::json root;
    try {
        input >> root;
    } catch (const nlohmann::json::exception& error) {
        HMI_LOG_WARNING(std::string("keybindings.json illisible (touches d'editeur) : ") +
                        error.what());
        return bindings;
    }

    const auto section = root.find("editeur");
    if (section == root.end() || !section->is_object()) {
        return bindings;  // pas de section "editeur" : defauts, fichier par ailleurs valide.
    }

    for (std::size_t index = 0; index < bindings._keys.size(); ++index) {
        const auto entry = section->find(actionName(static_cast<EditorAction>(index)));
        if (entry == section->end() || !entry->is_number_integer()) {
            continue;  // entree absente/invalide : reste a sa valeur par defaut.
        }
        const int code = entry->get<int>();
        if (code < 0 || code > 0xFF) {
            HMI_LOG_WARNING("Code de touche hors bornes ignore dans keybindings.json (editeur).");
            continue;
        }
        bindings._keys[index] = static_cast<Key>(code);
    }
    return bindings;
}

}  // namespace hmi
