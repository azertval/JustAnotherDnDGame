// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <array>
#include <cstddef>
#include <filesystem>

#include "HMI/Input/InputState.h"

/**
 * @file HMI/Input/GameKeyBindings.h
 * @brief Association remappable action de jeu -> touche clavier, avec persistance (`EX-CTRL-012`).
 */

namespace hmi {

/**
 * @brief Action de jeu logique, remappable indépendamment de la touche physique qui la déclenche.
 *
 * Miroir des sept lectures de `PlayerInputMapper.cpp`. Chaque action a une touche **unique** et
 * pleinement remappable — aucun alias fixe : un alias câblé en dur sur une touche donnée entrerait
 * en collision dès qu'une **autre** action est remappée sur cette même touche, les deux se
 * déclenchant à la fois (silencieusement annulé pour une paire opposée comme Gauche/Droite, voir
 * `PlayerInputMapper.cpp`).
 */
enum class GameAction {
    MoveLeft,
    MoveRight,
    AimUp,
    AimDown,
    Jump,
    Dash,
    Interact,
};

/// Nombre d'actions de jeu remappables (`GameAction`).
constexpr int GAME_ACTION_COUNT = 7;

/**
 * @brief Association action de jeu -> touche, avec persistance JSON (`EX-CTRL-012`, `LOT-29`).
 *
 * Une touche par action ; `setKey` échange automatiquement avec toute autre action qui détenait
 * déjà cette touche, pour ne jamais laisser deux actions du jeu sur la même touche. `load`/`save`
 * partagent un seul fichier avec `EditorKeyBindings` (section `"jeu"`, l'autre classe possédant sa
 * propre section `"editeur"` du même fichier, sans interférence) : chaque sauvegarde relit puis
 * fusionne, plutôt que d'écraser tout le fichier.
 */
class GameKeyBindings {
public:
    /// Construit avec les valeurs par défaut (flèches/Espace/Maj/E, cf. `PlayerInputMapper.cpp`).
    GameKeyBindings();

    /// @return La touche actuellement liée à @p action.
    [[nodiscard]] Key key(GameAction action) const noexcept;

    /**
     * @brief Lie @p action à @p newKey.
     *
     * Si @p newKey était déjà liée à une autre action de ce jeu de bindings, les deux échangent
     * leurs touches (jamais deux actions sur la même touche à l'issue de l'appel).
     */
    void setKey(GameAction action, Key newKey) noexcept;

    /// Restaure les sept actions à leurs touches par défaut (`defaultKey`).
    void resetToDefaults() noexcept;

    /// @return La touche par défaut de @p action, indépendante de l'état courant.
    [[nodiscard]] static Key defaultKey(GameAction action) noexcept;

    /**
     * @brief Sauvegarde dans la section `"jeu"` de @p path, en préservant une section
     *        `"editeur"` déjà présente (écrite par `EditorKeyBindings::save`).
     * @return Faux si le fichier n'a pas pu être écrit (dossier non créable, etc.).
     */
    bool save(const std::filesystem::path& path) const;

    /**
     * @brief Charge la section `"jeu"` de @p path.
     * @return Les bindings lus ; valeurs par défaut pour toute entrée absente, invalide, ou si le
     *         fichier est absent/corrompu (récupérable, jamais bloquant, `EX-NFR-040`).
     */
    [[nodiscard]] static GameKeyBindings load(const std::filesystem::path& path);

private:
    std::array<Key, GAME_ACTION_COUNT> _keys{};
};

}  // namespace hmi
