// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "HMI/Editor/LinkGesture.h"

#include "HMI/Editor/LinkGeometry.h"

namespace hmi {

LinkGestureDecision resolveLinkClick(std::optional<PendingLink> pending,
                                     core::GridPosition clickedCell, core::TileType clickedTileType,
                                     bool alreadyLinked) noexcept {
    const bool clickedIsTrigger = isTriggerTile(clickedTileType);
    const bool clickedIsTarget = isLinkTargetTile(clickedTileType);
    if (!clickedIsTrigger && !clickedIsTarget) {
        return LinkGestureDecision{.action = LinkGestureAction::Ignore,
                                   .cell = {},
                                   .switchPosition = {},
                                   .targetPosition = {}};
    }

    if (pending) {
        const bool pendingIsTrigger = isTriggerTile(pending->tileType);
        const bool pendingIsTarget = isLinkTargetTile(pending->tileType);
        if (pendingIsTrigger || pendingIsTarget) {
            const bool sameCategory =
                (pendingIsTrigger && clickedIsTrigger) || (pendingIsTarget && clickedIsTarget);
            if (sameCategory) {
                return LinkGestureDecision{.action = LinkGestureAction::ReplacePending,
                                           .cell = clickedCell,
                                           .switchPosition = {},
                                           .targetPosition = {}};
            }
            const core::GridPosition switchPosition =
                pendingIsTrigger ? pending->cell : clickedCell;
            const core::GridPosition targetPosition =
                pendingIsTrigger ? clickedCell : pending->cell;
            const LinkGestureAction action =
                alreadyLinked ? LinkGestureAction::Unlink : LinkGestureAction::Link;
            return LinkGestureDecision{.action = action,
                                       .cell = {},
                                       .switchPosition = switchPosition,
                                       .targetPosition = targetPosition};
        }
        // Attente perimee (la case a ete repeinte depuis) : traitee comme aucune attente.
    }

    return LinkGestureDecision{.action = LinkGestureAction::SetPending,
                               .cell = clickedCell,
                               .switchPosition = {},
                               .targetPosition = {}};
}

}  // namespace hmi
