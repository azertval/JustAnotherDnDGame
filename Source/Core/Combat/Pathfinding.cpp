// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Core/Combat/Pathfinding.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <functional>
#include <limits>
#include <queue>
#include <utility>

#include "Core/Rpg/Bestiary.h"
#include "Core/Rpg/CharacterSheet.h"
#include "Core/Rpg/Scale.h"

namespace core {

namespace {

constexpr int UNREACHED = std::numeric_limits<int>::max();
constexpr int NO_PREVIOUS = -1;

/// Tolérance de l'arrondi à la case inférieure : 9 / 1,5 tombe juste en binaire, mais une vitesse
/// issue d'une soustraction de flottants (malus d'encombrement) peut valoir 5,9999 cases, et en
/// perdre une sur une erreur d'arrondi serait un défaut invisible.
constexpr float SPEED_EPSILON = 1.0e-3F;

/// Les huit voisins, dans un ordre **fixe**. Le départage ne dépend pas de cet ordre (il porte sur
/// l'indice de case), mais un ordre fixe garde la file identique d'une exécution à l'autre.
constexpr std::array<std::pair<int, int>, 8> NEIGHBOURS{{
    {0, -1},
    {-1, 0},
    {1, 0},
    {0, 1},
    {-1, -1},
    {1, -1},
    {-1, 1},
    {1, 1},
}};

/// Coût d'entrée normal et en terrain difficile (Manuel des Joueurs, « Jouer sur un quadrillage »).
constexpr int NORMAL_COST = 1;
constexpr int DIFFICULT_COST = 2;

/// Ce qu'il faut savoir d'un combattant pour dire si un pas est permis, et ce qu'il coûte.
struct StepRules {
    const BattleGrid& grid;
    const Mover& mover;
    int side = 1;

    /// @return Le coût d'entrée de @p to depuis @p from, voisin, ou `std::nullopt` si le pas est
    /// interdit.
    [[nodiscard]] std::optional<int> cost(GridPosition from, GridPosition to) const {
        if (!grid.isClear(to, side, mover.locomotion)) {
            return std::nullopt;
        }
        const bool diagonal = from.column != to.column && from.row != to.row;
        // Le coin d'un mur : on regarde les deux places orthogonales que la diagonale enjambe. Le
        // coin se juge en vol — seule la matière qui « remplit l'espace » l'interdit, pas une eau
        // profonde qu'on ne fait que longer.
        if (diagonal && (!grid.isClear({to.column, from.row}, side, Locomotion::Fly) ||
                         !grid.isClear({from.column, to.row}, side, Locomotion::Fly))) {
            return std::nullopt;
        }
        bool difficult = false;
        for (int row = to.row; row < to.row + side; ++row) {
            for (int column = to.column; column < to.column + side; ++column) {
                const GridPosition cell{column, row};
                const std::optional<CombatantId> occupant = grid.occupantAt(cell);
                if (occupant.has_value() && *occupant != mover.combatant &&
                    !(mover.canPassThrough && mover.canPassThrough(*occupant))) {
                    return std::nullopt;
                }
                difficult = difficult || grid.isDifficult(cell);
            }
        }
        // Le terrain difficile est une gêne de sol : un volant ne la paie pas (`core::Locomotion`).
        return difficult && mover.locomotion == Locomotion::Walk ? DIFFICULT_COST : NORMAL_COST;
    }
};

/// Élément de file : clé de priorité, puis indice de case — l'ordre total qui rend la file
/// déterministe.
using QueueEntry = std::pair<int, std::size_t>;
using MinQueue = std::priority_queue<QueueEntry, std::vector<QueueEntry>, std::greater<>>;

/**
 * Relâche l'arc @p current → @p next de coût cumulé @p candidate.
 *
 * @return Vrai si le coût de @p next a **baissé** — il faut alors le remettre en file. À coût égal,
 * seul le prédécesseur change, et seulement pour un indice plus petit : c'est la règle de
 * départage, et elle est la même dans les deux algorithmes.
 */
bool relax(std::vector<int>& costs, std::vector<int>& previous, std::size_t current,
           std::size_t next, int candidate) {
    if (candidate < costs[next]) {
        costs[next] = candidate;
        previous[next] = static_cast<int>(current);
        return true;
    }
    if (candidate == costs[next] && static_cast<int>(current) < previous[next]) {
        previous[next] = static_cast<int>(current);
    }
    return false;
}

[[nodiscard]] Path rebuild(const std::vector<int>& costs, const std::vector<int>& previous,
                           std::size_t target, int width) {
    Path path;
    path.cost = costs[target];
    for (int index = static_cast<int>(target);
         previous[static_cast<std::size_t>(index)] != NO_PREVIOUS;
         index = previous[static_cast<std::size_t>(index)]) {
        path.steps.push_back({index % width, index / width});
    }
    std::reverse(path.steps.begin(), path.steps.end());
    return path;
}

}  // namespace

int movementBudget(float speedMeters) noexcept {
    if (!(speedMeters > 0.0F)) {
        return 0;
    }
    return static_cast<int>(std::floor(tilesFromMeters(speedMeters) + SPEED_EPSILON));
}

int movementBudget(const CharacterSheet& sheet) noexcept {
    return movementBudget(sheet.speedMeters);
}

int movementBudget(const Creature& creature, Locomotion locomotion) noexcept {
    switch (locomotion) {
        case Locomotion::Walk:
            return movementBudget(creature.speed.walk);
        case Locomotion::Fly:
            return movementBudget(creature.speed.fly.value_or(0.0F));
    }
    return 0;
}

ReachableArea::ReachableArea(const BattleGrid& grid, const Mover& mover, int budget)
    : _width(grid.width()),
      _height(grid.height()),
      _budget(std::max(budget, 0)),
      _costs(static_cast<std::size_t>(_width) * static_cast<std::size_t>(_height), UNREACHED),
      _previous(_costs.size(), NO_PREVIOUS),
      _endable(_costs.size(), false) {
    const std::optional<GridPosition> start = grid.positionOf(mover.combatant);
    if (!start.has_value()) {
        return;
    }
    _origin = *start;
    const StepRules rules{.grid = grid, .mover = mover, .side = grid.sideOf(mover.combatant)};

    MinQueue frontier;
    _costs[indexOf(_origin)] = 0;
    frontier.push({0, indexOf(_origin)});
    while (!frontier.empty()) {
        const auto [cost, index] = frontier.top();
        frontier.pop();
        if (cost > _costs[index]) {
            continue;  // Entrée périmée : la case a été atteinte moins cher depuis.
        }
        const GridPosition current{static_cast<int>(index) % _width,
                                   static_cast<int>(index) / _width};
        for (const auto& [deltaColumn, deltaRow] : NEIGHBOURS) {
            const GridPosition next{current.column + deltaColumn, current.row + deltaRow};
            if (!grid.inBounds(next)) {
                continue;
            }
            const std::optional<int> step = rules.cost(current, next);
            // Il faut pouvoir payer l'entrée entière : une case difficile à une case du budget est
            // hors de portée, pas « à moitié » atteinte.
            if (!step.has_value() || cost + *step > _budget) {
                continue;
            }
            const std::size_t nextIndex = indexOf(next);
            if (relax(_costs, _previous, index, nextIndex, cost + *step)) {
                frontier.push({_costs[nextIndex], nextIndex});
            }
        }
    }

    for (std::size_t index = 0; index < _costs.size(); ++index) {
        const GridPosition anchor{static_cast<int>(index) % _width,
                                  static_cast<int>(index) / _width};
        _endable[index] = _costs[index] != UNREACHED && anchor != _origin &&
                          grid.canStand(anchor, rules.side, mover.combatant, mover.locomotion);
    }
}

std::size_t ReachableArea::indexOf(GridPosition cell) const noexcept {
    return static_cast<std::size_t>(cell.row) * static_cast<std::size_t>(_width) +
           static_cast<std::size_t>(cell.column);
}

std::optional<int> ReachableArea::costTo(GridPosition anchor) const {
    if (anchor.column < 0 || anchor.row < 0 || anchor.column >= _width || anchor.row >= _height) {
        return std::nullopt;
    }
    const int cost = _costs[indexOf(anchor)];
    return cost == UNREACHED ? std::nullopt : std::optional<int>(cost);
}

bool ReachableArea::canEndAt(GridPosition anchor) const {
    if (anchor.column < 0 || anchor.row < 0 || anchor.column >= _width || anchor.row >= _height) {
        return false;
    }
    return _endable[indexOf(anchor)];
}

std::vector<GridPosition> ReachableArea::destinations() const {
    std::vector<GridPosition> cells;
    for (std::size_t index = 0; index < _endable.size(); ++index) {
        if (_endable[index]) {
            cells.push_back({static_cast<int>(index) % _width, static_cast<int>(index) / _width});
        }
    }
    return cells;
}

std::optional<Path> ReachableArea::pathTo(GridPosition anchor) const {
    if (!canEndAt(anchor)) {
        return std::nullopt;
    }
    return rebuild(_costs, _previous, indexOf(anchor), _width);
}

std::optional<Path> findPath(const BattleGrid& grid, const Mover& mover, GridPosition destination) {
    const std::optional<GridPosition> start = grid.positionOf(mover.combatant);
    const int side = grid.sideOf(mover.combatant);
    if (!start.has_value() ||
        !grid.canStand(destination, side, mover.combatant, mover.locomotion)) {
        return std::nullopt;
    }
    const int width = grid.width();
    const auto indexOf = [width](GridPosition cell) {
        return static_cast<std::size_t>(cell.row) * static_cast<std::size_t>(width) +
               static_cast<std::size_t>(cell.column);
    };
    // Distance de Tchebychev : chaque pas coûte au moins 1 et avance d'au plus une case sur chaque
    // axe. L'heuristique est donc cohérente — une case sortie de la file a son coût définitif.
    const auto estimate = [destination](GridPosition cell) {
        return std::max(std::abs(cell.column - destination.column),
                        std::abs(cell.row - destination.row));
    };
    const StepRules rules{.grid = grid, .mover = mover, .side = side};

    const std::size_t cellTotal =
        static_cast<std::size_t>(width) * static_cast<std::size_t>(grid.height());
    std::vector<int> costs(cellTotal, UNREACHED);
    std::vector<int> previous(cellTotal, NO_PREVIOUS);
    const std::size_t target = indexOf(destination);

    MinQueue frontier;
    costs[indexOf(*start)] = 0;
    frontier.push({estimate(*start), indexOf(*start)});
    while (!frontier.empty()) {
        const auto [priority, index] = frontier.top();
        // On ne s'arrête pas à la première sortie de la destination : tant qu'une case de même
        // estimation totale reste en file, elle peut offrir un prédécesseur de même coût et
        // d'indice plus petit, que le départage retient. S'arrêter plus tôt rendrait un chemin
        // juste, mais pas toujours celui de `ReachableArea::pathTo`.
        if (costs[target] != UNREACHED && priority > costs[target]) {
            break;
        }
        frontier.pop();
        const GridPosition current{static_cast<int>(index) % width,
                                   static_cast<int>(index) / width};
        if (priority - estimate(current) > costs[index]) {
            continue;  // Entrée périmée.
        }
        for (const auto& [deltaColumn, deltaRow] : NEIGHBOURS) {
            const GridPosition next{current.column + deltaColumn, current.row + deltaRow};
            if (!grid.inBounds(next)) {
                continue;
            }
            const std::optional<int> step = rules.cost(current, next);
            if (!step.has_value()) {
                continue;
            }
            const std::size_t nextIndex = indexOf(next);
            if (relax(costs, previous, index, nextIndex, costs[index] + *step)) {
                frontier.push({costs[nextIndex] + estimate(next), nextIndex});
            }
        }
    }

    if (costs[target] == UNREACHED) {
        return std::nullopt;
    }
    return rebuild(costs, previous, target, width);
}

}  // namespace core
