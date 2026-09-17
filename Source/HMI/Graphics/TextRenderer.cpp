// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "HMI/Graphics/TextRenderer.h"

#include <cmath>
#include <cstddef>

#include "HMI/Graphics/BitmapFont.h"

namespace hmi {

namespace {

// Arrondit une coordonnee ecran au pixel entier le plus proche (nettete, EX-ARCH-022).
[[nodiscard]] float roundToPixel(float value) noexcept {
    return std::round(value);
}

}  // namespace

// Construit la projection ecran -> clip du calque UI, independante de Camera2D.
//
// Meme formule que l'ancienne hmi::BitmapFont::screenProjection (LOT-38, retiree) : le nuanceur
// applique mul(float4(position, 0, 1), projection) (vecteur-ligne), la matrice transforme une
// position en pixels en coordonnees de clip [-1, 1], l'axe Y etant inverse pour que l'ordonnee
// croisse vers le bas de l'ecran.
DirectX::XMFLOAT4X4 screenProjectionMatrix(int viewportWidth, int viewportHeight) noexcept {
    const float width = viewportWidth > 0 ? static_cast<float>(viewportWidth) : 1.0F;
    const float height = viewportHeight > 0 ? static_cast<float>(viewportHeight) : 1.0F;

    // Lignes de la matrice (_11.._14, _21.._24, ...), les autres coefficients a zero.
    return {2.0F / width, 0.0F,           0.0F, 0.0F,  //
            0.0F,         -2.0F / height, 0.0F, 0.0F,  //
            0.0F,         0.0F,           1.0F, 0.0F,  //
            -1.0F,        1.0F,           0.0F, 1.0F};
}

// Compose une chaine en quads sur le calque UI, ancree a une position ecran.
void composeText(ComposedScene& scene, const FontMetrics& metrics, TextureHandle texture,
                 int textureWidth, int textureHeight, std::string_view text, float x, float y,
                 float scale, const core::Color& tint, TextAnchor anchor, std::int32_t sortOrder) {
    if (text.empty() || texture == nullptr || textureWidth <= 0 || textureHeight <= 0) {
        return;
    }

    // Boite englobante du texte (mesure pure, hmi::measureText) : c'est elle qui permet de
    // cadrer l'ancrage sans dessiner. Le bloc entier est ancre (pas chaque ligne separement) --
    // suffisant pour le HUD, dont les libelles tiennent sur une seule ligne.
    const TextExtent extent = measureText(metrics, text, scale);
    float originX = x;
    switch (anchor.horizontal) {
        case TextHorizontalAnchor::Left:
            break;
        case TextHorizontalAnchor::Center:
            originX -= extent.width * 0.5F;
            break;
        case TextHorizontalAnchor::Right:
            originX -= extent.width;
            break;
    }
    float originY = y;
    switch (anchor.vertical) {
        case TextVerticalAnchor::Top:
            break;
        case TextVerticalAnchor::Middle:
            originY -= extent.height * 0.5F;
            break;
        case TextVerticalAnchor::Bottom:
            originY -= extent.height;
            break;
    }
    originX = roundToPixel(originX);
    originY = roundToPixel(originY);

    const float invTextureWidth = 1.0F / static_cast<float>(textureWidth);
    const float invTextureHeight = 1.0F / static_cast<float>(textureHeight);
    const float lineAdvance = static_cast<float>(metrics.lineHeight) * scale;

    float penX = originX;
    float penY = originY;
    std::size_t index = 0;
    while (index < text.size()) {
        const char32_t codePoint = nextUtf8CodePoint(text, index);
        if (codePoint == U'\n') {
            penX = originX;
            penY += lineAdvance;
            continue;
        }

        const GlyphMetrics* glyph = metrics.glyph(codePoint);
        if (glyph == nullptr) {
            continue;  // police corrompue au point de n'avoir meme pas de remplacement.
        }

        // L'espace n'a aucun pixel visible : l'emettre en quad n'apporterait rien, seulement un
        // appel de dessin superflu.
        if (codePoint != U' ') {
            SpriteQuad quad;
            quad.x = roundToPixel(penX);
            quad.y = roundToPixel(penY);
            quad.width = static_cast<float>(glyph->width) * scale;
            quad.height = static_cast<float>(glyph->height) * scale;
            quad.u0 = static_cast<float>(glyph->x) * invTextureWidth;
            quad.v0 = static_cast<float>(glyph->y) * invTextureHeight;
            quad.u1 = static_cast<float>(glyph->x + glyph->width) * invTextureWidth;
            quad.v1 = static_cast<float>(glyph->y + glyph->height) * invTextureHeight;
            quad.r = tint.r;
            quad.g = tint.g;
            quad.b = tint.b;
            quad.a = tint.a;
            scene.addSprite(RenderLayer::UI, texture, sortOrder, quad);
        }

        penX += static_cast<float>(glyph->advance) * scale;
    }
}

// Raccourci de composeText prenant directement une BitmapFont.
void composeText(ComposedScene& scene, const BitmapFont& font, std::string_view text, float x,
                 float y, float scale, const core::Color& tint, TextAnchor anchor,
                 std::int32_t sortOrder) {
    composeText(scene, font.metrics(), font.textureHandle(), font.textureWidth(),
                font.textureHeight(), text, x, y, scale, tint, anchor, sortOrder);
}

}  // namespace hmi
