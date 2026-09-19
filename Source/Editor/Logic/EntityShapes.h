// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "Core/Levels/GridPosition.h"
#include "Core/Levels/MapEntity.h"
#include "Core/World/EntityKinds.h"

/**
 * @file Editor/Logic/EntityShapes.h
 * @brief Les entités à forme sur le canevas : rectangle, zone peinte, trajet, leurs poignées, le
 *        choix de l'entité sous le curseur et la liste filtrable (`LOT-EDITOR-05`, `EX-EDIT-070`,
 *        `EX-EDIT-072`).
 *
 * Tout part de la **forme** que la famille déclare (`core::EntityKind::shape`), jamais de son
 * type : une famille nouvelle reçoit poignées, tracé et déplacement sans une ligne d'éditeur (§5,
 * règle 2 de la feuille de route). Une famille inconnue est un point.
 *
 * Chaque fonction rend l'entité **modifiée**, sans toucher au brouillon : le canevas l'y écrit
 * par `core::LevelDraft::replaceEntity`, en un pas, et montre la même valeur en aperçu pendant le
 * glisser.
 */

namespace hmi {

/// @return La forme de la famille de @p entity ; `Point` pour une famille inconnue.
[[nodiscard]] core::EntityShape entityShape(const core::MapEntity& entity);

/// @brief Un rectangle de cases : son coin haut-gauche et sa taille, au moins 1 × 1.
struct CellRect {
    core::GridPosition origin{};
    int columns = 1;
    int rows = 1;

    /// @return Le coin bas-droit, inclus.
    [[nodiscard]] core::GridPosition last() const noexcept {
        return {.column = origin.column + columns - 1, .row = origin.row + rows - 1};
    }
    [[nodiscard]] bool contains(core::GridPosition cell) const noexcept;
    [[nodiscard]] bool operator==(const CellRect&) const = default;
};

/// @return Le rectangle dont @p a et @p b sont deux coins opposés, bornes comprises.
[[nodiscard]] CellRect rectangleBetween(core::GridPosition a, core::GridPosition b) noexcept;

/**
 * @return Le rectangle de @p entity : une forme `Rectangle`, ou une `Area` sans case peinte ; sa
 *         taille est lue dans `width` et `height` (1 si elle manque, ou ne vaut pas un entier
 *         positif — la saisie fautive est signalée ailleurs). Rien pour une autre forme.
 */
[[nodiscard]] std::optional<CellRect> entityRectangle(const core::MapEntity& entity);

/// @return @p entity posée sur @p rect : sa case au coin haut-gauche, sa taille dans `width` et
///         `height`.
[[nodiscard]] core::MapEntity withRectangle(core::MapEntity entity, const CellRect& rect);

/**
 * @return Les cases que @p entity occupe sur la carte : sa case pour un point, toutes les cases de
 *         son rectangle, ses cases peintes, ou ses points de passage dans l'ordre.
 */
[[nodiscard]] std::vector<core::GridPosition> entityCells(const core::MapEntity& entity);

/// @brief Une poignée : ce qu'on tire pour changer la forme d'une entité sélectionnée.
enum class HandleKind {
    NorthWest,
    North,
    NorthEast,
    East,
    SouthEast,
    South,
    SouthWest,
    West,
    /// Un point de passage d'un trajet ; `EntityHandle::waypoint` dit lequel.
    Waypoint,
};

/// @brief Une poignée et la case où elle se tient.
struct EntityHandle {
    HandleKind kind = HandleKind::NorthWest;
    core::GridPosition cell{};
    std::size_t waypoint = 0;

    [[nodiscard]] bool operator==(const EntityHandle&) const = default;
};

/**
 * @brief Les poignées de @p entity.
 *
 * Un rectangle en a huit : quatre coins, puis le milieu de chaque côté, omis quand il tomberait
 * sur un coin (un côté de moins de trois cases). Un trajet en a une par point de passage. Un
 * point et une zone peinte n'en ont pas : le premier se déplace, la seconde se peint (outil
 * Forme).
 */
[[nodiscard]] std::vector<EntityHandle> entityHandles(const core::MapEntity& entity);

/// @return La poignée de @p entity en @p cell, les coins avant les côtés ; rien sinon.
[[nodiscard]] std::optional<EntityHandle> handleAt(const core::MapEntity& entity,
                                                   core::GridPosition cell);

/**
 * @brief Le rectangle @p rect dont la poignée @p handle est tirée jusqu'à @p cell.
 *
 * Un coin déplace ses deux côtés, un milieu de côté le sien seul ; le côté opposé ne bouge pas.
 * Tirer au-delà du côté opposé retourne le rectangle, qui garde au moins une case.
 */
[[nodiscard]] CellRect resizeRectangle(const CellRect& rect, HandleKind handle,
                                       core::GridPosition cell) noexcept;

/**
 * @brief Ajoute (@p add) ou retire les cases @p cells de la zone peinte @p entity (décision D13).
 *
 * Une zone rectangle devient peinte à sa première retouche : ses cases se copient dans `cells`,
 * `width` et `height` partent. Les cases restent triées (ligne, colonne), sans doublon : l'ordre
 * d'une zone ne veut rien dire, et le diff du fichier reste stable. La case de l'entité reste une
 * case de la zone : si on la retire, la première des cases restantes la remplace.
 *
 * @return La zone retouchée ; @p entity telle quelle si la retouche la viderait.
 */
[[nodiscard]] core::MapEntity paintArea(core::MapEntity entity,
                                        const std::vector<core::GridPosition>& cells, bool add);

/// @return Le trajet @p entity prolongé d'un point en @p cell ; tel quel si c'est déjà son dernier.
[[nodiscard]] core::MapEntity withWaypointAdded(core::MapEntity entity, core::GridPosition cell);

/// @return Le trajet @p entity dont le point @p index est porté en @p cell ; le premier emporte la
///         case de l'entité.
[[nodiscard]] core::MapEntity withWaypointMoved(core::MapEntity entity, std::size_t index,
                                                core::GridPosition cell);

/// @return Le trajet @p entity sans son point @p index ; tel quel s'il ne lui en reste qu'un.
[[nodiscard]] core::MapEntity withWaypointRemoved(core::MapEntity entity, std::size_t index);

/**
 * @return @p entity déplacée de (@p columns, @p rows), sa case et toutes ses cases ; rien si une
 *         de ses cases sortirait d'une carte de @p width × @p height (un rectangle entier compris).
 */
[[nodiscard]] std::optional<core::MapEntity> translatedEntity(const core::MapEntity& entity,
                                                              int columns, int rows, int width,
                                                              int height);

/// @brief Ce que le curseur désigne : une entité, et la poignée tenue s'il y en a une.
struct EntityPick {
    std::size_t index = 0;
    std::optional<EntityHandle> handle;
    /// Vrai si le curseur est sur le **corps** d'une forme (un rectangle, une zone), et non sur sa
    /// case ni sur une poignée : poser une entité y reste possible.
    bool body = false;

    [[nodiscard]] bool operator==(const EntityPick&) const = default;
};

/**
 * @brief L'entité sous @p cell.
 *
 * Dans l'ordre : une poignée d'une entité sélectionnée ; une entité dont c'est la case ou un point
 * de passage, la dernière posée d'abord (celle qu'on voit au-dessus) ; enfin le corps d'une forme
 * qui couvre la case, la plus petite d'abord — un îlot de Martpart en couvre des centaines, et un
 * clic dans une zone posée dedans doit la prendre, elle.
 *
 * @param entities  Les entités de la carte.
 * @param cell      La case pointée.
 * @param selection Les rangs sélectionnés : seules leurs poignées se prennent.
 */
[[nodiscard]] std::optional<EntityPick> pickEntity(const std::vector<core::MapEntity>& entities,
                                                   core::GridPosition cell,
                                                   const std::vector<std::size_t>& selection);

/// @return La valeur de la propriété d'étiquette de @p entity (`core::EntityKind::labelProperty`),
///         vide sans étiquette.
[[nodiscard]] std::string entityLabel(const core::MapEntity& entity);

/**
 * @return Le nom de la figurine de @p entity (`core::EntityKind::figureProperty`), vide si sa
 *         famille n'en a pas ou si elle n'en nomme aucune.
 */
[[nodiscard]] std::string entityFigure(const core::MapEntity& entity);

/**
 * @brief Les rangs des entités que @p filter désigne, dans l'ordre de la carte.
 *
 * Sans tenir compte de la casse, le filtre cherche dans le type, l'identifiant et chaque valeur
 * texte des propriétés : `portal` les portails, `martpart` ce qui mène à Martpart, `e12` l'entité
 * `e12`. Vide : toutes.
 */
[[nodiscard]] std::vector<std::size_t> filterEntities(const std::vector<core::MapEntity>& entities,
                                                      std::string_view filter);

/// @return @p selection avec @p index ajouté s'il n'y était pas, retiré sinon ; triée.
[[nodiscard]] std::vector<std::size_t> toggledSelection(std::vector<std::size_t> selection,
                                                        std::size_t index);

}  // namespace hmi
