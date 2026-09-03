// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string_view>
#include <vector>

#include "Core/Levels/LevelOutcome.h"
#include "Core/Physics/PlayerInput.h"
#include "HMI/Game/IGameMode.h"

/**
 * @file HMI/Game/ExplorationMode.h
 * @brief Mode d'exploration : l'ordre des passes du pas fixe hors dialogue et hors combat.
 */

namespace hmi {

/**
 * @brief Le mode par défaut : le personnage parcourt la carte, les mécanismes répondent, la caméra
 *        suit, l'issue du niveau est évaluée à chaque pas.
 *
 * Premier mode extrait de `hmi::GameSession::update` (`LOT-05`), **à comportement constant** : il
 * enchaîne exactement les passes que la session enchaînait, dans le même ordre. Le mode dialogue
 * et le mode combat arrivent aux `LOT-15` et `LOT-18` — les écrire ici aurait mélangé un
 * refactoring et une nouveauté, et on n'aurait plus su lequel des deux avait cassé quoi.
 *
 * Sans état : l'état de la partie vit dans l'orchestrateur, le mode ne décide que de l'ordre. Un
 * mode qui aura besoin de mémoire (le tour courant d'un combat) la portera, lui.
 */
class ExplorationMode final : public IGameMode {
public:
    [[nodiscard]] std::string_view name() const override;
    [[nodiscard]] std::vector<std::string_view> passOrder() const override;
    void onLoad(IGameModePasses& passes) override;
    core::LevelOutcome step(IGameModePasses& passes, const core::PlayerInput& input,
                            float fixedDelta) override;
    void onUnload(IGameModePasses& passes) override;
};

}  // namespace hmi
