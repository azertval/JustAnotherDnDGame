// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "HMI/Editor/PixelOperations.h"

#include <array>
#include <cstdlib>
#include <utility>
#include <vector>

namespace hmi {

namespace {

// Index a plat d'un pixel (x, y) dans un tampon de largeur width -- appelant deja borne-verifie.
std::size_t pixelIndex(int width, int x, int y) noexcept {
    return (static_cast<std::size_t>(y) * static_cast<std::size_t>(width)) +
           static_cast<std::size_t>(x);
}

// Etend une region pour qu'elle couvre aussi (x, y).
PixelRegion expand(PixelRegion region, int x, int y) noexcept {
    if (region.empty()) {
        return PixelRegion{.minX = x, .minY = y, .maxX = x, .maxY = y};
    }
    return PixelRegion{
        .minX = region.minX < x ? region.minX : x,
        .minY = region.minY < y ? region.minY : y,
        .maxX = region.maxX > x ? region.maxX : x,
        .maxY = region.maxY > y ? region.maxY : y,
    };
}

// Parcourt un segment entre deux positions (Bresenham entier, aucun pixel saute meme sur une
// forte oblique) en appelant perPixel(x, y) -> region touchee a chaque etape, et renvoie
// l'enveloppe des regions touchees. Partagee par drawLine (pose une couleur) et eraseLine (efface)
// pour que les deux gestes de glisser restent alignes sur le meme trace, sans dupliquer le pas.
template <typename PerPixel>
PixelRegion walkLine(int x0, int y0, int x1, int y1, const PerPixel& perPixel) {
    const int dx = std::abs(x1 - x0);
    const int sx = x0 < x1 ? 1 : -1;
    const int dy = -std::abs(y1 - y0);
    const int sy = y0 < y1 ? 1 : -1;
    int error = dx + dy;

    PixelRegion region;
    int x = x0;
    int y = y0;
    for (;;) {
        if (!perPixel(x, y).empty()) {
            region = expand(region, x, y);
        }
        if (x == x1 && y == y1) {
            break;
        }
        const int doubledError = 2 * error;
        if (doubledError >= dy) {
            error += dy;
            x += sx;
        }
        if (doubledError <= dx) {
            error += dx;
            y += sy;
        }
    }
    return region;
}

}  // namespace

PixelRegion setPixel(DecodedImage& image, int x, int y, std::uint32_t color) {
    if (!pixelInBounds(image.width, image.height, x, y)) {
        return {};
    }
    image.pixels[pixelIndex(image.width, x, y)] = color;
    return PixelRegion{.minX = x, .minY = y, .maxX = x, .maxY = y};
}

PixelRegion erasePixel(DecodedImage& image, int x, int y) {
    if (!pixelInBounds(image.width, image.height, x, y)) {
        return {};
    }
    // Alpha nul, RVB conserve : re-colorer un pixel efface ne fait jamais resurgir une teinte
    // fantome, et effacer par-dessus une gomme reste sans effet visuel.
    image.pixels[pixelIndex(image.width, x, y)] &= 0x00FFFFFFU;
    return PixelRegion{.minX = x, .minY = y, .maxX = x, .maxY = y};
}

PixelRegion drawLine(DecodedImage& image, int x0, int y0, int x1, int y1, std::uint32_t color) {
    return walkLine(x0, y0, x1, y1,
                    [&image, color](int x, int y) { return setPixel(image, x, y, color); });
}

PixelRegion eraseLine(DecodedImage& image, int x0, int y0, int x1, int y1) {
    return walkLine(x0, y0, x1, y1, [&image](int x, int y) { return erasePixel(image, x, y); });
}

PixelRegion floodFill(DecodedImage& image, int x, int y, std::uint32_t color) {
    if (!pixelInBounds(image.width, image.height, x, y)) {
        return {};
    }
    const std::size_t startIndex = pixelIndex(image.width, x, y);
    const std::uint32_t startColor = image.pixels[startIndex];
    if (startColor == color) {
        // Deja la couleur cible : aucun changement, et surtout aucune raison de parcourir
        // l'image -- une image entierement uniforme ne doit jamais boucler.
        return {};
    }

    PixelRegion region{.minX = x, .minY = y, .maxX = x, .maxY = y};
    image.pixels[startIndex] = color;

    // Pile explicite (jamais de recursion : une grande zone deborderait la pile d'appel).
    std::vector<std::pair<int, int>> pending;
    pending.emplace_back(x, y);
    while (!pending.empty()) {
        const auto [currentX, currentY] = pending.back();
        pending.pop_back();

        static constexpr std::array<std::array<int, 2>, 4> NEIGHBOR_OFFSETS = {
            {{1, 0}, {-1, 0}, {0, 1}, {0, -1}}};
        for (const auto& offset : NEIGHBOR_OFFSETS) {
            const int neighborX = currentX + offset[0];
            const int neighborY = currentY + offset[1];
            if (!pixelInBounds(image.width, image.height, neighborX, neighborY)) {
                continue;
            }
            const std::size_t neighborIndex = pixelIndex(image.width, neighborX, neighborY);
            if (image.pixels[neighborIndex] != startColor) {
                continue;  // deja repeint (marquage a la mise en pile) ou hors zone connexe.
            }
            image.pixels[neighborIndex] = color;
            region = expand(region, neighborX, neighborY);
            pending.emplace_back(neighborX, neighborY);
        }
    }
    return region;
}

std::optional<std::uint32_t> pickColor(const DecodedImage& image, int x, int y) noexcept {
    if (!pixelInBounds(image.width, image.height, x, y)) {
        return std::nullopt;
    }
    return image.pixels[pixelIndex(image.width, x, y)];
}

std::vector<std::uint32_t> readRegion(const DecodedImage& image, const PixelRegion& region) {
    if (region.empty()) {
        return {};
    }
    std::vector<std::uint32_t> data;
    data.reserve(static_cast<std::size_t>(region.width()) *
                 static_cast<std::size_t>(region.height()));
    for (int yy = region.minY; yy <= region.maxY; ++yy) {
        for (int xx = region.minX; xx <= region.maxX; ++xx) {
            data.push_back(image.pixels[pixelIndex(image.width, xx, yy)]);
        }
    }
    return data;
}

void writeRegion(DecodedImage& image, const PixelRegion& region,
                 const std::vector<std::uint32_t>& data) {
    if (region.empty()) {
        return;
    }
    std::size_t index = 0;
    for (int yy = region.minY; yy <= region.maxY; ++yy) {
        for (int xx = region.minX; xx <= region.maxX; ++xx) {
            image.pixels[pixelIndex(image.width, xx, yy)] = data[index];
            ++index;
        }
    }
}

namespace {

// Efface une region entiere (alpha nul, RVB conserve -- meme contrat que erasePixel), pixel par
// pixel : reutilise pour la "zone quittee" d'un deplacement ou d'une rotation.
PixelRegion eraseRegion(DecodedImage& image, const PixelRegion& region) {
    PixelRegion touched;
    for (int y = region.minY; y <= region.maxY; ++y) {
        for (int x = region.minX; x <= region.maxX; ++x) {
            touched = unionPixelRegion(touched, erasePixel(image, x, y));
        }
    }
    return touched;
}

// Pivote un presse-papiers d'un quart de tour horaire (largeur/hauteur echangees). Fonction pure
// sur un PixelClipboard, partagee par rotateClockwise et rotateCounterClockwise (trois rotations
// horaires) pour n'ecrire la logique de rotation qu'une seule fois.
PixelClipboard rotateClipboardClockwise(const PixelClipboard& source) {
    PixelClipboard rotated;
    rotated.width = source.height;
    rotated.height = source.width;
    rotated.pixels.resize(source.pixels.size());
    for (int ny = 0; ny < rotated.height; ++ny) {
        for (int nx = 0; nx < rotated.width; ++nx) {
            const int sx = ny;
            const int sy = source.height - 1 - nx;
            rotated
                .pixels[(static_cast<std::size_t>(ny) * static_cast<std::size_t>(rotated.width)) +
                        static_cast<std::size_t>(nx)] =
                source.pixels[(static_cast<std::size_t>(sy) *
                               static_cast<std::size_t>(source.width)) +
                              static_cast<std::size_t>(sx)];
        }
    }
    return rotated;
}

}  // namespace

PixelRegion flipHorizontal(DecodedImage& image, const PixelRegion& region) {
    if (region.empty()) {
        return {};
    }
    for (int y = region.minY; y <= region.maxY; ++y) {
        int left = region.minX;
        int right = region.maxX;
        while (left < right) {
            const std::uint32_t leftColor = image.pixels[pixelIndex(image.width, left, y)];
            const std::uint32_t rightColor = image.pixels[pixelIndex(image.width, right, y)];
            image.pixels[pixelIndex(image.width, left, y)] = rightColor;
            image.pixels[pixelIndex(image.width, right, y)] = leftColor;
            ++left;
            --right;
        }
    }
    return region;
}

PixelRegion flipVertical(DecodedImage& image, const PixelRegion& region) {
    if (region.empty()) {
        return {};
    }
    for (int x = region.minX; x <= region.maxX; ++x) {
        int top = region.minY;
        int bottom = region.maxY;
        while (top < bottom) {
            const std::uint32_t topColor = image.pixels[pixelIndex(image.width, x, top)];
            const std::uint32_t bottomColor = image.pixels[pixelIndex(image.width, x, bottom)];
            image.pixels[pixelIndex(image.width, x, top)] = bottomColor;
            image.pixels[pixelIndex(image.width, x, bottom)] = topColor;
            ++top;
            --bottom;
        }
    }
    return region;
}

PixelRegion rotateClockwise(DecodedImage& image, const PixelRegion& region) {
    if (region.empty()) {
        return {};
    }
    const PixelClipboard rotated = rotateClipboardClockwise(copyRegion(image, region));
    PixelRegion touched = eraseRegion(image, region);
    touched = unionPixelRegion(touched, pasteClipboard(image, rotated, region.minX, region.minY));
    return touched;
}

PixelRegion rotateCounterClockwise(DecodedImage& image, const PixelRegion& region) {
    if (region.empty()) {
        return {};
    }
    // Trois rotations horaires = une antihoraire : evite de dupliquer la logique de rotation.
    PixelClipboard rotated = copyRegion(image, region);
    for (int i = 0; i < 3; ++i) {
        rotated = rotateClipboardClockwise(rotated);
    }
    PixelRegion touched = eraseRegion(image, region);
    touched = unionPixelRegion(touched, pasteClipboard(image, rotated, region.minX, region.minY));
    return touched;
}

PixelRegion moveRegion(DecodedImage& image, const PixelRegion& region, int dx, int dy) {
    if (region.empty()) {
        return {};
    }
    const PixelClipboard content = copyRegion(image, region);
    PixelRegion touched = eraseRegion(image, region);
    touched = unionPixelRegion(touched,
                               pasteClipboard(image, content, region.minX + dx, region.minY + dy));
    return touched;
}

PixelClipboard copyRegion(const DecodedImage& image, const PixelRegion& region) {
    if (region.empty()) {
        return {};
    }
    PixelClipboard clip;
    clip.width = region.width();
    clip.height = region.height();
    clip.pixels = readRegion(image, region);
    return clip;
}

PixelRegion pasteClipboard(DecodedImage& image, const PixelClipboard& clipboard, int x, int y) {
    if (clipboard.empty()) {
        return {};
    }
    PixelRegion touched;
    for (int row = 0; row < clipboard.height; ++row) {
        for (int column = 0; column < clipboard.width; ++column) {
            const std::uint32_t pixel =
                clipboard.pixels[(static_cast<std::size_t>(row) *
                                  static_cast<std::size_t>(clipboard.width)) +
                                 static_cast<std::size_t>(column)];
            touched = unionPixelRegion(touched, setPixel(image, x + column, y + row, pixel));
        }
    }
    return touched;
}

}  // namespace hmi
