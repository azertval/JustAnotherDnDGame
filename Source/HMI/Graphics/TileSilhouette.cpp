// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "HMI/Graphics/TileSilhouette.h"

#include <algorithm>
#include <cstddef>

namespace hmi {

bool hasSilhouette(core::TileType type) noexcept {
    return std::ranges::any_of(SILHOUETTE_TILE_TYPES,
                               [type](core::TileType candidate) { return candidate == type; });
}

bool isInsideSilhouette(core::TileType type, int localX, int localY, int tileSize) noexcept {
    // Un type sans silhouette remplit toute la case : aucun pixel n'en est exclu. Comme
    // SILHOUETTE_TILE_TYPES est vide depuis le LOT-01 (voir l'en-tete), c'est aujourd'hui le seul
    // chemin atteignable ; le calcul de forme du LOT-08 se branchera ici, type par type, a cote de
    // ce cas general.
    (void)type;
    (void)localX;
    (void)localY;
    (void)tileSize;
    return true;
}

void applySilhouetteMask(core::TileType type, int width, int height,
                         std::vector<std::uint32_t>& pixels) {
    // Un type sans silhouette n'est jamais decoupe, et une image non carree ne l'est pas non plus
    // (un skin destine a un type carre ne doit pas etre ampute par accident).
    if (!hasSilhouette(type) || width != height || width <= 0) {
        return;
    }
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (!isInsideSilhouette(type, x, y, width)) {
                pixels[(static_cast<std::size_t>(y) * static_cast<std::size_t>(width)) +
                       static_cast<std::size_t>(x)] = 0;  // transparent
            }
        }
    }
}

}  // namespace hmi
