// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_depth_sort.cpp
 * @brief Tests unitaires du tri par profondeur en vue de dessus (`EX-REN-018`, LOT-07).
 *        Composition pure, sans GPU (`EX-NFR-004`).
 */

#include <vector>

#include <gtest/gtest.h>

#include "Core/Ecs/Components/Sprite.h"
#include "Core/Ecs/Components/Transform.h"
#include "Core/Ecs/World.h"
#include "Core/Math/Vector2.h"
#include "HMI/Graphics/ComposedScene.h"
#include "HMI/Graphics/MissingTexture.h"
#include "HMI/Graphics/QuadRecorder.h"
#include "HMI/Graphics/RenderLayer.h"

namespace {

// Deux identites de texture distinctes : la composition ne fait que les comparer, jamais les
// dereferencer (EX-NFR-004). C'est le point du lot -- personnage et decor n'ont PAS la meme.
int firstTextureStorage = 0;
int secondTextureStorage = 0;
hmi::TextureHandle textureA = &firstTextureStorage;
hmi::TextureHandle textureB = &secondTextureStorage;

hmi::SceneTextures testTextures() {
    hmi::SceneTextures textures;
    textures.atlas = textureA;
    textures.atlasWidth = 80;
    textures.atlasHeight = 80;
    textures.missing = textureB;
    textures.missingWidth = hmi::MISSING_TEXTURE_SIZE;
    textures.missingHeight = hmi::MISSING_TEXTURE_SIZE;
    return textures;
}

// Rectangle d'une case, dont le PIED (bord bas) est a @p footY.
hmi::SpriteQuad quadWithFoot(float x, float footY) {
    hmi::SpriteQuad quad;
    quad.x = x;
    quad.y = footY - 1.0f;
    quad.width = 1.0f;
    quad.height = 1.0f;
    return quad;
}

// Entite d'une case, sur le calque demande.
core::Entity addSprite(core::World& world, hmi::RenderLayer layer, core::Vector2 position,
                       core::Vector2 size) {
    const core::Entity entity = world.createEntity();
    world.addComponent(entity, core::Transform{position, size, 0.0f});
    core::Sprite sprite;
    sprite.region = core::AtlasRegion{0, 0, 16, 16};
    world.addComponent(entity, sprite);
    world.addComponent(entity, hmi::RenderLayerTag{layer});
    return entity;
}

}  // namespace

/**
 * @brief Trois primitives de la bande de profondeur, à Y croissants, sortent dans l'ordre de leur
 * pied — **même quand leurs textures diffèrent** (`EX-REN-018`).
 *
 * C'est le cœur du lot : avant lui, le regroupement par texture tranchait avant le tri fin, et
 * l'ordre de profondeur ne survivait pas à deux textures.
 * \castest{<b>Trois primitives a Y croissants sortent dans l'ordre de leur pied.</b><br/>
 * \tcat Unitaire · Tri par profondeur<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Composer trois primitives de la bande de profondeur, dans le desordre, avec deux
 * textures differentes.<br/>2. Trier la scene.<br/>
 * \tattendu Les trois sortent par pied croissant, quelle que soit leur texture.
 * }
 */
TEST(TriParProfondeurTest, TroisPrimitivesSortentParPiedCroissant) {
    hmi::ComposedScene scene;
    scene.addSprite(hmi::RenderLayer::Object, textureA, hmi::depthSortOrder(6.0f),
                    quadWithFoot(0.0f, 6.0f));
    scene.addSprite(hmi::RenderLayer::Player, textureB, hmi::depthSortOrder(2.0f),
                    quadWithFoot(1.0f, 2.0f));
    scene.addSprite(hmi::RenderLayer::Object, textureB, hmi::depthSortOrder(4.0f),
                    quadWithFoot(2.0f, 4.0f));
    scene.sort();

    ASSERT_EQ(scene.size(), 3u);
    EXPECT_FLOAT_EQ(scene.quads()[0].sprite.x, 1.0f);  // pied 2
    EXPECT_FLOAT_EQ(scene.quads()[1].sprite.x, 2.0f);  // pied 4
    EXPECT_FLOAT_EQ(scene.quads()[2].sprite.x, 0.0f);  // pied 6
}

/**
 * @brief Le personnage passe **derrière** un objet plus bas et **devant** un objet plus haut : le
 * rang de calque ne tranche plus à l'intérieur de la bande (`EX-REN-018`).
 * \castest{<b>Le personnage passe derriere un objet plus bas et devant un objet plus haut.</b><br/>
 * \tcat Unitaire · Tri par profondeur<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Composer un arbre au-dessus du personnage, le personnage, puis un arbre en
 * dessous.<br/>2. Trier.<br/>
 * \tattendu L'arbre du haut sort avant le personnage, celui du bas apres.
 * }
 */
TEST(TriParProfondeurTest, PersonnageEntreDeuxObjets) {
    hmi::ComposedScene scene;
    scene.addSprite(hmi::RenderLayer::Object, textureA, hmi::depthSortOrder(3.0f),
                    quadWithFoot(0.0f, 3.0f));  // arbre plus haut a l'ecran
    scene.addSprite(hmi::RenderLayer::Player, textureB, hmi::depthSortOrder(5.0f),
                    quadWithFoot(1.0f, 5.0f));  // personnage
    scene.addSprite(hmi::RenderLayer::Object, textureA, hmi::depthSortOrder(7.0f),
                    quadWithFoot(2.0f, 7.0f));  // arbre plus bas a l'ecran
    scene.sort();

    ASSERT_EQ(scene.size(), 3u);
    EXPECT_EQ(scene.quads()[0].layer, hmi::RenderLayer::Object);
    EXPECT_EQ(scene.quads()[1].layer, hmi::RenderLayer::Player);
    EXPECT_EQ(scene.quads()[2].layer, hmi::RenderLayer::Object);
    EXPECT_FLOAT_EQ(scene.quads()[2].sprite.x, 2.0f);
}

/**
 * @brief À pied égal, l'ordre reste celui de la composition : deux sprites ne peuvent pas
 * scintiller d'une image à l'autre (`EX-REN-018`).
 * \castest{<b>A pied egal, l'ordre de composition est preserve : aucun scintillement.</b><br/>
 * \tcat Unitaire · Tri par profondeur<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Composer trois primitives de meme pied, a des positions differentes.<br/>2.
 * Trier.<br/>3. Recomposer et retrier a l'identique.<br/>
 * \tattendu Les deux tris rendent exactement le meme ordre.
 * }
 */
TEST(TriParProfondeurTest, PiedEgalConserveLOrdreDeComposition) {
    const auto composer = [] {
        hmi::ComposedScene scene;
        scene.addSprite(hmi::RenderLayer::Object, textureA, hmi::depthSortOrder(4.0f),
                        quadWithFoot(3.0f, 4.0f));
        scene.addSprite(hmi::RenderLayer::Player, textureB, hmi::depthSortOrder(4.0f),
                        quadWithFoot(1.0f, 4.0f));
        scene.addSprite(hmi::RenderLayer::Object, textureA, hmi::depthSortOrder(4.0f),
                        quadWithFoot(2.0f, 4.0f));
        scene.sort();
        std::vector<float> order;
        for (const hmi::ComposedQuad& quad : scene.quads()) {
            order.push_back(quad.sprite.x);
        }
        return order;
    };

    const std::vector<float> first = composer();
    const std::vector<float> second = composer();
    EXPECT_EQ(first, second);
    // Et l'ordre est bien celui de la composition, texture par texture regroupee ensuite.
    ASSERT_EQ(first.size(), 3u);
    EXPECT_FLOAT_EQ(first[0], 3.0f);
}

/**
 * @brief Un écart de moins d'un pixel ne départage pas deux profondeurs : la quantification au
 * pixel évite qu'un arrondi flottant fasse permuter deux sprites (`EX-REN-018`).
 * \castest{<b>Un ecart inferieur au pixel ne departage pas deux profondeurs.</b><br/>
 * \tcat Unitaire · Tri par profondeur<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Calculer l'ordre de deux pieds distants d'un centieme d'unite.<br/>2. Le comparer a
 * celui de deux pieds distants d'une demi-unite.<br/>
 * \tattendu Le premier couple partage le meme ordre, le second non.
 * }
 */
TEST(TriParProfondeurTest, QuantificationAuPixel) {
    EXPECT_EQ(hmi::depthSortOrder(4.0f), hmi::depthSortOrder(4.01f));
    EXPECT_LT(hmi::depthSortOrder(4.0f), hmi::depthSortOrder(4.5f));
    // Un Y plus grand (plus bas a l'ecran) se dessine plus tard, donc au-dessus.
    EXPECT_LT(hmi::depthSortOrder(1.0f), hmi::depthSortOrder(9.0f));
}

/**
 * @brief Les bandes restent hiérarchisées : la profondeur ne fait jamais passer un objet devant
 * un plan de premier plan, ni sous les tuiles (`EX-REN-014`).
 * \castest{<b>La profondeur ne deborde pas de sa bande.</b><br/>
 * \tcat Unitaire · Tri par profondeur<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Composer une tuile, un objet tres bas, un plan de premier plan tres haut.<br/>2.
 * Trier.<br/>
 * \tattendu La tuile sort en premier, l'objet ensuite, le premier plan en dernier.
 * }
 */
TEST(TriParProfondeurTest, LaProfondeurNeDebordePasDeSaBande) {
    hmi::ComposedScene scene;
    scene.addSprite(hmi::RenderLayer::Foreground, textureA, hmi::depthSortOrder(0.0f),
                    quadWithFoot(0.0f, 0.0f));
    scene.addSprite(hmi::RenderLayer::Object, textureA, hmi::depthSortOrder(99.0f),
                    quadWithFoot(1.0f, 99.0f));
    scene.addSprite(hmi::RenderLayer::Tile, textureA, 0, quadWithFoot(2.0f, 50.0f));
    scene.sort();

    hmi::QuadRecorder recorder;
    recorder.record(scene);

    ASSERT_EQ(scene.size(), 3u);
    EXPECT_EQ(scene.quads()[0].layer, hmi::RenderLayer::Tile);
    EXPECT_EQ(scene.quads()[1].layer, hmi::RenderLayer::Object);
    EXPECT_EQ(scene.quads()[2].layer, hmi::RenderLayer::Foreground);
    EXPECT_TRUE(recorder.isLayerOrderRespected()) << recorder.describe();
}

/**
 * @brief La composition depuis l'ECS alimente elle-même l'ordre de profondeur, depuis le **pied**
 * du quad : deux sprites de hauteurs différentes posés sur la même case s'ordonnent pareil
 * (`EX-REN-018`).
 * \castest{<b>La composition alimente l'ordre de profondeur depuis le pied du quad.</b><br/>
 * \tcat Unitaire · Tri par profondeur<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Peupler un monde d'un objet haut et d'un personnage bas, poses sur la meme
 * ligne.<br/>2. Composer depuis l'ECS et trier.<br/>
 * \tattendu Les deux portent le meme ordre de profondeur, celui de leur pied commun.
 * }
 */
TEST(TriParProfondeurTest, CompositionAlimenteLaProfondeurDepuisLePied) {
    core::World world;
    // Un arbre de deux cases de haut et un personnage d'une case, dont les PIEDS coincident.
    addSprite(world, hmi::RenderLayer::Object, core::Vector2{0.0f, 2.0f},
              core::Vector2{1.0f, 2.0f});
    addSprite(world, hmi::RenderLayer::Player, core::Vector2{1.0f, 3.0f},
              core::Vector2{1.0f, 1.0f});

    hmi::ComposedScene scene;
    hmi::composeWorldSprites(scene, world, hmi::RenderMode::Physique, testTextures(), 0.0f);
    scene.sort();

    ASSERT_EQ(scene.size(), 2u);
    EXPECT_EQ(scene.quads()[0].sortOrder, hmi::depthSortOrder(4.0f));
    EXPECT_EQ(scene.quads()[1].sortOrder, hmi::depthSortOrder(4.0f));
}

/**
 * @brief Hors de la bande de profondeur, `core::Sprite::layer` conserve son rôle de tri fin : le
 * rang de couche du `LOT-04` continue d'ordonner sol et décor entre eux (`EX-LVL-016`).
 * \castest{<b>Hors bande de profondeur, le tri fin reste celui de Sprite::layer.</b><br/>
 * \tcat Unitaire · Tri par profondeur<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Peupler un monde de deux tuiles de rangs de couche differents, la seconde plus
 * basse.<br/>2. Composer et trier.<br/>
 * \tattendu L'ordre suit le rang de couche, pas la position.
 * }
 */
TEST(TriParProfondeurTest, HorsBandeLeTriFinResteCeluiDeLaCouche) {
    core::World world;
    const core::Entity haut = addSprite(world, hmi::RenderLayer::Tile, core::Vector2{0.0f, 9.0f},
                                        core::Vector2{1.0f, 1.0f});
    world.getComponent<core::Sprite>(haut).layer = 1;  // decor : rang 1
    const core::Entity bas = addSprite(world, hmi::RenderLayer::Tile, core::Vector2{1.0f, 0.0f},
                                       core::Vector2{1.0f, 1.0f});
    world.getComponent<core::Sprite>(bas).layer = 0;  // sol : rang 0

    hmi::ComposedScene scene;
    hmi::composeWorldSprites(scene, world, hmi::RenderMode::Physique, testTextures(), 0.0f);
    scene.sort();

    ASSERT_EQ(scene.size(), 2u);
    EXPECT_EQ(scene.quads()[0].sortOrder, 0);
    EXPECT_EQ(scene.quads()[1].sortOrder, 1);
}
