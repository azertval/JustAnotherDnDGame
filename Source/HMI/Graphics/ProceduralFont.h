// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "HMI/Graphics/AssetContract.h"  // hmi::AssetValidation

/**
 * @file HMI/Graphics/ProceduralFont.h
 * @brief Métriques de police bitmap, mesure pure de texte, lecture JSON et repli procédural
 *        (`LOT-52`) — logique pure, sans GPU ni Qt (`EX-NFR-010`).
 */

namespace hmi {

/// Position, taille et avance horizontale d'un glyphe dans l'atlas de police, en pixels.
struct GlyphMetrics {
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;
    /// Distance horizontale jusqu'au glyphe suivant (peut différer de `width`).
    int advance = 0;
};

/**
 * @brief Métriques d'une police bitmap : région et avance de chaque point de code couvert.
 *
 * Donnée **pure** (`EX-NFR-010`) : ne référence ni fichier ni GPU, seulement ce qu'il faut pour
 * mesurer et composer du texte en quads. Un point de code absent retombe sur
 * `replacementCodePoint` (substitution déterministe, `EX-NFR-040`) — jamais un trou silencieux ni
 * un plantage.
 */
struct FontMetrics {
    /// Hauteur d'une ligne, en pixels (avance verticale d'un retour à la ligne `\n`).
    int lineHeight = 0;
    /// Point de code substitué à un caractère non couvert par `glyphs` (typiquement `?`).
    char32_t replacementCodePoint = U'?';
    /// Point de code → métriques de son glyphe dans l'atlas.
    std::unordered_map<char32_t, GlyphMetrics> glyphs;

    /**
     * @brief Métriques effectives d'un point de code, avec substitution.
     * @param codePoint Point de code cherché.
     * @return Ses métriques si couvert ; sinon celles de `replacementCodePoint` s'il l'est ;
     *         sinon `nullptr` (police corrompue au point de n'avoir même pas de remplacement).
     */
    [[nodiscard]] const GlyphMetrics* glyph(char32_t codePoint) const;
};

/// Dimensions mesurées d'un texte, en pixels (à l'échelle demandée).
struct TextExtent {
    float width = 0.0f;
    float height = 0.0f;
};

/**
 * @brief Décode le point de code UTF-8 suivant d'une chaîne.
 *
 * La mesure et la composition du texte (`hmi::TextRenderer`) doivent parcourir des **points de
 * code**, pas des octets : un caractère accentué du catalogue de traduction (`EX-REN-033`) occupe
 * plusieurs octets en UTF-8.
 * @param text  Chaîne UTF-8 parcourue.
 * @param index Décalage en octets ; avancé jusqu'au début du point de code suivant.
 * @return Le point de code décodé ; `0xFFFD` (caractère de remplacement Unicode) sur une séquence
 *         invalide ou tronquée (avance alors d'un seul octet, jamais de plantage).
 */
[[nodiscard]] char32_t nextUtf8CodePoint(std::string_view text, std::size_t& index) noexcept;

/**
 * @brief Largeur et hauteur qu'occuperait `text` avec `metrics`, à l'échelle `scale`.
 *
 * Fonction **pure**, sans GPU (`EX-NFR-010`) : c'est ce qui permet de cadrer un affichage sans le
 * dessiner (ancrage, `hmi::TextRenderer`). Un retour à la ligne (`\n`) revient à une largeur nulle
 * et ajoute une `lineHeight` ; un caractère non couvert (même après substitution) n'avance pas.
 * @param metrics Métriques de la police mesurée.
 * @param text    Texte UTF-8 mesuré.
 * @param scale   Facteur d'échelle appliqué aux dimensions.
 * @return La largeur de la ligne la plus longue et la hauteur totale (au moins une ligne).
 */
[[nodiscard]] TextExtent measureText(const FontMetrics& metrics, std::string_view text,
                                     float scale = 1.0f) noexcept;

/// Pixels d'un atlas de police généré en mémoire, au format `R8G8B8A8_UNORM` (ordre mémoire
/// R,G,B,A), ligne par ligne (haut en bas).
struct ProceduralFontImage {
    int width = 0;
    int height = 0;
    std::vector<std::uint32_t> pixels;
};

/// Atlas de police procédural complet : ses pixels et les métriques qui les décrivent.
struct ProceduralFont {
    ProceduralFontImage image;
    FontMetrics metrics;
};

/**
 * @brief Génère, en mémoire, une police bitmap minimale et déterministe (repli sans asset).
 *
 * Couvre l'ASCII imprimable (0x20-0x7E) et les lettres accentuées du français
 * (`é è à ç ù ê î ô û` et majuscules), en glyphes 5×7 pixels **blancs** sur fond transparent
 * (colorables par la teinte du quad, comme `hmi::buildProceduralAtlasImage`, `LOT-39`). Chasse
 * fixe (une seule avance pour tous les glyphes) : simple et suffisant pour un repli de secours.
 * Aucune dépendance fichier/GPU : le jeu reste lisible même sans aucun asset de police
 * (`EX-NFR-040`).
 * @return L'image générée et ses métriques, prêtes pour `hmi::BitmapFont`.
 */
[[nodiscard]] ProceduralFont buildProceduralFont();

/// Version du format de métriques écrite dans le fichier, et plus élevée qui soit lue
/// (`hmi::loadFontMetricsFromString`).
inline constexpr int FONT_METRICS_FORMAT_VERSION = 1;

/**
 * @brief Catégorie d'échec de lecture d'un fichier de métriques (même esprit que
 *        `hmi::AnimationCatalogError`).
 */
enum class FontMetricsError {
    None,                ///< Pas d'erreur.
    FileNotFound,        ///< Fichier absent. Cas attendu (repli procédural), pas une anomalie.
    ParseError,          ///< JSON malformé, ou racine qui n'est pas un objet.
    UnsupportedVersion,  ///< Numéro de version supérieur à `FONT_METRICS_FORMAT_VERSION`.
    MalformedStructure,  ///< Structure inattendue (glyphe sans région, point de code invalide…).
};

/**
 * @brief Résultat d'une lecture de métriques : soit des `FontMetrics`, soit une erreur décrite.
 *        Même patron que `hmi::AnimationDescriptionResult` (`EX-NFR-040`).
 */
struct FontMetricsResult {
    std::optional<FontMetrics> metrics;
    std::string error;
    FontMetricsError errorCode = FontMetricsError::None;

    /// @return true si la lecture a réussi.
    [[nodiscard]] bool ok() const noexcept {
        return metrics.has_value();
    }
};

/**
 * @brief Lit des métriques de police depuis une chaîne JSON.
 *
 * Format : `{"version":1,"lineHeight":10,"replacement":"?","glyphs":[{"char":"A","x":0,"y":0,
 * "width":6,"height":10,"advance":6}, ...]}`. `"replacement"` est optionnel (`?` par défaut).
 * @param json Contenu JSON.
 * @return Les métriques, ou une erreur décrite. Ne lève jamais.
 */
[[nodiscard]] FontMetricsResult loadFontMetricsFromString(std::string_view json);

/**
 * @brief Lit des métriques de police depuis un fichier.
 * @param path Chemin du fichier.
 * @return Les métriques, ou une erreur décrite (`FileNotFound` pour un fichier absent, cas
 *         attendu qui ne doit produire aucun avertissement côté appelant). Ne lève jamais.
 */
[[nodiscard]] FontMetricsResult loadFontMetricsFromFile(const std::filesystem::path& path);

/**
 * @brief Valide la cohérence entre des métriques et les dimensions **décodées** du PNG associé.
 *
 * Séparée de la lecture JSON : les dimensions réelles ne sont connues qu'après décodage du PNG,
 * en aval (même frontière que `hmi::AnimationCatalog::validateAgainstTexture`).
 * @param metrics       Métriques déjà lues.
 * @param fileName      Nom logique de l'asset, repris dans le message d'erreur.
 * @param textureWidth  Largeur décodée du PNG, en pixels.
 * @param textureHeight Hauteur décodée du PNG, en pixels.
 * @return Le verdict, avec un message exploitable si non conforme (une région de glyphe hors des
 *         bornes de la texture, ou une hauteur de ligne non positive).
 */
[[nodiscard]] AssetValidation validateFontMetricsAgainstTexture(const FontMetrics& metrics,
                                                                const std::string& fileName,
                                                                int textureWidth,
                                                                int textureHeight);

}  // namespace hmi
