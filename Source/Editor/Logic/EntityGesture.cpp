// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "Editor/Logic/EntityGesture.h"

#include <utility>

#include "Core/Levels/LevelDraft.h"
#include "Core/World/EntityKinds.h"

namespace hmi {

namespace {

[[nodiscard]] core::EntityShape shapeOfKind(const std::string& type) {
    const core::EntityKind* const kind = core::findEntityKind(type);
    return kind != nullptr ? kind->shape : core::EntityShape::Point;
}

// L'entite neuve de la famille @p type en @p cell : ses proprietes a leur defaut, ou aucune pour
// une famille inconnue (elle se pose tout de meme, EX-NFR-040).
[[nodiscard]] core::MapEntity newEntity(const std::string& type, core::GridPosition cell) {
    const core::EntityKind* const kind = core::findEntityKind(type);
    return kind != nullptr ? core::makeEntity(*kind, cell)
                           : core::MapEntity{.type = type, .position = cell, .properties = {}};
}

}  // namespace

EntityGestureDecision resolveEntityPress(const core::LevelDraft& draft, core::GridPosition cell,
                                         const std::vector<std::size_t>& selection,
                                         const std::string& kindToPlace,
                                         EntityPressModifiers modifiers) {
    if (!draft.tileMap().inBounds(cell.column, cell.row)) {
        return {};
    }
    const std::optional<EntityPick> pick = pickEntity(draft.entities(), cell, selection);
    if (modifiers.toggle) {
        if (!pick) {
            return {};
        }
        return EntityGestureDecision{.action = EntityGestureAction::Toggle,
                                     .entityIndex = pick->index,
                                     .cell = cell,
                                     .handle = std::nullopt};
    }
    const bool canPlace = !kindToPlace.empty();
    // Une poignee, ou une entite designee par sa case : on la prend, sauf a forcer la pose.
    if (pick && !pick->body && !(canPlace && modifiers.force)) {
        return EntityGestureDecision{.action = EntityGestureAction::Grab,
                                     .entityIndex = pick->index,
                                     .cell = cell,
                                     .handle = pick->handle};
    }
    if (canPlace) {
        const EntityGestureAction action = shapeOfKind(kindToPlace) == core::EntityShape::Point
                                               ? EntityGestureAction::Place
                                               : EntityGestureAction::Draw;
        return EntityGestureDecision{
            .action = action, .entityIndex = 0, .cell = cell, .handle = std::nullopt};
    }
    if (pick) {
        return EntityGestureDecision{.action = EntityGestureAction::Grab,
                                     .entityIndex = pick->index,
                                     .cell = cell,
                                     .handle = std::nullopt};
    }
    return EntityGestureDecision{
        .action = EntityGestureAction::Deselect, .entityIndex = 0, .cell = cell, .handle = {}};
}

EntityDragResult dragEntities(const EntityDrag& drag, const std::vector<core::MapEntity>& entities,
                              core::GridPosition to, int width, int height) {
    EntityDragResult result;
    switch (drag.mode) {
        case EntityDrag::Mode::Draw: {
            core::MapEntity entity = newEntity(drag.kind, drag.from);
            switch (shapeOfKind(drag.kind)) {
                case core::EntityShape::Rectangle:
                case core::EntityShape::Area:
                    entity = withRectangle(std::move(entity), rectangleBetween(drag.from, to));
                    break;
                case core::EntityShape::Path:
                    entity = withWaypointAdded(std::move(entity), to);
                    break;
                case core::EntityShape::Point:
                    break;
            }
            result.placed = std::move(entity);
            return result;
        }
        case EntityDrag::Mode::Reshape: {
            if (drag.indices.empty() || drag.indices.front() >= entities.size() || !drag.handle ||
                to == drag.from) {
                return result;
            }
            const std::size_t index = drag.indices.front();
            const core::MapEntity& entity = entities[index];
            if (drag.handle->kind == HandleKind::Waypoint) {
                result.replaced.emplace_back(index,
                                             withWaypointMoved(entity, drag.handle->waypoint, to));
            } else if (const std::optional<CellRect> rect = entityRectangle(entity)) {
                result.replaced.emplace_back(
                    index, withRectangle(entity, resizeRectangle(*rect, drag.handle->kind, to)));
            }
            return result;
        }
        case EntityDrag::Mode::Move:
            break;
    }
    const int columns = to.column - drag.from.column;
    const int rows = to.row - drag.from.row;
    if (columns == 0 && rows == 0) {
        return result;
    }
    for (const std::size_t index : drag.indices) {
        if (index >= entities.size()) {
            continue;
        }
        std::optional<core::MapEntity> moved =
            translatedEntity(entities[index], columns, rows, width, height);
        if (!moved) {
            // Le groupe bouge entier ou pas du tout : un groupe ecartele ne se remet pas en place.
            return EntityDragResult{.replaced = {}, .placed = std::nullopt, .refused = true};
        }
        result.replaced.emplace_back(index, std::move(*moved));
    }
    return result;
}

ShapeGestureDecision resolveShapePress(const core::MapEntity& entity, core::GridPosition cell,
                                       bool remove) {
    switch (entityShape(entity)) {
        case core::EntityShape::Area:
            return ShapeGestureDecision{
                .action = remove ? ShapeGestureAction::EraseCells : ShapeGestureAction::PaintCells,
                .waypoint = 0};
        case core::EntityShape::Path: {
            if (const std::optional<EntityHandle> handle = handleAt(entity, cell)) {
                return ShapeGestureDecision{.action = remove ? ShapeGestureAction::RemoveWaypoint
                                                             : ShapeGestureAction::GrabWaypoint,
                                            .waypoint = handle->waypoint};
            }
            if (remove) {
                return {};
            }
            return ShapeGestureDecision{.action = ShapeGestureAction::AppendWaypoint,
                                        .waypoint = 0};
        }
        case core::EntityShape::Point:
        case core::EntityShape::Rectangle:
            break;
    }
    return {};
}

}  // namespace hmi
