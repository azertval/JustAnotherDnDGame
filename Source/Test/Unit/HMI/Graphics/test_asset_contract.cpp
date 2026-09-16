// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

/**
 * @file test_asset_contract.cpp
 * @brief Tests unitaires du contrat de dimensions des assets graphiques (LOT-40, EX-REN-007).
 */

#include <gtest/gtest.h>

#include "HMI/Graphics/AssetContract.h"
#include "HMI/Graphics/TextureAtlas.h"

namespace {

/// Recherche d'un fragment dans un message, pour les assertions sur le texte d'erreur.
bool contains(const std::string& text, const std::string& fragment) {
    return text.find(fragment) != std::string::npos;
}

}  // namespace

/**
 * @brief Un skin de tuile exactement à la taille d'une case est accepté.
 * \castest{<b>Un skin de tuile de 16x16 px est accepte.</b><br/>
 * \tcat Unitaire · Asset Contract<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Valider un asset de la famille « skin de tuile » aux dimensions d'une case.<br/>
 * \tattendu La validation reussit et le message est vide.
 * }
 */
TEST(AssetContractTest, SkinDeTuileConformeAccepte) {
    const hmi::AssetValidation validation =
        hmi::validateAsset(hmi::AssetFamily::TileSkin, "mur.png", hmi::TextureAtlas::TILE_SIZE,
                           hmi::TextureAtlas::TILE_SIZE);

    EXPECT_TRUE(validation.valid);
    EXPECT_TRUE(validation.message.empty());
}

/**
 * @brief Un skin de tuile aux mauvaises dimensions est refusé, avec un message nommant le
 *        fichier, la dimension trouvée et la dimension attendue.
 * \castest{<b>Un skin de tuile non conforme est refuse avec un message exploitable.</b><br/>
 * \tcat Unitaire · Asset Contract<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Valider un skin de tuile de 24x16 px.<br/>2. Inspecter le message d'erreur.<br/>
 * \tattendu La validation echoue et le message nomme le fichier, le trouve et l'attendu.
 * }
 */
TEST(AssetContractTest, SkinDeTuileNonConformeRefuseAvecMessage) {
    const hmi::AssetValidation validation =
        hmi::validateAsset(hmi::AssetFamily::TileSkin, "mur.png", 24, 16);

    EXPECT_FALSE(validation.valid);
    EXPECT_TRUE(contains(validation.message, "mur.png")) << validation.message;
    EXPECT_TRUE(contains(validation.message, "24x16")) << validation.message;
    EXPECT_TRUE(contains(validation.message, "16x16")) << validation.message;
}

/**
 * @brief Un skin de tuile ANIME (`LOT-46`) est une spritesheet horizontale sur un seul rang :
 *        plusieurs cases de large, une case de haut. La cohérence fine (nombre d'images, indices
 *        référencés par les clips) relève de `hmi::AnimationCatalog::validateAgainstTexture`.
 * \castest{<b>Une spritesheet animee (plusieurs cases de large, une case de haut) est
 * acceptee.</b><br/>
 * \tcat Unitaire · Asset Contract<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Valider un skin de tuile de 64x16 px (4 images).<br/>
 * \tattendu La validation reussit.
 * }
 */
TEST(AssetContractTest, SkinDeTuileAnimeSpritesheetHorizontaleAcceptee) {
    const hmi::AssetValidation validation =
        hmi::validateAsset(hmi::AssetFamily::TileSkin, "water.png",
                           hmi::TextureAtlas::TILE_SIZE * 4, hmi::TextureAtlas::TILE_SIZE);

    EXPECT_TRUE(validation.valid) << validation.message;
}

/**
 * @brief Un skin de tuile de plus d'un rang (hauteur multiple de la case, mais pas exactement une
 *        case) reste refusé : la spritesheet animée n'a jamais qu'un seul rang (`LOT-46`).
 * \castest{<b>Une image sur plusieurs rangs (hauteur non conforme) est refusee.</b><br/>
 * \tcat Unitaire · Asset Contract<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Valider un skin de tuile de 16x32 px.<br/>
 * \tattendu La validation echoue.
 * }
 */
TEST(AssetContractTest, SkinDeTuileSurPlusieursRangsRefuse) {
    const hmi::AssetValidation validation =
        hmi::validateAsset(hmi::AssetFamily::TileSkin, "mur.png", hmi::TextureAtlas::TILE_SIZE,
                           hmi::TextureAtlas::TILE_SIZE * 2);

    EXPECT_FALSE(validation.valid);
}

/**
 * @brief Une planche à raccords doit être un multiple de la taille de tuile et compter au moins
 *        4x4 cases.
 * \castest{<b>Une planche a raccords exige un multiple de case et au moins 4x4 cases.</b><br/>
 * \tcat Unitaire · Asset Contract<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Valider une planche 4x4 cases, puis 3x4 cases, puis un multiple rompu.<br/>
 * \tattendu Seule la planche 4x4 cases est acceptee.
 * }
 */
TEST(AssetContractTest, PlancheARaccordsExigeQuatreCasesMinimum) {
    constexpr int TILE = hmi::TextureAtlas::TILE_SIZE;

    EXPECT_TRUE(
        hmi::validateAsset(hmi::AssetFamily::AutotileSheet, "sol.png", 4 * TILE, 4 * TILE).valid);
    EXPECT_FALSE(
        hmi::validateAsset(hmi::AssetFamily::AutotileSheet, "sol.png", 3 * TILE, 4 * TILE).valid);
    EXPECT_FALSE(
        hmi::validateAsset(hmi::AssetFamily::AutotileSheet, "sol.png", 4 * TILE + 1, 4 * TILE)
            .valid);
}

/**
 * @brief Un fond de niveau accepte n'importe quelles dimensions strictement positives : il est
 *        étiré au rendu et n'est jamais découpé en grille.
 * \castest{<b>Un fond de niveau accepte toute dimension strictement positive.</b><br/>
 * \tcat Unitaire · Asset Contract<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Valider un fond aux dimensions quelconques, puis un fond de largeur nulle.<br/>
 * \tattendu Le premier est accepte, le second refuse.
 * }
 */
TEST(AssetContractTest, FondDeNiveauSansContrainteDeGrille) {
    EXPECT_TRUE(hmi::validateAsset(hmi::AssetFamily::Background, "foret.png", 640, 361).valid);
    EXPECT_FALSE(hmi::validateAsset(hmi::AssetFamily::Background, "foret.png", 0, 361).valid);
}

/**
 * @brief Une spritesheet de personnage doit se découper en cases entières.
 * \castest{<b>Une spritesheet de personnage doit se decouper en cases entieres.</b><br/>
 * \tcat Unitaire · Asset Contract<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Valider une spritesheet multiple de la case, puis une spritesheet decalee.<br/>
 * \tattendu Seule la spritesheet multiple de la case est acceptee.
 * }
 */
TEST(AssetContractTest, SpritesheetPersonnageMultipleDeCase) {
    constexpr int TILE = hmi::TextureAtlas::TILE_SIZE;

    EXPECT_TRUE(
        hmi::validateAsset(hmi::AssetFamily::CharacterSheet, "heros.png", 5 * TILE, 3 * TILE)
            .valid);
    EXPECT_FALSE(
        hmi::validateAsset(hmi::AssetFamily::CharacterSheet, "heros.png", 5 * TILE, 3 * TILE + 7)
            .valid);
}

/**
 * @brief L'atlas historique reste conforme à son propre contrat : la validation introduite par ce
 *        lot ne peut pas refuser l'asset déjà livré.
 * \castest{<b>L'atlas historique reste conforme au contrat de sa famille.</b><br/>
 * \tcat Unitaire · Asset Contract<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Valider les dimensions de la grille d'atlas livree.<br/>
 * \tattendu La validation reussit.
 * }
 */
TEST(AssetContractTest, AtlasLivreConforme) {
    const int gridSide = hmi::TextureAtlas::TILE_SIZE * hmi::TextureAtlas::TILES_PER_SIDE;

    EXPECT_TRUE(hmi::validateAsset(hmi::AssetFamily::Atlas, "atlas.png", gridSide,
                                   gridSide + 2 * hmi::TextureAtlas::PLAYER_FRAME_SIZE)
                    .valid);
}

/**
 * @brief Chaque famille d'asset porte un nom lisible, repris dans les messages d'erreur.
 * \castest{<b>Chaque famille d'asset porte un nom lisible non vide.</b><br/>
 * \tcat Unitaire · Asset Contract<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Nommer chaque famille declaree.<br/>
 * \tattendu Aucun nom n'est vide ni « Inconnu ».
 * }
 */
TEST(AssetContractTest, ToutesLesFamillesSontNommees) {
    const hmi::AssetFamily families[] = {
        hmi::AssetFamily::Atlas,      hmi::AssetFamily::TileSkin, hmi::AssetFamily::AutotileSheet,
        hmi::AssetFamily::Background, hmi::AssetFamily::Object,   hmi::AssetFamily::CharacterSheet};

    for (const hmi::AssetFamily family : families) {
        const std::string name = hmi::assetFamilyName(family);
        EXPECT_FALSE(name.empty());
        EXPECT_NE(name, "Inconnu");
    }
}
