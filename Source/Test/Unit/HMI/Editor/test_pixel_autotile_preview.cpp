// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

/**
 * @file test_pixel_autotile_preview.cpp
 * @brief Tests unitaires du mode planche à raccords de l'atelier pixel art (LOT-54 TACHE-08,
 *        EX-EDIT-025).
 */

#include <cstdint>

#include <gtest/gtest.h>

#include "HMI/Editor/PixelAutotilePreview.h"

namespace {

constexpr int TILE = 4;  // taille de case arbitraire, petite pour des tests rapides.

hmi::DecodedImage sheetImage(int widthInTiles, int heightInTiles, std::uint32_t fill) {
    hmi::DecodedImage image;
    image.width = widthInTiles * TILE;
    image.height = heightInTiles * TILE;
    image.pixels.assign(
        static_cast<std::size_t>(image.width) * static_cast<std::size_t>(image.height), fill);
    return image;
}

}  // namespace

/**
 * @brief Une image aux dimensions exactes d'une planche 4×4 est reconnue comme planche à raccords ;
 *        une image de toute autre taille ne l'est pas.
 * \castest{<b>Seule une image de dimensions exactement 4x4 cases est une planche a raccords.</b>
 * <br/>
 * \tcat Unitaire · Planche a raccords (atelier)<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Tester une image 4x4 cases.<br/>2. Tester des images plus petites, plus grandes, et
 * non carrees.<br/>
 * \tattendu Seule l'image 4x4 cases est reconnue.
 * }
 */
TEST(PixelAutotilePreviewTest, SeuleUneImage4x4EstUnePlancheARaccords) {
    EXPECT_TRUE(hmi::isBitmask16Candidate(sheetImage(4, 4, 0), TILE));
    EXPECT_FALSE(hmi::isBitmask16Candidate(sheetImage(3, 4, 0), TILE));
    EXPECT_FALSE(hmi::isBitmask16Candidate(sheetImage(4, 3, 0), TILE));
    EXPECT_FALSE(hmi::isBitmask16Candidate(sheetImage(8, 8, 0), TILE));
    EXPECT_FALSE(hmi::isBitmask16Candidate(sheetImage(1, 1, 0), TILE));
}

/**
 * @brief Une position dans une planche à raccords résout la bonne case ; une position hors bornes,
 *        ou sur une image qui n'est pas une planche, ne résout rien.
 * \castest{<b>bitmaskCellAtPixel resout la case attendue ou nullopt.</b><br/>
 * \tcat Unitaire · Planche a raccords (atelier)<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Interroger une position au milieu de la case (2,1) d'une planche 4x4.<br/>
 * 2. Interroger une position hors bornes.<br/>3. Interroger la meme position sur une image qui
 * n'est pas une planche.<br/>
 * \tattendu Le premier cas resout (2,1) ; les deux autres renvoient nullopt.
 * }
 */
TEST(PixelAutotilePreviewTest, BitmaskCellAtPixelResoutLaCaseAttendue) {
    const hmi::DecodedImage sheet = sheetImage(4, 4, 0);

    const std::optional<hmi::AutotileCell> cell =
        hmi::bitmaskCellAtPixel(sheet, TILE, 2 * TILE + 1, 1 * TILE + 2);
    ASSERT_TRUE(cell.has_value());
    EXPECT_EQ(cell->column, 2);
    EXPECT_EQ(cell->row, 1);

    EXPECT_FALSE(hmi::bitmaskCellAtPixel(sheet, TILE, sheet.width, 0).has_value());
    EXPECT_FALSE(hmi::bitmaskCellAtPixel(sheet, TILE, -1, 0).has_value());

    const hmi::DecodedImage notASheet = sheetImage(3, 4, 0);
    EXPECT_FALSE(hmi::bitmaskCellAtPixel(notASheet, TILE, 1, 1).has_value());
}

/**
 * @brief L'aperçu d'assemblage assemble la même table que le rendu (`hmi::autotileCell`) : la case
 *        centrale de l'aperçu correspond exactement au contenu de la case « intérieur plein » de
 *        la planche source.
 * \castest{<b>L'apercu d'assemblage utilise la meme table que le rendu.</b><br/>
 * \tcat Unitaire · Planche a raccords (atelier)<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Construire une planche 4x4 dont chaque case a une couleur distincte.<br/>2.
 * Construire l'apercu d'assemblage.<br/>3. Comparer la case centrale de l'apercu a la case attendue
 * de la source (via autotileCell du masque plein).<br/> \tattendu Les pixels de la case centrale de
 * l'apercu sont identiques a ceux de la case source correspondante.
 * }
 */
TEST(PixelAutotilePreviewTest, ApercuDAssemblageUtiliseLaMemeTableQueLeRendu) {
    hmi::DecodedImage sheet = sheetImage(4, 4, 0);
    // Chaque case porte une couleur = son index (colonne + ligne*4), pour la distinguer sans
    // ambiguite dans l'apercu compose.
    for (int row = 0; row < 4; ++row) {
        for (int column = 0; column < 4; ++column) {
            const std::uint32_t color = 0xFF000000u | static_cast<std::uint32_t>(row * 4 + column);
            for (int y = 0; y < TILE; ++y) {
                for (int x = 0; x < TILE; ++x) {
                    hmi::setPixel(sheet, column * TILE + x, row * TILE + y, color);
                }
            }
        }
    }

    const hmi::DecodedImage preview = hmi::buildAutotileAssemblyPreview(sheet, TILE);
    ASSERT_EQ(preview.width, TILE * 3);
    ASSERT_EQ(preview.height, TILE * 3);

    constexpr std::uint8_t fullMask =
        hmi::NEIGHBOR_UP | hmi::NEIGHBOR_RIGHT | hmi::NEIGHBOR_DOWN | hmi::NEIGHBOR_LEFT;
    const hmi::AutotileCell expectedCell = hmi::autotileCell(fullMask);
    const std::uint32_t expectedColor =
        0xFF000000u | static_cast<std::uint32_t>(expectedCell.row * 4 + expectedCell.column);

    // Case centrale de l'apercu 3x3 : colonne/ligne 1 (sur 0..2), soit le pixel (TILE+1, TILE+1).
    EXPECT_EQ(hmi::pickColor(preview, TILE + 1, TILE + 1), expectedColor);
}

/**
 * @brief Une planche plus petite que prévu laisse la case correspondante transparente plutôt que
 *        de lire hors bornes.
 * \castest{<b>Une planche trop petite laisse la case correspondante transparente.</b><br/>
 * \tcat Unitaire · Planche a raccords (atelier)<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Construire l'apercu d'assemblage a partir d'une planche degeneree (1x1 case).<br/>
 * \tattendu Aucune exception ; l'apercu garde ses dimensions attendues.
 * }
 */
TEST(PixelAutotilePreviewTest, PlancheTropPetiteNeDebordePas) {
    const hmi::DecodedImage tinySheet = sheetImage(1, 1, 0xFF000000u);

    hmi::DecodedImage preview;
    EXPECT_NO_THROW(preview = hmi::buildAutotileAssemblyPreview(tinySheet, TILE));
    EXPECT_EQ(preview.width, TILE * 3);
    EXPECT_EQ(preview.height, TILE * 3);
}
