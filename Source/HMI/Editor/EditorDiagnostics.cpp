// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "HMI/Editor/EditorDiagnostics.h"

namespace hmi {

const char* entityIssueKey(core::EntityIssueCode code) noexcept {
    switch (code) {
        case core::EntityIssueCode::UnknownType:
            return "diagnostic.unknown_type";
        case core::EntityIssueCode::MissingProperty:
            return "diagnostic.missing_property";
        case core::EntityIssueCode::WrongValueType:
            return "diagnostic.wrong_value_type";
        case core::EntityIssueCode::InvalidChoice:
            return "diagnostic.invalid_choice";
        case core::EntityIssueCode::UnknownDialogue:
            return "diagnostic.unknown_dialogue";
        case core::EntityIssueCode::UnknownEncounter:
            return "diagnostic.unknown_encounter";
        case core::EntityIssueCode::UnknownTargetMap:
            return "diagnostic.unknown_target_map";
        case core::EntityIssueCode::UnknownArrivalPoint:
            return "diagnostic.unknown_arrival_point";
        case core::EntityIssueCode::DuplicateArrivalPoint:
            return "diagnostic.duplicate_arrival_point";
    }
    return "diagnostic.unknown_type";
}

const char* tacticalIssueKey(core::TacticalIssueCode code) noexcept {
    switch (code) {
        case core::TacticalIssueCode::CombatantOutOfBounds:
            return "diagnostic.terrain_out_of_bounds";
        case core::TacticalIssueCode::CombatantObstructed:
            return "diagnostic.terrain_obstructed";
        case core::TacticalIssueCode::CombatantsOverlap:
            return "diagnostic.terrain_overlap";
        case core::TacticalIssueCode::AreaTooNarrow:
            return "diagnostic.terrain_too_narrow";
    }
    return "diagnostic.terrain_too_narrow";
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
                                         .key = entityIssueKey(issue.code),
                                         .args = {entity.type, issue.key, issue.value}});
    }
    for (const core::EncounterTerrain& terrain : terrains) {
        for (const core::TacticalIssue& issue : terrain.issues) {
            EditorDiagnostic line{.kind = EditorDiagnosticKind::Terrain,
                                  .entityIndex = terrain.entityIndex,
                                  .cell = issue.cell,
                                  .key = tacticalIssueKey(issue.code),
                                  .args = {terrain.encounterId, issue.creatureId}};
            if (issue.code == core::TacticalIssueCode::AreaTooNarrow) {
                line.args = {terrain.encounterId, std::to_string(terrain.area.size()),
                             std::to_string(terrain.requiredCells)};
            }
            lines.push_back(std::move(line));
        }
    }
    return lines;
}

}  // namespace hmi
