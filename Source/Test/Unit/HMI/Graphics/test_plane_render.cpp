// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

/**
 * @file test_plane_render.cpp
 * @brief Tests unitaires de la composition des **plans picturaux** (`EX-REN-049`, `EX-DEC-040` à
 *        `EX-DEC-042`, LOT-69 TACHE-05).
 */

#include <cstdint>
#include <vector>

#include <gtest/gtest.h>

#include "Core/Levels/Plane.h"
#include "HMI/Graphics/ComposedScene.h"
#include "HMI/Graphics/PlaneVisibility.h"
#include "HMI/Graphics/PlaneVisuals.h"
#include "HMI/Graphics/QuadRecorder.h"
#include "HMI/Graphics/RenderMode.h"

namespace {

using core::Plane;
using core::PlaneDepth;

/// Largeur/hauteur du niveau témoin, en cases (donc en unités monde).
constexpr int LEVEL_WIDTH = 20;
constexpr int LEVEL_HEIGHT = 12;

/// Quatre textures distinctes : la composition ne compare que des identités opaques, un entier
/// adressable suffit donc (même patron que test_quad_recorder).
int textureStorage[4] = {0, 0, 0, 0};

/// @return Une texture de plan distincte pour chaque rang.
hmi::PlaneTexture planeTexture(int rank) {
    return hmi::PlaneTexture{&textureStorage[rank], 320, 192};
}

/// Construit @p count plans consécutifs, tous `Behind` et à densité native.
std::vector<Plane> planes(std::size_t count) {
    std::vector<Plane> result;
    result.reserve(count);
    for (std::size_t index = 0; index < count; ++index) {
        Plane plane;
        plane.fileName = "p" + std::to_string(index) + ".png";
        result.push_back(plane);
    }
    return result;
}

/// Textures alignées sur `planes(count)`.
std::vector<hmi::PlaneTexture> textures(std::size_t count) {
    std::vector<hmi::PlaneTexture> result;
    result.reserve(count);
    for (std::size_t index = 0; index < count; ++index) {
        result.push_back(planeTexture(static_cast<int>(index)));
    }
    return result;
}

}  // namespace

/**
 * @brief **Le point dur de la tâche** : quatre plans composés ressortent, après tri, dans l'ordre
 * du niveau — et non dans un ordre dicté par le regroupement par texture.
 *
 * `hmi::ComposedScene::sort()` intercale le rang de **première apparition de texture** entre le
 * calque et le `sortOrder`. Chaque plan portant sa propre image, ce sont donc les rangs de texture
 * qui les ordonnent, et cela ne coïncide avec l'ordre déclaré que parce que les plans sont composés
 * **en premier et dans l'ordre**. La propriété est invisible à la lecture : sans ce test, elle se
 * casserait silencieusement au premier réordonnancement d'appels.
 * \castest{<b>Quatre plans ressortent dans l'ordre du niveau apres tri.</b><br/>
 * \tcat Unitaire · Plans picturaux<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Composer quatre plans d'un meme calque, chacun avec sa propre texture.<br/>2. Trier
 * la scene, puis lire l'ordre des textures liees.<br/>
 * \tattendu Les quatre quads sortent dans l'ordre declare des plans.
 * }
 */
TEST(PlaneRenderTest, QuatrePlansRessortentDansLOrdreDuNiveau) {
    hmi::ComposedScene scene;
    hmi::composePlanes(scene, planes(4), textures(4), LEVEL_WIDTH, LEVEL_HEIGHT,
                       hmi::RenderMode::Texture);
    scene.sort();

    ASSERT_EQ(scene.quads().size(), 4u);
    for (std::size_t rank = 0; rank < 4; ++rank) {
        EXPECT_EQ(scene.quads()[rank].texture, planeTexture(static_cast<int>(rank)).texture)
            << "rang " << rank;
    }
}

/**
 * @brief La profondeur déclarée choisit le calque : derrière les tuiles, ou devant le personnage.
 * \castest{<b>La profondeur d'un plan choisit son calque de rendu.</b><br/>
 * \tcat Unitaire · Plans picturaux<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Projeter les deux profondeurs sur leur calque.<br/>2. Composer un plan de chaque
 * profondeur.<br/>
 * \tattendu Behind donne le calque Plane, Front le calque Foreground, et le plan de devant est
 * dessine apres le personnage.
 * }
 */
TEST(PlaneRenderTest, ProfondeurChoisitLeCalque) {
    EXPECT_EQ(hmi::planeRenderLayer(PlaneDepth::Behind), hmi::RenderLayer::Plane);
    EXPECT_EQ(hmi::planeRenderLayer(PlaneDepth::Front), hmi::RenderLayer::Foreground);
    // Contrat de lecture : ce qui passe devant le personnage n'est pas physique (EX-DEC-042).
    EXPECT_LT(hmi::RenderLayer::Plane, hmi::RenderLayer::Tile);
    EXPECT_LT(hmi::RenderLayer::Player, hmi::RenderLayer::Foreground);

    std::vector<Plane> levelPlanes = planes(2);
    levelPlanes[1].depth = PlaneDepth::Front;

    hmi::ComposedScene scene;
    hmi::composePlanes(scene, levelPlanes, textures(2), LEVEL_WIDTH, LEVEL_HEIGHT,
                       hmi::RenderMode::Texture);
    scene.sort();

    ASSERT_EQ(scene.quads().size(), 2u);
    EXPECT_EQ(scene.quads()[0].layer, hmi::RenderLayer::Plane);
    EXPECT_EQ(scene.quads()[1].layer, hmi::RenderLayer::Foreground);
}

/**
 * @brief Un plan couvre exactement les bornes du niveau, avec des UV pleines — et sa **densité**
 * n'y change rien.
 *
 * C'est le sens du réglage (`EX-DEC-041`) : la densité est un compromis de **mémoire**, pas de
 * cadrage. Un plan à 8 px/unité montre la même surface qu'un plan natif, en plus grossier.
 * \castest{<b>Un plan couvre les bornes du niveau, densite comprise.</b><br/>
 * \tcat Unitaire · Plans picturaux<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Composer un plan a densite native, puis le meme a densite 8.<br/>2. Comparer les
 * deux quads.<br/>
 * \tattendu Meme rectangle (0,0)-(largeur,hauteur) et memes UV pleines dans les deux cas.
 * }
 */
TEST(PlaneRenderTest, QuadCouvreLeNiveauQuelleQueSoitLaDensite) {
    hmi::ComposedScene natif;
    hmi::composePlanes(natif, planes(1), textures(1), LEVEL_WIDTH, LEVEL_HEIGHT,
                       hmi::RenderMode::Texture);

    std::vector<Plane> reduits = planes(1);
    reduits[0].pixelsPerUnit = 8;
    hmi::ComposedScene reduit;
    hmi::composePlanes(reduit, reduits, textures(1), LEVEL_WIDTH, LEVEL_HEIGHT,
                       hmi::RenderMode::Texture);

    ASSERT_EQ(natif.quads().size(), 1u);
    ASSERT_EQ(reduit.quads().size(), 1u);
    const hmi::SpriteQuad& first = natif.quads()[0].sprite;
    const hmi::SpriteQuad& second = reduit.quads()[0].sprite;

    EXPECT_FLOAT_EQ(first.x, 0.0f);
    EXPECT_FLOAT_EQ(first.y, 0.0f);
    EXPECT_FLOAT_EQ(first.width, static_cast<float>(LEVEL_WIDTH));
    EXPECT_FLOAT_EQ(first.height, static_cast<float>(LEVEL_HEIGHT));
    EXPECT_FLOAT_EQ(first.u0, 0.0f);
    EXPECT_FLOAT_EQ(first.v0, 0.0f);
    EXPECT_FLOAT_EQ(first.u1, 1.0f);
    EXPECT_FLOAT_EQ(first.v1, 1.0f);

    EXPECT_FLOAT_EQ(second.x, first.x);
    EXPECT_FLOAT_EQ(second.y, first.y);
    EXPECT_FLOAT_EQ(second.width, first.width);
    EXPECT_FLOAT_EQ(second.height, first.height);
    EXPECT_FLOAT_EQ(second.u1, first.u1);
    EXPECT_FLOAT_EQ(second.v1, first.v1);
}

/**
 * @brief L'opacité déclarée devient l'alpha du quad, borné à `[0, 1]`.
 * \castest{<b>L'opacite d'un plan devient l'alpha de son quad.</b><br/>
 * \tcat Unitaire · Plans picturaux<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Composer un plan a opacite 0,5 et un plan a opacite hors bornes.<br/>
 * \tattendu L'alpha vaut 0,5 pour le premier, et reste borne a 1 pour le second.
 * }
 */
TEST(PlaneRenderTest, OpaciteDevientAlphaDuQuad) {
    std::vector<Plane> levelPlanes = planes(2);
    levelPlanes[0].opacity = 0.5f;
    levelPlanes[1].opacity = 4.0f;  // hors bornes : borne plutot que propage

    hmi::ComposedScene scene;
    hmi::composePlanes(scene, levelPlanes, textures(2), LEVEL_WIDTH, LEVEL_HEIGHT,
                       hmi::RenderMode::Texture);
    scene.sort();

    ASSERT_EQ(scene.quads().size(), 2u);
    EXPECT_FLOAT_EQ(scene.quads()[0].sprite.a, 0.5f);
    EXPECT_FLOAT_EQ(scene.quads()[1].sprite.a, 1.0f);
}

/**
 * @brief Rien n'est composé en mode Physique : la lecture des collisions reste sans distraction.
 * \castest{<b>Aucun plan n'est compose en mode Physique.</b><br/>
 * \tcat Unitaire · Plans picturaux<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Composer trois plans en mode Physique.<br/>
 * \tattendu La scene reste vide.
 * }
 */
TEST(PlaneRenderTest, RienNEstComposeEnModePhysique) {
    hmi::ComposedScene scene;
    hmi::composePlanes(scene, planes(3), textures(3), LEVEL_WIDTH, LEVEL_HEIGHT,
                       hmi::RenderMode::Physique);
    EXPECT_EQ(scene.size(), 0u);
}

/**
 * @brief Un plan masqué n'émet rien ; isoler un plan masque tous les autres, quels que soient
 * leurs masquages individuels.
 * \castest{<b>Masquage et isolement d'un plan.</b><br/>
 * \tcat Unitaire · Plans picturaux<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Masquer un plan sur trois et composer.<br/>2. Isoler un plan et composer.<br/>3.
 * Tout reafficher et composer.<br/>
 * \tattendu Deux quads, puis un seul (celui isole), puis les trois.
 * }
 */
TEST(PlaneRenderTest, PlanMasqueOuIsoleNEmetRien) {
    hmi::PlaneVisibility visibility;
    visibility.setVisible(1, false);
    {
        hmi::ComposedScene scene;
        hmi::composePlanes(scene, planes(3), textures(3), LEVEL_WIDTH, LEVEL_HEIGHT,
                           hmi::RenderMode::Texture, visibility);
        scene.sort();
        ASSERT_EQ(scene.quads().size(), 2u);
        EXPECT_EQ(scene.quads()[0].texture, planeTexture(0).texture);
        EXPECT_EQ(scene.quads()[1].texture, planeTexture(2).texture);
    }

    // L'isolement prime sur les masquages individuels : le plan 1, masque ci-dessus, redevient le
    // seul visible.
    visibility.isolate(1);
    {
        hmi::ComposedScene scene;
        hmi::composePlanes(scene, planes(3), textures(3), LEVEL_WIDTH, LEVEL_HEIGHT,
                           hmi::RenderMode::Texture, visibility);
        ASSERT_EQ(scene.quads().size(), 1u);
        EXPECT_EQ(scene.quads()[0].texture, planeTexture(1).texture);
    }

    visibility.showAll();
    {
        hmi::ComposedScene scene;
        hmi::composePlanes(scene, planes(3), textures(3), LEVEL_WIDTH, LEVEL_HEIGHT,
                           hmi::RenderMode::Texture, visibility);
        EXPECT_EQ(scene.size(), 3u);
    }
}

/**
 * @brief Un plan dont l'image n'a pas pu être résolue n'émet rien plutôt que de composer un quad
 * sans texture — le repli en damier, lui, est décidé à la **résolution**, pas ici.
 * \castest{<b>Un plan sans texture resolue n'emet aucun quad.</b><br/>
 * \tcat Unitaire · Plans picturaux<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Composer trois plans dont celui du milieu sans texture.<br/>
 * \tattendu Deux quads, dans l'ordre des plans restants.
 * }
 */
TEST(PlaneRenderTest, PlanSansTextureNEmetRien) {
    std::vector<hmi::PlaneTexture> planeTextures = textures(3);
    planeTextures[1] = hmi::PlaneTexture{};

    hmi::ComposedScene scene;
    hmi::composePlanes(scene, planes(3), planeTextures, LEVEL_WIDTH, LEVEL_HEIGHT,
                       hmi::RenderMode::Texture);
    scene.sort();

    ASSERT_EQ(scene.quads().size(), 2u);
    EXPECT_EQ(scene.quads()[0].texture, planeTexture(0).texture);
    EXPECT_EQ(scene.quads()[1].texture, planeTexture(2).texture);
}

/**
 * @brief Le coût d'un plan est **constant en taille de niveau** : un quad et une passe, que le
 * niveau double ou non — propriété qui distingue les plans des tuiles.
 * \castest{<b>Le cout d'un plan ne depend pas de la taille du niveau.</b><br/>
 * \tcat Unitaire · Plans picturaux<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Composer quatre plans sur un niveau, puis sur un niveau deux fois plus grand.<br/>
 * \tattendu Meme nombre de primitives et meme nombre de passes dans les deux cas.
 * }
 */
TEST(PlaneRenderTest, CoutConstantEnTailleDeNiveau) {
    hmi::ComposedScene petit;
    hmi::composePlanes(petit, planes(4), textures(4), LEVEL_WIDTH, LEVEL_HEIGHT,
                       hmi::RenderMode::Texture);
    petit.sort();

    hmi::ComposedScene grand;
    hmi::composePlanes(grand, planes(4), textures(4), LEVEL_WIDTH * 2, LEVEL_HEIGHT * 2,
                       hmi::RenderMode::Texture);
    grand.sort();

    EXPECT_EQ(petit.size(), grand.size());
    EXPECT_EQ(petit.batchCount(), grand.batchCount());
    // Une passe par plan : chaque plan porte sa propre image, aucun regroupement n'est possible.
    EXPECT_EQ(grand.batchCount(), 4);
}
