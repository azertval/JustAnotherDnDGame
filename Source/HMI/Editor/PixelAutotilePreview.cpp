// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "HMI/Editor/PixelAutotilePreview.h"

namespace hmi {

bool isBitmask16Candidate(const DecodedImage& image, int tileSize) noexcept {
    if (tileSize <= 0) {
        return false;
    }
    return image.width == tileSize * AUTOTILE_SHEET_SIDE &&
           image.height == tileSize * AUTOTILE_SHEET_SIDE;
}

std::optional<AutotileCell> bitmaskCellAtPixel(const DecodedImage& image, int tileSize, int x,
                                               int y) noexcept {
    if (!isBitmask16Candidate(image, tileSize)) {
        return std::nullopt;
    }
    if (!pixelInBounds(image.width, image.height, x, y)) {
        return std::nullopt;
    }
    return AutotileCell{.column = x / tileSize, .row = y / tileSize};
}

DecodedImage buildAutotileAssemblyPreview(const DecodedImage& sheet, int tileSize) {
    DecodedImage preview;
    preview.width = tileSize * 3;
    preview.height = tileSize * 3;
    preview.pixels.assign(
        static_cast<std::size_t>(preview.width) * static_cast<std::size_t>(preview.height), 0U);

    const std::array<std::uint8_t, 9> masks = autotileAssemblyMasks();
    for (std::size_t i = 0; i < masks.size(); ++i) {
        const int destColumn = static_cast<int>(i % 3);
        const int destRow = static_cast<int>(i / 3);
        const AutotileCell sourceCell = autotileCell(masks[i]);

        const PixelRegion sourceRegion{.minX = sourceCell.column * tileSize,
                                       .minY = sourceCell.row * tileSize,
                                       .maxX = (sourceCell.column * tileSize) + tileSize - 1,
                                       .maxY = (sourceCell.row * tileSize) + tileSize - 1};
        if (sourceRegion.maxX >= sheet.width || sourceRegion.maxY >= sheet.height) {
            continue;  // planche plus petite que prevu : case laissee transparente, jamais lue.
        }

        const PixelRegion destRegion{.minX = destColumn * tileSize,
                                     .minY = destRow * tileSize,
                                     .maxX = (destColumn * tileSize) + tileSize - 1,
                                     .maxY = (destRow * tileSize) + tileSize - 1};
        writeRegion(preview, destRegion, readRegion(sheet, sourceRegion));
    }
    return preview;
}

}  // namespace hmi
