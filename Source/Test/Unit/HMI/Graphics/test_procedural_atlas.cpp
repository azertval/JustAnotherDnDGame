// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_procedural_atlas.cpp
 * @brief Tests unitaires de la génération procédurale de l'atlas de repli (LOT-39, EX-NFR-040).
 */

#include <map>

#include <gtest/gtest.h>

#include "Core/Levels/TileType.h"
#include "Core/Levels/TileTypeName.h"
#include "HMI/Graphics/ProceduralAtlas.h"
#include "HMI/Graphics/TextureAtlas.h"
#include "HMI/Graphics/TileVisuals.h"

/**
 * @brief L'image générée a les dimensions attendues : la grille de tuiles, plus une ligne
 *        d'images de personnage par tranche de `PLAYER_FRAME_COLUMNS` images.
 * \castest{<b>L'image générée a les dimensions attendues (grille de tuiles + lignes de
 * personnage).</b><br/>
 * \tcat Unitaire · Procedural Atlas<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu L'image générée a les dimensions attendues.
 * }
 */
TEST(ProceduralAtlasTest, DimensionsAttendues) {
    const hmi::ProceduralAtlasImage image = hmi::buildProceduralAtlasImage();

    const int gridSide = hmi::TextureAtlas::TILE_SIZE * hmi::TextureAtlas::TILES_PER_SIDE;
    const int totalFrames =
        hmi::PLAYER_IDLE_FRAME_COUNT + hmi::PLAYER_RUN_FRAME_COUNT + hmi::PLAYER_JUMP_FRAME_COUNT;
    const int frameRows = (totalFrames + hmi::TextureAtlas::PLAYER_FRAME_COLUMNS - 1) /
                          hmi::TextureAtlas::PLAYER_FRAME_COLUMNS;

    EXPECT_EQ(image.width, gridSide);
    EXPECT_EQ(image.height, gridSide + frameRows * hmi::TextureAtlas::PLAYER_FRAME_SIZE);
    EXPECT_EQ(image.pixels.size(),
              static_cast<std::size_t>(image.width) * static_cast<std::size_t>(image.height));
}

/**
 * @brief La génération est déterministe : deux appels produisent des pixels identiques.
 * \castest{<b>La génération est déterministe : deux appels produisent des pixels
 * identiques.</b><br/>
 * \tcat Unitaire · Procedural Atlas<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu La génération est déterministe.
 * }
 */
TEST(ProceduralAtlasTest, GenerationDeterministe) {
    const hmi::ProceduralAtlasImage first = hmi::buildProceduralAtlasImage();
    const hmi::ProceduralAtlasImage second = hmi::buildProceduralAtlasImage();

    EXPECT_EQ(first.width, second.width);
    EXPECT_EQ(first.height, second.height);
    EXPECT_EQ(first.pixels, second.pixels);
}

/**
 * @brief La dernière tuile de la grille (damier de transparence) contient à la fois des pixels
 *        opaques et des pixels transparents, pour exercer le canal alpha du rendu.
 * \castest{<b>La dernière tuile de la grille contient des pixels opaques et transparents
 * (damier).</b><br/>
 * \tcat Unitaire · Procedural Atlas<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu La dernière tuile de la grille contient des pixels opaques et transparents.
 * }
 */
TEST(ProceduralAtlasTest, DamierDeTransparenceDansLaDerniereTuile) {
    const hmi::ProceduralAtlasImage image = hmi::buildProceduralAtlasImage();

    const int tileSize = hmi::TextureAtlas::TILE_SIZE;
    const int lastColumn = hmi::TextureAtlas::TILES_PER_SIDE - 1;
    const int lastRow = hmi::TextureAtlas::TILES_PER_SIDE - 1;
    const int originX = lastColumn * tileSize;
    const int originY = lastRow * tileSize;

    bool sawOpaque = false;
    bool sawTransparent = false;
    for (int y = 0; y < tileSize && !(sawOpaque && sawTransparent); ++y) {
        for (int x = 0; x < tileSize && !(sawOpaque && sawTransparent); ++x) {
            const std::uint32_t pixel = image.pixels[static_cast<std::size_t>(originY + y) *
                                                         static_cast<std::size_t>(image.width) +
                                                     static_cast<std::size_t>(originX + x)];
            const std::uint32_t alpha = (pixel >> 24) & 0xFF;
            if (alpha == 0) {
                sawTransparent = true;
            } else {
                sawOpaque = true;
            }
        }
    }
    EXPECT_TRUE(sawOpaque);
    EXPECT_TRUE(sawTransparent);
}

/**
 * @brief Chaque type de tuile a une couleur de repli **visible et distincte** dans l'atlas
 * procédural : le jeu affiche une carte sans aucun fichier d'image (`EX-NFR-040`, `LOT-08`).
 *
 * C'est le garde-fou que le vocabulaire de terrain du `LOT-08` réclamait : ajouter un type sans
 * lui peindre de repli laisserait sa case noire, indiscernable de ses voisines — une carte
 * illisible plutôt qu'une carte laide.
 * \castest{<b>Chaque type de tuile a une couleur de repli visible et distincte.</b><br/>
 * \tcat Unitaire · Atlas procedural<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Generer l'atlas procedural.<br/>2. Echantillonner le centre de la case de chaque
 * type de tuile, hors case vide.<br/>
 * \tattendu Aucune couleur n'est noire, et deux types n'en partagent jamais une.
 * }
 */
TEST(ProceduralAtlasTest, ChaqueTypeDeTuileAUneCouleurDeRepliDistincte) {
    const hmi::ProceduralAtlasImage image = hmi::buildProceduralAtlasImage();
    const int tileSize = hmi::TextureAtlas::TILE_SIZE;

    std::map<std::uint32_t, core::TileType> seen;
    for (int raw = 0; raw < core::TILE_TYPE_COUNT; ++raw) {
        const auto type = static_cast<core::TileType>(raw);
        if (type == core::TileType::Empty) {
            continue;  // la case vide n'est jamais dessinee : sa region est arbitraire.
        }
        const core::AtlasRegion region = hmi::regionForTile(type);
        const int x = region.x + tileSize / 2;
        const int y = region.y + tileSize / 2;
        const std::uint32_t color =
            image.pixels[static_cast<std::size_t>(y) * static_cast<std::size_t>(image.width) +
                         static_cast<std::size_t>(x)];

        EXPECT_NE(color, 0xFF000000u) << core::tileTypeName(type) << " : case restee noire";
        const auto inserted = seen.emplace(color, type);
        EXPECT_TRUE(inserted.second) << core::tileTypeName(type) << " partage sa couleur avec "
                                     << core::tileTypeName(inserted.first->second);
    }
}
