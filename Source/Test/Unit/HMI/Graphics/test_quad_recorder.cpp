// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

/**
 * @file test_quad_recorder.cpp
 * @brief Tests unitaires de la composition du rendu et de sa capture sans GPU
 *        (LOT-40, EX-NFR-004, EX-REN-043, EX-REN-014).
 */

#include <gtest/gtest.h>

#include "HMI/Graphics/ComposedScene.h"
#include "HMI/Graphics/QuadRecorder.h"
#include "HMI/Graphics/RenderLayer.h"

namespace {

// Textures factices : la composition ne fait que **comparer** des identites de texture, elle n'en
// deference jamais aucune. Deux objets distincts suffisent donc a jouer deux textures distinctes,
// sans device Direct3D (EX-NFR-004).
int firstTextureStorage = 0;
int secondTextureStorage = 0;
hmi::TextureHandle textureA = &firstTextureStorage;
hmi::TextureHandle textureB = &secondTextureStorage;

/// Rectangle minimal a une position donnee, pour distinguer les primitives entre elles.
hmi::SpriteQuad quadAt(float x, float y) {
    hmi::SpriteQuad quad;
    quad.x = x;
    quad.y = y;
    quad.width = 1.0f;
    quad.height = 1.0f;
    return quad;
}

}  // namespace

/**
 * @brief Le calque prime sur la texture : une primitive d'un calque inférieur reste soumise avant
 *        une primitive d'un calque supérieur, quel que soit l'ordre de composition.
 * \castest{<b>Le calque prime sur la texture dans l'ordre de soumission.</b><br/>
 * \tcat Unitaire · Quad Recorder<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Composer un quad Player (texture A) puis un quad Tile (texture B).<br/>2. Trier et
 * capturer la scene.<br/>
 * \tattendu Le quad Tile est soumis avant le quad Player.
 * }
 */
TEST(QuadRecorderTest, CalquePrimeSurTexture) {
    hmi::ComposedScene scene;
    scene.addSprite(hmi::RenderLayer::Player, textureA, 0, quadAt(10.0f, 0.0f));
    scene.addSprite(hmi::RenderLayer::Tile, textureB, 0, quadAt(20.0f, 0.0f));
    scene.sort();

    hmi::QuadRecorder recorder;
    recorder.record(scene);

    ASSERT_EQ(recorder.size(), 2u);
    EXPECT_EQ(recorder.quads()[0].layer, hmi::RenderLayer::Tile);
    EXPECT_EQ(recorder.quads()[1].layer, hmi::RenderLayer::Player);
    EXPECT_TRUE(recorder.isLayerOrderRespected()) << recorder.describe();
}

/**
 * @brief Sur trois calques et deux textures, aucun quad d'un calque supérieur n'est soumis avant
 *        un quad d'un calque inférieur, et chaque texture forme un groupe contigu par calque.
 * \castest{<b>Trois calques et deux textures : ordre des calques et groupes de texture
 * contigus.</b><br/>
 * \tcat Unitaire · Quad Recorder<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Composer six quads en alternant calques et textures.<br/>2. Trier et capturer.<br/>
 * \tattendu L'ordre des calques est respecte et les groupes de texture sont contigus.
 * }
 */
TEST(QuadRecorderTest, TroisCalquesDeuxTexturesOrdonnes) {
    hmi::ComposedScene scene;
    scene.addSprite(hmi::RenderLayer::Player, textureB, 0, quadAt(0.0f, 0.0f));
    scene.addSprite(hmi::RenderLayer::Tile, textureA, 0, quadAt(1.0f, 0.0f));
    scene.addSprite(hmi::RenderLayer::Background, textureB, 0, quadAt(2.0f, 0.0f));
    scene.addSprite(hmi::RenderLayer::Tile, textureB, 0, quadAt(3.0f, 0.0f));
    scene.addSprite(hmi::RenderLayer::Player, textureA, 0, quadAt(4.0f, 0.0f));
    scene.addSprite(hmi::RenderLayer::Tile, textureA, 0, quadAt(5.0f, 0.0f));
    scene.sort();

    hmi::QuadRecorder recorder;
    recorder.record(scene);

    EXPECT_TRUE(recorder.isLayerOrderRespected()) << recorder.describe();
    EXPECT_TRUE(recorder.areTextureGroupsContiguous()) << recorder.describe();

    const std::vector<hmi::RenderLayer> layers = recorder.layerSequence();
    ASSERT_EQ(layers.size(), 3u);
    EXPECT_EQ(layers[0], hmi::RenderLayer::Background);
    EXPECT_EQ(layers[1], hmi::RenderLayer::Tile);
    EXPECT_EQ(layers[2], hmi::RenderLayer::Player);

    EXPECT_EQ(recorder.countOnLayer(hmi::RenderLayer::Tile), 3);
    EXPECT_EQ(recorder.countWithTexture(textureA), 3);
    EXPECT_EQ(recorder.countWithTexture(textureB), 3);
}

/**
 * @brief À calque, texture et tri fin égaux, l'ordre de composition est préservé (tri stable).
 * \castest{<b>Le tri est stable : a cle egale l'ordre de composition est preserve.</b><br/>
 * \tcat Unitaire · Quad Recorder<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Composer trois quads identiques en cle, a des positions differentes.<br/>2.
 * Trier.<br/>
 * \tattendu Les trois quads restent dans leur ordre de composition.
 * }
 */
TEST(QuadRecorderTest, TriStableAClefEgale) {
    hmi::ComposedScene scene;
    scene.addSprite(hmi::RenderLayer::Tile, textureA, 0, quadAt(3.0f, 0.0f));
    scene.addSprite(hmi::RenderLayer::Tile, textureA, 0, quadAt(1.0f, 0.0f));
    scene.addSprite(hmi::RenderLayer::Tile, textureA, 0, quadAt(2.0f, 0.0f));
    scene.sort();

    ASSERT_EQ(scene.size(), 3u);
    EXPECT_FLOAT_EQ(scene.quads()[0].sprite.x, 3.0f);
    EXPECT_FLOAT_EQ(scene.quads()[1].sprite.x, 1.0f);
    EXPECT_FLOAT_EQ(scene.quads()[2].sprite.x, 2.0f);
}

/**
 * @brief À l'intérieur d'un calque et d'une texture, `core::Sprite::layer` conserve son rôle de
 *        tri fin.
 * \castest{<b>Le tri fin par Sprite::layer subsiste a l'interieur d'un calque.</b><br/>
 * \tcat Unitaire · Quad Recorder<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Composer trois quads du meme calque avec des tris fins decroissants.<br/>2.
 * Trier.<br/>
 * \tattendu Les quads sont reordonnes par tri fin croissant.
 * }
 */
TEST(QuadRecorderTest, TriFinParSpriteLayer) {
    hmi::ComposedScene scene;
    scene.addSprite(hmi::RenderLayer::Tile, textureA, 2, quadAt(0.0f, 0.0f));
    scene.addSprite(hmi::RenderLayer::Tile, textureA, 0, quadAt(1.0f, 0.0f));
    scene.addSprite(hmi::RenderLayer::Tile, textureA, 1, quadAt(2.0f, 0.0f));
    scene.sort();

    ASSERT_EQ(scene.size(), 3u);
    EXPECT_EQ(scene.quads()[0].sortOrder, 0);
    EXPECT_EQ(scene.quads()[1].sortOrder, 1);
    EXPECT_EQ(scene.quads()[2].sortOrder, 2);
}

/**
 * @brief Une texture d'un même calque n'est jamais scindée en deux groupes : une passe
 *        `begin/end` par groupe suffit.
 * \castest{<b>Le nombre de passes egale le nombre de groupes de texture.</b><br/>
 * \tcat Unitaire · Quad Recorder<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Composer quatre quads d'un meme calque en alternant deux textures.<br/>2.
 * Trier.<br/>
 * \tattendu La scene ne compte que deux passes.
 * }
 */
TEST(QuadRecorderTest, DeuxTexturesDeuxPasses) {
    hmi::ComposedScene scene;
    scene.addSprite(hmi::RenderLayer::Tile, textureA, 0, quadAt(0.0f, 0.0f));
    scene.addSprite(hmi::RenderLayer::Tile, textureB, 0, quadAt(1.0f, 0.0f));
    scene.addSprite(hmi::RenderLayer::Tile, textureA, 0, quadAt(2.0f, 0.0f));
    scene.addSprite(hmi::RenderLayer::Tile, textureB, 0, quadAt(3.0f, 0.0f));
    scene.sort();

    hmi::QuadRecorder recorder;
    recorder.record(scene);

    EXPECT_EQ(scene.batchCount(), 2);
    EXPECT_EQ(recorder.textureSequence().size(), 2u);
    EXPECT_TRUE(recorder.areTextureGroupsContiguous()) << recorder.describe();
}

/**
 * @brief Segments épais et rectangles cohabitent dans la même liste ordonnée, chacun identifié par
 *        sa nature.
 * \castest{<b>Segments et rectangles cohabitent dans la meme scene ordonnee.</b><br/>
 * \tcat Unitaire · Quad Recorder<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Composer un rectangle puis un segment sur le meme calque.<br/>2. Trier et
 * capturer.<br/>
 * \tattendu Les deux primitives sont capturees dans l'ordre, avec la bonne nature.
 * }
 */
TEST(QuadRecorderTest, RectanglesEtSegmentsMelanges) {
    hmi::LineQuad line;
    line.ax = 0.0f;
    line.ay = 0.0f;
    line.bx = 4.0f;
    line.by = 4.0f;
    line.thickness = 0.05f;

    hmi::ComposedScene scene;
    scene.addSprite(hmi::RenderLayer::EditorOverlay, textureA, 0, quadAt(0.0f, 0.0f));
    scene.addLine(hmi::RenderLayer::EditorOverlay, textureA, 1, line);
    scene.sort();

    ASSERT_EQ(scene.size(), 2u);
    EXPECT_EQ(scene.quads()[0].kind, hmi::QuadKind::Sprite);
    EXPECT_EQ(scene.quads()[1].kind, hmi::QuadKind::Line);
}

/**
 * @brief L'ordonnancement déclaré place le premier plan au-dessus du personnage et les aides
 *        d'édition au-dessus de tout (`EX-REN-014`, `EX-DEC-002`).
 * \castest{<b>L'ordonnancement place le premier plan au-dessus du personnage.</b><br/>
 * \tcat Unitaire · Quad Recorder<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Comparer les valeurs declarees de l'enumeration des calques.<br/>
 * \tattendu L'ordre fond -> plans -> ombres -> tuiles -> objets -> personnage -> premier plan ->
 * interface -> edition est respecte.
 * }
 */
TEST(QuadRecorderTest, OrdonnancementDeclare) {
    EXPECT_LT(hmi::RenderLayer::Background, hmi::RenderLayer::Plane);
    EXPECT_LT(hmi::RenderLayer::Plane, hmi::RenderLayer::Shadow);
    EXPECT_LT(hmi::RenderLayer::Shadow, hmi::RenderLayer::Tile);
    EXPECT_LT(hmi::RenderLayer::Tile, hmi::RenderLayer::Object);
    EXPECT_LT(hmi::RenderLayer::Object, hmi::RenderLayer::Player);
    EXPECT_LT(hmi::RenderLayer::Player, hmi::RenderLayer::Foreground);
    EXPECT_LT(hmi::RenderLayer::Foreground, hmi::RenderLayer::UI);
    EXPECT_LT(hmi::RenderLayer::UI, hmi::RenderLayer::EditorOverlay);
}
