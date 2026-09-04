// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_follow_camera.cpp
 * @brief Tests unitaires de la caméra de suivi : zone morte, anticipation, lissage, bornage
 *        (`EX-REN-016`, LOT-64 TACHE-02).
 */

#include <vector>

#include <gtest/gtest.h>

#include "Core/Math/Rect.h"
#include "Core/Math/Vector2.h"
#include "HMI/Graphics/FollowCamera.h"

namespace {

// Niveau de reference : 40x20 unites, cadrage de suivi de 24x14 unites (memes proportions que le
// cadrage par defaut du mode par salle) -- assez grand pour borner sur les quatre bords sans que
// le niveau soit plus etroit que le cadrage.
const core::Rect LARGE_LEVEL{core::Vector2{0.0f, 0.0f}, core::Vector2{40.0f, 20.0f}};
const core::Vector2 VIEW_HALF_EXTENT{12.0f, 7.0f};
// Directions de marche des scenarios, en vecteurs depuis le LOT-07 : la camera anticipe
// desormais sur les deux axes, pas seulement a gauche et a droite.
const core::Vector2 RIGHT{1.0f, 0.0f};
const core::Vector2 LEFT{-1.0f, 0.0f};

constexpr float STEP = 1.0f / 60.0f;

}  // namespace

/**
 * @brief Le premier appel démarre la caméra directement sur le personnage (borné), sans lissage
 * depuis une position par défaut arbitraire.
 * \castest{<b>Le premier appel démarre la caméra sur le personnage.</b><br/>
 * \tcat Unitaire · Follow Camera<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Avancer un état non initialisé pour un personnage loin des bords.<br/>2. Vérifier le
 * centre résultant.<br/>
 * \tattendu Le centre résultant est exactement la position du personnage.
 * }
 */
TEST(FollowCameraTest, PremierAppelDemarreSurLePersonnage) {
    const hmi::FollowCameraState initial{};
    const core::Vector2 characterPosition{20.0f, 10.0f};
    const hmi::FollowCameraState result = hmi::advanceFollowCamera(
        initial, characterPosition, core::Vector2{}, LARGE_LEVEL, VIEW_HALF_EXTENT, STEP);

    EXPECT_TRUE(result.initialized);
    EXPECT_FLOAT_EQ(result.center.x, characterPosition.x);
    EXPECT_FLOAT_EQ(result.center.y, characterPosition.y);
}

/**
 * @brief Un déplacement à l'intérieur de la zone morte ne déplace pas la caméra ; en sortir la
 * déplace.
 * \castest{<b>La zone morte immobilise la caméra tant que le personnage y reste.</b><br/>
 * \tcat Unitaire · Follow Camera<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Initialiser la caméra sur le personnage.<br/>2. Déplacer le personnage d'une
 * distance inférieure à la zone morte.<br/>3. Le déplacer d'une distance qui en sort.<br/>
 * \tattendu Le déplacement interne à la zone morte ne change ni l'ancre ni le centre visé ; sortir
 * de la zone morte déplace l'ancre.
 * }
 */
TEST(FollowCameraTest, ZoneMorteImmobiliseLaCameraTantQueLePersonnageYReste) {
    const core::Vector2 start{20.0f, 10.0f};
    hmi::FollowCameraState state =
        hmi::advanceFollowCamera({}, start, core::Vector2{}, LARGE_LEVEL, VIEW_HALF_EXTENT, STEP);
    const core::Vector2 anchorAfterInit = state.anchor;

    // Deplacement interne a la zone morte (demi-largeur 1.5) : ne doit PAS deplacer l'ancre.
    const core::Vector2 smallMove{start.x + 0.5f, start.y};
    state = hmi::advanceFollowCamera(state, smallMove, core::Vector2{}, LARGE_LEVEL,
                                     VIEW_HALF_EXTENT, STEP);
    EXPECT_FLOAT_EQ(state.anchor.x, anchorAfterInit.x);
    EXPECT_FLOAT_EQ(state.anchor.y, anchorAfterInit.y);

    // Deplacement qui sort de la zone morte : doit deplacer l'ancre.
    const core::Vector2 bigMove{start.x + 5.0f, start.y};
    state = hmi::advanceFollowCamera(state, bigMove, core::Vector2{}, LARGE_LEVEL, VIEW_HALF_EXTENT,
                                     STEP);
    EXPECT_NE(state.anchor.x, anchorAfterInit.x);
}

/**
 * @brief Sur un niveau plus grand que le cadrage, la caméra ne montre jamais au-delà des limites,
 * sur les quatre bords.
 * \castest{<b>Le bornage empêche de montrer hors des limites du niveau, sur les quatre bords.</b>
 * <br/>
 * \tcat Unitaire · Follow Camera<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Amener le personnage successivement dans chaque coin du niveau.<br/>2. Avancer
 * suffisamment de pas pour que la caméra rattrape sa cible.<br/>
 * \tattendu Le centre reste toujours à au moins une demi-largeur/demi-hauteur de cadrage des
 * bords du niveau, sur les quatre côtés.
 * }
 */
TEST(FollowCameraTest, BornageEmpecheDeMontrerHorsDesLimitesSurLesQuatreBords) {
    const std::vector<core::Vector2> corners = {
        {0.5f, 0.5f}, {39.5f, 0.5f}, {0.5f, 19.5f}, {39.5f, 19.5f}};

    for (const core::Vector2& corner : corners) {
        hmi::FollowCameraState state{};
        for (int step = 0; step < 300; ++step) {
            state = hmi::advanceFollowCamera(state, corner, core::Vector2{}, LARGE_LEVEL,
                                             VIEW_HALF_EXTENT, STEP);
        }
        EXPECT_GE(state.center.x, LARGE_LEVEL.position.x + VIEW_HALF_EXTENT.x - 1e-3f);
        EXPECT_LE(state.center.x,
                  LARGE_LEVEL.position.x + LARGE_LEVEL.size.x - VIEW_HALF_EXTENT.x + 1e-3f);
        EXPECT_GE(state.center.y, LARGE_LEVEL.position.y + VIEW_HALF_EXTENT.y - 1e-3f);
        EXPECT_LE(state.center.y,
                  LARGE_LEVEL.position.y + LARGE_LEVEL.size.y - VIEW_HALF_EXTENT.y + 1e-3f);
    }
}

/**
 * @brief Un niveau plus étroit que le cadrage sur un axe centre la caméra sur cet axe, plutôt que
 * de la coller à un bord.
 * \castest{<b>Un axe plus étroit que le cadrage centre la caméra, sans la coller à un bord.</b>
 * <br/>
 * \tcat Unitaire · Follow Camera<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire un niveau plus étroit que le cadrage sur l'axe horizontal.<br/>2. Amener
 * le personnage tout contre le bord gauche puis avancer suffisamment de pas.<br/>
 * \tattendu Le centre horizontal reste au milieu du niveau, jamais collé au bord.
 * }
 */
TEST(FollowCameraTest, NiveauPlusEtroitQueLeCadrageCentreLaCameraSurCetAxe) {
    const core::Rect narrowLevel{core::Vector2{0.0f, 0.0f}, core::Vector2{10.0f, 20.0f}};
    hmi::FollowCameraState state{};
    for (int step = 0; step < 300; ++step) {
        state = hmi::advanceFollowCamera(state, core::Vector2{0.2f, 10.0f}, LEFT, narrowLevel,
                                         VIEW_HALF_EXTENT, STEP);
    }
    EXPECT_NEAR(state.center.x, narrowLevel.size.x * 0.5f, 1e-3f);
}

/**
 * @brief L'anticipation s'inverse progressivement au changement de sens : après un pas dans le
 * nouveau sens, elle n'a pas encore atteint la valeur opposée -- pas de discontinuité.
 * \castest{<b>L'anticipation s'inverse progressivement, sans discontinuité.</b><br/>
 * \tcat Unitaire · Follow Camera<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Avancer de nombreux pas vers la droite (anticipation proche de +1).<br/>2. Inverser
 * le sens d'un seul pas.<br/>
 * \tattendu L'anticipation reste strictement positive juste après l'inversion (elle a commencé à
 * revenir, mais n'a pas sauté à -1).
 * }
 */
TEST(FollowCameraTest, AnticipationSInverseProgressivementSansDiscontinuite) {
    hmi::FollowCameraState state{};
    core::Vector2 position{20.0f, 10.0f};
    for (int step = 0; step < 120; ++step) {
        position.x += 0.05f;
        state =
            hmi::advanceFollowCamera(state, position, RIGHT, LARGE_LEVEL, VIEW_HALF_EXTENT, STEP);
    }
    ASSERT_GT(state.anticipation.x, 0.9f);  // s'est bien etablie pres de +1

    const hmi::FollowCameraState afterReversal =
        hmi::advanceFollowCamera(state, position, LEFT, LARGE_LEVEL, VIEW_HALF_EXTENT, STEP);
    EXPECT_LT(afterReversal.anticipation.x, state.anticipation.x);  // a commence a baisser
    EXPECT_GT(afterReversal.anticipation.x, 0.0f);  // mais n'a pas saute a une valeur negative
}

/**
 * @brief À l'arrêt (sens nul), l'anticipation conserve sa dernière valeur plutôt que de revenir à
 * zéro.
 * \castest{<b>À l'arrêt, l'anticipation conserve sa dernière valeur.</b><br/>
 * \tcat Unitaire · Follow Camera<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Établir une anticipation vers la droite.<br/>2. Avancer un pas avec un sens
 * nul (personnage immobile).<br/>
 * \tattendu L'anticipation reste inchangée.
 * }
 */
TEST(FollowCameraTest, ArretConserveLaDerniereAnticipation) {
    hmi::FollowCameraState state{};
    core::Vector2 position{20.0f, 10.0f};
    for (int step = 0; step < 120; ++step) {
        position.x += 0.05f;
        state =
            hmi::advanceFollowCamera(state, position, RIGHT, LARGE_LEVEL, VIEW_HALF_EXTENT, STEP);
    }
    const float anticipationBeforeStop = state.anticipation.x;

    const hmi::FollowCameraState stopped = hmi::advanceFollowCamera(
        state, position, core::Vector2{}, LARGE_LEVEL, VIEW_HALF_EXTENT, STEP);
    EXPECT_FLOAT_EQ(stopped.anticipation.x, anticipationBeforeStop);
}

/**
 * @brief Mêmes entrées, même trajectoire de caméra (déterminisme, `EX-NFR-002`).
 * \castest{<b>Mêmes entrées, même trajectoire de caméra.</b><br/>
 * \tcat Unitaire · Follow Camera<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Rejouer deux fois la même séquence de positions depuis un état vierge.<br/>2.
 * Comparer les états finaux.<br/>
 * \tattendu Les deux trajectoires produisent exactement le même état final.
 * }
 */
TEST(FollowCameraTest, MemesEntreesMemeTrajectoire) {
    const auto replay = []() {
        hmi::FollowCameraState state{};
        core::Vector2 position{5.0f, 5.0f};
        for (int step = 0; step < 50; ++step) {
            position.x += 0.1f;
            state = hmi::advanceFollowCamera(state, position, RIGHT, LARGE_LEVEL, VIEW_HALF_EXTENT,
                                             STEP);
        }
        return state;
    };

    const hmi::FollowCameraState first = replay();
    const hmi::FollowCameraState second = replay();
    EXPECT_FLOAT_EQ(first.center.x, second.center.x);
    EXPECT_FLOAT_EQ(first.center.y, second.center.y);
    EXPECT_FLOAT_EQ(first.anticipation.x, second.anticipation.x);
}

/**
 * @brief Le suivi est **isotrope** : marcher vers le bas décale la caméra exactement autant que
 * marcher vers la droite (`EX-EXP-001`, `LOT-07`).
 *
 * La zone morte du jeu de plateforme était plus large que haute, et l'anticipation purement
 * horizontale : deux biais qui n'ont plus lieu d'être dès lors qu'aucun axe n'est privilégié.
 * \castest{<b>Le suivi est isotrope : haut, bas, gauche et droite se valent.</b><br/>
 * \tcat Unitaire · Camera de suivi<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Faire marcher le personnage vers la droite sur 60 pas, camera suivie.<br/>2.
 * Recommencer vers le bas.<br/>
 * \tattendu Les deux deplacements de camera ont la meme amplitude.
 * }
 */
TEST(FollowCameraTest, SuiviIsotropeSurLesDeuxAxes) {
    // Niveau CARRE et cadrage carre : toute asymetrie observee vient alors de la camera, pas de la
    // geometrie du tableau ni du bornage a ses bords.
    const core::Rect squareLevel{core::Vector2{0.0f, 0.0f}, core::Vector2{60.0f, 60.0f}};
    const core::Vector2 squareView{12.0f, 12.0f};
    const core::Vector2 DOWN{0.0f, 1.0f};
    const core::Vector2 start{30.0f, 30.0f};

    const auto parcourir = [&](core::Vector2 direction) {
        hmi::FollowCameraState state =
            hmi::advanceFollowCamera({}, start, direction, squareLevel, squareView, STEP);
        const core::Vector2 origine = state.center;
        for (int step = 1; step <= 60; ++step) {
            const core::Vector2 position = start + direction * (static_cast<float>(step) * 0.1f);
            state =
                hmi::advanceFollowCamera(state, position, direction, squareLevel, squareView, STEP);
        }
        return (state.center - origine).length();
    };

    const float versLeBas = parcourir(DOWN);
    ASSERT_GT(versLeBas, 0.0f);
    EXPECT_NEAR(versLeBas, parcourir(RIGHT), 1e-4f);
}

/**
 * @brief L'anticipation suit la direction de marche sur les **deux** axes : marcher vers le haut
 * décale la caméra vers le haut (`LOT-07`).
 * \castest{<b>L'anticipation suit la marche verticale, pas seulement l'horizontale.</b><br/>
 * \tcat Unitaire · Camera de suivi<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Faire marcher le personnage vers le haut, sans bouger, sur 60 pas.<br/>
 * \tattendu L'anticipation verticale s'etablit vers le haut, l'horizontale reste nulle.
 * }
 */
TEST(FollowCameraTest, AnticipationVerticale) {
    const core::Vector2 UP{0.0f, -1.0f};
    const core::Rect squareLevel{core::Vector2{0.0f, 0.0f}, core::Vector2{60.0f, 60.0f}};
    const core::Vector2 squareView{12.0f, 12.0f};
    const core::Vector2 position{30.0f, 30.0f};
    hmi::FollowCameraState state =
        hmi::advanceFollowCamera({}, position, UP, squareLevel, squareView, STEP);
    for (int step = 0; step < 60; ++step) {
        state = hmi::advanceFollowCamera(state, position, UP, squareLevel, squareView, STEP);
    }

    EXPECT_LT(state.anticipation.y, -0.9f);
    EXPECT_NEAR(state.anticipation.x, 0.0f, 1e-5f);
    EXPECT_LT(state.center.y, position.y);  // la camera regarde bien devant, vers le haut
}
