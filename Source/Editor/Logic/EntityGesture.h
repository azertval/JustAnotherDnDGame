// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "Core/Levels/GridPosition.h"
#include "Core/Levels/MapEntity.h"
#include "Editor/Logic/EntityShapes.h"

namespace core {
class LevelDraft;
}

/**
 * @file Editor/Logic/EntityGesture.h
 * @brief Gestes des outils « Entité » et « Forme » : sélectionner, poser, tirer une zone,
 *        déplacer un groupe, tirer une poignée, peindre une zone, tracer un trajet
 *        (`LOT-11`, `LOT-EDITOR-05`) — purs et testables.
 *
 * Un geste se résout en deux temps. À l'appui, `resolveEntityPress` dit ce que la case désigne ;
 * pendant le glisser et au relâchement, `dragEntities` dit ce que le geste **ferait** s'il
 * s'arrêtait sur la case courante : le canevas en montre l'aperçu, puis l'écrit au relâchement,
 * dans un seul geste du brouillon (`core::GestureScope`).
 */

namespace hmi {

/// @brief Ce que l'outil « Entité » fait d'un appui.
enum class EntityGestureAction {
    Ignore,    ///< Rien à faire : hors de la carte.
    Grab,      ///< Prendre l'entité `entityIndex` (et la sélectionner) : le glisser la déplace, ou
               ///< tire sa poignée `handle`.
    Toggle,    ///< `Maj` + clic : ajouter l'entité `entityIndex` à la sélection, ou l'en retirer.
    Deselect,  ///< Case libre sans famille à poser : la sélection se vide.
    Place,     ///< Poser une entité ponctuelle de la famille choisie en `cell`.
    Draw,      ///< Tirer une entité à forme (rectangle, zone, trajet) depuis `cell`.
};

/// @brief Décision rendue par `resolveEntityPress`.
struct EntityGestureDecision {
    EntityGestureAction action = EntityGestureAction::Ignore;
    std::size_t entityIndex = 0;
    core::GridPosition cell{};
    std::optional<EntityHandle> handle;

    [[nodiscard]] bool operator==(const EntityGestureDecision&) const = default;
};

/// @brief Les touches tenues pendant l'appui.
struct EntityPressModifiers {
    /// `Ctrl` : poser même sur une case occupée.
    bool force = false;
    /// `Maj` : basculer l'entité dans la sélection au lieu de la prendre.
    bool toggle = false;
};

/**
 * @brief Résout l'appui du bouton gauche sur @p cell.
 *
 * L'entité sous la case se choisit par `hmi::pickEntity` : une poignée d'une entité sélectionnée,
 * puis une entité dont c'est la case, puis le corps d'une forme. Une entité désignée par sa case ou
 * sa poignée se **prend** ; poser **sur** elle demande @p modifiers `force` (`Ctrl`), sans quoi un
 * clic pour la sélectionner en empilerait une seconde dessus. Le corps d'une forme, lui, n'empêche
 * pas de poser : on pose les entrées d'arène dans la zone de combat. Une famille à forme se
 * **tire** au lieu de se poser.
 *
 * @param draft       Le brouillon.
 * @param cell        La case cliquée, dans la grille.
 * @param selection   Les rangs sélectionnés.
 * @param kindToPlace Le type à poser, ou vide pour l'outil en simple sélection.
 * @param modifiers   Les touches tenues.
 */
[[nodiscard]] EntityGestureDecision resolveEntityPress(const core::LevelDraft& draft,
                                                       core::GridPosition cell,
                                                       const std::vector<std::size_t>& selection,
                                                       const std::string& kindToPlace,
                                                       EntityPressModifiers modifiers);

/// @brief Un glisser en cours de l'outil « Entité ».
struct EntityDrag {
    enum class Mode {
        /// Déplacer les entités `indices` (la sélection, ou la seule prise).
        Move,
        /// Tirer la poignée `handle` de l'entité `indices.front()`.
        Reshape,
        /// Tirer une entité neuve de la famille `kind`.
        Draw,
    };
    Mode mode = Mode::Move;
    std::vector<std::size_t> indices;
    std::optional<EntityHandle> handle;
    std::string kind;
    /// La case de l'appui.
    core::GridPosition from{};
};

/// @brief Ce qu'un glisser ferait s'il s'arrêtait sur une case.
struct EntityDragResult {
    /// Les entités remplacées : rang, et nouvelle valeur.
    std::vector<std::pair<std::size_t, core::MapEntity>> replaced;
    /// L'entité à poser (`Draw`).
    std::optional<core::MapEntity> placed;
    /// Vrai si le déplacement sortirait une entité de la carte : rien ne bouge.
    bool refused = false;

    [[nodiscard]] bool empty() const noexcept {
        return replaced.empty() && !placed;
    }
};

/**
 * @brief Le résultat de @p drag arrêté sur @p to, sur une carte de @p width × @p height.
 *
 * Un déplacement de groupe bouge tout le groupe ou rien : une entité qui sortirait de la carte le
 * refuse en entier. Un glisser qui revient sur sa case d'appui ne change rien. Un rectangle tiré
 * part de la case d'appui ; un trajet tiré a deux points, un seul s'il n'a pas bougé.
 */
[[nodiscard]] EntityDragResult dragEntities(const EntityDrag& drag,
                                            const std::vector<core::MapEntity>& entities,
                                            core::GridPosition to, int width, int height);

/// @brief Ce que l'outil « Forme » fait d'un appui sur l'entité sélectionnée.
enum class ShapeGestureAction {
    Ignore,          ///< Pas de forme à retoucher : un point, un rectangle que le jeu lit tel quel.
    PaintCells,      ///< Ajouter à la zone les cases du glisser.
    EraseCells,      ///< Les retirer (`Ctrl`).
    AppendWaypoint,  ///< Prolonger le trajet d'un point en `cell`.
    GrabWaypoint,    ///< Prendre le point `waypoint` : le glisser le porte.
    RemoveWaypoint,  ///< Retirer le point `waypoint` (`Ctrl`).
};

/// @brief Décision rendue par `resolveShapePress`.
struct ShapeGestureDecision {
    ShapeGestureAction action = ShapeGestureAction::Ignore;
    std::size_t waypoint = 0;

    [[nodiscard]] bool operator==(const ShapeGestureDecision&) const = default;
};

/**
 * @brief Résout l'appui de l'outil « Forme » sur @p cell, pour l'entité @p entity.
 *
 * Une zone de règles (`Area`) se peint : l'appui ajoute des cases, `Ctrl` en retire. Un trajet se
 * trace : un appui ailleurs le prolonge, un appui sur un de ses points le prend, `Ctrl` le retire.
 * Un rectangle que le jeu lit tel quel (zone de combat, îlot) garde ses poignées : l'outil ne le
 * change pas en cases.
 *
 * @param entity L'entité sélectionnée, que l'outil retouche.
 * @param cell   La case cliquée.
 * @param remove `Ctrl` est tenu.
 */
[[nodiscard]] ShapeGestureDecision resolveShapePress(const core::MapEntity& entity,
                                                     core::GridPosition cell, bool remove);

/// @brief Ce qu'un glisser a écrit dans le brouillon.
struct EntityDragApplied {
    /// Le brouillon a changé (un pas d'annulation de plus).
    bool changed = false;
    /// Le rang de l'entité tirée (`Draw`), si elle a été posée.
    std::optional<std::size_t> placed;
};

/**
 * @brief Écrit @p result dans @p draft, en **un** geste du brouillon (`core::GestureScope`) : un
 *        groupe déplacé, une zone tirée se défont en un pas.
 *
 * Ce que le canevas fait au relâchement, et ce que l'éditeur sans fenêtre rejoue
 * (`LOT-EDITOR-13`). Un résultat refusé ou vide n'écrit rien.
 */
EntityDragApplied applyEntityDrag(core::LevelDraft& draft, const EntityDragResult& result);

/**
 * @brief Pose en @p cell une entité neuve de la famille @p kind, ses propriétés à leur défaut
 *        (`core::makeEntity`) — aucune pour une famille inconnue, qui se pose tout de même.
 * @return Son rang, ou rien si le brouillon l'a refusée.
 */
std::optional<std::size_t> placeEntityOfKind(core::LevelDraft& draft, const std::string& kind,
                                             core::GridPosition cell);

/**
 * @brief Retire les entités @p indices, en un geste, du dernier rang au premier : un retrait ne
 *        décale pas les rangs qui restent à retirer.
 * @return Le nombre d'entités retirées.
 */
std::size_t removeEntities(core::LevelDraft& draft, std::vector<std::size_t> indices);

}  // namespace hmi
