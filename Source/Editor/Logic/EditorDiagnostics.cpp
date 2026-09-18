// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "Editor/Logic/EditorDiagnostics.h"

namespace hmi {

namespace {

// Remplace `%1`, `%2`, `%3` de @p pattern par les arguments, dans l'ordre.
[[nodiscard]] std::string format(std::string pattern, const std::vector<std::string>& args) {
    for (std::size_t index = 0; index < args.size(); ++index) {
        const std::string marker = "%" + std::to_string(index + 1);
        for (std::size_t at = pattern.find(marker); at != std::string::npos;
             at = pattern.find(marker, at + args[index].size())) {
            pattern.replace(at, marker.size(), args[index]);
        }
    }
    return pattern;
}

}  // namespace

const char* entityIssueTemplate(core::EntityIssueCode code) noexcept {
    switch (code) {
        case core::EntityIssueCode::UnknownType:
            return "Entity kind \"%1\" is unknown to the editor.";
        case core::EntityIssueCode::MissingProperty:
            return "%1: property \"%2\" must be filled in.";
        case core::EntityIssueCode::WrongValueType:
            return "%1: property \"%2\" has the wrong value type.";
        case core::EntityIssueCode::InvalidChoice:
            return "%1: \"%3\" is not an allowed value for \"%2\".";
        case core::EntityIssueCode::UnknownDialogue:
            return "%1: dialogue \"%3\" does not exist, or was rejected when loading.";
        case core::EntityIssueCode::UnknownEncounter:
            return "%1: encounter \"%3\" does not exist.";
        case core::EntityIssueCode::UnknownTargetMap:
            return "%1: map \"%3\" does not exist.";
        case core::EntityIssueCode::UnknownArrivalPoint:
            return "%1: the target map has no arrival point \"%3\".";
        case core::EntityIssueCode::DuplicateArrivalPoint:
            return "Two arrival points are named \"%3\".";
    }
    return "Entity kind \"%1\" is unknown to the editor.";
}

const char* tacticalIssueTemplate(core::TacticalIssueCode code) noexcept {
    switch (code) {
        case core::TacticalIssueCode::CombatantOutOfBounds:
            return "Encounter \"%1\": \"%2\" would stand off the map.";
        case core::TacticalIssueCode::CombatantObstructed:
            return "Encounter \"%1\": \"%2\" would stand on an obstacle.";
        case core::TacticalIssueCode::CombatantsOverlap:
            return "Encounter \"%1\": \"%2\" would overlap another combatant.";
        case core::TacticalIssueCode::AreaTooNarrow:
            return "Encounter \"%1\": area too narrow to fight in (%2 free cells, %3 required).";
    }
    return "Encounter \"%1\": area too narrow to fight in (%2 free cells, %3 required).";
}

std::vector<EditorDiagnostic> editorDiagnostics(
    const std::vector<core::MapEntity>& entities, const std::vector<core::EntityIssue>& issues,
    const std::vector<core::EncounterTerrain>& terrains) {
    std::vector<EditorDiagnostic> lines;
    for (const core::EntityIssue& issue : issues) {
        if (issue.entityIndex >= entities.size()) {
            continue;  // rapport perime : l'entite a disparu entre la validation et l'affichage.
        }
        const core::MapEntity& entity = entities[issue.entityIndex];
        lines.push_back(EditorDiagnostic{.kind = EditorDiagnosticKind::Reference,
                                         .entityIndex = issue.entityIndex,
                                         .cell = entity.position,
                                         .message = format(entityIssueTemplate(issue.code),
                                                           {entity.type, issue.key, issue.value})});
    }
    for (const core::EncounterTerrain& terrain : terrains) {
        for (const core::TacticalIssue& issue : terrain.issues) {
            const std::vector<std::string> args =
                issue.code == core::TacticalIssueCode::AreaTooNarrow
                    ? std::vector<std::string>{terrain.encounterId,
                                               std::to_string(terrain.area.size()),
                                               std::to_string(terrain.requiredCells)}
                    : std::vector<std::string>{terrain.encounterId, issue.creatureId};
            lines.push_back(
                EditorDiagnostic{.kind = EditorDiagnosticKind::Terrain,
                                 .entityIndex = terrain.entityIndex,
                                 .cell = issue.cell,
                                 .message = format(tacticalIssueTemplate(issue.code), args)});
        }
    }
    return lines;
}

}  // namespace hmi
