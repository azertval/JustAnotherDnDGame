// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <map>

#include "Core/Combat/BattleGrid.h"  // core::CombatantId

/**
 * @file HMI/Graphics/ArenaAnimationState.h
 * @brief L'image courante de chaque figurine du Colisée, telle que `hmi::composeArenaScene` la lit.
 *
 * Donnée seule : ce qui la fait avancer au temps réel du rendu (`ArenaAnimationDriver`) est une
 * phase distincte. La composition n'en lit que l'indice d'image, et ne le fait jamais avancer —
 * composer deux fois la même scène donne deux fois les mêmes quads.
 */

namespace hmi {

/// @brief L'animation d'une figurine : l'indice de l'image courante dans sa bande.
struct ArenaFigureAnimation {
    /// Indice dans la bande de la planche, à partir de 0. La composition le ramène dans le nombre
    /// d'images de la figurine : un indice hors bande ne lit jamais hors de la texture.
    int frame = 0;
};

/**
 * @brief L'état d'animation des figurines, par combattant.
 *
 * Un combattant absent de la table est à sa première image : un état vide compose une scène
 * figée, ce qui suffit à tout ce qui n'anime pas (tests, capture).
 */
struct ArenaAnimationState {
    std::map<core::CombatantId, ArenaFigureAnimation> figures;

    /// @return L'image courante de @p combatant, 0 s'il n'a pas d'état.
    [[nodiscard]] int frameOf(core::CombatantId combatant) const {
        const auto found = figures.find(combatant);
        return found != figures.end() ? found->second.frame : 0;
    }
};

}  // namespace hmi
