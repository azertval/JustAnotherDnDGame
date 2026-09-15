// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "HMI/Graphics/PlaneVisuals.h"

#include <algorithm>
#include <cstdint>

#include "HMI/Editor/PlaneFileNaming.h"
#include "HMI/Graphics/GraphicsLog.h"
#include "HMI/Graphics/Parallax.h"

namespace hmi {

// Projette la profondeur d'un plan sur le calque de rendu correspondant.
RenderLayer planeRenderLayer(core::PlaneDepth depth) noexcept {
    switch (depth) {
        case core::PlaneDepth::Behind:
            return RenderLayer::Plane;
        case core::PlaneDepth::Front:
            return RenderLayer::Foreground;
    }
    return RenderLayer::Plane;
}

// Compose les plans picturaux d'un niveau, dans l'ordre declare.
void composePlanes(ComposedScene& scene, const std::vector<core::Plane>& planes,
                   const std::vector<PlaneTexture>& textures, int levelWidth, int levelHeight,
                   RenderMode mode, const PlaneVisibility& visibility,
                   const PlaneParallax& parallax) {
    if (mode != RenderMode::Texture || levelWidth <= 0 || levelHeight <= 0) {
        return;  // mode Physique : lecture nue des collisions, aucun habillage (EX-REN-046).
    }

    const std::size_t count = (std::min)(planes.size(), textures.size());
    for (std::size_t rank = 0; rank < count; ++rank) {
        if (!visibility.visible(rank)) {
            continue;
        }
        const PlaneTexture& texture = textures[rank];
        if (texture.texture == nullptr) {
            continue;
        }
        const core::Plane& plane = planes[rank];

        // Decalage de parallaxe (EX-DEC-043), par axe. Trois etapes dans CET ordre :
        // 1. le decalage relatif au centre de la salle (jamais absolu : EX-REN-015) ;
        // 2. le bornage, sans lequel le bord du plan se decouvrirait en fin de course ;
        // 3. l'arrondi au pixel ecran, sans lequel le pixel art tremblerait.
        // Inverser 2 et 3 laisserait un decalage borne fractionnaire : l'arrondi doit venir en
        // dernier.
        core::Vector2 origin{0.0f, 0.0f};
        if (parallax.active) {
            const core::Rect planeBounds{
                core::Vector2{0.0f, 0.0f},
                core::Vector2{static_cast<float>(levelWidth), static_cast<float>(levelHeight)}};
            const core::Vector2 factor{plane.parallaxX, plane.parallaxY};
            const core::Vector2 rendered =
                parallaxRenderPosition(origin, factor, parallax.cameraBounds);
            const core::Vector2 clamped =
                clampPlaneOffset(rendered - origin, planeBounds, parallax.cameraBounds);
            origin = roundToScreenPixel(origin + clamped, parallax.pixelsPerWorldUnit);
        }

        SpriteQuad quad;
        quad.x = origin.x;
        quad.y = origin.y;
        quad.width = static_cast<float>(levelWidth);
        quad.height = static_cast<float>(levelHeight);
        // UV pleines, quelle que soit la densite : l'image EST le niveau a son echelle propre.
        quad.u0 = 0.0f;
        quad.v0 = 0.0f;
        quad.u1 = 1.0f;
        quad.v1 = 1.0f;
        // Opacite (EX-DEC-040) portee par la teinte : le pipeline multiplie deja texture x teinte,
        // aucun etat de melange supplementaire n'est necessaire.
        quad.a = std::clamp(plane.opacity, 0.0f, 1.0f);

        // `sortOrder` = rang du plan : depart le tri fin a l'interieur d'un calque quand deux plans
        // partagent la meme image, cas ou le rang de texture ne les departage pas.
        scene.addSprite(planeRenderLayer(plane.depth), texture.texture,
                        static_cast<std::int32_t>(rank), quad);
        // Second axe du budget (EX-NFR-043) : ce qui est REELLEMENT lie, repli en damier compris.
        // Le poids exige par le format, lui, se lit sans GPU (hmi::planesTextureMemoryBytes) --
        // deux questions distinctes, et c'est la seconde que plafonne le test de budget.
        scene.addTextureBytes(static_cast<std::size_t>((std::max)(texture.width, 0)) *
                              static_cast<std::size_t>((std::max)(texture.height, 0)) *
                              PLANE_TEXTURE_BYTES_PER_PIXEL);
    }
}

// Memoire de texture qu'un plan exige du format.
std::size_t planeTextureMemoryBytes(const core::Plane& plane, int levelWidth,
                                    int levelHeight) noexcept {
    const PlanePixelSize size = planePixelSize(levelWidth, levelHeight, plane.pixelsPerUnit);
    return static_cast<std::size_t>(size.width) * static_cast<std::size_t>(size.height) *
           PLANE_TEXTURE_BYTES_PER_PIXEL;
}

// Somme sur tous les plans d'un niveau.
std::size_t planesTextureMemoryBytes(const std::vector<core::Plane>& planes, int levelWidth,
                                     int levelHeight) noexcept {
    std::size_t total = 0;
    for (const core::Plane& plane : planes) {
        total += planeTextureMemoryBytes(plane, levelWidth, levelHeight);
    }
    return total;
}

}  // namespace hmi
