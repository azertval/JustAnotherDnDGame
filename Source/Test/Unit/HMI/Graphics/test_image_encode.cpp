// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

/**
 * @file test_image_encode.cpp
 * @brief Tests unitaires de l'encodage/enregistrement d'image, symétrique du décodage.
 */

#include <cstdint>
#include <filesystem>
#include <string>
#include <system_error>

#include <gtest/gtest.h>

#include "HMI/Graphics/TextureLoader.h"

namespace {

// Assemble une couleur RVBA (octets) en un pixel `R8G8B8A8_UNORM` (ordre memoire R,G,B,A) --
// meme convention que ProceduralAtlas.cpp/MissingTexture.cpp.
constexpr std::uint32_t pack(std::uint8_t red, std::uint8_t green, std::uint8_t blue,
                             std::uint8_t alpha) {
    return static_cast<std::uint32_t>(red) | (static_cast<std::uint32_t>(green) << 8) |
           (static_cast<std::uint32_t>(blue) << 16) | (static_cast<std::uint32_t>(alpha) << 24);
}

// Image 2x2 couvrant les cas d'alpha du round-trip : opaque, totalement transparent, et deux
// niveaux de transparence partielle sur des couleurs differentes.
hmi::DecodedImage sampleImage() {
    hmi::DecodedImage image;
    image.width = 2;
    image.height = 2;
    image.pixels = {
        pack(255, 0, 0, 255),  // rouge opaque
        pack(0, 0, 0, 0),      // totalement transparent
        pack(0, 255, 0, 128),  // vert, semi-transparent
        pack(18, 52, 86, 64),  // couleur quelconque, alpha partiel
    };
    return image;
}

// Fournit un dossier temporaire vierge par test.
class ImageEncode : public ::testing::Test {
protected:
    std::filesystem::path dir;

    void SetUp() override {
        const std::string suffix = std::to_string(reinterpret_cast<std::uintptr_t>(this));
        dir = std::filesystem::temp_directory_path() / ("pg_imgenc_" + suffix);
        std::filesystem::create_directories(dir);
    }
    void TearDown() override {
        std::error_code error;
        std::filesystem::remove_all(dir, error);
    }

    // Nombre de fichiers presents dans le dossier temporaire (detecte un `.tmp` residuel).
    [[nodiscard]] std::size_t fileCount() const {
        std::size_t count = 0;
        std::error_code error;
        for (const auto& entry : std::filesystem::directory_iterator(dir, error)) {
            (void)entry;
            ++count;
        }
        return count;
    }
};

}  // namespace

/**
 * @brief Encoder puis décoder restitue exactement les mêmes pixels, canal alpha compris (opaque,
 *        transparent et partiellement transparent).
 * \castest{<b>Encoder puis decoder une image restitue exactement les memes pixels, alpha
 * compris.</b><br/>
 * \tcat Unitaire · Encodage image<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Encoder une image 2x2 couvrant plusieurs niveaux d'alpha vers un fichier PNG.<br/>
 * 2. Decoder ce fichier.<br/>
 * \tattendu Les dimensions et tous les pixels decodes sont identiques a l'image d'origine.
 * }
 */
TEST_F(ImageEncode, AllerRetourExactAlphaCompris) {
    const hmi::DecodedImage original = sampleImage();
    const std::filesystem::path path = dir / "asset.png";

    ASSERT_TRUE(hmi::encodeImageFile(path, original));

    const std::optional<hmi::DecodedImage> decoded = hmi::decodeImageFile(path);
    ASSERT_TRUE(decoded.has_value());
    EXPECT_EQ(decoded->width, original.width);
    EXPECT_EQ(decoded->height, original.height);
    EXPECT_EQ(decoded->pixels, original.pixels);
}

/**
 * @brief Un dossier de destination inexistant est une erreur récupérable : aucune exception,
 *        résultat exploitable.
 * \castest{<b>Encoder vers un dossier inexistant echoue proprement, sans exception.</b><br/>
 * \tcat Unitaire · Encodage image<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Tenter d'encoder une image vers un chemin dont le dossier parent n'existe pas.<br/>
 * \tattendu La fonction renvoie false, sans lever d'exception.
 * }
 */
TEST_F(ImageEncode, DossierInexistantEchoueProprement) {
    const hmi::DecodedImage original = sampleImage();
    const std::filesystem::path path = dir / "dossier_absent" / "asset.png";

    bool result = true;
    EXPECT_NO_THROW(result = hmi::encodeImageFile(path, original));
    EXPECT_FALSE(result);
}

/**
 * @brief Une image invalide (dimensions non conformes au tampon fourni) est refusée sans
 *        exception plutôt que d'écrire un fichier corrompu.
 * \castest{<b>Encoder une image aux dimensions incoherentes echoue proprement.</b><br/>
 * \tcat Unitaire · Encodage image<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire une image dont le nombre de pixels ne correspond pas a largeur*hauteur.
 * <br/>2. Tenter de l'encoder.<br/>
 * \tattendu La fonction renvoie false, sans lever d'exception, et n'ecrit aucun fichier.
 * }
 */
TEST_F(ImageEncode, DimensionsIncoherentesEchouentProprement) {
    hmi::DecodedImage broken;
    broken.width = 4;
    broken.height = 4;
    broken.pixels = {pack(0, 0, 0, 255)};  // un seul pixel, pas seize
    const std::filesystem::path path = dir / "asset.png";

    bool result = true;
    EXPECT_NO_THROW(result = hmi::encodeImageFile(path, broken));
    EXPECT_FALSE(result);
    EXPECT_EQ(fileCount(), 0U);
}

/**
 * @brief L'écriture est atomique : aucun fichier temporaire ne subsiste après un encodage réussi.
 * \castest{<b>Aucun fichier temporaire ne subsiste apres un encodage reussi.</b><br/>
 * \tcat Unitaire · Encodage image<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Encoder une image vers un fichier.<br/>2. Lister le dossier de destination.<br/>
 * \tattendu Seul le fichier final est present, aucun fichier `.tmp`.
 * }
 */
TEST_F(ImageEncode, AucunFichierTemporaireApresSucces) {
    ASSERT_TRUE(hmi::encodeImageFile(dir / "asset.png", sampleImage()));

    EXPECT_EQ(fileCount(), 1U);
    EXPECT_TRUE(std::filesystem::exists(dir / "asset.png"));
}

/**
 * @brief L'écriture est atomique : un enregistrement par-dessus un asset existant préserve
 *        l'ancien fichier tant que le nouveau n'est pas complet, et ne laisse rien de résiduel
 *        après coup.
 * \castest{<b>Enregistrer par-dessus un asset existant remplace son contenu sans residu.</b><br/>
 * \tcat Unitaire · Encodage image<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Encoder une premiere image vers un fichier.<br/>2. Encoder une seconde image
 * differente vers le meme fichier.<br/>3. Decoder le fichier et lister le dossier.<br/>
 * \tattendu Le fichier contient la seconde image, et aucun fichier temporaire ne subsiste.
 * }
 */
TEST_F(ImageEncode, EcrasementAtomiqueSansResidu) {
    const std::filesystem::path path = dir / "asset.png";
    ASSERT_TRUE(hmi::encodeImageFile(path, sampleImage()));

    hmi::DecodedImage second;
    second.width = 1;
    second.height = 1;
    second.pixels = {pack(9, 9, 9, 255)};
    ASSERT_TRUE(hmi::encodeImageFile(path, second));

    const std::optional<hmi::DecodedImage> decoded = hmi::decodeImageFile(path);
    ASSERT_TRUE(decoded.has_value());
    EXPECT_EQ(decoded->width, 1);
    EXPECT_EQ(decoded->height, 1);
    EXPECT_EQ(fileCount(), 1U);
}
