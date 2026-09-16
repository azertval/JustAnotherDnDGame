// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "HMI/Graphics/Parallax.h"

#include <algorithm>
#include <cmath>

namespace hmi {

// Position de rendu decalee par la parallaxe, relative au centre de la salle (voir
// en-tete) : centre + (position - centre) * facteur -- inchangee a facteur 1.0, par axe.
core::Vector2 parallaxRenderPosition(core::Vector2 modelPosition, core::Vector2 factor,
                                     const core::Rect& cameraBounds) noexcept {
    const core::Vector2 center = cameraBounds.position + cameraBounds.size * 0.5F;
    return core::Vector2{center.x + ((modelPosition.x - center.x) * factor.x),
                         center.y + ((modelPosition.y - center.y) * factor.y)};
}

// Inverse de parallaxRenderPosition (voir en-tete) : centre + (position_rendu - centre) / facteur.
core::Vector2 parallaxModelPosition(core::Vector2 renderPosition, core::Vector2 factor,
                                    const core::Rect& cameraBounds) noexcept {
    const core::Vector2 center = cameraBounds.position + cameraBounds.size * 0.5F;
    // Un axe a facteur nul n'est pas inversible : il est laisse tel quel plutot que de produire un
    // infini -- l'autre axe reste correctement inverse (robustesse, aucun plan du projet n'a un
    // facteur nul).
    const float x =
        factor.x == 0.0F ? renderPosition.x : center.x + ((renderPosition.x - center.x) / factor.x);
    const float y =
        factor.y == 0.0F ? renderPosition.y : center.y + ((renderPosition.y - center.y) / factor.y);
    return core::Vector2{x, y};
}

namespace {

// Borne le decalage d'un axe pour que [origin, origin + extent] recouvre [cameraOrigin,
// cameraOrigin + cameraExtent]. Renvoie 0 si le plan est plus petit que le cadrage : aucune
// translation ne peut le couvrir, et rester aligne sur le niveau est le comportement stable.
float clampAxis(float offset, float origin, float extent, float cameraOrigin,
                float cameraExtent) noexcept {
    const float maximum = cameraOrigin - origin;                              // bord gauche/haut
    const float minimum = (cameraOrigin + cameraExtent) - (origin + extent);  // bord droit/bas
    if (minimum > maximum) {
        return 0.0F;
    }
    return std::clamp(offset, minimum, maximum);
}

}  // namespace

// Borne le decalage d'un plan pour qu'il couvre toujours le cadrage (voir en-tete).
core::Vector2 clampPlaneOffset(core::Vector2 offset, const core::Rect& planeBounds,
                               const core::Rect& cameraBounds) noexcept {
    return core::Vector2{clampAxis(offset.x, planeBounds.position.x, planeBounds.size.x,
                                   cameraBounds.position.x, cameraBounds.size.x),
                         clampAxis(offset.y, planeBounds.position.y, planeBounds.size.y,
                                   cameraBounds.position.y, cameraBounds.size.y)};
}

// Arrondit une position monde au pixel ecran le plus proche (voir en-tete).
core::Vector2 roundToScreenPixel(core::Vector2 worldPosition, float pixelsPerWorldUnit) noexcept {
    if (pixelsPerWorldUnit <= 0.0F) {
        return worldPosition;
    }
    return core::Vector2{std::round(worldPosition.x * pixelsPerWorldUnit) / pixelsPerWorldUnit,
                         std::round(worldPosition.y * pixelsPerWorldUnit) / pixelsPerWorldUnit};
}

}  // namespace hmi
