// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "HMI/Game/PlaytestInteraction.h"

#include <cmath>
#include <optional>

#include "Core/Levels/MapEntity.h"
#include "Core/Rpg/Dialogue.h"
#include "Core/World/EntityKinds.h"
#include "HMI/Localization/Localization.h"

namespace hmi {

namespace {

// Valeur texte non vide d'une propriete, ou rien : un entier la ou l'on attend un nom de carte ne
// vaut pas une carte (la validation de l'editeur le signale deja, EntityIssueCode::WrongValueType).
[[nodiscard]] std::optional<std::string> textProperty(const core::MapEntity& entity,
                                                      std::string_view key) {
    const auto found = entity.properties.find(std::string{key});
    if (found == entity.properties.end()) {
        return std::nullopt;
    }
    const std::string* text = std::get_if<std::string>(&found->second);
    if (text == nullptr || text->empty()) {
        return std::nullopt;
    }
    return *text;
}

}  // namespace

PlaytestMessage playtestInteractionMessage(const core::MapEntity& entity, bool alreadyConsumed) {
    if (entity.type == "chest") {
        return {std::string{alreadyConsumed ? PLAYTEST_CHEST_ALREADY_OPENED_KEY
                                            : PLAYTEST_CHEST_OPENED_KEY},
                {}};
    }
    if (entity.type == "sign") {
        return {std::string{PLAYTEST_SIGN_READ_KEY}, {}};
    }
    if (entity.type == core::NPC_ENTITY_TYPE) {
        // Meme lecture que le jeu (LOT-15) : un PNJ sans dialogue n'est pas un declencheur.
        if (const std::optional<core::DialogueTrigger> trigger = core::dialogueTriggerFor(entity)) {
            return {std::string{PLAYTEST_NPC_DIALOGUE_KEY}, {trigger->dialogueId}};
        }
        return {std::string{PLAYTEST_NPC_SILENT_KEY}, {}};
    }
    if (entity.type == core::PORTAL_ENTITY_TYPE) {
        const std::optional<std::string> target =
            textProperty(entity, core::PORTAL_TARGET_MAP_PROPERTY);
        const std::optional<std::string> arrival =
            textProperty(entity, core::PORTAL_ARRIVAL_PROPERTY);
        if (target && arrival) {
            return {std::string{PLAYTEST_PORTAL_KEY}, {*target, *arrival}};
        }
        return {std::string{PLAYTEST_PORTAL_INCOMPLETE_KEY}, {}};
    }
    return {std::string{PLAYTEST_OTHER_KEY}, {entity.type}};
}

std::string formatPlaytestMessage(const PlaytestMessage& message,
                                  const Localization& localization) {
    std::string text = localization.text(message.key);
    // Du dernier au premier : `%1` ne doit pas mordre dans un `%10`.
    for (std::size_t index = message.args.size(); index > 0; --index) {
        const std::string placeholder = "%" + std::to_string(index);
        const std::size_t position = text.find(placeholder);
        if (position != std::string::npos) {
            text.replace(position, placeholder.size(), message.args[index - 1]);
        }
    }
    return text;
}

int playtestMessageSteps(float fixedDelta) {
    if (!(fixedDelta > 0.0f)) {
        return 1;
    }
    const long steps = std::lround(PLAYTEST_MESSAGE_SECONDS / fixedDelta);
    return steps < 1 ? 1 : static_cast<int>(steps);
}

}  // namespace hmi
