// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

/**
 * @file test_screen_shake.cpp
 * @brief Tests unitaires de la secousse d'écran (`LOT-53` TACHE-03, `EX-REN-008`).
 */

#include <gtest/gtest.h>

#include "Core/Math/Vector2.h"
#include "HMI/Graphics/Camera2D.h"

namespace {
constexpr int WIDTH = 800;
constexpr int HEIGHT = 600;
}  // namespace

/**
 * @brief Une secousse jamais déclenchée (état par défaut) n'a aucun décalage.
 * \castest{<b>État par défaut : aucun décalage.</b><br/>
 * \tcat Unitaire · Secousse d'écran<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire un `hmi::ScreenShakeState` par défaut.<br/>2. Lire son décalage.<br/>
 * \tattendu Décalage nul sur les deux axes.
 * }
 */
TEST(ScreenShakeTest, EtatParDefautAucunDecalage) {
    const hmi::ScreenShakeState state;
    const core::Vector2 offset = hmi::screenShakeOffset(state);
    EXPECT_FLOAT_EQ(offset.x, 0.0f);
    EXPECT_FLOAT_EQ(offset.y, 0.0f);
}

/**
 * @brief Le décalage décroît linéairement de l'amplitude à zéro sur la durée prévue, et reste nul
 * une fois la durée écoulée (jamais avant, jamais après).
 * \castest{<b>Décroissance jusqu'à zéro dans la durée prévue.</b><br/>
 * \tcat Unitaire · Secousse d'écran<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Déclencher une secousse (amplitude 4, durée 0,2 s).<br/>2. Lire le décalage au
 * déclenchement, à mi-durée, à la durée exacte, puis bien au-delà.<br/>
 * \tattendu Amplitude pleine au déclenchement ; moitié à mi-durée ; nul à la durée exacte et
 * au-delà (jamais négatif).
 * }
 */
TEST(ScreenShakeTest, DecroitLineairementJusquAZeroALaDureePrevue) {
    hmi::ScreenShakeState state;
    hmi::triggerScreenShake(state, 4.0f, 0.2f);
    EXPECT_FLOAT_EQ(hmi::screenShakeOffset(state).y, 4.0f);

    hmi::advanceScreenShake(state, 0.1f);  // mi-duree
    EXPECT_FLOAT_EQ(hmi::screenShakeOffset(state).y, 2.0f);

    hmi::advanceScreenShake(state, 0.1f);  // duree exacte ecoulee
    EXPECT_FLOAT_EQ(hmi::screenShakeOffset(state).y, 0.0f);

    hmi::advanceScreenShake(state, 0.5f);  // bien au-dela : reste nul
    EXPECT_FLOAT_EQ(hmi::screenShakeOffset(state).y, 0.0f);
}

/**
 * @brief Le décalage est arrondi au pixel écran entier (`EX-ARCH-022`), même quand l'amplitude et
 * la fraction de durée écoulée produisent une valeur fractionnaire.
 * \castest{<b>Décalage arrondi au pixel entier.</b><br/>
 * \tcat Unitaire · Secousse d'écran<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Déclencher une secousse (amplitude 5, durée 0,3 s).<br/>2. Avancer d'un tiers de la
 * durée (décalage brut 3,33).<br/>
 * \tattendu Le décalage lu est l'entier arrondi (3), pas la valeur brute.
 * }
 */
TEST(ScreenShakeTest, DecalageArrondiAuPixelEntier) {
    hmi::ScreenShakeState state;
    hmi::triggerScreenShake(state, 5.0f, 0.3f);
    hmi::advanceScreenShake(state, 0.1f);  // 1/3 de la duree ecoulee -> brut = 5 * 2/3 = 3.33...

    EXPECT_FLOAT_EQ(hmi::screenShakeOffset(state).y, 3.0f);
}

/**
 * @brief La secousse ne décale jamais l'axe horizontal (discrétion, cf. `epic.md`).
 * \castest{<b>Aucun décalage horizontal.</b><br/>
 * \tcat Unitaire · Secousse d'écran<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Déclencher une secousse.<br/>2. Lire le décalage à plusieurs instants.<br/>
 * \tattendu `offset.x == 0` en toute circonstance.
 * }
 */
TEST(ScreenShakeTest, AucunDecalageHorizontal) {
    hmi::ScreenShakeState state;
    hmi::triggerScreenShake(state, 4.0f, 0.2f);
    EXPECT_FLOAT_EQ(hmi::screenShakeOffset(state).x, 0.0f);
    hmi::advanceScreenShake(state, 0.1f);
    EXPECT_FLOAT_EQ(hmi::screenShakeOffset(state).x, 0.0f);
}

/**
 * @brief La secousse d'écran ne modifie jamais le centre ni le cadrage visible de la caméra :
 * `setShakeOffsetPixels` n'affecte que la projection de rendu, jamais `_center`, donc ne peut
 * structurellement pas provoquer de bascule de salle (pilotée par la position du personnage, pas
 * par la caméra) ni fausser le culling.
 * \castest{<b>La secousse ne modifie ni le centre ni le cadrage visible.</b><br/>
 * \tcat Unitaire · Secousse d'écran<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Poser un centre/zoom sur une caméra, noter `center()`/`visibleBounds()`.<br/>
 * 2. Appliquer un décalage de secousse non nul.<br/>
 * \tattendu `center()`, `zoom()` et `visibleBounds()` restent strictement inchangés.
 * }
 */
TEST(ScreenShakeTest, NeModifieNiLeCentreNiLeCadrageVisible) {
    hmi::Camera2D camera(WIDTH, HEIGHT);
    camera.setCenter(core::Vector2{12.0f, 7.0f});
    camera.setZoom(2.0f);
    const core::Vector2 centerBefore = camera.center();
    const core::Rect boundsBefore = camera.visibleBounds();

    camera.setShakeOffsetPixels(core::Vector2{0.0f, 6.0f});

    EXPECT_FLOAT_EQ(camera.center().x, centerBefore.x);
    EXPECT_FLOAT_EQ(camera.center().y, centerBefore.y);
    EXPECT_FLOAT_EQ(camera.zoom(), 2.0f);
    const core::Rect boundsAfter = camera.visibleBounds();
    EXPECT_FLOAT_EQ(boundsAfter.position.x, boundsBefore.position.x);
    EXPECT_FLOAT_EQ(boundsAfter.position.y, boundsBefore.position.y);
    EXPECT_FLOAT_EQ(boundsAfter.size.x, boundsBefore.size.x);
    EXPECT_FLOAT_EQ(boundsAfter.size.y, boundsBefore.size.y);
}
