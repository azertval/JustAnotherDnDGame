// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <string_view>
#include <vector>

#include "Core/Levels/LevelOutcome.h"
#include "Core/Physics/PlayerInput.h"
#include "HMI/Game/IGameMode.h"

/**
 * @file HMI/Game/DialogueMode.h
 * @brief Mode dialogue : l'ordre des passes quand une conversation **gèle** le monde (`LOT-15`).
 */

namespace hmi {

/**
 * @brief Le troisième mode de jeu : on parle à un PNJ, et le monde attend.
 *
 * ## Ce qui est gelé
 *
 * Les mêmes passes que le combat (`hmi::CombatMode`), pour des raisons voisines :
 *
 * - `moveCharacter` — l'intention du joueur choisit une **réponse**, elle ne déplace personne. La
 *   laisser active ferait marcher le personnage pendant qu'il répond, et le sortir de la portée
 *   du PNJ au milieu d'une phrase.
 * - `updateMechanisms`, `detectEvents`, `updateMechanismVisuals` — une plaque de pression, une
 *   porte à minuterie ou un piège qui avanceraient pendant la lecture d'une réplique puniraient le
 *   joueur de lire.
 * - `evaluateOutcome` — rien de ce qu'on dit ne fait perdre le niveau.
 *
 * Ce qui reste tourne parce que la scène reste **visible** derrière le parchemin : les particules
 * finissent leur vie, les animations respirent, la caméra ne saute pas.
 *
 * ## Pourquoi un mode distinct du combat, à passes égales
 *
 * Aujourd'hui les deux gèlent la même chose. Ils ne le feront pas toujours : un combat avance au
 * tour et à l'initiative, une conversation au fil des réponses, et une cinématique de dialogue
 * voudra cadrer l'interlocuteur. Deux modes qui se ressemblent le premier jour coûtent une classe ;
 * un mode partagé qu'il faudrait séparer ensuite coûterait chaque `if` ajouté entre-temps.
 *
 * ## Où vit la conversation
 *
 * **Hors de ce mode**, dans `core::DialogueRunner`, pour la même raison que le combat vit dans
 * `core::CombatState` : se jouer en test, sans fenêtre. Le mode ne décide que de l'ordre des passes
 * et reste sans état. Aucune session de jeu ne bascule encore dans ce mode depuis une interaction :
 * `hmi::GameSession` ne câble pas l'interaction du `LOT-10`, et la surface de rendu Qt Quick
 * n'affiche encore aucune scène — l'écran de dialogue joue le runner directement
 * (`hmi::DialogueModel`).
 */
class DialogueMode final : public IGameMode {
public:
    [[nodiscard]] std::string_view name() const override;
    [[nodiscard]] std::vector<std::string_view> passOrder() const override;
    void onLoad(IGameModePasses& passes) override;
    core::LevelOutcome step(IGameModePasses& passes, const core::PlayerInput& input,
                            float fixedDelta) override;
    void onUnload(IGameModePasses& passes) override;
};

}  // namespace hmi
