// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "HMI/Editor/PixelCanvasGeometry.h"

#include <cmath>

namespace hmi {

PixelScreenRect imagePixelScreenRect(const PixelCanvasView& view, int x, int y) noexcept {
    // Echelle RATIONNELLE depuis le LOT-69 TACHE-07. INVARIANT : a denominateur 1 -- le defaut --
    // elle vaut l'echelle entiere au bit pres. C'est ce qui garantit que l'atelier pixel art reste
    // exempt de tout demi-pixel tant qu'aucun zoom fractionnaire n'est demande.
    const double scale = pixelCanvasScale(view);
    return PixelScreenRect{
        .x = static_cast<double>(x - view.panX) * scale,
        .y = static_cast<double>(y - view.panY) * scale,
        .width = scale,
        .height = scale,
    };
}

std::optional<std::pair<int, int>> screenToImagePixel(const PixelCanvasView& view, int imageWidth,
                                                      int imageHeight, double screenX,
                                                      double screenY) noexcept {
    const double scale = pixelCanvasScale(view);
    if (scale <= 0.0) {
        return std::nullopt;
    }
    // floor(), jamais une division/cast tronquant vers zero : une position ecran negative doit
    // produire une colonne/ligne strictement decroissante, pas revenir vers 0.
    const int column = view.panX + static_cast<int>(std::floor(screenX / scale));
    const int row = view.panY + static_cast<int>(std::floor(screenY / scale));
    if (column < 0 || row < 0 || column >= imageWidth || row >= imageHeight) {
        return std::nullopt;
    }
    return std::make_pair(column, row);
}

PixelCanvasRealSize pixelCanvasRealSize(int imageWidth, int imageHeight, int zoom,
                                        double scaleFactor) noexcept {
    return PixelCanvasRealSize{
        .width = thumbnailPixelSize(imageWidth * zoom, scaleFactor),
        .height = thumbnailPixelSize(imageHeight * zoom, scaleFactor),
    };
}

PixelCanvasRealSize pixelCanvasRealSize(int imageWidth, int imageHeight,
                                        const PixelCanvasView& view, double scaleFactor) noexcept {
    const double scale = pixelCanvasScale(view);
    return PixelCanvasRealSize{
        .width = thumbnailPixelSize(static_cast<int>(std::lround(imageWidth * scale)), scaleFactor),
        .height =
            thumbnailPixelSize(static_cast<int>(std::lround(imageHeight * scale)), scaleFactor),
    };
}

}  // namespace hmi
