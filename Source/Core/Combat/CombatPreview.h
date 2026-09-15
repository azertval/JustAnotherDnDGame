// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

/**
 * @file Core/Combat/CombatPreview.h
 * @brief Ce que l'écran de combat montre **avant** que le joueur ne s'engage (`LOT-24`,
 *        `EX-CBT-020`, `EX-IHM-003`).
 *
 * ## Une prévisualisation qui ne ment pas
 *
 * Une prévisualisation qui calcule à part finit par diverger du jet : l'écran promet 55 %, et le
 * journal écrit une CA que l'écran n'a jamais montrée. Tout ici passe donc par **les fonctions du
 * jet réel** — `core::checkTarget` pour la cible, `core::attackCircumstances` et
 * `core::ArenaSession::circumstancesAgainst` pour les sources d'avantage et de désavantage,
 * `core::coverBetween` pour l'abri, `core::rollStance` pour la posture — et la chance de toucher
 * vient du jet requis du *Guide du Maître*, comme pour l'IA (`core::hitChance`). Un test compare la
 * prévisualisation au jet que la session jette ensuite : même CA, même posture, mêmes sources.
 *
 * Ce qu'elle ne peut pas savoir : ce qu'un greffon `BeforeRoll` changera au jet (une capacité de
 * classe), et les dés. Le premier n'existe encore pour personne ; les seconds sont le jeu.
 *
 * ## Le déplacement
 *
 * Le chemin est celui de `core::ReachableArea::pathTo`, que `move` suivra ; les attaques
 * d'opportunité sont celles que `move` déclenchera (`core::ArenaSession::previewOpportunities`),
 * choix du joueur et politique de l'IA compris.
 */

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

#include "Core/Combat/Arena.h"
#include "Core/Combat/Attack.h"

namespace core {

/// @brief Une attaque telle qu'elle serait jetée.
struct AttackPreview {
    /// `Valid`, ou pourquoi on ne peut pas viser.
    TargetCheck check = TargetCheck::NotOnGrid;
    std::size_t attackIndex = 0;
    std::string label;
    /// La CA visée, abri compris.
    int armorClass = 0;
    Cover cover = Cover::None;
    /// Le jet requis du *Guide du Maître* : la CA visée moins le bonus d'attaque.
    int requiredRoll = 0;
    RollStance stance = RollStance::Normal;
    std::vector<std::string> advantages;
    std::vector<std::string> disadvantages;
    /// La chance de toucher, en quatre-centièmes (`core::CHANCE_SCALE`).
    int hitChance = 0;
    /// L'espérance de dégâts, en huit-centièmes de point (`core::expectedDamage`).
    long long expectedDamage = 0;

    /// @brief La chance en pour cent, arrondie au plus proche.
    [[nodiscard]] int hitPercent() const noexcept {
        return (hitChance * 100 + 200) / 400;
    }
};

/**
 * @brief L'attaque @p attackIndex du combattant actif contre @p target, telle qu'elle serait
 *        jetée maintenant.
 * @return Vide sans tour actif, sans attaque de cet indice, ou si la cible est inconnue.
 */
[[nodiscard]] std::optional<AttackPreview> previewAttack(const ArenaSession& session,
                                                         CombatantId target,
                                                         std::size_t attackIndex);

/**
 * @brief La première attaque du combattant actif qui peut viser @p target — celle que le geste
 *        « attaquer » choisit quand le joueur n'en a pas désigné.
 */
[[nodiscard]] std::optional<std::size_t> firstValidAttack(const ArenaSession& session,
                                                          CombatantId target);

/// @brief Un déplacement tel qu'il serait joué.
struct MovePreview {
    /// Le chemin, départ exclu ; vide si la case n'est pas une fin de déplacement permise.
    std::optional<Path> path;
    /// Les cases de déplacement qui resteraient.
    int movementLeft = 0;
    /// Qui frapperait en chemin, dans l'ordre.
    std::vector<CombatantId> opportunities;
};

/// @brief Le déplacement du combattant actif jusqu'à @p destination.
[[nodiscard]] MovePreview previewMove(const ArenaSession& session, GridPosition destination);

}  // namespace core
