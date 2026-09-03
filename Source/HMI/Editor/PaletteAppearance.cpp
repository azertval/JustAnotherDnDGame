// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Editor/PaletteAppearance.h"

#include <optional>

#include "HMI/Graphics/MissingTexture.h"
#include "HMI/Graphics/TileSilhouette.h"
#include "HMI/Graphics/TextureAtlas.h"
#include "HMI/Graphics/TileAutotile.h"

namespace hmi {

namespace {

// Damier en entier : repli commun, identique a celui du canevas.
[[nodiscard]] PaletteThumbnail missingThumbnail() {
    PaletteThumbnail thumbnail;
    thumbnail.source = PaletteThumbnailSource::MissingTexture;
    thumbnail.region = core::AtlasRegion{
        .x = 0, .y = 0, .width = MISSING_TEXTURE_SIZE, .height = MISSING_TEXTURE_SIZE};
    return thumbnail;
}

}  // namespace

PaletteThumbnail paletteThumbnail(RenderMode mode, core::TileType type,
                                  const core::AtlasRegion& physicalRegion,
                                  const SkinCatalog* catalog, std::string_view setName) {
    if (mode == RenderMode::Physique) {
        // Mode de reference : la couleur plate, exactement comme le canevas.
        PaletteThumbnail thumbnail;
        thumbnail.source = PaletteThumbnailSource::Atlas;
        thumbnail.region = physicalRegion;
        return thumbnail;
    }

    if (catalog == nullptr) {
        return missingThumbnail();
    }
    const std::optional<SkinEntry> entry = catalog->resolve(setName, type);
    if (!entry.has_value()) {
        // La palette doit SIGNALER ce qui reste a habiller, pas le masquer : un type non skinne
        // s'y voit en damier, comme dans le niveau.
        return missingThumbnail();
    }

    constexpr int SIZE = TextureAtlas::TILE_SIZE;
    PaletteThumbnail thumbnail;
    thumbnail.source = PaletteThumbnailSource::Skin;
    thumbnail.asset = entry->asset;
    thumbnail.masked = hasSilhouette(type);
    if (entry->mode == SkinMode::Bitmask16) {
        // Case representative (interieur plein) : la planche entiere serait illisible en vignette,
        // et la case zero (tuile isolee) serait un cas particulier peu representatif du motif.
        const AutotileCell cell = autotileRepresentativeCell();
        thumbnail.region = core::AtlasRegion{
            .x = cell.column * SIZE, .y = cell.row * SIZE, .width = SIZE, .height = SIZE};
    } else {
        thumbnail.region = core::AtlasRegion{.x = 0, .y = 0, .width = SIZE, .height = SIZE};
    }
    return thumbnail;
}

}  // namespace hmi
