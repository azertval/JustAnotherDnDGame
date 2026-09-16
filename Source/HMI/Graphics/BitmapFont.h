// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <memory>
#include <string>
#include <string_view>

#include "HMI/Graphics/ProceduralFont.h"
#include "HMI/Graphics/RenderLayer.h"

class QRhiTexture;

/**
 * @file HMI/Graphics/BitmapFont.h
 * @brief Police bitmap chargée depuis un asset (avec métriques), repli procédural (`LOT-52`).
 */

namespace hmi {

struct RhiContext;

/// Sous-dossier des assets de police, relatif au dossier d'assets (`LOT-52`).
inline const std::string FONTS_SUBDIRECTORY = "Fonts/";

/**
 * @brief Police bitmap chargée depuis `Assets/Fonts/`, avec repli **procédural** (`EX-NFR-040`).
 *
 * À la construction, essaie de charger `Assets/Fonts/font.png` accompagné de ses métriques
 * (`Assets/Fonts/font.json`, `hmi::loadFontMetricsFromFile`), validés par le contrat d'asset
 * (`AssetFamily::Font`, `EX-REN-007`) puis par leur cohérence mutuelle
 * (`hmi::validateFontMetricsAgainstTexture`). Si l'un des deux est absent, illisible ou
 * incohérent, retombe sans plantage sur `hmi::buildProceduralFont` (`LOT-52`), sur le modèle de
 * `hmi::TextureAtlas` vis-à-vis de `hmi::buildProceduralAtlasImage` : possède sa **propre**
 * texture GPU (RAII), chargée une fois au démarrage — pas de rechargement à chaud (comme l'atlas
 * de tuiles, hors du périmètre de `LOT-43` TACHE-03).
 */
class BitmapFont {
public:
    /// Nom du fichier d'atlas de glyphes attendu dans `Assets/Fonts/`.
    static constexpr const char* FONT_ASSET_FILE_NAME = "font.png";
    /// Nom du fichier de métriques attendu dans `Assets/Fonts/`, à côté de l'atlas.
    static constexpr const char* FONT_METRICS_FILE_NAME = "font.json";

    /**
     * @brief Charge la police (fichier, avec repli procédural) et crée la texture GPU associée.
     * @param context Interface de rendu et lot de mises à jour de l'image courante.
     */
    explicit BitmapFont(const RhiContext& context);

    /// @return L'identité opaque de la texture de police (non possédée par l'appelant).
    [[nodiscard]] TextureHandle textureHandle() const {
        return _texture.get();
    }

    /// @return Largeur de l'atlas de glyphes, en pixels.
    [[nodiscard]] int textureWidth() const noexcept {
        return _textureWidth;
    }

    /// @return Hauteur de l'atlas de glyphes, en pixels.
    [[nodiscard]] int textureHeight() const noexcept {
        return _textureHeight;
    }

    /// @return Les métriques de la police effectivement chargée (fichier ou repli procédural).
    [[nodiscard]] const FontMetrics& metrics() const noexcept {
        return _metrics;
    }

    /// Raccourci pour `hmi::measureText(metrics(), text, scale)`.
    [[nodiscard]] TextExtent measure(std::string_view text, float scale = 1.0f) const {
        return measureText(_metrics, text, scale);
    }

private:
    /// Essaie de charger `Assets/Fonts/font.png` + `font.json`. @return true si les deux ont été
    /// chargés, validés, et la texture créée avec succès.
    bool loadFromAssets(const RhiContext& context);
    /// Génère la police procédurale (`hmi::buildProceduralFont`) et crée la texture associée.
    void generateProcedural(const RhiContext& context);

    FontMetrics _metrics;
    int _textureWidth = 0;
    int _textureHeight = 0;
    std::shared_ptr<QRhiTexture> _texture;
};

}  // namespace hmi
