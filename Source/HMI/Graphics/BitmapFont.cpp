// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "HMI/Graphics/BitmapFont.h"

#include <optional>
#include <stdexcept>
#include <utility>

#include "HMI/Graphics/AssetContract.h"
#include "HMI/Graphics/AssetPaths.h"
#include "HMI/Graphics/GraphicsLog.h"
#include "HMI/Graphics/RhiContext.h"
#include "HMI/Graphics/TextureLoader.h"
#include "HMI/Platform/ExecutableDirectory.h"

namespace hmi {

// Charge la police (fichier, avec repli procedural) et cree la texture GPU associee.
BitmapFont::BitmapFont(const RhiContext& context) {
    if (loadFromAssets(context)) {
        return;
    }
    generateProcedural(context);
}

// Essaie de charger Assets/Fonts/font.png + font.json. true si les deux ont ete charges, valides,
// et la texture creee avec succes.
bool BitmapFont::loadFromAssets(const RhiContext& context) {
    const AssetPaths assetPaths(executableDirectory() / "Assets");

    const std::optional<std::filesystem::path> imagePath =
        assetPaths.resolve(FONTS_SUBDIRECTORY + FONT_ASSET_FILE_NAME);
    if (!imagePath) {
        GRAPHICS_LOG_INFO("BitmapFont : asset '" + FONTS_SUBDIRECTORY + FONT_ASSET_FILE_NAME +
                          "' absent, repli sur la police procedurale");
        return false;
    }
    std::optional<LoadedTexture> loaded = loadTextureFromFile(context, *imagePath);
    if (!loaded) {
        GRAPHICS_LOG_WARNING("BitmapFont : echec du chargement de '" + imagePath->string() +
                             "', repli sur la police procedurale");
        return false;
    }
    const AssetValidation dimensionValidation =
        validateAsset(AssetFamily::Font, FONT_ASSET_FILE_NAME, loaded->width, loaded->height);
    if (!dimensionValidation.valid) {
        GRAPHICS_LOG_WARNING("BitmapFont : " + dimensionValidation.message +
                             " Repli sur la police procedurale.");
        return false;
    }

    const std::optional<std::filesystem::path> metricsPath =
        assetPaths.resolve(FONTS_SUBDIRECTORY + FONT_METRICS_FILE_NAME);
    if (!metricsPath) {
        GRAPHICS_LOG_WARNING("BitmapFont : fichier de metriques '" + FONTS_SUBDIRECTORY +
                             FONT_METRICS_FILE_NAME + "' absent, repli sur la police procedurale");
        return false;
    }
    const FontMetricsResult metricsResult = loadFontMetricsFromFile(*metricsPath);
    if (!metricsResult.ok()) {
        GRAPHICS_LOG_WARNING("BitmapFont : " + metricsResult.error +
                             " Repli sur la police procedurale.");
        return false;
    }
    const AssetValidation coherence = validateFontMetricsAgainstTexture(
        *metricsResult.metrics, FONT_ASSET_FILE_NAME, loaded->width, loaded->height);
    if (!coherence.valid) {
        GRAPHICS_LOG_WARNING("BitmapFont : " + coherence.message +
                             " Repli sur la police procedurale.");
        return false;
    }

    _textureWidth = loaded->width;
    _textureHeight = loaded->height;
    _texture = std::move(loaded->texture);
    _metrics = *metricsResult.metrics;
    GRAPHICS_LOG_INFO("BitmapFont : police chargee depuis '" + imagePath->string() + "'");
    return true;
}

// Genere la police procedurale et cree la texture GPU associee.
void BitmapFont::generateProcedural(const RhiContext& context) {
    const ProceduralFont font = buildProceduralFont();
    std::optional<LoadedTexture> loaded =
        createTexture(context, font.image.width, font.image.height, font.image.pixels);
    if (!loaded) {
        // Echec de creation GPU d'un contenu genere en memoire : erreur d'initialisation non
        // recuperable (device perdu, ressources epuisees), pas un cas metier attendu -- meme
        // discipline que TextureAtlas::generateProcedural.
        throw std::runtime_error("Echec de creation de la texture de police procedurale");
    }

    _textureWidth = loaded->width;
    _textureHeight = loaded->height;
    _texture = std::move(loaded->texture);
    _metrics = font.metrics;
    GRAPHICS_LOG_TRACE("BitmapFont : police procedurale generee");
}

}  // namespace hmi
