// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

/**
 * @file test_text_renderer.cpp
 * @brief Tests unitaires de la composition de texte en quads sur le calque UI (LOT-52 TACHE-02).
 *        Sans GPU : composition pure (`hmi::ComposedScene`/`hmi::QuadRecorder`, `EX-NFR-004`).
 */

#include <gtest/gtest.h>

#include "HMI/Graphics/ProceduralFont.h"
#include "HMI/Graphics/QuadRecorder.h"
#include "HMI/Graphics/TextRenderer.h"

namespace {

// Texture factice : la composition ne fait que comparer des identites de texture (EX-NFR-004).
int fontTextureStorage = 0;
hmi::TextureHandle fontTexture = &fontTextureStorage;

// Metriques de reference : chasse fixe 6x10 px, comme le repli procedural (LOT-52 TACHE-01),
// couvrant 'A', 'B', l'espace et 'E' accentue -- suffisant pour exercer la composition sans
// dependre du contenu exact du repli procedural.
hmi::FontMetrics testMetrics() {
    hmi::FontMetrics metrics;
    metrics.lineHeight = 10;
    metrics.replacementCodePoint = U'?';
    metrics.glyphs[U'A'] = hmi::GlyphMetrics{0, 0, 6, 10, 6};
    metrics.glyphs[U'B'] = hmi::GlyphMetrics{6, 0, 6, 10, 6};
    metrics.glyphs[U' '] = hmi::GlyphMetrics{12, 0, 6, 10, 6};
    metrics.glyphs[U'?'] = hmi::GlyphMetrics{18, 0, 6, 10, 6};
    metrics.glyphs[0x00C9] = hmi::GlyphMetrics{24, 0, 6, 10, 6};  // 'E'
    return metrics;
}

constexpr int TEXTURE_WIDTH = 96;
constexpr int TEXTURE_HEIGHT = 10;

}  // namespace

/**
 * @brief Une chaîne vide ne produit aucun quad.
 * \castest{<b>Une chaîne vide ne produit aucun quad.</b><br/>
 * \tcat Unitaire · Text Renderer<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Aucun quad n'est composé.
 * }
 */
TEST(TextRendererTest, ChaineVideNeProduitAucunQuad) {
    hmi::ComposedScene scene;
    hmi::composeText(scene, testMetrics(), fontTexture, TEXTURE_WIDTH, TEXTURE_HEIGHT, "", 0.0f,
                     0.0f, 1.0f, core::Color{});
    EXPECT_EQ(scene.size(), 0u);
}

/**
 * @brief Une chaîne de N caractères (hors espaces) produit N quads, aux positions attendues selon
 *        l'avance de chaque glyphe.
 * \castest{<b>Une chaîne de N caractères produit N quads, aux positions attendues.</b><br/>
 * \tcat Unitaire · Text Renderer<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Composer « AB » en ancrage haut-gauche.<br/>2. Capturer la scène.<br/>
 * \tattendu Deux quads sont composés, aux abscisses 0 et 6 (avance du glyphe 'A').
 * }
 */
TEST(TextRendererTest, ChaineDeuxCaracteresProduitDeuxQuadsAuxPositionsAttendues) {
    hmi::ComposedScene scene;
    hmi::composeText(scene, testMetrics(), fontTexture, TEXTURE_WIDTH, TEXTURE_HEIGHT, "AB", 0.0f,
                     0.0f, 1.0f, core::Color{});
    scene.sort();

    hmi::QuadRecorder recorder;
    recorder.record(scene);
    ASSERT_EQ(recorder.size(), 2u);
    EXPECT_TRUE(recorder.containsSpriteAt(0.0f, 0.0f));
    EXPECT_TRUE(recorder.containsSpriteAt(6.0f, 0.0f));
}

/**
 * @brief L'espace n'émet aucun quad (aucun pixel visible), mais fait avancer le stylo.
 * \castest{<b>L'espace n'émet aucun quad mais fait avancer le stylo.</b><br/>
 * \tcat Unitaire · Text Renderer<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Composer « A B » (A, espace, B).<br/>2. Capturer la scène.<br/>
 * \tattendu Deux quads seulement (A et B), B décalé de deux avances (espace inclus).
 * }
 */
TEST(TextRendererTest, EspaceNEmetAucunQuadMaisAvanceLeStylo) {
    hmi::ComposedScene scene;
    hmi::composeText(scene, testMetrics(), fontTexture, TEXTURE_WIDTH, TEXTURE_HEIGHT, "A B", 0.0f,
                     0.0f, 1.0f, core::Color{});
    scene.sort();

    hmi::QuadRecorder recorder;
    recorder.record(scene);
    ASSERT_EQ(recorder.size(), 2u);
    EXPECT_TRUE(recorder.containsSpriteAt(0.0f, 0.0f));
    EXPECT_TRUE(recorder.containsSpriteAt(12.0f, 0.0f));  // B, apres 'A' (6) + espace (6)
}

/**
 * @brief Une chaîne accentuée produit un nombre de quads correspondant aux POINTS DE CODE, pas
 *        aux octets UTF-8.
 * \castest{<b>Une chaîne accentuée produit un quad par point de code, pas par octet.</b><br/>
 * \tcat Unitaire · Text Renderer<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Composer « É » (deux octets UTF-8, un point de code).<br/>2. Capturer la scène.<br/>
 * \tattendu Un seul quad est composé.
 * }
 */
TEST(TextRendererTest, ChaineAccentueeCompteLesPointsDeCode) {
    hmi::ComposedScene scene;
    // "É" = U+00C9, deux octets UTF-8 (0xC3 0x89).
    hmi::composeText(scene, testMetrics(), fontTexture, TEXTURE_WIDTH, TEXTURE_HEIGHT, "\xC3\x89",
                     0.0f, 0.0f, 1.0f, core::Color{});
    EXPECT_EQ(scene.size(), 1u);
}

/**
 * @brief L'ancrage centre horizontalement/verticalement décale la boîte du texte de la moitié de
 *        ses dimensions mesurées.
 * \castest{<b>L'ancrage centré décale la boîte du texte de sa moitié.</b><br/>
 * \tcat Unitaire · Text Renderer<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Composer « AB » avec un ancrage centré aux deux axes.<br/>2. Capturer la scène.<br/>
 * \tattendu Le premier quad démarre à (-6, -5) : moitié de la largeur (12) et de la hauteur (10).
 * }
 */
TEST(TextRendererTest, AncrageCentreDecaleLaBoiteDeSaMoitie) {
    hmi::ComposedScene scene;
    hmi::TextAnchor anchor{hmi::TextHorizontalAnchor::Center, hmi::TextVerticalAnchor::Middle};
    hmi::composeText(scene, testMetrics(), fontTexture, TEXTURE_WIDTH, TEXTURE_HEIGHT, "AB", 0.0f,
                     0.0f, 1.0f, core::Color{}, anchor);

    hmi::QuadRecorder recorder;
    recorder.record(scene);
    ASSERT_EQ(recorder.size(), 2u);
    // Largeur totale "AB" = 12 px, hauteur = 10 px (une ligne) : centre -> decalage (-6, -5).
    EXPECT_TRUE(recorder.containsSpriteAt(-6.0f, -5.0f)) << recorder.describe();
}

/**
 * @brief L'ancrage à droite/en bas positionne la fin de la boîte du texte sur le point d'ancrage.
 * \castest{<b>L'ancrage droite/bas positionne la fin de la boîte sur le point d'ancrage.</b><br/>
 * \tcat Unitaire · Text Renderer<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Composer « AB » avec un ancrage droite/bas à (100, 100).<br/>2. Capturer la
 * scène.<br/>
 * \tattendu Le premier quad démarre à (88, 90) : 100 moins la largeur/hauteur totales (12, 10).
 * }
 */
TEST(TextRendererTest, AncrageDroiteBasPositionneLaFinDeLaBoite) {
    hmi::ComposedScene scene;
    hmi::TextAnchor anchor{hmi::TextHorizontalAnchor::Right, hmi::TextVerticalAnchor::Bottom};
    hmi::composeText(scene, testMetrics(), fontTexture, TEXTURE_WIDTH, TEXTURE_HEIGHT, "AB", 100.0f,
                     100.0f, 1.0f, core::Color{}, anchor);

    hmi::QuadRecorder recorder;
    recorder.record(scene);
    ASSERT_EQ(recorder.size(), 2u);
    EXPECT_TRUE(recorder.containsSpriteAt(88.0f, 90.0f)) << recorder.describe();
}

/**
 * @brief Toutes les primitives composées le sont sur le calque `UI`, jamais un autre.
 * \castest{<b>Toutes les primitives composées le sont sur le calque UI.</b><br/>
 * \tcat Unitaire · Text Renderer<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Composer un texte de plusieurs caractères.<br/>2. Capturer la scène.<br/>
 * \tattendu Toutes les primitives sont sur `RenderLayer::UI`.
 * }
 */
TEST(TextRendererTest, PrimitivesSurLeCalqueUi) {
    hmi::ComposedScene scene;
    hmi::composeText(scene, testMetrics(), fontTexture, TEXTURE_WIDTH, TEXTURE_HEIGHT, "AB", 0.0f,
                     0.0f, 1.0f, core::Color{});
    scene.sort();

    hmi::QuadRecorder recorder;
    recorder.record(scene);
    EXPECT_EQ(recorder.countOnLayer(hmi::RenderLayer::UI), 2);
    EXPECT_EQ(recorder.countOnLayer(hmi::RenderLayer::Tile), 0);
}

/**
 * @brief Un point de code non couvert par la police est substitué par le glyphe de remplacement,
 *        jamais ignoré silencieusement.
 * \castest{<b>Un point de code non couvert est substitué par le glyphe de remplacement.</b><br/>
 * \tcat Unitaire · Text Renderer<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Composer un caractère absent de `testMetrics()` ('#').<br/>2. Capturer la
 * scène.<br/>
 * \tattendu Un quad est tout de même composé (substitution), pas un trou silencieux.
 * }
 */
TEST(TextRendererTest, CaractereAbsentEstSubstitueEtComposeUnQuad) {
    hmi::ComposedScene scene;
    hmi::composeText(scene, testMetrics(), fontTexture, TEXTURE_WIDTH, TEXTURE_HEIGHT, "#", 0.0f,
                     0.0f, 1.0f, core::Color{});
    EXPECT_EQ(scene.size(), 1u);
}

/**
 * @brief Le texte composé n'est jamais soumis au culling de cadrage : une scène sans cadrage
 *        (état par défaut, garanti par le contrat de `hmi::composeText`) conserve les primitives
 *        même à une position écran éloignée de l'origine.
 * \castest{<b>Le texte n'est jamais soumis au culling par cadrage.</b><br/>
 * \tcat Unitaire · Text Renderer<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Composer un texte à une position écran éloignée (10000, 10000), sans cadrage actif
 * sur la scène.<br/>2. Vérifier la scène.<br/>
 * \tattendu Le quad est conservé (aucun culling), la scène n'a pas de cadrage actif.
 * }
 */
TEST(TextRendererTest, TexteNestJamaisCulle) {
    hmi::ComposedScene scene;
    ASSERT_FALSE(scene.isCullingEnabled());
    hmi::composeText(scene, testMetrics(), fontTexture, TEXTURE_WIDTH, TEXTURE_HEIGHT, "A",
                     10000.0f, 10000.0f, 1.0f, core::Color{});
    EXPECT_EQ(scene.size(), 1u);
}
