// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_parchment_ornaments.cpp
 * @brief Ornements tracés de l'habillage (`LOT-76`, `EX-IHM-075`).
 *
 * Tests **purs** : aucune instance Qt. Ce qui est couvert ici est ce qu'un coup d'œil ne voit pas
 * — un sommet qui sort de sa boîte, une aile qui grandit avec la largeur, un ornement qui se peint
 * encore alors qu'il n'est plus reconnaissable.
 */

#include <algorithm>
#include <vector>

#include <gtest/gtest.h>

#include "HMI/Presentation/ParchmentOrnaments.h"

namespace {

using hmi::bannerWingSpan;
using hmi::cabochonShapes;
using hmi::ORNAMENT_MINIMUM_SIZE;
using hmi::OrnamentPoint;
using hmi::OrnamentRole;
using hmi::OrnamentShape;
using hmi::titleBannerShapes;

/// Boîte englobante d'un jeu de formes.
struct Bounds {
    float left = 0.0f;
    float top = 0.0f;
    float right = 0.0f;
    float bottom = 0.0f;
};

[[nodiscard]] Bounds boundsOf(const std::vector<OrnamentShape>& shapes) {
    Bounds bounds{1e9F, 1e9F, -1e9F, -1e9F};
    for (const OrnamentShape& shape : shapes) {
        for (const OrnamentPoint& point : shape.points) {
            bounds.left = std::min(bounds.left, point.x);
            bounds.top = std::min(bounds.top, point.y);
            bounds.right = std::max(bounds.right, point.x);
            bounds.bottom = std::max(bounds.bottom, point.y);
        }
    }
    return bounds;
}

[[nodiscard]] int countRole(const std::vector<OrnamentShape>& shapes, OrnamentRole role) {
    return static_cast<int>(
        std::count_if(shapes.begin(), shapes.end(),
                      [role](const OrnamentShape& shape) { return shape.role == role; }));
}

// -- Cabochon -------------------------------------------------------------------------------

/**
 * @brief Aucun sommet du cabochon ne sort de son carre englobant, a toutes les tailles utiles.
 * \castest{<b>Le cabochon ne deborde jamais de son carre d'angle.</b><br/>
 * \tcat Unitaire · Ornements traces<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Produire la geometrie pour cinq tailles, de 8 a 96 pixels.<br/>2. Relever la boite
 * englobante de toutes les formes.<br/>
 * \tattendu La boite tient dans [0,taille] x [0,taille] : un sommet qui sort deborderait sur le
 * parchemin, et la pierre se lirait comme une bavure a l'angle du panneau.
 * }
 */
TEST(CabochonTest, ToutTientDansLeCarreEnglobant) {
    for (const int size : {8, 12, 24, 48, 96}) {
        const std::vector<OrnamentShape> shapes = cabochonShapes(size);
        ASSERT_FALSE(shapes.empty()) << size;
        const Bounds bounds = boundsOf(shapes);
        // Un sommet qui sort ne leve aucune erreur : il deborde sur le champ de parchemin, et le
        // cabochon se lit comme une bavure a l'angle du panneau.
        EXPECT_GE(bounds.left, 0.0f) << size;
        EXPECT_GE(bounds.top, 0.0f) << size;
        EXPECT_LE(bounds.right, static_cast<float>(size)) << size;
        EXPECT_LE(bounds.bottom, static_cast<float>(size)) << size;
    }
}

/**
 * @brief Sous la taille minimale, le cabochon n'est plus dessine du tout.
 * \castest{<b>Un cabochon trop petit n'est pas dessine plutot que reduit a une tache.</b><br/>
 * \tcat Unitaire · Ornements traces<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Demander la geometrie pour toutes les tailles de -4 a la taille minimale
 * exclue.<br/>2. Demander la geometrie a la taille minimale.<br/>
 * \tattendu Vide en dessous, non vide a partir du seuil : un serti qu'on ne reconnait pas ne
 * designe rien.
 * }
 */
TEST(CabochonTest, SousLaTailleMinimaleIlNEstPlusDessine) {
    for (int size = -4; size < ORNAMENT_MINIMUM_SIZE; ++size) {
        EXPECT_TRUE(cabochonShapes(size).empty()) << size;
    }
    EXPECT_FALSE(cabochonShapes(ORNAMENT_MINIMUM_SIZE).empty());
}

/**
 * @brief Les facettes de la gemme sont strictement a l'interieur de l'octogone du serti.
 * \castest{<b>La gemme est inscrite dans son serti, sans l'affleurer.</b><br/>
 * \tcat Unitaire · Ornements traces<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Produire un cabochon de 64 pixels.<br/>2. Comparer la boite de chaque facette a celle
 * du serti.<br/>
 * \tattendu Chaque facette est strictement interieure : une gemme qui affleure le bord ne se lit
 * plus comme sertie, mais comme un aplat rouge a angles coupes.
 * }
 */
TEST(CabochonTest, LaGemmeEstInscriteDansLeSerti) {
    const int size = 64;
    const std::vector<OrnamentShape> shapes = cabochonShapes(size);
    const Bounds bezel = boundsOf({shapes.front()});

    // Les facettes de la gemme sont STRICTEMENT a l'interieur du serti : une gemme qui affleure le
    // bord de l'octogone ne se lit plus comme sertie, mais comme un aplat rouge a angles coupes.
    for (const OrnamentShape& shape : shapes) {
        if (shape.role != OrnamentRole::Gem && shape.role != OrnamentRole::GemShadow) {
            continue;
        }
        const Bounds gem = boundsOf({shape});
        EXPECT_GT(gem.left, bezel.left);
        EXPECT_GT(gem.top, bezel.top);
        EXPECT_LT(gem.right, bezel.right);
        EXPECT_LT(gem.bottom, bezel.bottom);
    }
}

/**
 * @brief L'ordre de dessin place le serti en premier et l'eclat en dernier.
 * \castest{<b>L'ordre de dessin du cabochon met l'eclat par-dessus la facette qu'il
 * eclaire.</b><br/>
 * \tcat Unitaire · Ornements traces<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Produire un cabochon de 64 pixels.<br/>2. Lire le role de la premiere et de la
 * derniere forme, et compter les traits d'encre.<br/>
 * \tattendu Serti d'abord, eclat en dernier, un seul trait : un reflet peint avant la facette qu'il
 * eclaire disparait dessous, sans qu'aucune erreur ne le dise.
 * }
 */
TEST(CabochonTest, LEclatEstPoseEnDernier) {
    const std::vector<OrnamentShape> shapes = cabochonShapes(64);
    ASSERT_FALSE(shapes.empty());
    // L'ordre EST le dessin : un reflet peint avant la facette qu'il eclaire disparait dessous,
    // sans qu'aucune erreur ne le dise.
    EXPECT_EQ(shapes.back().role, OrnamentRole::OrnamentLight);
    EXPECT_EQ(shapes.front().role, OrnamentRole::Ornament);
    EXPECT_EQ(countRole(shapes, OrnamentRole::Edge), 1);
}

/**
 * @brief Aucune forme du cabochon n'est degeneree, a aucune taille.
 * \castest{<b>Aucune forme du cabochon n'a moins de trois sommets.</b><br/>
 * \tcat Unitaire · Ornements traces<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Produire la geometrie pour cinq tailles.<br/>2. Compter les sommets de chaque
 * forme.<br/>
 * \tattendu Trois sommets au moins : le peintre saute les formes plus courtes, et l'ornement serait
 * incomplet en silence.
 * }
 */
TEST(CabochonTest, ToutesLesFormesOntAuMoinsTroisSommets) {
    // Le peintre saute les formes de moins de trois sommets ; une forme degeneree ne serait donc
    // pas dessinee, et l'ornement serait incomplet en silence.
    for (const int size : {8, 9, 10, 16, 64}) {
        for (const OrnamentShape& shape : cabochonShapes(size)) {
            EXPECT_GE(shape.points.size(), 3U) << size;
        }
    }
}

// -- Bandeau de titre -----------------------------------------------------------------------

/**
 * @brief L'envergure des ailes du bandeau depend de sa hauteur, et d'elle seule.
 * \castest{<b>Les ailes du bandeau gardent leur envergure quand sa largeur triple.</b><br/>
 * \tcat Unitaire · Ornements traces<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Produire trois bandeaux de meme hauteur et de 600, 900 puis 1400 pixels de
 * large.<br/>2. Mesurer la largeur de l'aile gauche de chacun.<br/>
 * \tattendu La meme envergure dans les trois cas : c'est l'invariant du lot, et exactement ce
 * qu'une image etiree ne sait pas tenir.
 * }
 */
TEST(TitleBannerTest, LEnvergureSuitLaHauteurEtPasLaLargeur) {
    // L'invariant du lot. Une aile qui grandit avec la largeur donne, sur un titre long, deux
    // ailes de chauve-souris demesurees autour d'une plaque restee fine -- et un bandeau court
    // ferait l'inverse. C'est exactement ce qu'une image etiree produit, et ce que le trace evite.
    const int height = 40;
    const float reference = static_cast<float>(bannerWingSpan(height));
    float first = 0.0f;
    for (const int width : {600, 900, 1400}) {
        const std::vector<OrnamentShape> shapes = titleBannerShapes(width, height);
        ASSERT_GE(shapes.size(), 5U) << width;
        const Bounds wing = boundsOf({shapes.front()});
        const float measured = wing.right - wing.left;
        // Comparee a l'envergure annoncee, a la marge pres que laisse la pointe -- le bord
        // d'attaque ne touche pas exactement le bord du bandeau, pour que l'anticrenelage ait de
        // quoi poser sa demi-teinte.
        EXPECT_NEAR(measured, reference, reference * 0.05f) << width;
        if (first == 0.0f) {
            first = measured;
        }
        // Et surtout : identique d'une largeur a l'autre. C'est l'invariant, l'ecart a la valeur
        // annoncee n'en est que la mesure.
        EXPECT_FLOAT_EQ(measured, first) << width;
    }
    EXPECT_GT(bannerWingSpan(80), bannerWingSpan(40));
}

/**
 * @brief Sur un bandeau trop etroit, les ailes se reduisent puis disparaissent ; la plaque, jamais.
 * \castest{<b>Sur un bandeau etroit, ce sont les ailes qui cedent et non la plaque.</b><br/>
 * \tcat Unitaire · Ornements traces<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Produire un bandeau de 60 x 40, puis un de 20 x 40.<br/>2. Compter les formes dorees
 * et mesurer l'aile.<br/>
 * \tattendu A 60 les ailes sont reduites, a 20 elles ont disparu, et la plaque est presente dans
 * les deux cas : c'est elle qui porte le titre.
 * }
 */
TEST(TitleBannerTest, SurUnBandeauEtroitCeSontLesAilesQuiCedent) {
    // 60 px de large pour 40 de haut : l'envergure voulue (50) ne tient pas deux fois. Ce sont les
    // ailes qui se reduisent, jamais la plaque -- une plaque rognee pour loger des ailes entieres
    // ne porterait plus son titre, et c'est le titre qui est la raison du bandeau.
    const std::vector<OrnamentShape> narrow = titleBannerShapes(60, 40);
    ASSERT_FALSE(narrow.empty());
    EXPECT_EQ(countRole(narrow, OrnamentRole::Ornament), 3);  // deux ailes reduites, et le filet
    const Bounds wing = boundsOf({narrow.front()});
    EXPECT_LT(wing.right - wing.left, static_cast<float>(bannerWingSpan(40)));

    // Plus etroit encore, l'aile passe sous la taille minimale : elle n'est plus dessinee du tout.
    // Un moignon de six pixels ne se lit pas comme une aile, il se lit comme une bavure.
    const std::vector<OrnamentShape> tiny = titleBannerShapes(20, 40);
    ASSERT_FALSE(tiny.empty());
    EXPECT_EQ(countRole(tiny, OrnamentRole::Ornament), 1);  // le filet seul
    EXPECT_GE(countRole(tiny, OrnamentRole::GemShadow), 1);
    EXPECT_GE(countRole(tiny, OrnamentRole::Gem), 1);
}

/**
 * @brief Aucun sommet du bandeau ne sort de son rectangle, sur douze combinaisons de tailles.
 * \castest{<b>Aucune forme du bandeau ne deborde de son rectangle.</b><br/>
 * \tcat Unitaire · Ornements traces<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Produire la geometrie pour trois largeurs et quatre hauteurs.<br/>2. Relever la boite
 * englobante de chaque bandeau non vide.<br/>
 * \tattendu La boite tient dans [0,largeur] x [0,hauteur] : un debordement peindrait par-dessus le
 * widget voisin.
 * }
 */
TEST(TitleBannerTest, ToutTientDansLeBandeau) {
    for (const int width : {60, 200, 800}) {
        for (const int height : {8, 24, 40, 96}) {
            const std::vector<OrnamentShape> shapes = titleBannerShapes(width, height);
            if (shapes.empty()) {
                continue;
            }
            const Bounds bounds = boundsOf(shapes);
            EXPECT_GE(bounds.left, 0.0f) << width << 'x' << height;
            EXPECT_GE(bounds.top, 0.0f) << width << 'x' << height;
            EXPECT_LE(bounds.right, static_cast<float>(width)) << width << 'x' << height;
            EXPECT_LE(bounds.bottom, static_cast<float>(height)) << width << 'x' << height;
        }
    }
}

/**
 * @brief L'aile droite est le miroir exact de l'aile gauche.
 * \castest{<b>Les deux ailes du bandeau sont symetriques au centieme de pixel.</b><br/>
 * \tcat Unitaire · Ornements traces<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Produire un bandeau de 800 x 40.<br/>2. Comparer les boites des deux premieres
 * formes, l'une reflechie autour de l'axe vertical.<br/>
 * \tattendu Les deux boites coincident : une asymetrie d'un pixel ne se verrait pas, une aile
 * miroir oubliee si -- et on l'attribuerait a la police avant de la chercher dans la geometrie.
 * }
 */
TEST(TitleBannerTest, LesDeuxAilesSontSymetriques) {
    const int width = 800;
    const std::vector<OrnamentShape> shapes = titleBannerShapes(width, 40);
    ASSERT_GE(shapes.size(), 2U);
    const Bounds left = boundsOf({shapes[0]});
    const Bounds right = boundsOf({shapes[1]});
    // Une asymetrie d'un pixel ne se verrait pas ; une aile miroir oubliee, si -- et c'est le
    // genre de defaut qu'on attribue a la police avant de le chercher dans la geometrie.
    EXPECT_NEAR(left.left, static_cast<float>(width) - right.right, 0.01f);
    EXPECT_NEAR(left.right, static_cast<float>(width) - right.left, 0.01f);
    EXPECT_NEAR(left.top, right.top, 0.01f);
    EXPECT_NEAR(left.bottom, right.bottom, 0.01f);
}

/**
 * @brief Une largeur nulle ou negative, ou une hauteur sous le seuil, ne produit aucune forme.
 * \castest{<b>Un bandeau degenere ne produit aucune forme.</b><br/>
 * \tcat Unitaire · Ornements traces<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Demander la geometrie pour une largeur nulle, negative, puis une hauteur sous le
 * seuil.<br/>2. Demander l'envergure pour une hauteur nulle et negative.<br/>
 * \tattendu Aucune forme, et une envergure nulle : mieux vaut rien qu'un ornement ecrase.
 * }
 */
TEST(TitleBannerTest, UnBandeauDegenereNeDonneRien) {
    EXPECT_TRUE(titleBannerShapes(0, 40).empty());
    EXPECT_TRUE(titleBannerShapes(-10, 40).empty());
    EXPECT_TRUE(titleBannerShapes(800, ORNAMENT_MINIMUM_SIZE - 1).empty());
    EXPECT_EQ(bannerWingSpan(0), 0);
    EXPECT_EQ(bannerWingSpan(-5), 0);
}

/**
 * @brief Le filet d'or du bandeau est la seule forme tracee, et la derniere posee.
 * \castest{<b>Le filet du bandeau est trace, non rempli, et pose en dernier.</b><br/>
 * \tcat Unitaire · Ornements traces<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Produire un bandeau de 800 x 40.<br/>2. Verifier que la derniere forme est doree et
 * non remplie, et que toutes les autres sont remplies.<br/>
 * \tattendu Le filet ferme la silhouette par-dessus tout : rempli, il serait la plaque elle-meme et
 * recouvrirait la face eclairee qu'il borde.
 * }
 */
TEST(TitleBannerTest, LeFiletEstTraceEtPoseEnDernier) {
    const std::vector<OrnamentShape> shapes = titleBannerShapes(800, 40);
    ASSERT_FALSE(shapes.empty());
    // Trace et non rempli : un contour rempli serait la plaque elle-meme, et recouvrirait la face
    // eclairee qu'il est cense border.
    EXPECT_FALSE(shapes.back().filled);
    EXPECT_EQ(shapes.back().role, OrnamentRole::Ornament);
    for (std::size_t i = 0; i + 1 < shapes.size(); ++i) {
        EXPECT_TRUE(shapes[i].filled) << i;
    }
}

}  // namespace
