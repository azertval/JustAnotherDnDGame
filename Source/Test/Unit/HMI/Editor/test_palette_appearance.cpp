// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_palette_appearance.cpp
 * @brief Tests unitaires du choix de vignette de la palette (LOT-42 TACHE-05, EX-EDIT-027).
 */

#include <gtest/gtest.h>

#include "Core/Levels/TileType.h"
#include "HMI/Editor/PaletteAppearance.h"
#include "HMI/Graphics/MissingTexture.h"
#include "HMI/Graphics/RenderMode.h"
#include "HMI/Graphics/SkinCatalog.h"
#include "HMI/Graphics/TextureAtlas.h"
#include "HMI/Graphics/TileAppearance.h"
#include "HMI/Graphics/TileAutotile.h"
#include "HMI/Graphics/TileSkinTag.h"
#include "HMI/Graphics/TileVisuals.h"

namespace {

constexpr int TILE = hmi::TextureAtlas::TILE_SIZE;

/// Catalogue de reference : un type a raccords et un type a image unique.
hmi::SkinCatalog referenceCatalog() {
    hmi::SkinCatalog catalog;
    catalog.assign("foret", core::TileType::Solid,
                   hmi::SkinEntry{"stone.png", hmi::SkinMode::Bitmask16});
    catalog.assign("foret", core::TileType::Block,
                   hmi::SkinEntry{"crate.png", hmi::SkinMode::Single});
    return catalog;
}

}  // namespace

/**
 * @brief En mode Physique, la palette montre la couleur plate.
 * \castest{<b>En mode Physique, la palette montre la couleur plate du type.</b><br/>
 * \tcat Unitaire · Vignette de palette<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Demander la vignette de Solid en mode Physique, alors qu'un skin lui est
 * assigne.<br/> \tattendu La vignette vient de l'atlas, a la region d'atlas du type.
 * }
 */
TEST(PaletteAppearanceTest, ModePhysiqueMontreLaCouleurPlate) {
    const hmi::SkinCatalog catalog = referenceCatalog();
    const core::AtlasRegion physical = hmi::regionForTile(core::TileType::Solid);

    const hmi::PaletteThumbnail thumbnail = hmi::paletteThumbnail(
        hmi::RenderMode::Physique, core::TileType::Solid, physical, &catalog, "foret");

    EXPECT_EQ(thumbnail.source, hmi::PaletteThumbnailSource::Atlas);
    EXPECT_EQ(thumbnail.region.x, physical.x);
    EXPECT_EQ(thumbnail.region.y, physical.y);
    EXPECT_FALSE(thumbnail.masked);
}

/**
 * @brief En mode Texture, la palette montre le skin assigne.
 * \castest{<b>En mode Texture, la palette montre le fichier reellement assigne.</b><br/>
 * \tcat Unitaire · Vignette de palette<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Demander la vignette de Block, assigne a un skin en mode image unique.<br/>
 * \tattendu La vignette vient du skin, nomme son fichier, et couvre une case entiere.
 * }
 */
TEST(PaletteAppearanceTest, ModeTextureMontreLeSkin) {
    const hmi::SkinCatalog catalog = referenceCatalog();

    const hmi::PaletteThumbnail thumbnail =
        hmi::paletteThumbnail(hmi::RenderMode::Texture, core::TileType::Block,
                              hmi::regionForTile(core::TileType::Block), &catalog, "foret");

    // Sans cela, le level designer choisirait « la tuile violette » sans voir ce qu'il pose.
    EXPECT_EQ(thumbnail.source, hmi::PaletteThumbnailSource::Skin);
    EXPECT_EQ(thumbnail.asset, "crate.png");
    EXPECT_EQ(thumbnail.region.width, TILE);
    EXPECT_FALSE(thumbnail.masked);
}

/**
 * @brief Un skin a raccords montre sa case representative.
 * \castest{<b>Un skin a raccords montre l'interieur plein de sa planche.</b><br/>
 * \tcat Unitaire · Vignette de palette<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Demander la vignette de Solid, assigne a une planche a raccords.<br/>
 * \tattendu La region est celle de la case representative, et non la case zero.
 * }
 */
TEST(PaletteAppearanceTest, SkinARaccordsMontreLaCaseRepresentative) {
    const hmi::SkinCatalog catalog = referenceCatalog();

    const hmi::PaletteThumbnail thumbnail =
        hmi::paletteThumbnail(hmi::RenderMode::Texture, core::TileType::Solid,
                              hmi::regionForTile(core::TileType::Solid), &catalog, "foret");

    // La planche entiere serait illisible en vignette, et la case zero (tuile isolee) serait un
    // cas particulier peu representatif du motif.
    const hmi::AutotileCell cell = hmi::autotileRepresentativeCell();
    EXPECT_EQ(thumbnail.region.x, cell.column * TILE);
    EXPECT_EQ(thumbnail.region.y, cell.row * TILE);
    EXPECT_EQ(thumbnail.region.width, TILE);
}

/**
 * @brief Un type non skinne montre le damier dans la palette.
 * \castest{<b>Un type non skinne montre le damier dans la palette, comme dans le niveau.</b><br/>
 * \tcat Unitaire · Vignette de palette<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Demander la vignette d'un type absent du catalogue, en mode Texture.<br/>
 * 2. Demander une vignette sans catalogue du tout.<br/>
 * \tattendu Les deux rendent le damier en entier.
 * }
 */
TEST(PaletteAppearanceTest, TypeNonSkinneMontreLeDamier) {
    const hmi::SkinCatalog catalog = referenceCatalog();

    // La palette doit SIGNALER ce qui reste a habiller, pas le masquer.
    const hmi::PaletteThumbnail unassigned =
        hmi::paletteThumbnail(hmi::RenderMode::Texture, core::TileType::Danger,
                              hmi::regionForTile(core::TileType::Danger), &catalog, "foret");
    EXPECT_EQ(unassigned.source, hmi::PaletteThumbnailSource::MissingTexture);
    EXPECT_EQ(unassigned.region.width, hmi::MISSING_TEXTURE_SIZE);

    const hmi::PaletteThumbnail noCatalog =
        hmi::paletteThumbnail(hmi::RenderMode::Texture, core::TileType::Solid,
                              hmi::regionForTile(core::TileType::Solid), nullptr, "foret");
    EXPECT_EQ(noCatalog.source, hmi::PaletteThumbnailSource::MissingTexture);
}

/**
 * @brief La palette et le canevas prennent la meme decision.
 * \castest{<b>La palette et le canevas resolvent la meme source pour chaque type.</b><br/>
 * \tcat Unitaire · Vignette de palette<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Pour plusieurs types et les deux modes, comparer la source choisie par la palette a
 * celle choisie par la resolution du canevas.<br/>
 * \tattendu Les deux designent toujours la meme origine d'image.
 * }
 */
TEST(PaletteAppearanceTest, PaletteEtCanevasDecidentPareil) {
    const hmi::SkinCatalog catalog = referenceCatalog();

    hmi::SceneTextures textures;
    textures.missingWidth = hmi::MISSING_TEXTURE_SIZE;
    textures.missingHeight = hmi::MISSING_TEXTURE_SIZE;
    textures.skins = {
        hmi::SkinTexture{"stone.png", std::nullopt, nullptr, TILE * 4, TILE * 4},
        hmi::SkinTexture{"crate.png", std::nullopt, nullptr, TILE, TILE},
    };
    textures.skinCatalog = &catalog;
    textures.skinSet = "foret";

    // Une palette qui divergerait du canevas ferait poser autre chose que ce qui est montre.
    for (const hmi::RenderMode mode : {hmi::RenderMode::Physique, hmi::RenderMode::Texture}) {
        for (const core::TileType type :
             {core::TileType::Solid, core::TileType::Block, core::TileType::Danger}) {
            const core::AtlasRegion physical = hmi::regionForTile(type);
            const hmi::TileSkinTag tag{type, 0};

            const hmi::PaletteThumbnail thumbnail =
                hmi::paletteThumbnail(mode, type, physical, &catalog, "foret");
            const hmi::TileAppearance appearance =
                hmi::resolveTileAppearance(mode, physical, &tag, textures).value();

            const bool sameAtlas = thumbnail.source == hmi::PaletteThumbnailSource::Atlas &&
                                   appearance.source == hmi::AppearanceSource::Atlas;
            const bool sameSkin = thumbnail.source == hmi::PaletteThumbnailSource::Skin &&
                                  appearance.source == hmi::AppearanceSource::Skin;
            const bool sameMissing =
                thumbnail.source == hmi::PaletteThumbnailSource::MissingTexture &&
                appearance.source == hmi::AppearanceSource::MissingTexture;

            EXPECT_TRUE(sameAtlas || sameSkin || sameMissing)
                << "divergence palette/canevas pour le type " << static_cast<int>(type)
                << " en mode " << static_cast<int>(mode);
        }
    }
}
