// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "HMI/Graphics/TextureCache.h"

#include <optional>

#include "Core/Resources/AssetMarker.h"
#include "HMI/Graphics/EntityMarkers.h"
#include "HMI/Graphics/GraphicsLog.h"

namespace hmi {

TextureCache::TextureCache(const RhiContext& context) : _context(context) {}

// Texture du marqueur genere d'une cle d'asset, creee une seule fois (voir en-tete).
const LoadedTexture* TextureCache::markerTexture(const std::string& key) {
    return _markerEntries.getOrLoad(key, [this, &key]() -> std::optional<LoadedTexture> {
        const core::MarkerImage image =
            core::assetMarker(key, ENTITY_MARKER_SIZE_PIXELS, ENTITY_MARKER_SIZE_PIXELS);
        if (image.isEmpty()) {
            // Cle malformee : core::assetMarker la refuse plutot que de lui inventer un marqueur.
            GRAPHICS_LOG_WARNING("Marqueur refuse : cle d'asset malformee '" + key + "'.");
            return std::nullopt;
        }
        std::optional<LoadedTexture> texture =
            createTexture(_context, image.width, image.height, markerPixelsRgba8(image));
        if (!texture) {
            GRAPHICS_LOG_WARNING("Creation GPU du marqueur " + key + " impossible.");
            return std::nullopt;
        }
        return texture;
    });
}

void TextureCache::invalidateAll() {
    _markerEntries.invalidateAll();
}

}  // namespace hmi
