// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

/**
 * @file test_parallax.cpp
 * @brief Tests unitaires du décalage de parallaxe, de son bornage et de sa table d'activation
 *        (LOT-49 TACHE-03, généralisé au LOT-69 TACHE-06 : facteur **par axe**, porté par le plan).
 */

#include <cmath>

#include <gtest/gtest.h>

#include "Core/Levels/CameraFraming.h"
#include "Core/Math/Rect.h"
#include "Core/Math/Vector2.h"
#include "HMI/Graphics/Camera2D.h"
#include "HMI/Graphics/Parallax.h"

namespace {

/// Facteur neutre sur les deux axes.
const core::Vector2 NEUTRAL{hmi::PARALLAX_FACTOR_NONE, hmi::PARALLAX_FACTOR_NONE};

}  // namespace

/**
 * @brief À facteur `1.0`, la position de rendu est strictement inchangée, quel que soit le
 * cadrage de la caméra.
 * \castest{<b>Facteur 1 laisse la position inchangee.</b><br/>
 * \tcat Unitaire · Parallax<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Calculer la position de rendu a facteur 1.0 sur les deux axes.<br/>
 * \tattendu La position renvoyee est identique a la position modele.
 * }
 */
TEST(ParallaxTest, FacteurUnLaissePositionInchangee) {
    const core::Rect bounds{core::Vector2{40.0f, 45.0f}, core::Vector2{20.0f, 10.0f}};
    const core::Vector2 position{12.0f, 7.5f};

    const core::Vector2 rendered = hmi::parallaxRenderPosition(position, NEUTRAL, bounds);

    EXPECT_FLOAT_EQ(rendered.x, position.x);
    EXPECT_FLOAT_EQ(rendered.y, position.y);
}

/**
 * @brief Le facteur est **par axe** : un plan peut défiler horizontalement sans bouger
 * verticalement, ce qu'un facteur unique ne pouvait pas exprimer.
 * \castest{<b>Le facteur de parallaxe agit axe par axe.</b><br/>
 * \tcat Unitaire · Parallax<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Calculer la position de rendu avec un facteur different sur X et sur Y.<br/>
 * \tattendu X est decale, Y reste inchange.
 * }
 */
TEST(ParallaxTest, FacteurParAxeAgitIndependamment) {
    const core::Rect bounds{core::Vector2{0.0f, 0.0f}, core::Vector2{100.0f, 100.0f}};
    const core::Vector2 position{80.0f, 80.0f};  // centre du cadrage : (50, 50)

    const core::Vector2 rendered =
        hmi::parallaxRenderPosition(position, core::Vector2{0.5f, 1.0f}, bounds);

    EXPECT_FLOAT_EQ(rendered.x, 65.0f);  // 50 + (80 - 50) * 0,5
    EXPECT_FLOAT_EQ(rendered.y, position.y);
}

/**
 * @brief Deux facteurs équidistants de `1.0` produisent des décalages de sens opposés et de même
 * amplitude (proportionnalité au facteur).
 * \castest{<b>Des facteurs opposes autour de 1 produisent des decalages opposes.</b><br/>
 * \tcat Unitaire · Parallax<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Calculer le decalage pour un facteur inferieur et un facteur superieur a 1,
 * equidistants.<br/>
 * \tattendu Les deux decalages sont de signe oppose et de meme amplitude.
 * }
 */
TEST(ParallaxTest, FacteursOpposesProduisentDesDecalagesOpposesEtProportionnels) {
    const core::Rect bounds{core::Vector2{0.0f, 0.0f}, core::Vector2{100.0f, 100.0f}};
    const core::Vector2 position{80.0f, 50.0f};

    const core::Vector2 lower =
        hmi::parallaxRenderPosition(position, core::Vector2{0.5f, 0.5f}, bounds);
    const core::Vector2 higher =
        hmi::parallaxRenderPosition(position, core::Vector2{1.5f, 1.5f}, bounds);

    const float offsetLower = lower.x - position.x;
    const float offsetHigher = higher.x - position.x;
    EXPECT_FLOAT_EQ(offsetLower, -offsetHigher);
    EXPECT_NE(offsetLower, 0.0f);
}

/**
 * @brief Le décalage se calcule depuis le centre de la salle **courante** : un contenu à la même
 * position relative dans deux salles différentes se retrouve au même endroit à l'écran — c'est le
 * test qui verrouille l'arbitrage (`decors.md`).
 * \castest{<b>Meme position relative -> meme position ecran dans deux salles.</b><br/>
 * \tcat Unitaire · Parallax<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Placer un point a la meme position RELATIVE dans deux salles de centres
 * differents.<br/>2. Comparer leurs positions de rendu, ramenees au centre de leur salle.<br/>
 * \tattendu Les deux ecarts au centre sont identiques.
 * }
 */
TEST(ParallaxTest, MemePositionRelativeDonneMemePositionEcranEntreSalles) {
    const core::Vector2 relativeOffset{4.0f, -2.0f};
    const core::Vector2 factor{0.5f, 0.5f};

    const core::Rect room1{core::Vector2{0.0f, 0.0f}, core::Vector2{20.0f, 10.0f}};
    const core::Vector2 room1Center = room1.position + room1.size * 0.5f;
    const core::Vector2 position1 = room1Center + relativeOffset;

    const core::Rect room2{core::Vector2{120.0f, 60.0f}, core::Vector2{20.0f, 10.0f}};
    const core::Vector2 room2Center = room2.position + room2.size * 0.5f;
    const core::Vector2 position2 = room2Center + relativeOffset;

    const core::Vector2 rendered1 = hmi::parallaxRenderPosition(position1, factor, room1);
    const core::Vector2 rendered2 = hmi::parallaxRenderPosition(position2, factor, room2);

    EXPECT_FLOAT_EQ(rendered1.x - room1Center.x, rendered2.x - room2Center.x);
    EXPECT_FLOAT_EQ(rendered1.y - room1Center.y, rendered2.y - room2Center.y);
}

/**
 * @brief `parallaxModelPosition` est l'inverse **exact** de `parallaxRenderPosition`.
 * \castest{<b>parallaxModelPosition est l'inverse de parallaxRenderPosition.</b><br/>
 * \tcat Unitaire · Parallax<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Convertir une position modele en position de rendu, puis revenir.<br/>
 * \tattendu La position de depart est retrouvee sur les deux axes.
 * }
 */
TEST(ParallaxTest, ParallaxModelPositionEstLInverseDeParallaxRenderPosition) {
    const core::Rect bounds{core::Vector2{10.0f, 20.0f}, core::Vector2{20.0f, 10.0f}};
    const core::Vector2 factor{0.5f, 1.25f};
    const core::Vector2 model{17.0f, 23.5f};

    const core::Vector2 rendered = hmi::parallaxRenderPosition(model, factor, bounds);
    const core::Vector2 back = hmi::parallaxModelPosition(rendered, factor, bounds);

    EXPECT_FLOAT_EQ(back.x, model.x);
    EXPECT_FLOAT_EQ(back.y, model.y);
}

/**
 * @brief Un plan à facteur `0,5`, caméra au **bord** du niveau, couvre toujours le cadrage : le
 * décalage est borné avant d'être appliqué.
 *
 * Sans ce bornage, une bande vide apparaîtrait au bout de la course — le défaut ne se verrait
 * qu'au bord d'un niveau, en jeu, et pas une seule fois pendant l'édition.
 * \castest{<b>Un plan borne couvre toujours le cadrage, camera au bord du niveau.</b><br/>
 * \tcat Unitaire · Parallax<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Placer la camera contre le bord droit d'un niveau et calculer le decalage d'un plan
 * a facteur 0,1.<br/>2. Borner ce decalage.<br/>
 * \tattendu Le plan decale contient encore entierement le cadrage.
 * }
 */
TEST(ParallaxTest, DecalageBorneLaisseLePlanCouvrantAuBordDuNiveau) {
    const core::Rect plane{core::Vector2{0.0f, 0.0f}, core::Vector2{60.0f, 30.0f}};
    // Camera collee au bord droit/bas du niveau : c'est la que le bord du plan se decouvrirait.
    const core::Rect camera{core::Vector2{40.0f, 20.0f}, core::Vector2{20.0f, 10.0f}};
    // Facteur tres lointain (0,1) : c'est la que la course depasse ce que le plan peut couvrir.
    // A 0,5, le decalage reste dans les bornes -- le bornage ne mord pas, et ne doit pas mordre.
    const core::Vector2 factor{0.1f, 0.1f};

    const core::Vector2 origin{0.0f, 0.0f};
    const core::Vector2 rendered = hmi::parallaxRenderPosition(origin, factor, camera);
    const core::Vector2 clamped = hmi::clampPlaneOffset(rendered - origin, plane, camera);

    // Le decalage brut decouvrirait le bord ; le bornage le ramene a ce qui reste couvrant.
    EXPECT_NE(rendered.x - origin.x, clamped.x);

    const float left = plane.position.x + clamped.x;
    const float top = plane.position.y + clamped.y;
    EXPECT_LE(left, camera.position.x);
    EXPECT_GE(left + plane.size.x, camera.position.x + camera.size.x);
    EXPECT_LE(top, camera.position.y);
    EXPECT_GE(top + plane.size.y, camera.position.y + camera.size.y);
}

/**
 * @brief Un plan plus **petit** que le cadrage ne peut être couvrant : son décalage est ramené à
 * zéro plutôt que de le laisser dériver.
 * \castest{<b>Un plan plus petit que le cadrage n'est pas decale.</b><br/>
 * \tcat Unitaire · Parallax<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Borner un decalage pour un plan plus petit que le cadrage.<br/>
 * \tattendu Le decalage vaut zero sur les deux axes.
 * }
 */
TEST(ParallaxTest, PlanPlusPetitQueLeCadrageNEstPasDecale) {
    const core::Rect plane{core::Vector2{0.0f, 0.0f}, core::Vector2{5.0f, 5.0f}};
    const core::Rect camera{core::Vector2{0.0f, 0.0f}, core::Vector2{20.0f, 10.0f}};

    const core::Vector2 clamped = hmi::clampPlaneOffset(core::Vector2{7.0f, -3.0f}, plane, camera);

    EXPECT_FLOAT_EQ(clamped.x, 0.0f);
    EXPECT_FLOAT_EQ(clamped.y, 0.0f);
}

/**
 * @brief L'arrondi au pixel écran s'applique **après** le bornage : un décalage borné reste
 * fractionnaire, et c'est lui qu'il faut aligner.
 * \castest{<b>roundToScreenPixel arrondit au pixel ecran le plus proche.</b><br/>
 * \tcat Unitaire · Parallax<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Arrondir une position fractionnaire a une echelle de 16 px par unite.<br/>2.
 * Recommencer a echelle nulle.<br/>
 * \tattendu La position est alignee au pixel ; a echelle nulle, elle est renvoyee inchangee.
 * }
 */
TEST(ParallaxTest, RoundToScreenPixelArrondiAuPixelEcran) {
    constexpr float SCALE = hmi::Camera2D::PIXELS_PER_UNIT;  // 16 px par unite, zoom 1

    const core::Vector2 rounded = hmi::roundToScreenPixel(core::Vector2{1.03f, -2.97f}, SCALE);

    EXPECT_FLOAT_EQ(rounded.x * SCALE, std::round(1.03f * SCALE));
    EXPECT_FLOAT_EQ(rounded.y * SCALE, std::round(-2.97f * SCALE));

    // Echelle nulle ou negative : position renvoyee telle quelle (robustesse).
    const core::Vector2 untouched = hmi::roundToScreenPixel(core::Vector2{1.03f, -2.97f}, 0.0f);
    EXPECT_FLOAT_EQ(untouched.x, 1.03f);
    EXPECT_FLOAT_EQ(untouched.y, -2.97f);
}

/**
 * @brief La table d'activation est **exhaustive** : trois modes de cadrage × deux valeurs du
 * drapeau de niveau, aucune combinaison laissée à l'interprétation.
 *
 * Le mode *suivi* y est **actif**, ce qui inverse la décision du `LOT-64` — assumé : celui-ci avait
 * coupé la parallaxe parce qu'un décor est un objet collé au contenu et paraissait « suivre » la
 * caméra ; un plan est un fond, l'argument tombe.
 * \castest{<b>La table d'activation de la parallaxe est exhaustive.</b><br/>
 * \tcat Unitaire · Parallax<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Interroger planeParallaxActive pour les trois modes de cadrage, drapeau actif puis
 * inactif.<br/>
 * \tattendu Actif en suivi et par salle, neutralise en niveau entier ; toujours inactif si le
 * drapeau du niveau est faux.
 * }
 */
TEST(ParallaxTest, TableDActivationExhaustive) {
    using core::CameraFramingMode;

    // Drapeau du niveau actif : seul le cadrage niveau entier neutralise.
    EXPECT_TRUE(hmi::planeParallaxActive(CameraFramingMode::Follow, true));
    EXPECT_TRUE(hmi::planeParallaxActive(CameraFramingMode::PerRoom, true));
    EXPECT_FALSE(hmi::planeParallaxActive(CameraFramingMode::WholeLevel, true));

    // Drapeau du niveau inactif : aucun mode ne la reactive.
    EXPECT_FALSE(hmi::planeParallaxActive(CameraFramingMode::Follow, false));
    EXPECT_FALSE(hmi::planeParallaxActive(CameraFramingMode::PerRoom, false));
    EXPECT_FALSE(hmi::planeParallaxActive(CameraFramingMode::WholeLevel, false));
}
