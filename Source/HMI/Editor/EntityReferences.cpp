// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "HMI/Editor/EntityReferences.h"

#include <algorithm>
#include <system_error>
#include <variant>

#include "Core/Rpg/Dialogue.h"

namespace hmi {

namespace {

// Vrai si @p directory existe : les chargeurs du Core tolerent un dossier absent, mais certains le
// journalisent comme une erreur -- l'editeur deploye sans catalogue n'en est pas une.
[[nodiscard]] bool isDirectory(const std::filesystem::path& directory) {
    std::error_code error;
    return std::filesystem::is_directory(directory, error);
}

}  // namespace

EditorReferences loadEditorReferences(const std::filesystem::path& root) {
    EditorReferences references;
    if (const std::filesystem::path dialogues = root / "World" / "dialogues";
        isDirectory(dialogues)) {
        for (const core::DialogueGraph& graph : core::loadDialogues(dialogues).dialogues) {
            references.dialogues.push_back(graph.id);
        }
        std::ranges::sort(references.dialogues);
    }
    if (const std::filesystem::path encounters = root / "Rpg" / "encounters";
        isDirectory(encounters)) {
        references.encounters = core::loadEncounters(encounters);
    }
    if (const std::filesystem::path creatures = root / "Rpg" / "creatures";
        isDirectory(creatures)) {
        references.bestiary = core::loadBestiary(creatures);
    }
    references.world = core::loadWorldGraph(root / "Levels");
    return references;
}

core::EntityReferenceContext referenceContext(const EditorReferences& references,
                                              std::string_view editedMapId,
                                              const std::vector<core::MapEntity>& editedEntities) {
    core::EntityReferenceContext context;
    context.dialogues.insert(references.dialogues.begin(), references.dialogues.end());
    for (const core::Encounter& encounter : references.encounters.encounters) {
        context.encounters.insert(encounter.id);
    }
    for (const core::WorldMapNode& map : references.world.maps) {
        context.arrivalPointsByMap[map.mapId].insert(map.arrivalPoints.begin(),
                                                     map.arrivalPoints.end());
    }
    if (!editedMapId.empty()) {
        context.arrivalPointsByMap[std::string{editedMapId}] =
            core::arrivalPointNames(editedEntities);
    }
    return context;
}

std::vector<std::string> entityChoices(const core::EntityPropertySpec& spec,
                                       const core::MapEntity& entity,
                                       const core::EntityReferenceContext& context) {
    std::vector<std::string> choices;
    if (spec.kind != core::EntityPropertyKind::Choice) {
        return choices;
    }
    switch (spec.source) {
        case core::EntityChoiceSource::Fixed:
            for (const std::string_view choice : spec.fixedChoices) {
                choices.emplace_back(choice);
            }
            break;
        case core::EntityChoiceSource::Dialogues:
            choices.assign(context.dialogues.begin(), context.dialogues.end());
            break;
        case core::EntityChoiceSource::Encounters:
            choices.assign(context.encounters.begin(), context.encounters.end());
            break;
        case core::EntityChoiceSource::Maps:
            for (const auto& [mapId, points] : context.arrivalPointsByMap) {
                static_cast<void>(points);
                choices.push_back(mapId);
            }
            break;
        case core::EntityChoiceSource::ArrivalPoints: {
            const auto target =
                entity.properties.find(std::string{core::PORTAL_TARGET_MAP_PROPERTY});
            if (target == entity.properties.end()) {
                break;
            }
            const auto* const mapId = std::get_if<std::string>(&target->second);
            if (mapId == nullptr) {
                break;
            }
            if (const auto points = context.arrivalPointsByMap.find(*mapId);
                points != context.arrivalPointsByMap.end()) {
                choices.assign(points->second.begin(), points->second.end());
            }
            break;
        }
    }
    std::ranges::sort(choices);
    return choices;
}

}  // namespace hmi
