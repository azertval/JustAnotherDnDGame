// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <cstddef>
#include <map>
#include <set>
#include <string>
#include <string_view>
#include <vector>

#include "Core/Levels/GridPosition.h"
#include "Core/Levels/LevelProperties.h"
#include "Core/Levels/MapEntity.h"

/**
 * @file Core/World/EntityKinds.h
 * @brief Les familles d'entités de carte que l'éditeur sait poser, et ce qu'une carte leur doit
 *        (`LOT-11`).
 */

namespace core {

/// @brief Type d'entité d'un portail : il mène à une autre carte (`LOT-09` le traversera).
inline constexpr std::string_view PORTAL_ENTITY_TYPE = "portal";
/// @brief Propriété d'un portail : la carte cible, nommée par son **identifiant** — le nom de son
///        fichier dans `Source/Elements/Levels/`, sans extension.
inline constexpr std::string_view PORTAL_TARGET_MAP_PROPERTY = "targetMap";
/// @brief Propriété d'un portail : le point d'arrivée **nommé** de la carte cible. Jamais des
///        coordonnées, qui se désynchroniseraient au premier redimensionnement de la cible.
inline constexpr std::string_view PORTAL_ARRIVAL_PROPERTY = "arrival";
/// @brief Propriété d'un portail : le drapeau de monde qu'il exige pour s'ouvrir (`LOT-16` le
///        pose ; le `LOT-09` le lit). Absent ou vide, le portail est toujours franchissable.
inline constexpr std::string_view PORTAL_REQUIRED_FLAG_PROPERTY = "requiresFlag";
/// @brief Type d'entité d'un point d'arrivée nommé : là où l'on apparaît en entrant par un portail.
inline constexpr std::string_view SPAWN_POINT_ENTITY_TYPE = "spawnPoint";
/// @brief Propriété d'un point d'arrivée : son nom, unique dans la carte.
inline constexpr std::string_view SPAWN_POINT_NAME_PROPERTY = "name";

/// @brief Nature d'une propriété d'entité, qui décide du contrôle que l'éditeur lui donne.
enum class EntityPropertyKind {
    Text,
    Integer,
    Boolean,
    /// Une valeur texte choisie dans une liste (fixe, ou tirée d'un catalogue).
    Choice,
};

/// @brief D'où viennent les choix d'une propriété `Choice`.
enum class EntityChoiceSource {
    /// Les valeurs de `EntityPropertySpec::fixedChoices`.
    Fixed,
    /// Les dialogues **acceptés** au chargement (`core::loadDialogues`).
    Dialogues,
    /// Les rencontres du catalogue (`core::loadEncounters`).
    Encounters,
    /// Les cartes du dossier des niveaux, par identifiant.
    Maps,
    /// Les points d'arrivée de la carte nommée par `targetMap` de la **même** entité.
    ArrivalPoints,
};

/// @brief Une propriété qu'une famille d'entités déclare.
struct EntityPropertySpec {
    std::string_view key;
    EntityPropertyKind kind = EntityPropertyKind::Text;
    EntityChoiceSource source = EntityChoiceSource::Fixed;
    std::vector<std::string_view> fixedChoices;
    /// Une propriété requise absente (ou texte vide) est signalée ; elle n'empêche pas
    /// d'enregistrer.
    bool required = false;
    /// Valeur posée à la création de l'entité.
    PropertyValue defaultValue;
};

/// @brief Une famille d'entités : son type, et les propriétés qu'elle déclare.
struct EntityKind {
    std::string_view type;
    std::vector<EntityPropertySpec> properties;

    [[nodiscard]] const EntityPropertySpec* find(std::string_view key) const;
};

/**
 * @brief Les familles que l'éditeur sait poser, dans l'ordre de sa liste.
 *
 * Coffre, panneau, PNJ, rencontre, portail, point d'arrivée, entrée d'arène. Les types et leurs
 * propriétés sont ceux que le gameplay lit déjà (`core::knownInteractableKinds`,
 * `core::dialogueTriggerFor`, `core::encounterTriggerFor`, `core::arenaEntryPoints`) : la table ne
 * les invente pas, elle les rassemble. Un type absent de la table reste **légal** sur une carte
 * (`EX-NFR-040`) : l'éditeur le transporte et en montre les propriétés brutes.
 */
[[nodiscard]] const std::vector<EntityKind>& knownEntityKinds();

/// @return La famille de @p type, ou `nullptr` si elle n'est pas dans la table.
[[nodiscard]] const EntityKind* findEntityKind(std::string_view type);

/// @brief Une entité neuve de la famille @p kind en @p position, ses propriétés à leur défaut.
[[nodiscard]] MapEntity makeEntity(const EntityKind& kind, GridPosition position);

/// @brief Ce qu'une carte peut référencer hors d'elle-même : les catalogues et les autres cartes.
struct EntityReferenceContext {
    std::set<std::string, std::less<>> dialogues;
    std::set<std::string, std::less<>> encounters;
    /// Identifiant de carte → noms de ses points d'arrivée. La carte **éditée** y figure aussi : un
    /// portail peut ramener ailleurs sur la même carte.
    std::map<std::string, std::set<std::string, std::less<>>, std::less<>> arrivalPointsByMap;
};

/// @brief Ce qui ne va pas dans une entité. `Core` n'écrit pas de texte (`EX-NFR-011`) : l'éditeur
///        traduit le code et ses arguments.
enum class EntityIssueCode {
    /// Le type n'est pas dans `knownEntityKinds` — toléré, signalé pour information.
    UnknownType,
    /// Une propriété requise manque ou est vide. `key` la nomme.
    MissingProperty,
    /// Une propriété a une valeur du mauvais type (un entier là où l'on attend un texte…).
    WrongValueType,
    /// Une valeur `Choice` fixe hors de sa liste. `key` et `value`.
    InvalidChoice,
    /// Le dialogue nommé n'est pas dans le catalogue, ou y a été refusé. `value`.
    UnknownDialogue,
    /// La rencontre nommée n'est pas dans le catalogue. `value`.
    UnknownEncounter,
    /// La carte cible d'un portail n'existe pas. `value`.
    UnknownTargetMap,
    /// Le point d'arrivée d'un portail n'existe pas dans sa carte cible. `value`.
    UnknownArrivalPoint,
    /// Deux points d'arrivée de la carte portent le même nom. `value`.
    DuplicateArrivalPoint,
};

/// @brief Un problème relevé sur l'entité de rang `entityIndex`.
struct EntityIssue {
    std::size_t entityIndex = 0;
    EntityIssueCode code = EntityIssueCode::UnknownType;
    std::string key;
    std::string value;

    [[nodiscard]] bool operator==(const EntityIssue&) const = default;
};

/**
 * @brief Relève ce que les entités de @p entities référencent sans l'atteindre.
 *
 * **Avertit, ne refuse pas** : une carte s'écrit dans le désordre — le portail vers la forêt avant
 * la forêt —, et refuser de l'enregistrer tant que la cible n'existe pas imposerait un ordre de
 * travail que rien ne justifie. C'est au chargement du graphe du monde (`LOT-09`) qu'un portail
 * orphelin devient une erreur.
 *
 * @return Les problèmes, dans l'ordre des entités, puis des propriétés déclarées.
 */
[[nodiscard]] std::vector<EntityIssue> validateMapEntities(const std::vector<MapEntity>& entities,
                                                           const EntityReferenceContext& context);

/// @return Les noms des points d'arrivée de @p entities, sans doublon.
[[nodiscard]] std::set<std::string, std::less<>> arrivalPointNames(
    const std::vector<MapEntity>& entities);

}  // namespace core
