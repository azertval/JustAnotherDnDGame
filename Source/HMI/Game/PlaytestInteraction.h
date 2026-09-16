// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

/**
 * @file HMI/Game/PlaytestInteraction.h
 * @brief Ce que l'essai immédiat de l'éditeur dit d'une interaction avec une entité posée
 *        (`LOT-11`).
 */

#include <string>
#include <string_view>
#include <vector>

namespace core {
struct MapEntity;
}

namespace hmi {

class Localization;

/// @brief Durée d'affichage d'un compte rendu d'interaction, en secondes. Convertie en **pas de
///        simulation** (`playtestMessageSteps`) : le compte rendu s'efface au même pas quelle que
///        soit la cadence d'affichage.
inline constexpr float PLAYTEST_MESSAGE_SECONDS = 3.0f;

/// @brief Clés de localisation des comptes rendus de l'essai immédiat.
inline constexpr std::string_view PLAYTEST_CHEST_OPENED_KEY = "playtest.interaction.chest_opened";
inline constexpr std::string_view PLAYTEST_CHEST_ALREADY_OPENED_KEY =
    "playtest.interaction.chest_already_opened";
inline constexpr std::string_view PLAYTEST_SIGN_READ_KEY = "playtest.interaction.sign_read";
inline constexpr std::string_view PLAYTEST_NPC_DIALOGUE_KEY = "playtest.interaction.npc_dialogue";
inline constexpr std::string_view PLAYTEST_NPC_SILENT_KEY = "playtest.interaction.npc_silent";
inline constexpr std::string_view PLAYTEST_PORTAL_KEY = "playtest.interaction.portal";
inline constexpr std::string_view PLAYTEST_PORTAL_INCOMPLETE_KEY =
    "playtest.interaction.portal_incomplete";
inline constexpr std::string_view PLAYTEST_OTHER_KEY = "playtest.interaction.other";

/// @brief Un compte rendu : une clé de localisation et les valeurs de ses `%1`, `%2`…
struct PlaytestMessage {
    std::string key;
    std::vector<std::string> args;

    [[nodiscard]] bool operator==(const PlaytestMessage&) const = default;
};

/**
 * @brief Le compte rendu de l'interaction avec @p entity.
 *
 * - **Coffre** : ouvert, ou déjà ouvert si @p alreadyConsumed.
 * - **Panneau** : lu.
 * - **PNJ** : l'identifiant de son dialogue (`core::dialogueTriggerFor`), ou « sans dialogue ».
 * - **Portail** : sa carte cible et son point d'arrivée — la traversée vient avec le `LOT-09`. Un
 *   portail à qui manque l'une des deux se dit incomplet plutôt que de promettre un voyage.
 * - Tout autre type : son nom, sans promesse.
 *
 * Pure : aucune lecture de monde, aucun texte — l'appelant traduit (`EX-NFR-011`).
 * @param entity L'entité désignée.
 * @param alreadyConsumed Vrai si son drapeau de consommation était **déjà** levé avant ce geste.
 */
[[nodiscard]] PlaytestMessage playtestInteractionMessage(const core::MapEntity& entity,
                                                         bool alreadyConsumed);

/// @brief Traduit un compte rendu : le texte de sa clé, `%1`, `%2`… remplacés par ses valeurs.
[[nodiscard]] std::string formatPlaytestMessage(const PlaytestMessage& message,
                                                const Localization& localization);

/// @brief Les pas de simulation pendant lesquels un compte rendu reste affiché, pour un pas fixe
///        de @p fixedDelta secondes. Au moins un ; un pas non positif en donne un.
[[nodiscard]] int playtestMessageSteps(float fixedDelta);

}  // namespace hmi
