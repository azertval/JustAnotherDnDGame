// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "HMI/Editor/PlaneReference.h"

#include <algorithm>
#include <cmath>

#include "Core/Levels/Plane.h"
#include "Core/Levels/TileMap.h"
#include "HMI/Graphics/ProceduralAtlas.h"
#include "HMI/Graphics/TextureAtlas.h"
#include "HMI/Graphics/TileVisuals.h"

namespace hmi {

namespace {

// Extrait les composantes d'un pixel RGBA (R en poids faible, alpha NON premultiplie).
struct Channels {
    float r;
    float g;
    float b;
    float a;
};

Channels unpack(std::uint32_t pixel) noexcept {
    return Channels{.r = static_cast<float>(pixel & 0xFFU) / 255.0F,
                    .g = static_cast<float>((pixel >> 8) & 0xFFU) / 255.0F,
                    .b = static_cast<float>((pixel >> 16) & 0xFFU) / 255.0F,
                    .a = static_cast<float>((pixel >> 24) & 0xFFU) / 255.0F};
}

std::uint32_t pack(const Channels& channels) noexcept {
    const auto quantize = [](float value) {
        return static_cast<std::uint32_t>(std::lround(std::clamp(value, 0.0F, 1.0F) * 255.0F));
    };
    return quantize(channels.r) | (quantize(channels.g) << 8) | (quantize(channels.b) << 16) |
           (quantize(channels.a) << 24);
}

// Applique une opacite globale a l'alpha d'un pixel, sans toucher a sa couleur (alpha non
// premultiplie : c'est exactement la meme convention que la teinte du pipeline de rendu).
std::uint32_t withOpacity(std::uint32_t pixel, float opacity) noexcept {
    if (opacity >= 1.0F) {
        return pixel;
    }
    Channels channels = unpack(pixel);
    channels.a *= std::clamp(opacity, 0.0F, 1.0F);
    return pack(channels);
}

// Couleur plate du mode Physique pour un type de tuile : lue DANS l'atlas procedural, au centre de
// la region du type. Aucune palette n'est redeclaree ici -- deux jeux de couleurs pour la meme
// lecture divergeraient au premier ajustement.
std::uint32_t physiqueColor(const ProceduralAtlasImage& atlas, core::TileType type) {
    const core::AtlasRegion region = regionForTile(type);
    const int x = region.x + (region.width / 2);
    const int y = region.y + (region.height / 2);
    if (x < 0 || y < 0 || x >= atlas.width || y >= atlas.height) {
        return 0;  // hors atlas : transparent plutot qu'une couleur inventee.
    }
    return atlas.pixels[(static_cast<std::size_t>(y) * static_cast<std::size_t>(atlas.width)) +
                        static_cast<std::size_t>(x)];
}

}  // namespace

// Compose un pixel au-dessus d'un autre (alpha-over, alpha non premultiplie).
std::uint32_t alphaOver(std::uint32_t below, std::uint32_t above) noexcept {
    const Channels top = unpack(above);
    if (top.a >= 1.0F) {
        return above;  // opaque : rien du dessous ne subsiste, et aucun arrondi ne s'introduit.
    }
    if (top.a <= 0.0F) {
        return below;
    }
    const Channels bottom = unpack(below);
    const float alpha = top.a + (bottom.a * (1.0F - top.a));
    if (alpha <= 0.0F) {
        return 0;
    }
    // Formule alpha-over standard, ramenee en non premultiplie : les couleurs sont ponderees par
    // leur propre alpha puis redivisees par l'alpha resultant.
    const auto mix = [&](float topColor, float bottomColor) {
        return ((topColor * top.a) + (bottomColor * bottom.a * (1.0F - top.a))) / alpha;
    };
    return pack(Channels{.r = mix(top.r, bottom.r),
                         .g = mix(top.g, bottom.g),
                         .b = mix(top.b, bottom.b),
                         .a = alpha});
}

// Reechantillonne une image d'une densite a une autre, par ratio entier.
DecodedImage resamplePlane(const DecodedImage& image, int fromPixelsPerUnit, int toPixelsPerUnit) {
    if (fromPixelsPerUnit <= 0 || toPixelsPerUnit <= 0 || image.width <= 0 || image.height <= 0) {
        return DecodedImage{};
    }
    if (fromPixelsPerUnit == toPixelsPerUnit) {
        return image;
    }

    DecodedImage result;
    if (fromPixelsPerUnit > toPixelsPerUnit) {
        const int ratio = fromPixelsPerUnit / toPixelsPerUnit;
        if (ratio * toPixelsPerUnit != fromPixelsPerUnit) {
            return DecodedImage{};  // rapport non entier : refuse plutot que d'interpoler.
        }
        // Sous-echantillonnage : un pixel sur `ratio`, jamais une moyenne -- moyenner fabriquerait
        // des couleurs que l'artiste n'a pas posees.
        result.width = image.width / ratio;
        result.height = image.height / ratio;
        if (result.width <= 0 || result.height <= 0) {
            return DecodedImage{};
        }
        result.pixels.resize(static_cast<std::size_t>(result.width) *
                             static_cast<std::size_t>(result.height));
        for (int y = 0; y < result.height; ++y) {
            for (int x = 0; x < result.width; ++x) {
                const std::size_t source =
                    (static_cast<std::size_t>(y * ratio) * static_cast<std::size_t>(image.width)) +
                    static_cast<std::size_t>(x * ratio);
                result
                    .pixels[(static_cast<std::size_t>(y) * static_cast<std::size_t>(result.width)) +
                            static_cast<std::size_t>(x)] = image.pixels[source];
            }
        }
        return result;
    }

    const int ratio = toPixelsPerUnit / fromPixelsPerUnit;
    if (ratio * fromPixelsPerUnit != toPixelsPerUnit) {
        return DecodedImage{};
    }
    // Sur-echantillonnage : duplication pure (chaque pixel devient un carre `ratio` x `ratio`).
    result.width = image.width * ratio;
    result.height = image.height * ratio;
    result.pixels.resize(static_cast<std::size_t>(result.width) *
                         static_cast<std::size_t>(result.height));
    for (int y = 0; y < result.height; ++y) {
        for (int x = 0; x < result.width; ++x) {
            const std::size_t source =
                (static_cast<std::size_t>(y / ratio) * static_cast<std::size_t>(image.width)) +
                static_cast<std::size_t>(x / ratio);
            result.pixels[(static_cast<std::size_t>(y) * static_cast<std::size_t>(result.width)) +
                          static_cast<std::size_t>(x)] = image.pixels[source];
        }
    }
    return result;
}

// Construit la pelure d'oignon des tuiles : une couleur plate par type.
DecodedImage buildTileOnionSkin(const core::LevelDraft& draft, int pixelsPerUnit) {
    const core::TileMap& map = draft.tileMap();
    if (!core::isValidPlaneDensity(pixelsPerUnit) || map.width() <= 0 || map.height() <= 0) {
        return DecodedImage{};
    }

    DecodedImage result;
    result.width = map.width() * pixelsPerUnit;
    result.height = map.height() * pixelsPerUnit;
    result.pixels.assign(
        static_cast<std::size_t>(result.width) * static_cast<std::size_t>(result.height), 0U);

    const ProceduralAtlasImage atlas = buildProceduralAtlasImage();
    for (int row = 0; row < map.height(); ++row) {
        for (int column = 0; column < map.width(); ++column) {
            const core::TileType type = map.tile(column, row);
            if (type == core::TileType::Empty) {
                continue;  // case vide : transparente, on peint par-dessus.
            }
            const std::uint32_t color = physiqueColor(atlas, type);
            for (int y = 0; y < pixelsPerUnit; ++y) {
                const std::size_t line = (static_cast<std::size_t>((row * pixelsPerUnit) + y) *
                                          static_cast<std::size_t>(result.width)) +
                                         static_cast<std::size_t>(column * pixelsPerUnit);
                std::fill_n(result.pixels.begin() + static_cast<std::ptrdiff_t>(line),
                            pixelsPerUnit, color);
            }
        }
    }
    return result;
}

// Aplatit des plans en une seule image, par alpha-over dans l'ordre donne.
DecodedImage flattenPlanes(const std::vector<PlaneLayer>& layers, int pixelsPerUnit, int widthUnits,
                           int heightUnits) {
    if (pixelsPerUnit <= 0 || widthUnits <= 0 || heightUnits <= 0) {
        return DecodedImage{};
    }

    DecodedImage result;
    result.width = widthUnits * pixelsPerUnit;
    result.height = heightUnits * pixelsPerUnit;
    result.pixels.assign(
        static_cast<std::size_t>(result.width) * static_cast<std::size_t>(result.height), 0U);

    for (const PlaneLayer& layer : layers) {
        if (!layer.visible || layer.image == nullptr) {
            continue;  // masquee ou absente : ignoree, comme si elle n'existait pas.
        }
        const DecodedImage scaled = resamplePlane(*layer.image, layer.pixelsPerUnit, pixelsPerUnit);
        if (scaled.width <= 0 || scaled.height <= 0) {
            continue;  // densite incompatible : ignoree plutot que deformee.
        }
        const int columns = (std::min)(scaled.width, result.width);
        const int rows = (std::min)(scaled.height, result.height);
        for (int y = 0; y < rows; ++y) {
            for (int x = 0; x < columns; ++x) {
                const std::size_t target =
                    (static_cast<std::size_t>(y) * static_cast<std::size_t>(result.width)) +
                    static_cast<std::size_t>(x);
                const std::size_t source =
                    (static_cast<std::size_t>(y) * static_cast<std::size_t>(scaled.width)) +
                    static_cast<std::size_t>(x);
                result.pixels[target] = alphaOver(
                    result.pixels[target], withOpacity(scaled.pixels[source], layer.opacity));
            }
        }
    }
    return result;
}

}  // namespace hmi
