// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

/**
 * @file test_bitmap_font.cpp
 * @brief Tests unitaires de la police bitmap (LOT-52 TACHE-01) : mesure pure, lecture des
 *        métriques JSON, repli procédural déterministe. Sans GPU ni Qt.
 */

#include <cstdint>
#include <ios>
#include <string>

#include <gtest/gtest.h>

#include "HMI/Graphics/ProceduralFont.h"

namespace {
constexpr const char* VALID_METRICS_JSON = R"({
  "version": 1,
  "lineHeight": 10,
  "replacement": "?",
  "glyphs": [
    { "char": "A", "x": 0,  "y": 0, "width": 6, "height": 10, "advance": 6 },
    { "char": "B", "x": 6,  "y": 0, "width": 6, "height": 10, "advance": 6 },
    { "char": "?", "x": 12, "y": 0, "width": 6, "height": 10, "advance": 6 }
  ]
})";
}  // namespace

/**
 * @brief La mesure d'une chaîne vide donne une largeur nulle et une hauteur d'une ligne.
 * \castest{<b>La mesure d'une chaîne vide donne une largeur nulle et une hauteur d'une
 * ligne.</b><br/>
 * \tcat Unitaire · Police bitmap<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu La mesure d'une chaîne vide donne une largeur nulle et une hauteur d'une ligne.
 * }
 */
TEST(BitmapFontTest, MesureChaineVideLargeurNulle) {
    const hmi::ProceduralFont font = hmi::buildProceduralFont();
    const hmi::TextExtent extent = hmi::measureText(font.metrics, "", 1.0f);
    EXPECT_FLOAT_EQ(extent.width, 0.0f);
    EXPECT_FLOAT_EQ(extent.height, static_cast<float>(font.metrics.lineHeight));
}

/**
 * @brief La mesure d'un seul caractère donne exactement son avance.
 * \castest{<b>La mesure d'un seul caractère donne exactement son avance.</b><br/>
 * \tcat Unitaire · Police bitmap<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu La mesure d'un seul caractère donne exactement son avance.
 * }
 */
TEST(BitmapFontTest, MesureUnCaractere) {
    const hmi::ProceduralFont font = hmi::buildProceduralFont();
    const hmi::GlyphMetrics* glyphA = font.metrics.glyph(U'A');
    ASSERT_NE(glyphA, nullptr);

    const hmi::TextExtent extent = hmi::measureText(font.metrics, "A", 1.0f);
    EXPECT_FLOAT_EQ(extent.width, static_cast<float>(glyphA->advance));
    EXPECT_FLOAT_EQ(extent.height, static_cast<float>(font.metrics.lineHeight));
}

/**
 * @brief Une chaîne accentuée est mesurée par POINTS DE CODE, pas par octets UTF-8.
 * \castest{<b>Une chaîne accentuée est mesurée par points de code, pas par octets UTF-8.</b><br/>
 * \tcat Unitaire · Police bitmap<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu La largeur mesurée correspond au nombre de points de code, pas au nombre d'octets.
 * }
 */
TEST(BitmapFontTest, MesureChaineAccentueeParPointsDeCode) {
    const hmi::ProceduralFont font = hmi::buildProceduralFont();
    // "Épuisé" : 6 points de code, mais plus de 6 octets en UTF-8 (E, accents multi-octets).
    constexpr const char* text = "\xC3\x89puis\xC3\xA9";  // "Épuisé"
    const hmi::TextExtent extent = hmi::measureText(font.metrics, text, 1.0f);

    const hmi::GlyphMetrics* glyphE = font.metrics.glyph(0x00C9);  // É
    ASSERT_NE(glyphE, nullptr);
    // Chasse fixe (repli procédural) : 6 glyphes * la même avance.
    EXPECT_FLOAT_EQ(extent.width, static_cast<float>(glyphE->advance) * 6.0f);
}

/**
 * @brief Un caractère absent de la police est substitué par le glyphe de remplacement, jamais un
 *        trou silencieux ni un plantage.
 * \castest{<b>Un caractère absent de la police est substitué par le glyphe de
 * remplacement.</b><br/> \tcat Unitaire · Police bitmap<br/> \tcrit Majeur<br/> \tetapes 1. Mettre
 * en place le contexte du test (arrangement).<br/>2. Executer le scenario et verifier les
 * assertions.<br/> \tattendu Le caractère absent est substitué par le glyphe de remplacement.
 * }
 */
TEST(BitmapFontTest, CaractereAbsentEstSubstitue) {
    const hmi::FontMetricsResult result = hmi::loadFontMetricsFromString(VALID_METRICS_JSON);
    ASSERT_TRUE(result.ok()) << result.error;

    // '#' n'est pas declare dans VALID_METRICS_JSON : doit retomber sur le remplacement ('?').
    const hmi::GlyphMetrics* substituted = result.metrics->glyph(U'#');
    const hmi::GlyphMetrics* replacement = result.metrics->glyph(U'?');
    ASSERT_NE(substituted, nullptr);
    ASSERT_NE(replacement, nullptr);
    EXPECT_EQ(substituted->x, replacement->x);
    EXPECT_EQ(substituted->y, replacement->y);
}

/**
 * @brief Un fichier de métriques JSON valide se lit correctement (champs et glyphes).
 * \castest{<b>Un fichier de métriques JSON valide se lit correctement (champs et
 * glyphes).</b><br/>
 * \tcat Unitaire · Police bitmap<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Les métriques et les glyphes déclarés sont lus fidèlement.
 * }
 */
TEST(BitmapFontTest, MetriquesJsonValideEstLue) {
    const hmi::FontMetricsResult result = hmi::loadFontMetricsFromString(VALID_METRICS_JSON);
    ASSERT_TRUE(result.ok()) << result.error;
    EXPECT_EQ(result.metrics->lineHeight, 10);
    EXPECT_EQ(result.metrics->replacementCodePoint, U'?');

    const hmi::GlyphMetrics* glyphB = result.metrics->glyph(U'B');
    ASSERT_NE(glyphB, nullptr);
    EXPECT_EQ(glyphB->x, 6);
    EXPECT_EQ(glyphB->y, 0);
    EXPECT_EQ(glyphB->width, 6);
    EXPECT_EQ(glyphB->height, 10);
    EXPECT_EQ(glyphB->advance, 6);
}

/**
 * @brief Un JSON de métriques malformé est une erreur exploitable, jamais une exception.
 * \castest{<b>Un JSON de métriques malformé est une erreur exploitable, jamais une
 * exception.</b><br/>
 * \tcat Unitaire · Police bitmap<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu La lecture échoue avec un message d'erreur exploitable, sans exception.
 * }
 */
TEST(BitmapFontTest, JsonInvalideEstUneErreurExploitable) {
    const hmi::FontMetricsResult result = hmi::loadFontMetricsFromString("{ pas du json");
    EXPECT_FALSE(result.ok());
    EXPECT_EQ(result.errorCode, hmi::FontMetricsError::ParseError);
    EXPECT_FALSE(result.error.empty());
}

/**
 * @brief Une version de format supérieure à celle gérée est refusée.
 * \castest{<b>Une version de format supérieure à celle gérée est refusée.</b><br/>
 * \tcat Unitaire · Police bitmap<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu La lecture échoue avec le code UnsupportedVersion.
 * }
 */
TEST(BitmapFontTest, VersionInconnueEstRefusee) {
    constexpr const char* json = R"({
      "version": 99, "lineHeight": 10,
      "glyphs": [ { "char": "A", "x": 0, "y": 0, "width": 6, "height": 10, "advance": 6 } ]
    })";
    const hmi::FontMetricsResult result = hmi::loadFontMetricsFromString(json);
    EXPECT_FALSE(result.ok());
    EXPECT_EQ(result.errorCode, hmi::FontMetricsError::UnsupportedVersion);
}

/**
 * @brief Un fichier de métriques absent est un échec `FileNotFound`, sans exception.
 * \castest{<b>Un fichier de métriques absent est un échec FileNotFound, sans exception.</b><br/>
 * \tcat Unitaire · Police bitmap<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu La lecture échoue avec le code FileNotFound.
 * }
 */
TEST(BitmapFontTest, FichierMetriquesAbsentEstFileNotFound) {
    const hmi::FontMetricsResult result =
        hmi::loadFontMetricsFromFile("Z:/chemin/totalement/inexistant/font.json");
    EXPECT_FALSE(result.ok());
    EXPECT_EQ(result.errorCode, hmi::FontMetricsError::FileNotFound);
}

/**
 * @brief Des métriques cohérentes avec les dimensions du PNG sont validées.
 * \castest{<b>Des métriques cohérentes avec les dimensions du PNG sont validées.</b><br/>
 * \tcat Unitaire · Police bitmap<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu La validation réussit (aucun message d'erreur).
 * }
 */
TEST(BitmapFontTest, MetriquesCoherentesAvecLePngValidees) {
    const hmi::FontMetricsResult result = hmi::loadFontMetricsFromString(VALID_METRICS_JSON);
    ASSERT_TRUE(result.ok());
    // Les trois glyphes couvrent [0,18)x[0,10) : une image 18x10 les contient tous.
    const hmi::AssetValidation valid =
        hmi::validateFontMetricsAgainstTexture(*result.metrics, "font.png", 18, 10);
    EXPECT_TRUE(valid.valid) << valid.message;
}

/**
 * @brief Des métriques dont un glyphe déborde des dimensions du PNG sont refusées.
 * \castest{<b>Des métriques dont un glyphe déborde des dimensions du PNG sont refusées.</b><br/>
 * \tcat Unitaire · Police bitmap<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu La validation échoue avec un message nommant le fichier.
 * }
 */
TEST(BitmapFontTest, MetriquesIncoherentesAvecLePngRefusees) {
    const hmi::FontMetricsResult result = hmi::loadFontMetricsFromString(VALID_METRICS_JSON);
    ASSERT_TRUE(result.ok());
    // Image trop petite (10x10) : le glyphe '?' (x=12) deborde en largeur.
    const hmi::AssetValidation invalid =
        hmi::validateFontMetricsAgainstTexture(*result.metrics, "font.png", 10, 10);
    EXPECT_FALSE(invalid.valid);
    EXPECT_NE(invalid.message.find("font.png"), std::string::npos);
}

/**
 * @brief Le repli procédural couvre l'ASCII imprimable et les accents français, et est
 *        déterministe (deux appels produisent des pixels et métriques identiques).
 * \castest{<b>Le repli procédural couvre l'ASCII imprimable et les accents français, de façon
 * déterministe.</b><br/>
 * \tcat Unitaire · Police bitmap<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Tous les caractères ASCII imprimables et accentués (é è à ç ù ê î ô û) sont couverts ;
 * deux générations produisent le même résultat.
 * }
 */
TEST(BitmapFontTest, ReplitProceduralCouvreAsciiEtAccentsDeFaconDeterministe) {
    const hmi::ProceduralFont first = hmi::buildProceduralFont();
    const hmi::ProceduralFont second = hmi::buildProceduralFont();

    EXPECT_EQ(first.image.width, second.image.width);
    EXPECT_EQ(first.image.height, second.image.height);
    EXPECT_EQ(first.image.pixels, second.image.pixels);
    EXPECT_EQ(first.metrics.glyphs.size(), second.metrics.glyphs.size());

    // ASCII imprimable complet.
    for (char32_t codePoint = 0x20; codePoint <= 0x7E; ++codePoint) {
        EXPECT_NE(first.metrics.glyph(codePoint), nullptr)
            << "code point 0x" << std::hex << static_cast<std::uint32_t>(codePoint);
    }
    // Accents du francais utilises par le catalogue de traduction : e è à ç ù ê î ô û. Points de
    // code numeriques (pas de litteral U'e...') : le fichier source est lu en page de code
    // systeme par le compilateur, un litteral accentue serait ambigu (meme piege que l'ancien
    // hmi::BitmapFont, LOT-38).
    constexpr char32_t FRENCH_ACCENTED_CODE_POINTS[] = {0x00E9, 0x00E8, 0x00E0, 0x00E7, 0x00F9,
                                                        0x00EA, 0x00EE, 0x00F4, 0x00FB};
    for (const char32_t codePoint : FRENCH_ACCENTED_CODE_POINTS) {
        EXPECT_NE(first.metrics.glyph(codePoint), nullptr);
    }
}

/**
 * @brief Chaque glyphe généré procéduralement désigne une région entièrement contenue dans
 *        l'image générée (repli utilisable tel quel, sans plantage au rendu).
 * \castest{<b>Chaque glyphe procédural désigne une région entièrement contenue dans l'image
 * générée.</b><br/>
 * \tcat Unitaire · Police bitmap<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Toutes les régions de glyphes sont contenues dans l'image, et les métriques sont
 * cohérentes avec elle.
 * }
 */
TEST(BitmapFontTest, ReplitProceduralEstAutoCoherent) {
    const hmi::ProceduralFont font = hmi::buildProceduralFont();
    const hmi::AssetValidation coherence = hmi::validateFontMetricsAgainstTexture(
        font.metrics, "font-procedural.png", font.image.width, font.image.height);
    EXPECT_TRUE(coherence.valid) << coherence.message;
}
