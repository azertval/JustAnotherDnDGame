// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string_view>
#include <vector>

#include "Core/Levels/LevelOutcome.h"
#include "Core/Physics/PlayerInput.h"
#include "HMI/Game/IGameMode.h"

/**
 * @file HMI/Game/CombatMode.h
 * @brief Mode combat : l'ordre des passes quand le monde est **gelé** (`LOT-18`, `EX-CBT-001`).
 */

namespace hmi {

/**
 * @brief Le second mode de jeu : le combat se déroule **sur la carte d'exploration**, mais
 *        l'exploration, elle, ne tourne plus.
 *
 * ## Ce qui est gelé, et pourquoi
 *
 * Trois passes de l'exploration **disparaissent** ici, et chacune pour une raison précise :
 *
 * - `moveCharacter` — le personnage ne se déplace plus à l'intention du joueur mais au **budget de
 *   déplacement** de son tour (`LOT-19`). Laisser la passe d'exploration active donnerait un
 *   combat où l'on marche librement pendant le tour d'un autre.
 * - `updateMechanisms` — une plaque de pression qui s'enfoncerait au milieu d'un tour ferait
 *   dépendre le combat de la position d'un pion sur un mécanisme, ce qu'aucune règle ne décrit.
 * - `evaluateOutcome` — l'issue du **niveau** n'a pas de sens pendant un combat : tomber à zéro
 *   point de vie est une issue du **combat** (`core::CombatOutcome`), et l'évaluer ici ferait
 *   recharger le niveau au lieu d'ouvrir l'agonie (`LOT-72`).
 *
 * Ce qui reste tourne parce que le combat reste une **scène** : les particules et la secousse
 * d'écran finissent ce qu'elles ont commencé, les animations continuent de jouer — un combattant
 * immobile respire —, et la caméra suit ce qu'il faut regarder.
 *
 * ## Ce que ce mode ne fait pas encore
 *
 * Ni initiative, ni tour actif, ni résolution d'action : c'est le `LOT-20`. Ce lot-ci livre la
 * **bascule** — geler le monde, monter la rencontre, en revenir sans rien perdre — et l'état de la
 * rencontre engagée vit dans `core::EncounterRun`, hors de ce mode, précisément pour que
 * l'aller-retour se vérifie sans fenêtre.
 *
 * Sans état, donc, comme `hmi::ExplorationMode` : le mode ne décide que de l'ordre. Le tour
 * courant, lui, sera une mémoire, et c'est le `LOT-20` qui la portera.
 */
class CombatMode final : public IGameMode {
public:
    [[nodiscard]] std::string_view name() const override;
    [[nodiscard]] std::vector<std::string_view> passOrder() const override;
    void onLoad(IGameModePasses& passes) override;
    core::LevelOutcome step(IGameModePasses& passes, const core::PlayerInput& input,
                            float fixedDelta) override;
    void onUnload(IGameModePasses& passes) override;
};

}  // namespace hmi
