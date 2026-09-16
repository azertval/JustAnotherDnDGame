// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

/**
 * @file test_background_fit.cpp
 * @brief Tests unitaires du recadrage et de la composition du fond de niveau (LOT-44,
 *        EX-REN-044, EX-NFR-004).
 */

#include <gtest/gtest.h>

#include "HMI/Graphics/BackgroundRenderer.h"
#include "HMI/Graphics/QuadRecorder.h"

namespace {

// Texture factice : la composition ne fait que comparer des identites (cf. test_quad_recorder).
int textureStorage = 0;
hmi::TextureHandle texture = &textureStorage;

}  // namespace

/**
 * @brief Une image et une cible de même ratio ne subissent aucun recadrage.
 * \castest{<b>Meme ratio : aucun recadrage.</b><br/>
 * \tcat Unitaire · Background Fit<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Calculer le recadrage d'une image carree sur une cible carree.<br/>
 * \tattendu Les UV couvrent l'image entiere ([0,1] sur les deux axes).
 * }
 */
TEST(BackgroundFitTest, MemeRatioAucunRecadrage) {
    const hmi::BackgroundFit fit = hmi::computeBackgroundFit(64.0f, 64.0f, 10.0f, 10.0f);

    EXPECT_FLOAT_EQ(fit.u0, 0.0f);
    EXPECT_FLOAT_EQ(fit.v0, 0.0f);
    EXPECT_FLOAT_EQ(fit.u1, 1.0f);
    EXPECT_FLOAT_EQ(fit.v1, 1.0f);
}

/**
 * @brief Une image plus large que la cible (ratio supérieur) est recadrée horizontalement,
 * centrée, la hauteur restant pleine.
 * \castest{<b>Image plus large que la cible : recadrage horizontal centre.</b><br/>
 * \tcat Unitaire · Background Fit<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Calculer le recadrage d'une image 16:9 sur une cible carree.<br/>
 * \tattendu La hauteur est pleine (v0=0, v1=1) et la largeur est recadree, centree.
 * }
 */
TEST(BackgroundFitTest, ImagePlusLargeRecadreeHorizontalementEtCentree) {
    // Image 16:9 (ratio 1.777...) sur une cible carree (ratio 1.0) : imageAspect > targetAspect.
    const hmi::BackgroundFit fit = hmi::computeBackgroundFit(1920.0f, 1080.0f, 10.0f, 10.0f);

    EXPECT_FLOAT_EQ(fit.v0, 0.0f);
    EXPECT_FLOAT_EQ(fit.v1, 1.0f);
    EXPECT_GT(fit.u0, 0.0f);
    EXPECT_LT(fit.u1, 1.0f);
    // Recadrage centre : symetrique autour de 0.5.
    EXPECT_FLOAT_EQ(fit.u0, 1.0f - fit.u1);
    EXPECT_NEAR(fit.u1 - fit.u0, 9.0f / 16.0f, 1e-4f);  // largeur visible = 10/(16/9) / 10
}

/**
 * @brief Une image plus haute que la cible (ratio inférieur) est recadrée verticalement, centrée,
 * la largeur restant pleine.
 * \castest{<b>Image plus haute que la cible : recadrage vertical centre.</b><br/>
 * \tcat Unitaire · Background Fit<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Calculer le recadrage d'une image 9:16 sur une cible carree.<br/>
 * \tattendu La largeur est pleine (u0=0, u1=1) et la hauteur est recadree, centree.
 * }
 */
TEST(BackgroundFitTest, ImagePlusHauteRecadreeVerticalementEtCentree) {
    // Image 9:16 (ratio 0.5625) sur une cible carree (ratio 1.0) : imageAspect < targetAspect.
    const hmi::BackgroundFit fit = hmi::computeBackgroundFit(1080.0f, 1920.0f, 10.0f, 10.0f);

    EXPECT_FLOAT_EQ(fit.u0, 0.0f);
    EXPECT_FLOAT_EQ(fit.u1, 1.0f);
    EXPECT_GT(fit.v0, 0.0f);
    EXPECT_LT(fit.v1, 1.0f);
    EXPECT_FLOAT_EQ(fit.v0, 1.0f - fit.v1);
    EXPECT_NEAR(fit.v1 - fit.v0, 9.0f / 16.0f, 1e-4f);
}

/**
 * @brief Sans fond désigné (texture absente), aucun quad n'est composé, quel que soit le mode.
 * \castest{<b>Sans fond designe, aucun quad n'est compose.</b><br/>
 * \tcat Unitaire · Background Fit<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Composer une scene sans texture de fond, en mode Texture.<br/>
 * \tattendu Aucun quad n'est ajoute a la scene (etat normal, pas une anomalie).
 * }
 */
TEST(BackgroundFitTest, SansFondDesigneAucunQuadCompose) {
    hmi::ComposedScene scene;
    hmi::composeBackground(scene, hmi::BackgroundTexture{}, 20, 12, hmi::RenderMode::Texture);

    EXPECT_EQ(scene.size(), 0u);
}

/**
 * @brief Un fond désigné et résolu est composé en un quad couvrant les bornes du niveau, sur le
 * calque Background, en mode Texture.
 * \castest{<b>Fond designe et resolu : quad emis sur Background, bornes du niveau.</b><br/>
 * \tcat Unitaire · Background Fit<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Composer une scene avec une texture de fond resolue, en mode Texture.<br/>
 * \tattendu Un quad est emis sur RenderLayer::Background, cadre sur (0,0)-(largeur,hauteur).
 * }
 */
TEST(BackgroundFitTest, FondResoluEmetUnQuadSurLesBornesDuNiveau) {
    hmi::ComposedScene scene;
    const hmi::BackgroundTexture background{texture, 64, 64};

    hmi::composeBackground(scene, background, 20, 12, hmi::RenderMode::Texture);

    hmi::QuadRecorder recorder;
    recorder.record(scene);
    ASSERT_EQ(recorder.size(), 1u);
    EXPECT_EQ(recorder.countOnLayer(hmi::RenderLayer::Background), 1);
    EXPECT_TRUE(recorder.containsSpriteAt(0.0f, 0.0f)) << recorder.describe();

    const hmi::ComposedQuad& quad = recorder.quads().front();
    EXPECT_FLOAT_EQ(quad.sprite.width, 20.0f);
    EXPECT_FLOAT_EQ(quad.sprite.height, 12.0f);
}

/**
 * @brief Un même fond résolu ne produit aucun quad en mode Physique : la lecture des collisions
 * reste sans distraction (`EX-REN-046`).
 * \castest{<b>Mode Physique : aucun quad de fond, meme fond resolu.</b><br/>
 * \tcat Unitaire · Background Fit<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Composer la meme scene qu'en mode Texture, mais en mode Physique.<br/>
 * \tattendu Aucun quad n'est ajoute a la scene.
 * }
 */
TEST(BackgroundFitTest, ModePhysiqueAucunQuadDeFond) {
    hmi::ComposedScene scene;
    const hmi::BackgroundTexture background{texture, 64, 64};

    hmi::composeBackground(scene, background, 20, 12, hmi::RenderMode::Physique);

    EXPECT_EQ(scene.size(), 0u);
}

/**
 * @brief Le repli en damier (texture résolue substituée, mêmes dimensions logiques) produit un
 * quad identique à un fond valide : la composition ne distingue pas les deux cas, c'est
 * `resolveBackgroundTexture` qui a déjà tranché en amont (`EX-NFR-040`).
 * \castest{<b>Repli en damier : meme composition qu'un fond valide.</b><br/>
 * \tcat Unitaire · Background Fit<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Composer une scene avec une texture de repli (damier), en mode Texture.<br/>
 * \tattendu Un quad est emis sur RenderLayer::Background, comme pour un fond valide.
 * }
 */
TEST(BackgroundFitTest, RepliEnDamierEmetUnQuadCommeUnFondValide) {
    hmi::ComposedScene scene;
    // Le damier de repli (hmi::MissingTexture) a ses propres dimensions ; seule l'identite de
    // texture et les dimensions different d'un fond reel, la composition est identique.
    const hmi::BackgroundTexture placeholder{texture, 16, 16};

    hmi::composeBackground(scene, placeholder, 20, 12, hmi::RenderMode::Texture);

    EXPECT_EQ(scene.size(), 1u);
    EXPECT_EQ(scene.statistics().submitted, 1);
}
