// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "HMI/Editor/MechanismAnimationAssignments.h"

#include <optional>

#include "HMI/Editor/TileTaxonomy.h"
#include "HMI/Graphics/AnimationCatalog.h"
#include "HMI/Graphics/MechanismVisuals.h"

namespace hmi {

namespace {

// Libelle d'un type, repris de la taxonomie de la palette -- source unique, evite de dupliquer les
// chaines francaises deja portees par hmi::tileTaxonomy (meme raison que hmi::SkinRow::typeLabel).
std::string labelForType(core::TileType type) {
    for (const TileCategory& category : tileTaxonomy()) {
        for (const TileEntry& entry : category.tiles) {
            if (entry.type == type) {
                return entry.label;
            }
        }
        for (const TileSubgroup& subgroup : category.subgroups) {
            for (const TileEntry& entry : subgroup.tiles) {
                if (entry.type == type) {
                    return entry.label;
                }
            }
        }
    }
    return {};
}

}  // namespace

std::vector<MechanismAnimationRow> buildMechanismAnimationRows(
    const SkinCatalog& catalog, std::string_view setName,
    const std::filesystem::path& skinsDirectory) {
    std::vector<MechanismAnimationRow> rows;
    rows.reserve(std::size(MECHANISM_ANIMATION_TYPES));

    for (const core::TileType type : MECHANISM_ANIMATION_TYPES) {
        MechanismAnimationRow row;
        row.type = type;
        row.typeLabel = labelForType(type);
        if (const std::optional<SkinEntry> entry = catalog.resolve(setName, type)) {
            row.asset = entry->asset;
        }

        if (!row.asset.empty()) {
            const AnimationDescriptionResult result = AnimationCatalog::loadFromFile(
                skinsDirectory / AnimationCatalog::descriptorFileName(row.asset));
            for (const std::string& expectedClip : mechanismExpectedClips(type)) {
                const bool present =
                    result.ok() && result.description->clips.indexOf(expectedClip) >= 0;
                if (!present) {
                    row.missingClips.push_back(expectedClip);
                }
            }
        }

        rows.push_back(std::move(row));
    }
    return rows;
}

}  // namespace hmi
