// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "HMI/Editor/TextureAssignGesture.h"

namespace hmi {

TextureAssignDecision resolveTextureAssignClick(core::GridPosition clickedCell,
                                                core::TileType clickedTileType,
                                                const std::optional<std::string>& existingOverride,
                                                const std::optional<std::string>& selectedAsset,
                                                bool rightClick) noexcept {
    if (clickedTileType == core::TileType::Empty) {
        return TextureAssignDecision{
            .action = TextureAssignAction::Ignore, .cell = {}, .assetName = {}};
    }

    if (rightClick) {
        if (!existingOverride) {
            return TextureAssignDecision{
                .action = TextureAssignAction::Ignore, .cell = {}, .assetName = {}};
        }
        return TextureAssignDecision{
            .action = TextureAssignAction::Remove, .cell = clickedCell, .assetName = {}};
    }

    if (!selectedAsset) {
        return TextureAssignDecision{
            .action = TextureAssignAction::Ignore, .cell = {}, .assetName = {}};
    }
    if (existingOverride && *existingOverride == *selectedAsset) {
        // Recliquer le meme asset deja assigne retire l'override (bascule) : evite d'exiger un
        // clic droit pour le cas le plus courant (defaire son propre geste).
        return TextureAssignDecision{
            .action = TextureAssignAction::Remove, .cell = clickedCell, .assetName = {}};
    }
    return TextureAssignDecision{
        .action = TextureAssignAction::Assign, .cell = clickedCell, .assetName = *selectedAsset};
}

}  // namespace hmi
