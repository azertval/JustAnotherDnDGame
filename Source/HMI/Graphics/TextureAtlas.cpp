// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "HMI/Graphics/TextureAtlas.h"

#include <filesystem>
#include <optional>
#include <stdexcept>
#include <string>

#include "HMI/Graphics/AssetPaths.h"
#include "HMI/Graphics/GraphicsLog.h"
#include "HMI/Graphics/ProceduralAtlas.h"
#include "HMI/Graphics/RhiContext.h"
#include "HMI/Graphics/TextureLoader.h"
#include "HMI/Platform/ExecutableDirectory.h"

namespace hmi {

// Charge l'atlas (fichier, avec repli procédural) et crée la texture GPU associée.
TextureAtlas::TextureAtlas(const RhiContext& context) {
    if (loadFromFile(context)) {
        return;
    }
    generateProcedural(context);
}

// Essaie de charger Assets/atlas.png. true si la texture a été créée avec succès.
bool TextureAtlas::loadFromFile(const RhiContext& context) {
    const AssetPaths assetPaths(executableDirectory() / "Assets");
    const std::optional<std::filesystem::path> path = assetPaths.resolve(ATLAS_FILE_NAME);
    if (!path) {
        GRAPHICS_LOG_INFO("TextureAtlas : asset '" + std::string(ATLAS_FILE_NAME) +
                          "' absent, repli sur l'atlas procedural");
        return false;
    }

    std::optional<LoadedTexture> loaded = loadTextureFromFile(context, *path);
    if (!loaded) {
        GRAPHICS_LOG_WARNING("TextureAtlas : echec du chargement de '" + path->string() +
                             "', repli sur l'atlas procedural");
        return false;
    }

    _width = loaded->width;
    _height = loaded->height;
    _texture = std::move(loaded->texture);
    GRAPHICS_LOG_INFO("TextureAtlas : atlas charge depuis '" + path->string() + "'");
    return true;
}

// Génère l'atlas procédural et crée la texture GPU associée.
void TextureAtlas::generateProcedural(const RhiContext& context) {
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
