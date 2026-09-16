// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "HMI/Editor/EntityGesture.h"

#include "Core/Levels/LevelDraft.h"

namespace hmi {

EntityGestureDecision resolveEntityPress(const core::LevelDraft& draft, core::GridPosition cell,
                                         const std::string& kindToPlace, bool forcePlace) {
    if (!draft.tileMap().inBounds(cell.column, cell.row)) {
        return {};
    }
    const std::optional<std::size_t> occupant = draft.entityAt(cell);
    const bool canPlace = !kindToPlace.empty();
    if (canPlace && (!occupant || forcePlace)) {
        return EntityGestureDecision{
            .action = EntityGestureAction::Place, .entityIndex = 0, .cell = cell};
    }
    if (occupant) {
        return EntityGestureDecision{
            .action = EntityGestureAction::Select, .entityIndex = *occupant, .cell = cell};
    }
    return EntityGestureDecision{
        .action = EntityGestureAction::Deselect, .entityIndex = 0, .cell = cell};
}

EntityGestureDecision resolveEntityRelease(std::optional<std::size_t> grabbed,
                                           core::GridPosition pressCell,
                                           core::GridPosition releaseCell) {
    if (!grabbed || pressCell == releaseCell) {
        return {};
    }
    return EntityGestureDecision{
        .action = EntityGestureAction::Move, .entityIndex = *grabbed, .cell = releaseCell};
}

}  // namespace hmi
