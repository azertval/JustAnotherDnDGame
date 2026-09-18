// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "Core/World/EntityKinds.h"

#include <algorithm>
#include <cstdint>
#include <optional>
#include <utility>
#include <variant>

#include "Core/Combat/Arena.h"
#include "Core/World/CityBlock.h"
#include "Core/World/CombatZone.h"
#include "Core/Combat/CombatTransition.h"
#include "Core/Rpg/Dialogue.h"

namespace core {

namespace {

// Proprietes lues par core::encounterTriggerFor (CombatTransition.cpp) : ecrites en dur la-bas, et
// nommees ici une seule fois pour la table.
constexpr std::string_view ENCOUNTER_ID_PROPERTY = "encounterId";
constexpr std::string_view ENCOUNTER_RESPAWNS_PROPERTY = "respawns";

[[nodiscard]] EntityPropertySpec choice(std::string_view key, EntityChoiceSource source,
                                        bool required) {
    return EntityPropertySpec{.key = key,
                              .kind = EntityPropertyKind::Choice,
                              .source = source,
                              .fixedChoices = {},
                              .required = required,
                              .defaultValue = std::string{}};
}

// Vrai si @p value a le type qu'attend @p kind.
[[nodiscard]] bool hasExpectedType(const PropertyValue& value, EntityPropertyKind kind) {
    switch (kind) {
        case EntityPropertyKind::Text:
        case EntityPropertyKind::Choice:
            return std::holds_alternative<std::string>(value);
        case EntityPropertyKind::Integer:
            return std::holds_alternative<std::int64_t>(value);
        case EntityPropertyKind::Boolean:
            return std::holds_alternative<bool>(value);
    }
    return false;
}

[[nodiscard]] std::string textOf(const PropertyMap& properties, std::string_view key) {
    const auto found = properties.find(std::string{key});
    if (found == properties.end()) {
        return {};
    }
    const auto* const text = std::get_if<std::string>(&found->second);
    return text != nullptr ? *text : std::string{};
}

}  // namespace

const EntityPropertySpec* EntityKind::find(std::string_view key) const {
    const auto found = std::ranges::find(properties, key, &EntityPropertySpec::key);
    return found != properties.end() ? &*found : nullptr;
}

const std::vector<EntityKind>& knownEntityKinds() {
    // Construite une seule fois. L'ordre est celui de la liste de l'editeur : ce qu'on pose le plus
    // souvent d'abord.
    static const std::vector<EntityKind> familles = {
        // Coffre et panneau (LOT-10) : aucune propriete lue par le gameplay aujourd'hui. Le contenu
        // d'un coffre arrive avec le butin (LOT-26) ; la table ne l'invente pas avant.
        EntityKind{.type = "chest", .properties = {}},
        EntityKind{.type = "sign", .properties = {}},
        // PNJ (LOT-15) : un figurant sans dialogue est legal, d'ou `required = false`.
        EntityKind{.type = NPC_ENTITY_TYPE,
                   .properties = {choice(NPC_DIALOGUE_PROPERTY, EntityChoiceSource::Dialogues,
                                         /*required=*/false),
                                  // La figurine de l'atelier (LOT-91) que la carte lui donne : un
                                  // PNJ sans figurine se parle et ne se dessine pas (LOT-09).
                                  EntityPropertySpec{.key = NPC_FIGURE_PROPERTY,
                                                     .kind = EntityPropertyKind::Text,
                                                     .source = EntityChoiceSource::Fixed,
                                                     .fixedChoices = {},
                                                     .required = false,
                                                     .defaultValue = std::string{}},
                                  // La sentinelle d'une porte gardee (LOT-96) : le quartier
                                  // qu'elle ferme, par sa fiche d'atlas.
                                  EntityPropertySpec{.key = NPC_GUARDED_DISTRICT_PROPERTY,
                                                     .kind = EntityPropertyKind::Text,
                                                     .source = EntityChoiceSource::Fixed,
                                                     .fixedChoices = {},
                                                     .required = false,
                                                     .defaultValue = std::string{}}}},
        // Rencontre (LOT-18) : sans rencontre nommee, le declencheur n'en est pas un.
        EntityKind{.type = ENCOUNTER_ENTITY_TYPE,
                   .properties = {choice(ENCOUNTER_ID_PROPERTY, EntityChoiceSource::Encounters,
                                         /*required=*/true),
                                  EntityPropertySpec{.key = ENCOUNTER_RESPAWNS_PROPERTY,
                                                     .kind = EntityPropertyKind::Boolean,
                                                     .source = EntityChoiceSource::Fixed,
                                                     .fixedChoices = {},
                                                     .required = false,
                                                     .defaultValue = false}}},
        // Portail et point d'arrivee (LOT-11, traverses au LOT-09).
        EntityKind{.type = PORTAL_ENTITY_TYPE,
                   .properties = {choice(PORTAL_TARGET_MAP_PROPERTY, EntityChoiceSource::Maps,
                                         /*required=*/true),
                                  choice(PORTAL_ARRIVAL_PROPERTY, EntityChoiceSource::ArrivalPoints,
                                         /*required=*/true),
                                  // Le drapeau exige n'est pas requis : un portail ordinaire
                                  // s'ouvre toujours, et c'est le LOT-16 qui posera les drapeaux
                                  // que celui-ci lit (LOT-09).
                                  EntityPropertySpec{.key = PORTAL_REQUIRED_FLAG_PROPERTY,
                                                     .kind = EntityPropertyKind::Text,
                                                     .source = EntityChoiceSource::Fixed,
                                                     .fixedChoices = {},
                                                     .required = false,
                                                     .defaultValue = std::string{}}}},
        EntityKind{.type = SPAWN_POINT_ENTITY_TYPE,
                   .properties = {EntityPropertySpec{.key = SPAWN_POINT_NAME_PROPERTY,
                                                     .kind = EntityPropertyKind::Text,
                                                     .source = EntityChoiceSource::Fixed,
                                                     .fixedChoices = {},
                                                     .required = true,
                                                     .defaultValue = std::string{}}}},
        // Zone de combat (LOT-09) : le rectangle nomme ou l'on se bat, et lui seul (EX-LVL-018).
        EntityKind{.type = COMBAT_ZONE_ENTITY_TYPE,
                   .properties = {EntityPropertySpec{.key = COMBAT_ZONE_NAME_PROPERTY,
                                                     .kind = EntityPropertyKind::Text,
                                                     .source = EntityChoiceSource::Fixed,
                                                     .fixedChoices = {},
                                                     .required = true,
                                                     .defaultValue = std::string{}},
                                  EntityPropertySpec{.key = COMBAT_ZONE_WIDTH_PROPERTY,
                                                     .kind = EntityPropertyKind::Integer,
                                                     .source = EntityChoiceSource::Fixed,
                                                     .fixedChoices = {},
                                                     .required = true,
                                                     .defaultValue = std::int64_t{1}},
                                  EntityPropertySpec{.key = COMBAT_ZONE_HEIGHT_PROPERTY,
                                                     .kind = EntityPropertyKind::Integer,
                                                     .source = EntityChoiceSource::Fixed,
                                                     .fixedChoices = {},
                                                     .required = true,
                                                     .defaultValue = std::int64_t{1}}}},
        // Ilot d'un quartier (LOT-96) : le rectangle nomme que le plan de la ville montre.
        EntityKind{.type = CITY_BLOCK_ENTITY_TYPE,
                   .properties = {EntityPropertySpec{.key = CITY_BLOCK_NAME_PROPERTY,
                                                     .kind = EntityPropertyKind::Text,
                                                     .source = EntityChoiceSource::Fixed,
                                                     .fixedChoices = {},
                                                     .required = true,
                                                     .defaultValue = std::string{}},
                                  EntityPropertySpec{.key = CITY_BLOCK_WIDTH_PROPERTY,
                                                     .kind = EntityPropertyKind::Integer,
                                                     .source = EntityChoiceSource::Fixed,
                                                     .fixedChoices = {},
                                                     .required = true,
                                                     .defaultValue = std::int64_t{1}},
                                  EntityPropertySpec{.key = CITY_BLOCK_HEIGHT_PROPERTY,
                                                     .kind = EntityPropertyKind::Integer,
                                                     .source = EntityChoiceSource::Fixed,
                                                     .fixedChoices = {},
                                                     .required = true,
                                                     .defaultValue = std::int64_t{1}}}},
        // Entree d'arene (LOT-50).
        EntityKind{.type = ARENA_ENTRY_ENTITY_TYPE,
                   .properties = {EntityPropertySpec{.key = ARENA_SIDE_PROPERTY,
                                                     .kind = EntityPropertyKind::Choice,
                                                     .source = EntityChoiceSource::Fixed,
                                                     .fixedChoices = {"allies", "enemies"},
                                                     .required = true,
                                                     .defaultValue = std::string{"allies"}},
                                  EntityPropertySpec{.key = ARENA_RANK_PROPERTY,
                                                     .kind = EntityPropertyKind::Integer,
                                                     .source = EntityChoiceSource::Fixed,
                                                     .fixedChoices = {},
                                                     .required = true,
                                                     .defaultValue = std::int64_t{1}}}},
    };
    return familles;
}

const EntityKind* findEntityKind(std::string_view type) {
    const auto found = std::ranges::find(knownEntityKinds(), type, &EntityKind::type);
    return found != knownEntityKinds().end() ? &*found : nullptr;
}

MapEntity makeEntity(const EntityKind& kind, GridPosition position) {
    MapEntity entity{.type = std::string{kind.type}, .position = position, .properties = {}};
    for (const EntityPropertySpec& spec : kind.properties) {
        entity.properties.emplace(std::string{spec.key}, spec.defaultValue);
    }
    return entity;
}

std::set<std::string, std::less<>> arrivalPointNames(const std::vector<MapEntity>& entities) {
    std::set<std::string, std::less<>> names;
    for (const MapEntity& entity : entities) {
        if (entity.type != SPAWN_POINT_ENTITY_TYPE) {
            continue;
        }
        if (std::string name = textOf(entity.properties, SPAWN_POINT_NAME_PROPERTY);
            !name.empty()) {
            names.insert(std::move(name));
        }
    }
    return names;
}

namespace {

// Defaut d'une propriete de choix non vide : la valeur n'est pas dans la liste que la source
// designe. Rien pour une valeur admise.
[[nodiscard]] std::optional<EntityIssueCode> choiceIssue(const MapEntity& entity,
                                                         const EntityPropertySpec& spec,
                                                         const std::string& text,
                                                         const EntityReferenceContext& context) {
    switch (spec.source) {
        case EntityChoiceSource::Fixed:
            if (std::ranges::find(spec.fixedChoices, std::string_view{text}) ==
                spec.fixedChoices.end()) {
                return EntityIssueCode::InvalidChoice;
            }
            break;
        case EntityChoiceSource::Dialogues:
            if (!context.dialogues.contains(text)) {
                return EntityIssueCode::UnknownDialogue;
            }
            break;
        case EntityChoiceSource::Encounters:
            if (!context.encounters.contains(text)) {
                return EntityIssueCode::UnknownEncounter;
            }
            break;
        case EntityChoiceSource::Maps:
            if (!context.arrivalPointsByMap.contains(text)) {
                return EntityIssueCode::UnknownTargetMap;
            }
            break;
        case EntityChoiceSource::ArrivalPoints: {
            // Un point d'arrivee ne se juge que dans une carte connue : une carte inconnue
            // est deja signalee, et la signaler deux fois n'apprendrait rien.
            const auto target = context.arrivalPointsByMap.find(
                textOf(entity.properties, PORTAL_TARGET_MAP_PROPERTY));
            if (target != context.arrivalPointsByMap.end() && !target->second.contains(text)) {
                return EntityIssueCode::UnknownArrivalPoint;
            }
            break;
        }
    }
    return std::nullopt;
}

// Defaut d'une propriete de l'entite au regard de sa specification, avec la valeur a citer.
// Rien pour une propriete conforme, ou facultative et absente.
[[nodiscard]] std::optional<std::pair<EntityIssueCode, std::string>> propertyIssue(
    const MapEntity& entity, const EntityPropertySpec& spec,
    const EntityReferenceContext& context) {
    const auto found = entity.properties.find(std::string{spec.key});
    if (found == entity.properties.end()) {
        if (spec.required) {
            return std::pair{EntityIssueCode::MissingProperty, std::string{}};
        }
        return std::nullopt;
    }
    if (!hasExpectedType(found->second, spec.kind)) {
        return std::pair{EntityIssueCode::WrongValueType, std::string{}};
    }
    const auto* const text = std::get_if<std::string>(&found->second);
    if (text == nullptr) {
        return std::nullopt;  // entier ou booleen du bon type : rien d'autre a verifier.
    }
    if (text->empty()) {
        if (spec.required) {
            return std::pair{EntityIssueCode::MissingProperty, std::string{}};
        }
        return std::nullopt;
    }
    if (spec.kind != EntityPropertyKind::Choice) {
        return std::nullopt;
    }
    if (const std::optional<EntityIssueCode> code = choiceIssue(entity, spec, *text, context)) {
        return std::pair{*code, *text};
    }
    return std::nullopt;
}

}  // namespace

std::vector<EntityIssue> validateMapEntities(const std::vector<MapEntity>& entities,
                                             const EntityReferenceContext& context) {
    std::vector<EntityIssue> issues;
    const auto report = [&issues](std::size_t index, EntityIssueCode code, std::string_view key,
                                  std::string value) {
        issues.push_back(EntityIssue{.entityIndex = index,
                                     .code = code,
                                     .key = std::string{key},
                                     .value = std::move(value)});
    };

    // Noms de points d'arrivee deja vus : le SECOND porteur d'un nom est signale, jamais le
    // premier.
    std::set<std::string, std::less<>> seenArrivals;

    for (std::size_t index = 0; index < entities.size(); ++index) {
        const MapEntity& entity = entities[index];
        const EntityKind* const kind = findEntityKind(entity.type);
        if (kind == nullptr) {
            report(index, EntityIssueCode::UnknownType, {}, entity.type);
            continue;
        }

        for (const EntityPropertySpec& spec : kind->properties) {
            if (auto issue = propertyIssue(entity, spec, context)) {
                report(index, issue->first, spec.key, std::move(issue->second));
            }
        }

        if (entity.type == SPAWN_POINT_ENTITY_TYPE) {
            const std::string name = textOf(entity.properties, SPAWN_POINT_NAME_PROPERTY);
            if (!name.empty() && !seenArrivals.insert(name).second) {
                report(index, EntityIssueCode::DuplicateArrivalPoint, SPAWN_POINT_NAME_PROPERTY,
                       name);
            }
        }
    }
    return issues;
}

}  // namespace core
