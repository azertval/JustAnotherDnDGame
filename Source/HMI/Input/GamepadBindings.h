// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <array>
#include <cstddef>
#include <filesystem>

#include "HMI/Input/GameKeyBindings.h"
#include "HMI/Input/GamepadButton.h"

/**
 * @file HMI/Input/GamepadBindings.h
 * @brief Association remappable action de jeu -> bouton manette, avec persistance
 *        (`EX-CTRL-002`, `EX-CTRL-012`).
 */

namespace hmi {

/**
 * @brief Association action de jeu (`GameAction`, partagée avec `GameKeyBindings`) -> bouton
 *        manette, avec persistance JSON (`LOT-30`).
 *
 * Même mécanique que `GameKeyBindings` (une touche par action, échange automatique en cas de
 * conflit, réinitialisation) mais sur `GamepadButton` au lieu de `Key` — classe séparée plutôt
 * qu'une abstraction commune, comme `GameKeyBindings`/`EditorKeyBindings` l'ont déjà choisi.
 * `load`/`save` partagent le même fichier que `GameKeyBindings`/`EditorKeyBindings` (section
 * `"manette"`, fusionnée plutôt qu'écrasée à chaque sauvegarde).
 */
class GamepadBindings {
public:
    /// Construit avec les valeurs par défaut (miroir du câblage historique de
    /// `Window::pollGamepad` : D-pad/stick gauche, bouton A, épaule droite).
    GamepadBindings();

    /// @return Le bouton actuellement lié à @p action.
    [[nodiscard]] GamepadButton button(GameAction action) const noexcept;

    /**
     * @brief Lie @p action à @p newButton.
     *
     * Si @p newButton était déjà lié à une autre action, les deux échangent leurs boutons (jamais
     * deux actions sur le même bouton à l'issue de l'appel).
     */
    void setKey(GameAction action, GamepadButton newButton) noexcept;

    /// Restaure les sept actions à leurs boutons par défaut (`defaultButton`).
    void resetToDefaults() noexcept;

    /// @return Le bouton par défaut de @p action, indépendant de l'état courant.
    [[nodiscard]] static GamepadButton defaultButton(GameAction action) noexcept;

    /**
     * @brief Sauvegarde dans la section `"manette"` de @p path, en préservant les sections
     *        `"jeu"`/`"editeur"` déjà présentes.
     * @return Faux si le fichier n'a pas pu être écrit (dossier non créable, etc.).
     */
    bool save(const std::filesystem::path& path) const;

    /**
     * @brief Charge la section `"manette"` de @p path.
     * @return Les bindings lus ; valeurs par défaut pour toute entrée absente, invalide, ou si le
     *         fichier est absent/corrompu (récupérable, jamais bloquant, `EX-NFR-040`).
     */
    [[nodiscard]] static GamepadBindings load(const std::filesystem::path& path);

private:
    std::array<GamepadButton, GAME_ACTION_COUNT> _buttons{};
};

}  // namespace hmi
