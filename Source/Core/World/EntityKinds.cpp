// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "Core/World/EntityKinds.h"

#include <algorithm>
#include <cstdint>
#include <variant>

#include "Core/Combat/Arena.h"
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
                                         /*required=*/false)}},
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
                                         /*required=*/true)}},
        EntityKind{.type = SPAWN_POINT_ENTITY_TYPE,
                   .properties = {EntityPropertySpec{.key = SPAWN_POINT_NAME_PROPERTY,
                                                     .kind = EntityPropertyKind::Text,
                                                     .source = EntityChoiceSource::Fixed,
                                                     .fixedChoices = {},
                                                     .required = true,
                                                     .defaultValue = std::string{}}}},
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
            const auto found = entity.properties.find(std::string{spec.key});
            if (found == entity.properties.end()) {
                if (spec.required) {
                    report(index, EntityIssueCode::MissingProperty, spec.key, {});
                }
                continue;
            }
            if (!hasExpectedType(found->second, spec.kind)) {
                report(index, EntityIssueCode::WrongValueType, spec.key, {});
                continue;
            }
            const auto* const text = std::get_if<std::string>(&found->second);
            if (text == nullptr) {
                continue;  // entier ou booleen du bon type : rien d'autre a verifier.
            }
            if (text->empty()) {
                if (spec.required) {
                    report(index, EntityIssueCode::MissingProperty, spec.key, {});
                }
                continue;
            }
            if (spec.kind != EntityPropertyKind::Choice) {
                continue;
            }
            switch (spec.source) {
                case EntityChoiceSource::Fixed:
                    if (std::ranges::find(spec.fixedChoices, std::string_view{*text}) ==
                        spec.fixedChoices.end()) {
                        report(index, EntityIssueCode::InvalidChoice, spec.key, *text);
                    }
                    break;
                case EntityChoiceSource::Dialogues:
                    if (!context.dialogues.contains(*text)) {
                        report(index, EntityIssueCode::UnknownDialogue, spec.key, *text);
                    }
                    break;
                case EntityChoiceSource::Encounters:
                    if (!context.encounters.contains(*text)) {
                        report(index, EntityIssueCode::UnknownEncounter, spec.key, *text);
                    }
                    break;
                case EntityChoiceSource::Maps:
                    if (!context.arrivalPointsByMap.contains(*text)) {
                        report(index, EntityIssueCode::UnknownTargetMap, spec.key, *text);
                    }
                    break;
                case EntityChoiceSource::ArrivalPoints: {
                    // Un point d'arrivee ne se juge que dans une carte connue : une carte inconnue
                    // est deja signalee, et la signaler deux fois n'apprendrait rien.
                    const auto target = context.arrivalPointsByMap.find(
                        textOf(entity.properties, PORTAL_TARGET_MAP_PROPERTY));
                    if (target != context.arrivalPointsByMap.end() &&
                        !target->second.contains(*text)) {
                        report(index, EntityIssueCode::UnknownArrivalPoint, spec.key, *text);
                    }
                    break;
                }
            }
        }

        if (entity.type == SPAWN_POINT_ENTITY_TYPE) {
            std::string name = textOf(entity.properties, SPAWN_POINT_NAME_PROPERTY);
            if (!name.empty() && !seenArrivals.insert(name).second) {
                report(index, EntityIssueCode::DuplicateArrivalPoint, SPAWN_POINT_NAME_PROPERTY,
                       std::move(name));
            }
        }
    }
    return issues;
}

}  // namespace core
