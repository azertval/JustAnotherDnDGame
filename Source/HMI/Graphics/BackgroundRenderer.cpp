// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "HMI/Graphics/BackgroundRenderer.h"

namespace hmi {

// Calcule le recadrage UV d'une image pour couvrir un rectangle cible sans deformation (motif
// "cover" : ratio preserve, recadrage centre sur la dimension excedentaire).
BackgroundFit computeBackgroundFit(float imageWidth, float imageHeight, float targetWidth,
                                   float targetHeight) noexcept {
    const float imageAspect = imageWidth / imageHeight;
    const float targetAspect = targetWidth / targetHeight;

    BackgroundFit fit;
    if (imageAspect > targetAspect) {
        // Image relativement plus large que la cible : recadrage horizontal, hauteur pleine.
        const float visibleWidthFraction = targetAspect / imageAspect;
        fit.u0 = (1.0F - visibleWidthFraction) * 0.5F;
        fit.u1 = 1.0F - fit.u0;
    } else if (imageAspect < targetAspect) {
        // Image relativement plus haute que la cible : recadrage vertical, largeur pleine.
        const float visibleHeightFraction = imageAspect / targetAspect;
        fit.v0 = (1.0F - visibleHeightFraction) * 0.5F;
        fit.v1 = 1.0F - fit.v0;
    }
    return fit;
}

// Compose le fond du niveau sur RenderLayer::Background, s'il y a lieu (trois cas, cf. header).
void composeBackground(ComposedScene& scene, const BackgroundTexture& background, int levelWidth,
                       int levelHeight, RenderMode mode) {
    if (mode != RenderMode::Texture || background.texture == nullptr) {
        return;  // mode Physique, ou aucun fond designe : rien a composer (etat normal).
    }

    const BackgroundFit fit = computeBackgroundFit(
        static_cast<float>(background.width), static_cast<float>(background.height),
        static_cast<float>(levelWidth), static_cast<float>(levelHeight));

    SpriteQuad quad;
    quad.x = 0.0F;
    quad.y = 0.0F;
    quad.width = static_cast<float>(levelWidth);
    quad.height = static_cast<float>(levelHeight);
    quad.u0 = fit.u0;
    quad.v0 = fit.v0;
    quad.u1 = fit.u1;
    quad.v1 = fit.v1;
    scene.addSprite(RenderLayer::Background, background.texture, 0, quad);
}

}  // namespace hmi
