// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

/**
 * @file test_plane_reference.cpp
 * @brief Tests unitaires des repères du mode création : pelure d'oignon, aplatissement et
 *        rééchantillonnage (`EX-EDIT-046`, LOT-69 TACHE-07).
 */

#include <cstdint>
#include <cstdlib>
#include <vector>

#include <gtest/gtest.h>

#include "Core/Levels/LevelDraft.h"
#include "Core/Levels/TileType.h"
#include "HMI/Editor/PlaneReference.h"

namespace {

/// Couleur RGBA (R en poids faible), comme `hmi::DecodedImage` la stocke.
constexpr std::uint32_t rgba(std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a) {
    return static_cast<std::uint32_t>(r) | (static_cast<std::uint32_t>(g) << 8) |
           (static_cast<std::uint32_t>(b) << 16) | (static_cast<std::uint32_t>(a) << 24);
}

/// Image carrée remplie d'une seule couleur.
hmi::DecodedImage uniform(int side, std::uint32_t color) {
    hmi::DecodedImage image;
    image.width = side;
    image.height = side;
    image.pixels.assign(static_cast<std::size_t>(side) * static_cast<std::size_t>(side), color);
    return image;
}

/// Damier 4x4 de quatre couleurs distinctes, par blocs de 2x2 — le motif qui rend un
/// sous-échantillonnage vérifiable pixel à pixel.
hmi::DecodedImage witness() {
    const std::uint32_t a = rgba(255, 0, 0, 255);
    const std::uint32_t b = rgba(0, 255, 0, 255);
    const std::uint32_t c = rgba(0, 0, 255, 255);
    const std::uint32_t d = rgba(255, 255, 255, 255);
    hmi::DecodedImage image;
    image.width = 4;
    image.height = 4;
    image.pixels = {a, a, b, b, a, a, b, b, c, c, d, d, c, c, d, d};
    return image;
}

}  // namespace

/**
 * @brief Le rééchantillonnage 16 → 8 garde **un pixel sur deux**, sans jamais moyenner.
 * \castest{<b>Le reechantillonnage 16 vers 8 est exact, sans interpolation.</b><br/>
 * \tcat Unitaire · Reference de plan<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Reechantillonner un damier 4x4 de la densite 16 vers la densite 8.<br/>
 * \tattendu L'image fait 2x2 et chaque pixel vaut exactement un pixel source, jamais une moyenne.
 * }
 */
TEST(PlaneReferenceTest, ReechantillonnageSeizeVersHuitEstExact) {
    const hmi::DecodedImage source = witness();

    const hmi::DecodedImage reduced = hmi::resamplePlane(source, 16, 8);

    ASSERT_EQ(reduced.width, 2);
    ASSERT_EQ(reduced.height, 2);
    EXPECT_EQ(reduced.pixels[0], source.pixels[0]);
    EXPECT_EQ(reduced.pixels[1], source.pixels[2]);
    EXPECT_EQ(reduced.pixels[2], source.pixels[8]);
    EXPECT_EQ(reduced.pixels[3], source.pixels[10]);
}

/**
 * @brief Le rééchantillonnage 8 → 16 **duplique** chaque pixel : aucun demi-ton n'apparaît.
 * \castest{<b>Le reechantillonnage 8 vers 16 duplique chaque pixel.</b><br/>
 * \tcat Unitaire · Reference de plan<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Reechantillonner une image 2x2 de la densite 8 vers la densite 16.<br/>
 * \tattendu Chaque pixel source occupe un carre 2x2, a l'identique.
 * }
 */
TEST(PlaneReferenceTest, ReechantillonnageHuitVersSeizeDuplique) {
    hmi::DecodedImage source;
    source.width = 2;
    source.height = 2;
    source.pixels = {rgba(10, 20, 30, 255), rgba(40, 50, 60, 255), rgba(70, 80, 90, 255),
                     rgba(100, 110, 120, 255)};

    const hmi::DecodedImage grown = hmi::resamplePlane(source, 8, 16);

    ASSERT_EQ(grown.width, 4);
    ASSERT_EQ(grown.height, 4);
    for (int y = 0; y < 4; ++y) {
        for (int x = 0; x < 4; ++x) {
            const std::uint32_t expected =
                source
                    .pixels[static_cast<std::size_t>(y / 2) * 2 + static_cast<std::size_t>(x / 2)];
            EXPECT_EQ(grown.pixels[static_cast<std::size_t>(y) * 4 + static_cast<std::size_t>(x)],
                      expected)
                << "pixel (" << x << ", " << y << ')';
        }
    }
}

/**
 * @brief Un rapport de densités **non entier** est refusé plutôt qu'interpolé.
 * \castest{<b>Un rapport de densites non entier est refuse.</b><br/>
 * \tcat Unitaire · Reference de plan<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Reechantillonner de la densite 16 vers une densite qui n'en est pas un diviseur.<br/>
 * \tattendu L'image renvoyee est vide.
 * }
 */
TEST(PlaneReferenceTest, RapportNonEntierRefuse) {
    const hmi::DecodedImage reduced = hmi::resamplePlane(witness(), 16, 6);
    EXPECT_EQ(reduced.width, 0);
    EXPECT_TRUE(reduced.pixels.empty());
}

/**
 * @brief L'alpha-over est **associatif** : aplatir trois couches d'un coup donne le même résultat
 * que deux fois deux — sans quoi l'ordre d'aplatissement changerait l'image.
 * \castest{<b>L'alpha-over est associatif.</b><br/>
 * \tcat Unitaire · Reference de plan<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Composer trois pixels semi-transparents en groupant a gauche, puis a droite.<br/>
 * \tattendu Les deux resultats coincident.
 * }
 */
TEST(PlaneReferenceTest, AlphaOverEstAssociatif) {
    const std::uint32_t bottom = rgba(200, 40, 40, 180);
    const std::uint32_t middle = rgba(40, 200, 40, 120);
    const std::uint32_t top = rgba(40, 40, 200, 90);

    const std::uint32_t left = hmi::alphaOver(hmi::alphaOver(bottom, middle), top);
    const std::uint32_t right = hmi::alphaOver(bottom, hmi::alphaOver(middle, top));

    // Tolerance d'une unite par canal : les deux groupements passent par des arrondis differents.
    for (int shift = 0; shift < 32; shift += 8) {
        const auto leftChannel = static_cast<int>((left >> shift) & 0xFFu);
        const auto rightChannel = static_cast<int>((right >> shift) & 0xFFu);
        EXPECT_LE(std::abs(leftChannel - rightChannel), 1) << "canal decale de " << shift;
    }
}

/**
 * @brief L'aplatissement **ignore** une couche masquée, exactement comme si elle n'existait pas.
 * \castest{<b>L'aplatissement ignore une couche masquee.</b><br/>
 * \tcat Unitaire · Reference de plan<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Aplatir deux couches opaques dont la seconde est masquee.<br/>2. Recommencer sans la
 * seconde couche.<br/>
 * \tattendu Les deux images aplaties sont identiques.
 * }
 */
TEST(PlaneReferenceTest, AplatissementIgnoreUneCoucheMasquee) {
    const hmi::DecodedImage first = uniform(4, rgba(10, 200, 10, 255));
    const hmi::DecodedImage second = uniform(4, rgba(200, 10, 10, 255));

    std::vector<hmi::PlaneLayer> withHidden{
        hmi::PlaneLayer{&first, 16, 1.0f, true},
        hmi::PlaneLayer{&second, 16, 1.0f, false},
    };
    const std::vector<hmi::PlaneLayer> withoutSecond{hmi::PlaneLayer{&first, 16, 1.0f, true}};

    const hmi::DecodedImage flattenedWithHidden = hmi::flattenPlanes(withHidden, 16, 1, 1);
    const hmi::DecodedImage flattenedWithout = hmi::flattenPlanes(withoutSecond, 16, 1, 1);

    ASSERT_EQ(flattenedWithHidden.pixels.size(), flattenedWithout.pixels.size());
    EXPECT_EQ(flattenedWithHidden.pixels, flattenedWithout.pixels);
    // Et la couche visible, elle, est bien presente.
    EXPECT_EQ(flattenedWithHidden.pixels.front(), first.pixels.front());
}

/**
 * @brief Une couche de densité **plus faible** est ramenée à la densité cible avant d'être
 * composée : l'aplatissement n'exige pas que tous les plans partagent une densité.
 * \castest{<b>L'aplatissement ramene chaque couche a la densite cible.</b><br/>
 * \tcat Unitaire · Reference de plan<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Aplatir une couche de densite 8 sur une cible de densite 16.<br/>
 * \tattendu L'image aplatie fait la taille de la cible et porte la couleur de la couche.
 * }
 */
TEST(PlaneReferenceTest, AplatissementRameneChaqueCoucheALaDensiteCible) {
    const hmi::DecodedImage coarse = uniform(8, rgba(30, 60, 90, 255));
    const std::vector<hmi::PlaneLayer> layers{hmi::PlaneLayer{&coarse, 8, 1.0f, true}};

    const hmi::DecodedImage flattened = hmi::flattenPlanes(layers, 16, 1, 1);

    ASSERT_EQ(flattened.width, 16);
    ASSERT_EQ(flattened.height, 16);
    EXPECT_EQ(flattened.pixels.front(), coarse.pixels.front());
    EXPECT_EQ(flattened.pixels.back(), coarse.pixels.front());
}

/**
 * @brief La pelure d'oignon est **déterministe** : une couleur plate par type de tuile, cases vides
 * transparentes, aux dimensions du plan.
 * \castest{<b>La pelure d'oignon est deterministe et aux dimensions du plan.</b><br/>
 * \tcat Unitaire · Reference de plan<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Construire la pelure d'un brouillon a deux tuiles, en densite 8.<br/>2. La
 * reconstruire a l'identique.<br/>
 * \tattendu Memes dimensions et memes pixels ; la case vide reste transparente, la case solide est
 * opaque et uniforme.
 * }
 */
TEST(PlaneReferenceTest, PelureDOignonDeterministe) {
    core::LevelDraft draft = core::LevelDraft::empty("N", 3, 2);
    draft.paintTile(0, 0, core::TileType::Solid);

    constexpr int DENSITY = 8;
    const hmi::DecodedImage skin = hmi::buildTileOnionSkin(draft, DENSITY);
    const hmi::DecodedImage again = hmi::buildTileOnionSkin(draft, DENSITY);

    ASSERT_EQ(skin.width, 3 * DENSITY);
    ASSERT_EQ(skin.height, 2 * DENSITY);
    EXPECT_EQ(skin.pixels, again.pixels);

    // Case (0,0) peinte : opaque et uniforme sur tout le carre de la case.
    const std::uint32_t painted = skin.pixels[0];
    EXPECT_EQ((painted >> 24) & 0xFFu, 255u);
    for (int y = 0; y < DENSITY; ++y) {
        for (int x = 0; x < DENSITY; ++x) {
            EXPECT_EQ(
                skin.pixels[static_cast<std::size_t>(y) * static_cast<std::size_t>(skin.width) +
                            static_cast<std::size_t>(x)],
                painted);
        }
    }

    // Case (1,0) vide : entierement transparente -- c'est un repere, pas un fond.
    EXPECT_EQ(skin.pixels[static_cast<std::size_t>(DENSITY)], 0u);
}

/**
 * @brief Une densité invalide ne produit pas de repère : le format n'accepte que 4, 8 et 16.
 * \castest{<b>Une densite invalide ne produit aucune pelure d'oignon.</b><br/>
 * \tcat Unitaire · Reference de plan<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire la pelure a une densite hors format.<br/>
 * \tattendu L'image renvoyee est vide.
 * }
 */
TEST(PlaneReferenceTest, DensiteInvalideNeProduitAucunePelure) {
    const core::LevelDraft draft = core::LevelDraft::empty("N", 3, 2);
    const hmi::DecodedImage skin = hmi::buildTileOnionSkin(draft, 5);
    EXPECT_EQ(skin.width, 0);
    EXPECT_TRUE(skin.pixels.empty());
}
