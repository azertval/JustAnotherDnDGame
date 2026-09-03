// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <vector>

#include "Core/Levels/GridPosition.h"
#include "Core/Levels/TileType.h"
#include "Core/Math/Vector2.h"

/**
 * @file HMI/Editor/LinkGeometry.h
 * @brief Géométrie et modèle d'affichage des liens de mécanismes (LOT-37), purs et testables.
 */

namespace core {
class LevelDraft;
}

namespace hmi {

/// @return true si @p type est un déclencheur de liaison (interrupteur, plaque de pression ou
///         clé, `EX-GP-023`).
[[nodiscard]] constexpr bool isTriggerTile(core::TileType type) noexcept {
    return type == core::TileType::Switch || type == core::TileType::PressurePlate ||
           type == core::TileType::Key;
}

/// @return true si @p type est une cible de liaison (porte ou porte verrouillée, `EX-GP-023`).
[[nodiscard]] constexpr bool isLinkTargetTile(core::TileType type) noexcept {
    return type == core::TileType::Door || type == core::TileType::LockedDoor;
}

/// @brief Segment reliant le centre d'un déclencheur au centre de sa cible, en unités monde.
struct LinkSegment {
    core::Vector2 a;
    core::Vector2 b;
};

/**
 * @brief Calcule le segment reliant le centre de la case @p trigger au centre de la case
 *        @p target, avec un éventuel décalage perpendiculaire anti-superposition de l'extrémité
 *        cible.
 *
 * Quand plusieurs liens partagent le même déclencheur, `fanIndex`/`fanCount` (index de ce lien
 * parmi ses frères, nombre total de frères) écartent les cibles les unes des autres
 * perpendiculairement à leur direction, pour rester lisibles (`EX-IHM-030`) — **en éventail** :
 * l'extrémité `a` (centre du déclencheur) reste **la même** pour tous les frères, seule
 * l'extrémité `b` (cible) se décale. Un décalage des deux extrémités produirait des traits
 * parallèles empilés, moins lisible qu'un éventail partant d'une base commune.
 * @param trigger   Case du déclencheur (origine du segment).
 * @param target    Case de la cible (extrémité du segment).
 * @param fanIndex  Index de ce lien parmi les liens partageant le même déclencheur (0-based).
 * @param fanCount  Nombre total de liens partageant ce déclencheur (>= 1).
 * @return Le segment (centres de case), décalé si `fanCount > 1`.
 */
[[nodiscard]] LinkSegment linkSegment(core::GridPosition trigger, core::GridPosition target,
                                      int fanIndex = 0, int fanCount = 1) noexcept;

/// @brief Les deux ailes de la pointe d'une flèche, à son extrémité.
struct ArrowHead {
    core::Vector2 left;
    core::Vector2 right;
};

/**
 * @brief Calcule les deux ailes de la pointe d'une flèche allant de @p from vers @p to.
 * @param from Origine du trait (sert seulement à orienter la pointe).
 * @param to   Extrémité du trait (sommet de la pointe).
 * @return Les deux points d'aile ; `to` deux fois si le segment est dégénéré (`from == to`).
 */
[[nodiscard]] ArrowHead arrowHead(core::Vector2 from, core::Vector2 to) noexcept;

/// @brief Nature d'une liaison affichée (couleur/libellé distincts dans le panneau et le rendu).
enum class LinkKind { Mechanism };

/// @brief Une ligne du panneau « Liens » (ou une entrée à dessiner dans le viewport).
struct LinkRow {
    LinkKind kind;
    core::GridPosition trigger;
    core::GridPosition target;
};

/**
 * @brief Construit la liste des liaisons d'un brouillon, dans un ordre déterministe.
 *
 * Alimente à la fois le panneau « Liens » (`hmi::LinkPanel`) et le rendu (`hmi::DraftRenderer`) —
 * source unique, aucune duplication (`EX-EDIT-010`). Ordre : celui de `draft.mechanisms()`.
 * @param draft Brouillon dont on liste les liaisons.
 * @return Les liaisons, dans un ordre stable.
 */
[[nodiscard]] std::vector<LinkRow> buildLinkRows(const core::LevelDraft& draft);

}  // namespace hmi
