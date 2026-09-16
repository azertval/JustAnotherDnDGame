// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

/**
 * @file test_missing_texture.cpp
 * @brief Tests unitaires du repli « texture manquante » en damier magenta (LOT-40, EX-NFR-040).
 */

#include <gtest/gtest.h>

#include "HMI/Graphics/MissingTexture.h"

namespace {

/// Pixel `R8G8B8A8_UNORM` attendu pour le magenta opaque du damier.
constexpr std::uint32_t MAGENTA = 0xFFFF00FFu;
/// Pixel `R8G8B8A8_UNORM` attendu pour le noir opaque du damier.
constexpr std::uint32_t BLACK = 0xFF000000u;

/// Pixel (x, y) d'une image générée.
std::uint32_t pixelAt(const hmi::ProceduralAtlasImage& image, int x, int y) {
    return image.pixels[static_cast<std::size_t>(y) * static_cast<std::size_t>(image.width) +
                        static_cast<std::size_t>(x)];
}

}  // namespace

/**
 * @brief Le damier généré a les dimensions demandées et autant de pixels.
 * \castest{<b>Le damier genere a les dimensions demandees.</b><br/>
 * \tcat Unitaire · Missing Texture<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Generer le damier par defaut, puis un damier de 32 px.<br/>
 * \tattendu Les dimensions et le nombre de pixels correspondent.
 * }
 */
TEST(MissingTextureTest, DimensionsAttendues) {
    const hmi::ProceduralAtlasImage image = hmi::buildMissingTextureImage();

    EXPECT_EQ(image.width, hmi::MISSING_TEXTURE_SIZE);
    EXPECT_EQ(image.height, hmi::MISSING_TEXTURE_SIZE);
    EXPECT_EQ(image.pixels.size(),
              static_cast<std::size_t>(image.width) * static_cast<std::size_t>(image.height));

    const hmi::ProceduralAtlasImage large = hmi::buildMissingTextureImage(32);
    EXPECT_EQ(large.width, 32);
    EXPECT_EQ(large.height, 32);
}

/**
 * @brief La génération est déterministe : deux appels produisent les mêmes pixels.
 * \castest{<b>La generation du damier est deterministe.</b><br/>
 * \tcat Unitaire · Missing Texture<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Generer deux fois le damier.<br/>
 * \tattendu Les deux images sont identiques.
 * }
 */
TEST(MissingTextureTest, GenerationDeterministe) {
    EXPECT_EQ(hmi::buildMissingTextureImage().pixels, hmi::buildMissingTextureImage().pixels);
}

/**
 * @brief Les carreaux alternent magenta et noir : c'est le motif qui rend le repli impossible à
 *        confondre avec un asset réel.
 * \castest{<b>Les carreaux du damier alternent magenta et noir.</b><br/>
 * \tcat Unitaire · Missing Texture<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Generer le damier.<br/>2. Lire un pixel de chacun des quatre premiers
 * carreaux.<br/>
 * \tattendu Les carreaux voisins ont des couleurs differentes, magenta et noir.
 * }
 */
TEST(MissingTextureTest, CarreauxAlternesMagentaEtNoir) {
    const hmi::ProceduralAtlasImage image = hmi::buildMissingTextureImage();
    constexpr int STEP = hmi::MISSING_TEXTURE_CHECKER_SIZE;

    EXPECT_EQ(pixelAt(image, 0, 0), MAGENTA);
    EXPECT_EQ(pixelAt(image, STEP, 0), BLACK);
    EXPECT_EQ(pixelAt(image, 0, STEP), BLACK);
    EXPECT_EQ(pixelAt(image, STEP, STEP), MAGENTA);
}

/**
 * @brief Tous les pixels sont **opaques** : un repli transparent passerait inaperçu, alors qu'un
 *        asset manquant doit se voir.
 * \castest{<b>Tous les pixels du damier sont opaques.</b><br/>
 * \tcat Unitaire · Missing Texture<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Generer le damier.<br/>2. Verifier le canal alpha de chaque pixel.<br/>
 * \tattendu Aucun pixel n'est transparent.
 * }
 */
TEST(MissingTextureTest, DamierEntierementOpaque) {
    const hmi::ProceduralAtlasImage image = hmi::buildMissingTextureImage();

    for (const std::uint32_t pixel : image.pixels) {
        EXPECT_EQ(pixel >> 24, 0xFFu);
    }
}

/**
 * @brief Le message d'avertissement nomme l'asset attendu, seule information qui permette à
 *        l'auteur de savoir quoi créer.
 * \castest{<b>L'avertissement de texture manquante nomme l'asset attendu.</b><br/>
 * \tcat Unitaire · Missing Texture<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire le message pour un asset donne.<br/>
 * \tattendu Le message contient le nom du fichier attendu.
 * }
 */
TEST(MissingTextureTest, AvertissementNommeAsset) {
    const std::string message = hmi::missingTextureWarning("Backgrounds/foret.png");

    EXPECT_NE(message.find("Backgrounds/foret.png"), std::string::npos) << message;
}
