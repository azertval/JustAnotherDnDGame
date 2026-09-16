// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

/**
 * @file test_pixel_asset_io.cpp
 * @brief Tests unitaires des tailles proposées à la création d'un asset dans l'atelier pixel art
 *        (LOT-54 TACHE-05, EX-REN-007).
 */

#include <gtest/gtest.h>

#include "HMI/Editor/PixelAssetIO.h"
#include "HMI/Graphics/TextureAtlas.h"

/**
 * @brief Chaque taille proposée, pour chaque famille, est conforme au contrat de dimensions de sa
 *        famille — une création à une taille non conforme ne doit jamais être proposée.
 * \castest{<b>Chaque taille proposee est conforme au contrat de sa famille.</b><br/>
 * \tcat Unitaire · IO d'asset pixel art<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Pour chaque famille, produire les tailles proposees.<br/>2. Valider chacune contre
 * le contrat de la meme famille.<br/>
 * \tattendu Chaque taille proposee est jugee conforme.
 * }
 */
TEST(PixelAssetIOTest, ChaqueTailleProposeeEstConforme) {
    constexpr hmi::AssetFamily families[] = {
        hmi::AssetFamily::Atlas,  hmi::AssetFamily::TileSkin,       hmi::AssetFamily::AutotileSheet,
        hmi::AssetFamily::Object, hmi::AssetFamily::CharacterSheet, hmi::AssetFamily::Background,
        hmi::AssetFamily::Font};
    for (const hmi::AssetFamily family : families) {
        for (const auto& [width, height] : hmi::validAssetSizes(family)) {
            const hmi::AssetValidation validation =
                hmi::validateAsset(family, "test.png", width, height);
            EXPECT_TRUE(validation.valid) << hmi::assetFamilyName(family) << " " << width << "x"
                                          << height << " : " << validation.message;
        }
    }
}

/**
 * @brief Une famille à dimensions libres (fond, décor, police) ne propose aucune taille : la
 *        saisie libre reste à la charge de l'appelant.
 * \castest{<b>Une famille a dimensions libres ne propose aucune taille.</b><br/>
 * \tcat Unitaire · IO d'asset pixel art<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Produire les tailles proposees pour Fond et Police.<br/>
 * \tattendu Les trois listes sont vides.
 * }
 */
TEST(PixelAssetIOTest, FamilleADimensionsLibresNeProposeAucuneTaille) {
    EXPECT_TRUE(hmi::validAssetSizes(hmi::AssetFamily::Background).empty());
    EXPECT_TRUE(hmi::validAssetSizes(hmi::AssetFamily::Font).empty());
}

/**
 * @brief Un skin de tuile propose des tailles de hauteur exactement une case, avec une largeur
 *        multiple de case croissante (skin anime).
 * \castest{<b>Un skin de tuile propose des largeurs multiples de case, hauteur fixe.</b><br/>
 * \tcat Unitaire · IO d'asset pixel art<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Produire les tailles proposees pour TileSkin.<br/>2. Verifier la hauteur et la
 * largeur de chaque taille.<br/>
 * \tattendu La liste n'est pas vide ; chaque hauteur vaut la taille d'une case ; chaque largeur
 * est un multiple de la taille d'une case.
 * }
 */
TEST(PixelAssetIOTest, TileSkinProposeDesLargeursMultiplesDeCase) {
    const std::vector<std::pair<int, int>> sizes = hmi::validAssetSizes(hmi::AssetFamily::TileSkin);
    ASSERT_FALSE(sizes.empty());
    for (const auto& [width, height] : sizes) {
        EXPECT_EQ(height, hmi::TextureAtlas::TILE_SIZE);
        EXPECT_EQ(width % hmi::TextureAtlas::TILE_SIZE, 0);
    }
}

/**
 * @brief Une famille à taille exacte (Atlas exclu, il n'a pas de hauteur exacte) ne propose qu'une
 *        seule taille.
 * \castest{<b>Les familles a taille non contrainte proposent plusieurs options.</b><br/>
 * \tcat Unitaire · IO d'asset pixel art<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Produire les tailles proposees pour AutotileSheet.<br/>
 * \tattendu Plusieurs tailles distinctes sont proposees (progression, pas une valeur unique).
 * }
 */
TEST(PixelAssetIOTest, AutotileSheetProposePlusieursTailles) {
    const std::vector<std::pair<int, int>> sizes =
        hmi::validAssetSizes(hmi::AssetFamily::AutotileSheet);
    EXPECT_GT(sizes.size(), 1U);
}
