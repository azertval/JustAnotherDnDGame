// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "HMI/Graphics/TextureAtlas.h"

#include <optional>
#include <stdexcept>
#include <utility>

#include "HMI/Graphics/GraphicsLog.h"
#include "HMI/Graphics/ProceduralAtlas.h"
#include "HMI/Graphics/RhiContext.h"
#include "HMI/Graphics/TextureLoader.h"

namespace hmi {

// Génère l'atlas procédural et crée la texture GPU associée.
TextureAtlas::TextureAtlas(const RhiContext& context) {
    const ProceduralAtlasImage image = buildProceduralAtlasImage();
    std::optional<LoadedTexture> loaded =
        createTexture(context, image.width, image.height, image.pixels);
    if (!loaded) {
        // Echec de creation GPU d'un contenu genere en memoire : erreur d'initialisation non
        // recuperable (device perdu, ressources epuisees), pas un cas metier attendu.
        throw std::runtime_error("Echec de creation de la texture d'atlas procedural");
    }

    _width = loaded->width;
    _height = loaded->height;
    _texture = std::move(loaded->texture);
    GRAPHICS_LOG_TRACE("TextureAtlas : atlas procedural genere");
}

}  // namespace hmi
