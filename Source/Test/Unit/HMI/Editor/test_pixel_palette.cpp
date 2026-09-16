// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

/**
 * @file test_pixel_palette.cpp
 * @brief Tests unitaires de la palette de projet, de l'extraction et de la contrainte de couleur
 *        (LOT-54 TACHE-07, EX-REN-042).
 */

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>

#include <gtest/gtest.h>

#include "HMI/Editor/PixelPalette.h"

namespace {

constexpr std::uint32_t RED = 0xFF0000FFu;
constexpr std::uint32_t GREEN = 0xFF00FF00u;
constexpr std::uint32_t BLUE_SEMI = 0x80FF0000u;  // bleu, alpha partiel.

hmi::DecodedImage uniformImage(int width, int height, std::uint32_t color) {
    hmi::DecodedImage image;
    image.width = width;
    image.height = height;
    image.pixels.assign(static_cast<std::size_t>(width) * static_cast<std::size_t>(height), color);
    return image;
}

// Fournit un chemin de fichier temporaire unique par test.
class PixelPaletteFile : public ::testing::Test {
protected:
    std::filesystem::path path;

    void SetUp() override {
        const std::string suffix = std::to_string(reinterpret_cast<std::uintptr_t>(this));
        path = std::filesystem::temp_directory_path() / ("pg_palette_" + suffix + ".json");
    }
    void TearDown() override {
        std::error_code error;
        std::filesystem::remove(path, error);
    }
};

}  // namespace

/**
 * @brief Écrire puis relire une palette restitue les mêmes couleurs, les mêmes noms et le même
 *        ordre, alpha compris.
 * \castest{<b>Ecrire puis relire une palette restitue le meme contenu, alpha compris.</b><br/>
 * \tcat Unitaire · Palette pixel art<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Construire une palette de trois entrees, dont une avec alpha partiel.<br/>
 * 2. L'enregistrer puis la relire.<br/>
 * \tattendu Les entrees relues sont identiques (nom, couleur, ordre) a l'origine.
 * }
 */
TEST_F(PixelPaletteFile, AllerRetourJsonRestitueLeContenu) {
    hmi::PixelPalette original;
    original.add("Rouge", RED);
    original.add("Vert", GREEN);
    original.add("Bleu translucide", BLUE_SEMI);

    ASSERT_TRUE(original.saveToFile(path));
    const hmi::PixelPalette reloaded = hmi::PixelPalette::loadFromFile(path);

    ASSERT_EQ(reloaded.entries().size(), 3U);
    EXPECT_EQ(reloaded.entries()[0].name, "Rouge");
    EXPECT_EQ(reloaded.entries()[0].color, RED);
    EXPECT_EQ(reloaded.entries()[1].name, "Vert");
    EXPECT_EQ(reloaded.entries()[1].color, GREEN);
    EXPECT_EQ(reloaded.entries()[2].name, "Bleu translucide");
    EXPECT_EQ(reloaded.entries()[2].color, BLUE_SEMI);
}

/**
 * @brief Un fichier absent produit une palette vide, sans exception : un état de départ légitime,
 *        pas une erreur.
 * \castest{<b>Un fichier absent produit une palette vide sans exception.</b><br/>
 * \tcat Unitaire · Palette pixel art<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Charger un chemin qui n'existe pas.<br/>
 * \tattendu Aucune exception ; la palette renvoyee est vide.
 * }
 */
TEST_F(PixelPaletteFile, FichierAbsentProduitUnePaletteVide) {
    hmi::PixelPalette palette;
    EXPECT_NO_THROW(palette = hmi::PixelPalette::loadFromFile(path));
    EXPECT_TRUE(palette.entries().empty());
}

/**
 * @brief Un fichier JSON malformé produit une palette vide, sans exception.
 * \castest{<b>Un fichier malforme produit une palette vide sans exception.</b><br/>
 * \tcat Unitaire · Palette pixel art<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Ecrire un contenu JSON invalide au chemin de test.<br/>2. Le charger.<br/>
 * \tattendu Aucune exception ; la palette renvoyee est vide.
 * }
 */
TEST_F(PixelPaletteFile, FichierMalformeProduitUnePaletteVide) {
    {
        std::ofstream file(path);
        file << "{ceci n'est pas du json valide";
    }
    hmi::PixelPalette palette;
    EXPECT_NO_THROW(palette = hmi::PixelPalette::loadFromFile(path));
    EXPECT_TRUE(palette.entries().empty());
}

/**
 * @brief Deux extractions du même asset produisent la même liste, dans le même ordre, avec des
 *        occurrences exactes.
 * \castest{<b>Deux extractions du meme asset produisent la meme liste.</b><br/>
 * \tcat Unitaire · Palette pixel art<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Construire une image avec des couleurs repetees.<br/>2. L'extraire deux fois.<br/>
 * \tattendu Les deux extractions sont identiques ; les occurrences comptees sont exactes.
 * }
 */
TEST(PixelPaletteTest, DeuxExtractionsProduisentLaMemeListe) {
    hmi::DecodedImage image = uniformImage(4, 1, RED);
    image.pixels[1] = GREEN;
    image.pixels[2] = RED;
    image.pixels[3] = GREEN;

    const std::vector<hmi::PixelPaletteExtractionEntry> first = hmi::extractPalette(image);
    const std::vector<hmi::PixelPaletteExtractionEntry> second = hmi::extractPalette(image);

    ASSERT_EQ(first.size(), 2U);
    EXPECT_EQ(first[0].color, RED);
    EXPECT_EQ(first[0].count, 2);
    EXPECT_EQ(first[1].color, GREEN);
    EXPECT_EQ(first[1].count, 2);

    ASSERT_EQ(second.size(), first.size());
    for (std::size_t i = 0; i < first.size(); ++i) {
        EXPECT_EQ(first[i].color, second[i].color) << i;
        EXPECT_EQ(first[i].count, second[i].count) << i;
    }
}

/**
 * @brief Une image entièrement transparente produit une liste d'extraction vide.
 * \castest{<b>Une image entierement transparente produit une extraction vide.</b><br/>
 * \tcat Unitaire · Palette pixel art<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire une image entierement transparente.<br/>2. L'extraire.<br/>
 * \tattendu La liste extraite est vide.
 * }
 */
TEST(PixelPaletteTest, ImageEntierementTransparenteProduitUneExtractionVide) {
    const hmi::DecodedImage image = uniformImage(3, 3, 0x00000000u);
    EXPECT_TRUE(hmi::extractPalette(image).empty());
}

/**
 * @brief Une couleur déjà présente dans la palette est renvoyée telle quelle.
 * \castest{<b>Une couleur deja presente est renvoyee telle quelle.</b><br/>
 * \tcat Unitaire · Palette pixel art<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Contraindre une couleur presente dans la palette.<br/>
 * \tattendu Le resultat est strictement egal a la couleur d'origine.
 * }
 */
TEST(PixelPaletteTest, CouleurDejaPresenteRenvoyeeTelleQuelle) {
    const std::vector<std::uint32_t> palette{RED, GREEN, BLUE_SEMI};
    EXPECT_EQ(hmi::nearestPaletteColor(GREEN, palette), GREEN);
}

/**
 * @brief À distance égale entre deux couleurs de la palette, le départage retient la première
 *        rencontrée : déterministe et stable.
 * \castest{<b>Le departage a distance egale est stable.</b><br/>
 * \tcat Unitaire · Palette pixel art<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Construire une palette avec deux couleurs equidistantes d'une couleur cible.<br/>
 * 2. Contraindre la couleur cible plusieurs fois.<br/>
 * \tattendu Le meme resultat (la premiere entree equidistante) est obtenu a chaque appel.
 * }
 */
TEST(PixelPaletteTest, DepartageADistanceEgaleEstStable) {
    // Cible (128,128,128) : (100,128,128) et (156,128,128) sont a distance egale (28) sur le seul
    // canal rouge -- ne different que par l'ordre dans la palette.
    constexpr std::uint32_t TARGET = 0xFF808080u;       // r=128,g=128,b=128,a=255
    constexpr std::uint32_t CANDIDATE_A = 0xFF808064u;  // r=100,g=128,b=128
    constexpr std::uint32_t CANDIDATE_B = 0xFF80809Cu;  // r=156,g=128,b=128
    const std::vector<std::uint32_t> palette{CANDIDATE_A, CANDIDATE_B};

    const std::uint32_t first = hmi::nearestPaletteColor(TARGET, palette);
    const std::uint32_t second = hmi::nearestPaletteColor(TARGET, palette);
    EXPECT_EQ(first, CANDIDATE_A);
    EXPECT_EQ(second, first);
}

/**
 * @brief Une palette vide laisse la couleur inchangée plutôt que de produire une couleur
 *        indéfinie.
 * \castest{<b>Une palette vide laisse la couleur inchangee.</b><br/>
 * \tcat Unitaire · Palette pixel art<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Contraindre une couleur avec une palette vide.<br/>
 * \tattendu Le resultat est strictement egal a la couleur d'origine.
 * }
 */
TEST(PixelPaletteTest, PaletteVideLaisseLaCouleurInchangee) {
    EXPECT_EQ(hmi::nearestPaletteColor(RED, {}), RED);
}

/**
 * @brief La contrainte ne transforme jamais un pixel transparent en pixel opaque : l'alpha
 *        d'origine est toujours préservé.
 * \castest{<b>La contrainte preserve toujours l'alpha d'origine.</b><br/>
 * \tcat Unitaire · Palette pixel art<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Contraindre un pixel totalement transparent.<br/>2. Contraindre un pixel
 * semi-transparent.<br/>
 * \tattendu Le premier resultat est inchange (alpha nul preserve) ; le second conserve son alpha
 * d'origine tout en adoptant la teinte RVB la plus proche.
 * }
 */
TEST(PixelPaletteTest, ContraintePreserveToujoursLAlpha) {
    const std::vector<std::uint32_t> palette{RED, GREEN};

    const std::uint32_t transparent = 0x00123456u;
    EXPECT_EQ(hmi::nearestPaletteColor(transparent, palette), transparent)
        << "un pixel transparent ne doit jamais devenir opaque";

    const std::uint32_t semiTransparentGreen = 0x8000FF00u;  // vert, alpha 128.
    const std::uint32_t constrained = hmi::nearestPaletteColor(semiTransparentGreen, palette);
    EXPECT_EQ(constrained & 0xFF000000u, semiTransparentGreen & 0xFF000000u)
        << "l'alpha d'origine doit etre preserve";
    EXPECT_EQ(constrained & 0x00FFFFFFu, GREEN & 0x00FFFFFFu);
}
