// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "Editor/Logic/EntityShapes.h"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <utility>
#include <variant>

namespace hmi {

namespace {

// Un entier positif de propriete, ou 1 : une taille manquante ou fautive se dessine sur une case,
// et l'avertissement de la propriete dit le reste.
[[nodiscard]] int sizeOf(const core::MapEntity& entity, std::string_view key) {
    const auto found = entity.properties.find(std::string{key});
    if (found == entity.properties.end()) {
        return 1;
    }
    const auto* const value = std::get_if<std::int64_t>(&found->second);
    if (value == nullptr || *value < 1) {
        return 1;
    }
    return static_cast<int>((std::min)(*value, std::int64_t{std::numeric_limits<int>::max()}));
}

[[nodiscard]] std::string textOf(const core::MapEntity& entity, std::string_view key) {
    if (key.empty()) {
        return {};
    }
    const auto found = entity.properties.find(std::string{key});
    if (found == entity.properties.end()) {
        return {};
    }
    const auto* const text = std::get_if<std::string>(&found->second);
    return text != nullptr ? *text : std::string{};
}

// L'ordre de lecture d'une zone : ligne, puis colonne.
[[nodiscard]] bool readingOrder(core::GridPosition a, core::GridPosition b) noexcept {
    return a.row != b.row ? a.row < b.row : a.column < b.column;
}

[[nodiscard]] std::string lowered(std::string_view text) {
    std::string result(text);
    std::ranges::transform(result, result.begin(), [](unsigned char character) {
        return static_cast<char>(std::tolower(character));
    });
    return result;
}

// Les cases d'un trajet : ses points, ou sa seule case tant qu'il n'en a pas.
[[nodiscard]] std::vector<core::GridPosition> waypointsOf(const core::MapEntity& entity) {
    return entity.cells.empty() ? std::vector<core::GridPosition>{entity.position} : entity.cells;
}

[[nodiscard]] long long areaOf(const core::MapEntity& entity) {
    if (const std::optional<CellRect> rect = entityRectangle(entity)) {
        return static_cast<long long>(rect->columns) * rect->rows;
    }
    return static_cast<long long>(entity.cells.size());
}

}  // namespace

core::EntityShape entityShape(const core::MapEntity& entity) {
    const core::EntityKind* const kind = core::findEntityKind(entity.type);
    return kind != nullptr ? kind->shape : core::EntityShape::Point;
}

bool CellRect::contains(core::GridPosition cell) const noexcept {
    return cell.column >= origin.column && cell.row >= origin.row &&
           cell.column < origin.column + columns && cell.row < origin.row + rows;
}

CellRect rectangleBetween(core::GridPosition a, core::GridPosition b) noexcept {
    return CellRect{
        .origin = {.column = (std::min)(a.column, b.column), .row = (std::min)(a.row, b.row)},
        .columns = std::abs(a.column - b.column) + 1,
        .rows = std::abs(a.row - b.row) + 1};
}

std::optional<CellRect> entityRectangle(const core::MapEntity& entity) {
    const core::EntityShape shape = entityShape(entity);
    if (shape != core::EntityShape::Rectangle &&
        (shape != core::EntityShape::Area || !entity.cells.empty())) {
        return std::nullopt;
    }
    return CellRect{.origin = entity.position,
                    .columns = sizeOf(entity, core::SHAPE_WIDTH_PROPERTY),
                    .rows = sizeOf(entity, core::SHAPE_HEIGHT_PROPERTY)};
}

core::MapEntity withRectangle(core::MapEntity entity, const CellRect& rect) {
    entity.position = rect.origin;
    entity.properties[std::string{core::SHAPE_WIDTH_PROPERTY}] = std::int64_t{rect.columns};
    entity.properties[std::string{core::SHAPE_HEIGHT_PROPERTY}] = std::int64_t{rect.rows};
    return entity;
}

std::vector<core::GridPosition> entityCells(const core::MapEntity& entity) {
    switch (entityShape(entity)) {
        case core::EntityShape::Point:
            return {entity.position};
        case core::EntityShape::Path:
            return waypointsOf(entity);
        case core::EntityShape::Rectangle:
        case core::EntityShape::Area:
            break;
    }
    if (const std::optional<CellRect> rect = entityRectangle(entity)) {
        std::vector<core::GridPosition> cells;
        cells.reserve(static_cast<std::size_t>(rect->columns) *
                      static_cast<std::size_t>(rect->rows));
        for (int row = 0; row < rect->rows; ++row) {
            for (int column = 0; column < rect->columns; ++column) {
                cells.push_back(
                    {.column = rect->origin.column + column, .row = rect->origin.row + row});
            }
        }
        return cells;
    }
    return entity.cells;
}

std::vector<EntityHandle> entityHandles(const core::MapEntity& entity) {
    std::vector<EntityHandle> handles;
    if (entityShape(entity) == core::EntityShape::Path) {
        const std::vector<core::GridPosition> points = waypointsOf(entity);
        for (std::size_t index = 0; index < points.size(); ++index) {
            handles.push_back(EntityHandle{
                .kind = HandleKind::Waypoint, .cell = points[index], .waypoint = index});
        }
        return handles;
    }
    const std::optional<CellRect> rect = entityRectangle(entity);
    if (!rect) {
        return handles;
    }
    const core::GridPosition first = rect->origin;
    const core::GridPosition last = rect->last();
    const int middleColumn = first.column + ((rect->columns - 1) / 2);
    const int middleRow = first.row + ((rect->rows - 1) / 2);
    const auto add = [&handles](HandleKind kind, int column, int row) {
        handles.push_back(
            EntityHandle{.kind = kind, .cell = {.column = column, .row = row}, .waypoint = 0});
    };
    // Les coins d'abord : sur un rectangle d'une case de large, ils se superposent, et c'est le
    // coin qu'on veut prendre.
    add(HandleKind::NorthWest, first.column, first.row);
    add(HandleKind::NorthEast, last.column, first.row);
    add(HandleKind::SouthEast, last.column, last.row);
    add(HandleKind::SouthWest, first.column, last.row);
    if (rect->columns >= 3) {
        add(HandleKind::North, middleColumn, first.row);
        add(HandleKind::South, middleColumn, last.row);
    }
    if (rect->rows >= 3) {
        add(HandleKind::East, last.column, middleRow);
        add(HandleKind::West, first.column, middleRow);
    }
    return handles;
}

std::optional<EntityHandle> handleAt(const core::MapEntity& entity, core::GridPosition cell) {
    for (const EntityHandle& handle : entityHandles(entity)) {
        if (handle.cell == cell) {
            return handle;
        }
    }
    return std::nullopt;
}

CellRect resizeRectangle(const CellRect& rect, HandleKind handle,
                         core::GridPosition cell) noexcept {
    int left = rect.origin.column;
    int top = rect.origin.row;
    int right = rect.last().column;
    int bottom = rect.last().row;
    switch (handle) {
        case HandleKind::NorthWest:
            left = cell.column;
            top = cell.row;
            break;
        case HandleKind::North:
            top = cell.row;
            break;
        case HandleKind::NorthEast:
            right = cell.column;
            top = cell.row;
            break;
        case HandleKind::East:
            right = cell.column;
            break;
        case HandleKind::SouthEast:
            right = cell.column;
            bottom = cell.row;
            break;
        case HandleKind::South:
            bottom = cell.row;
            break;
        case HandleKind::SouthWest:
            left = cell.column;
            bottom = cell.row;
            break;
        case HandleKind::West:
            left = cell.column;
            break;
        case HandleKind::Waypoint:
            return rect;
    }
    // Au-dela du cote oppose, le rectangle se retourne : les deux bords echangent leurs roles.
    return rectangleBetween({.column = left, .row = top}, {.column = right, .row = bottom});
}

core::MapEntity paintArea(core::MapEntity entity, const std::vector<core::GridPosition>& cells,
                          bool add) {
    std::vector<core::GridPosition> painted = entityCells(entity);
    if (add) {
        painted.insert(painted.end(), cells.begin(), cells.end());
    } else {
        std::erase_if(painted, [&cells](core::GridPosition cell) {
            return std::ranges::find(cells, cell) != cells.end();
        });
    }
    std::ranges::sort(painted, readingOrder);
    const auto [first, last] = std::ranges::unique(painted);
    painted.erase(first, last);
    if (painted.empty()) {
        return entity;  // une zone sans case n'est plus une zone : la gomme ne va pas jusque-la.
    }
    entity.properties.erase(std::string{core::SHAPE_WIDTH_PROPERTY});
    entity.properties.erase(std::string{core::SHAPE_HEIGHT_PROPERTY});
    if (std::ranges::find(painted, entity.position) == painted.end()) {
        entity.position = painted.front();
    }
    entity.cells = std::move(painted);
    return entity;
}

core::MapEntity withWaypointAdded(core::MapEntity entity, core::GridPosition cell) {
    std::vector<core::GridPosition> points = waypointsOf(entity);
    if (points.back() == cell) {
        return entity;
    }
    points.push_back(cell);
    entity.cells = std::move(points);
    return entity;
}

core::MapEntity withWaypointMoved(core::MapEntity entity, std::size_t index,
                                  core::GridPosition cell) {
    std::vector<core::GridPosition> points = waypointsOf(entity);
    if (index >= points.size()) {
        return entity;
    }
    points[index] = cell;
    entity.position = points.front();
    entity.cells = std::move(points);
    return entity;
}

core::MapEntity withWaypointRemoved(core::MapEntity entity, std::size_t index) {
    std::vector<core::GridPosition> points = waypointsOf(entity);
    if (index >= points.size() || points.size() <= 1) {
        return entity;
    }
    points.erase(points.begin() + static_cast<std::ptrdiff_t>(index));
    entity.position = points.front();
    entity.cells = std::move(points);
    return entity;
}

std::optional<core::MapEntity> translatedEntity(const core::MapEntity& entity, int columns,
                                                int rows, int width, int height) {
    const auto moved = [columns, rows](core::GridPosition cell) {
        return core::GridPosition{.column = cell.column + columns, .row = cell.row + rows};
    };
    const auto inside = [width, height](core::GridPosition cell) {
        return cell.column >= 0 && cell.row >= 0 && cell.column < width && cell.row < height;
    };
    core::MapEntity result = entity;
    result.position = moved(entity.position);
    for (core::GridPosition& cell : result.cells) {
        cell = moved(cell);
    }
    // Un rectangle entier doit rester sur la carte, pas seulement son coin.
    const std::vector<core::GridPosition> covered = entityCells(result);
    if (!inside(result.position) || !std::ranges::all_of(covered, inside)) {
        return std::nullopt;
    }
    return result;
}

std::optional<EntityPick> pickEntity(const std::vector<core::MapEntity>& entities,
                                     core::GridPosition cell,
                                     const std::vector<std::size_t>& selection) {
    // 1. Une poignee d'une entite selectionnee, la derniere selectionnee d'abord.
    for (auto selected = selection.rbegin(); selected != selection.rend(); ++selected) {
        if (*selected >= entities.size()) {
            continue;
        }
        if (const std::optional<EntityHandle> handle = handleAt(entities[*selected], cell)) {
            return EntityPick{.index = *selected, .handle = handle, .body = false};
        }
    }
    // 2. Une entite dont c'est la case, ou un point de passage : la derniere posee d'abord.
    for (std::size_t index = entities.size(); index-- > 0;) {
        const core::MapEntity& entity = entities[index];
        const bool onPath = entityShape(entity) == core::EntityShape::Path &&
                            std::ranges::find(entity.cells, cell) != entity.cells.end();
        if (entity.position == cell || onPath) {
            return EntityPick{.index = index, .handle = std::nullopt, .body = false};
        }
    }
    // 3. Le corps d'une forme, la plus petite d'abord ; a egalite, la derniere posee.
    std::optional<EntityPick> best;
    long long bestArea = 0;
    for (std::size_t index = 0; index < entities.size(); ++index) {
        const core::MapEntity& entity = entities[index];
        const core::EntityShape shape = entityShape(entity);
        if (shape != core::EntityShape::Rectangle && shape != core::EntityShape::Area) {
            continue;
        }
        const std::optional<CellRect> rect = entityRectangle(entity);
        const bool covers = rect ? rect->contains(cell)
                                 : std::ranges::find(entity.cells, cell) != entity.cells.end();
        if (!covers) {
            continue;
        }
        const long long area = areaOf(entity);
        if (!best || area <= bestArea) {
            best = EntityPick{.index = index, .handle = std::nullopt, .body = true};
            bestArea = area;
        }
    }
    return best;
}

std::string entityLabel(const core::MapEntity& entity) {
    const core::EntityKind* const kind = core::findEntityKind(entity.type);
    return kind != nullptr ? textOf(entity, kind->labelProperty) : std::string{};
}

std::string entityFigure(const core::MapEntity& entity) {
    const core::EntityKind* const kind = core::findEntityKind(entity.type);
    return kind != nullptr ? textOf(entity, kind->figureProperty) : std::string{};
}

std::vector<std::size_t> filterEntities(const std::vector<core::MapEntity>& entities,
                                        std::string_view filter) {
    const std::string wanted = lowered(filter);
    const auto matches = [&wanted](std::string_view text) {
        return lowered(text).find(wanted) != std::string::npos;
    };
    std::vector<std::size_t> kept;
    for (std::size_t index = 0; index < entities.size(); ++index) {
        const core::MapEntity& entity = entities[index];
        bool found = wanted.empty() || matches(entity.type) || matches(entity.id);
        for (const auto& property : entity.properties) {
            if (found) {
                break;
            }
            if (const auto* const text = std::get_if<std::string>(&property.second)) {
                found = matches(*text);
            }
        }
        if (found) {
            kept.push_back(index);
        }
    }
    return kept;
}

std::vector<std::size_t> toggledSelection(std::vector<std::size_t> selection, std::size_t index) {
    if (const auto found = std::ranges::find(selection, index); found != selection.end()) {
        selection.erase(found);
    } else {
        selection.push_back(index);
    }
    std::ranges::sort(selection);
    return selection;
}

}  // namespace hmi
