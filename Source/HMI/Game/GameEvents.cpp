// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Game/GameEvents.h"

#include <algorithm>

namespace hmi {

std::vector<GameEvent> detectMechanismEvents(const MechanismEventState& previous,
                                             const MechanismEventState& current,
                                             const std::vector<bool>& isContinuous) {
    std::vector<GameEvent> events;

    const std::size_t count =
        std::min({previous.doorOpen.size(), current.doorOpen.size(), isContinuous.size()});
    for (std::size_t index = 0; index < count; ++index) {
        const bool was = previous.doorOpen[index];
        const bool is = current.doorOpen[index];
        if (was == is) {
            continue;
        }
        // Un seul evenement par mecanisme transitionne : celui du declencheur (interrupteur ou
        // plaque). La porte partage le meme booleen mais n'emet pas un second son -- ce serait
        // deux sons superposes pour une seule action physique.
        if (isContinuous[index]) {
            events.push_back(is ? GameEvent::PressurePlatePressed
                                : GameEvent::PressurePlateReleased);
        } else {
            events.push_back(GameEvent::SwitchToggled);
        }
    }

    return events;
}

std::optional<GameEvent> detectOutcomeEvent(core::LevelOutcome outcome) {
    switch (outcome) {
        case core::LevelOutcome::Playing:
            return std::nullopt;
        case core::LevelOutcome::Won:
            return GameEvent::ExitReached;
        case core::LevelOutcome::Lost:
            return GameEvent::Died;
    }
    return std::nullopt;  // inatteignable : le switch couvre tout l'enum.
}

}  // namespace hmi
