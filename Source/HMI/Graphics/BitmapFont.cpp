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

namespace {

// Charge l'image de la police et valide ses dimensions. std::nullopt (avec journal) si l'asset est
// absent, illisible ou hors contrat.
std::optional<LoadedTexture> loadFontImage(const RhiContext& context, const AssetPaths& assetPaths,
                                           std::filesystem::path& imagePathOut) {
    const std::optional<std::filesystem::path> imagePath =
        assetPaths.resolve(FONTS_SUBDIRECTORY + BitmapFont::FONT_ASSET_FILE_NAME);
    if (!imagePath) {
        GRAPHICS_LOG_INFO("BitmapFont : asset '" + FONTS_SUBDIRECTORY +
                          BitmapFont::FONT_ASSET_FILE_NAME +
                          "' absent, repli sur la police procedurale");
        return std::nullopt;
    }
    std::optional<LoadedTexture> loaded = loadTextureFromFile(context, *imagePath);
    if (!loaded) {
        GRAPHICS_LOG_WARNING("BitmapFont : echec du chargement de '" + imagePath->string() +
                             "', repli sur la police procedurale");
        return std::nullopt;
    }
    const AssetValidation dimensionValidation = validateAsset(
        AssetFamily::Font, BitmapFont::FONT_ASSET_FILE_NAME, loaded->width, loaded->height);
    if (!dimensionValidation.valid) {
        GRAPHICS_LOG_WARNING("BitmapFont : " + dimensionValidation.message +
                             " Repli sur la police procedurale.");
        return std::nullopt;
    }
    imagePathOut = *imagePath;
    return loaded;
}

// Charge les metriques de la police et verifie leur coherence avec la texture. std::nullopt (avec
// journal) si le fichier est absent, invalide ou incoherent.
std::optional<FontMetrics> loadFontMetrics(const AssetPaths& assetPaths, int textureWidth,
                                           int textureHeight) {
    const std::optional<std::filesystem::path> metricsPath =
        assetPaths.resolve(FONTS_SUBDIRECTORY + BitmapFont::FONT_METRICS_FILE_NAME);
    if (!metricsPath) {
        GRAPHICS_LOG_WARNING("BitmapFont : fichier de metriques '" + FONTS_SUBDIRECTORY +
                             BitmapFont::FONT_METRICS_FILE_NAME +
                             "' absent, repli sur la police procedurale");
        return std::nullopt;
    }
    const FontMetricsResult metricsResult = loadFontMetricsFromFile(*metricsPath);
    if (!metricsResult.ok()) {
        GRAPHICS_LOG_WARNING("BitmapFont : " + metricsResult.error +
                             " Repli sur la police procedurale.");
        return std::nullopt;
    }
    const AssetValidation coherence = validateFontMetricsAgainstTexture(
        *metricsResult.metrics, BitmapFont::FONT_ASSET_FILE_NAME, textureWidth, textureHeight);
    if (!coherence.valid) {
        GRAPHICS_LOG_WARNING("BitmapFont : " + coherence.message +
                             " Repli sur la police procedurale.");
        return std::nullopt;
    }
    return *metricsResult.metrics;
}

}  // namespace

// Essaie de charger Assets/Fonts/font.png + font.json. true si les deux ont ete charges, valides,
// et la texture creee avec succes.
bool BitmapFont::loadFromAssets(const RhiContext& context) {
    const AssetPaths assetPaths(executableDirectory() / "Assets");

    std::filesystem::path imagePath;
    std::optional<LoadedTexture> loaded = loadFontImage(context, assetPaths, imagePath);
    if (!loaded) {
        return false;
    }
    std::optional<FontMetrics> metrics = loadFontMetrics(assetPaths, loaded->width, loaded->height);
    if (!metrics) {
        return false;
    }

    _textureWidth = loaded->width;
    _textureHeight = loaded->height;
    _texture = std::move(loaded->texture);
    _metrics = std::move(*metrics);
    GRAPHICS_LOG_INFO("BitmapFont : police chargee depuis '" + imagePath.string() + "'");
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
