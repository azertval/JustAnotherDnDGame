// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_tile_autotile.cpp
 * @brief Tests unitaires des raccords automatiques (LOT-42, EX-EDIT-025).
 */

#include <array>
#include <cstdint>
#include <set>
#include <string_view>

#include <gtest/gtest.h>

#include "Core/Levels/TileMap.h"
#include "Core/Levels/TileType.h"
#include "HMI/Graphics/TileAutotile.h"

namespace {

/// Grille 5x5 entierement vide, dans laquelle les tests posent des solides.
core::TileMap emptyGrid() {
    return core::TileMap{5, 5};
}

}  // namespace

/**
 * @brief Les seize configurations donnent seize cases distinctes de la planche.
 * \castest{<b>Les seize configurations de voisinage donnent seize cases distinctes.</b><br/>
 * \tcat Unitaire · Raccords automatiques<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Calculer la case de chacun des seize masques possibles.<br/>
 * \tattendu Les seize cases sont deux a deux differentes et tiennent dans la planche 4x4.
 * }
 */
TEST(TileAutotileTest, SeizeConfigurationsSeizeCasesDistinctes) {
    std::set<std::pair<int, int>> cells;
    for (std::uint8_t mask = 0; mask < hmi::AUTOTILE_CONFIGURATION_COUNT; ++mask) {
        const hmi::AutotileCell cell = hmi::autotileCell(mask);

        EXPECT_GE(cell.column, 0);
        EXPECT_LT(cell.column, hmi::AUTOTILE_SHEET_SIDE);
        EXPECT_GE(cell.row, 0);
        EXPECT_LT(cell.row, hmi::AUTOTILE_SHEET_SIDE);

        cells.insert({cell.column, cell.row});
    }

    // Deux configurations partageant une case rendraient une des seize inaccessible a l'auteur.
    EXPECT_EQ(cells.size(), static_cast<std::size_t>(hmi::AUTOTILE_CONFIGURATION_COUNT));
}

/**
 * @brief Chaque masque designe la case attendue de la planche.
 * \castest{<b>Chaque masque de voisinage designe la case attendue de la planche.</b><br/>
 * \tcat Unitaire · Raccords automatiques<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Verifier la case des seize masques, un a un.<br/>
 * \tattendu Chaque masque rend la case documentee dans la table de correspondance.
 * }
 */
TEST(TileAutotileTest, TableDeCorrespondanceExhaustive) {
    // Ancre de non-regression : changer cette table change l'apparence de toutes les planches
    // deja dessinees par un auteur.
    const struct {
        std::uint8_t mask;
        int column;
        int row;
    } expectations[] = {
        {0, 0, 0},
        {hmi::NEIGHBOR_UP, 1, 0},
        {hmi::NEIGHBOR_RIGHT, 2, 0},
        {hmi::NEIGHBOR_UP | hmi::NEIGHBOR_RIGHT, 3, 0},
        {hmi::NEIGHBOR_DOWN, 0, 1},
        {hmi::NEIGHBOR_UP | hmi::NEIGHBOR_DOWN, 1, 1},
        {hmi::NEIGHBOR_RIGHT | hmi::NEIGHBOR_DOWN, 2, 1},
        {hmi::NEIGHBOR_UP | hmi::NEIGHBOR_RIGHT | hmi::NEIGHBOR_DOWN, 3, 1},
        {hmi::NEIGHBOR_LEFT, 0, 2},
        {hmi::NEIGHBOR_UP | hmi::NEIGHBOR_LEFT, 1, 2},
        {hmi::NEIGHBOR_RIGHT | hmi::NEIGHBOR_LEFT, 2, 2},
        {hmi::NEIGHBOR_UP | hmi::NEIGHBOR_RIGHT | hmi::NEIGHBOR_LEFT, 3, 2},
        {hmi::NEIGHBOR_DOWN | hmi::NEIGHBOR_LEFT, 0, 3},
        {hmi::NEIGHBOR_UP | hmi::NEIGHBOR_DOWN | hmi::NEIGHBOR_LEFT, 1, 3},
        {hmi::NEIGHBOR_RIGHT | hmi::NEIGHBOR_DOWN | hmi::NEIGHBOR_LEFT, 2, 3},
        {hmi::NEIGHBOR_UP | hmi::NEIGHBOR_RIGHT | hmi::NEIGHBOR_DOWN | hmi::NEIGHBOR_LEFT, 3, 3},
    };

    for (const auto& expected : expectations) {
        const hmi::AutotileCell cell = hmi::autotileCell(expected.mask);
        EXPECT_EQ(cell.column, expected.column) << "masque " << static_cast<int>(expected.mask);
        EXPECT_EQ(cell.row, expected.row) << "masque " << static_cast<int>(expected.mask);
    }
}

/**
 * @brief L'exterieur de la grille compte comme solide.
 * \castest{<b>L'exterieur de la grille compte comme solide.</b><br/>
 * \tcat Unitaire · Raccords automatiques<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Placer un solide isole dans le coin haut-gauche d'une grille vide.<br/>
 * \tattendu Ses voisins haut et gauche, hors grille, sont comptes solides ; bas et droite non.
 * }
 */
TEST(TileAutotileTest, ExterieurDeLaGrilleCompteSolide) {
    core::TileMap tiles = emptyGrid();
    tiles.setTile(0, 0, core::TileType::Solid);

    // Convention explicite : un mur de bordure ne doit pas dessiner de contour sur sa face
    // invisible, hors du niveau.
    const std::uint8_t mask = hmi::solidNeighborMask(tiles, 0, 0);
    EXPECT_TRUE((mask & hmi::NEIGHBOR_UP) != 0);
    EXPECT_TRUE((mask & hmi::NEIGHBOR_LEFT) != 0);
    EXPECT_FALSE((mask & hmi::NEIGHBOR_RIGHT) != 0);
    EXPECT_FALSE((mask & hmi::NEIGHBOR_DOWN) != 0);
}

/**
 * @brief Une tuile entouree de vide au centre de la grille est isolee.
 * \castest{<b>Une tuile solide entouree de vide donne la configuration isolee.</b><br/>
 * \tcat Unitaire · Raccords automatiques<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Placer un solide au centre d'une grille vide, loin des bords.<br/>
 * \tattendu Son masque est nul et sa case est celle de la tuile isolee.
 * }
 */
TEST(TileAutotileTest, TuileIsoleeAuCentre) {
    core::TileMap tiles = emptyGrid();
    tiles.setTile(2, 2, core::TileType::Solid);

    EXPECT_EQ(hmi::solidNeighborMask(tiles, 2, 2), 0);
    EXPECT_EQ(hmi::autotileCell(0), (hmi::AutotileCell{0, 0}));
}

/**
 * @brief Le dessus d'une plateforme differe de son interieur.
 * \castest{<b>Le dessus d'une plateforme pleine differe de son interieur.</b><br/>
 * \tcat Unitaire · Raccords automatiques<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Remplir de solide les lignes 2 a 4 d'une grille vide.<br/>
 * 2. Comparer la case de la ligne de surface et celle d'une case enfouie.<br/>
 * \tattendu La surface rend la case « bord haut », l'interieur la case « plein » ; elles different.
 * }
 */
TEST(TileAutotileTest, DessusDePlateformeDistinctDeLInterieur) {
    core::TileMap tiles = emptyGrid();
    for (int row = 2; row < 5; ++row) {
        for (int column = 0; column < 5; ++column) {
            tiles.setTile(column, row, core::TileType::Solid);
        }
    }

    // C'est exactement ce que le rendu en couleurs plates ne savait pas faire : la grille restait
    // visible et le dessus d'une plateforme etait identique a son interieur.
    const hmi::AutotileCell surface = hmi::autotileCell(hmi::solidNeighborMask(tiles, 2, 2));
    const hmi::AutotileCell inside = hmi::autotileCell(hmi::solidNeighborMask(tiles, 2, 3));

    EXPECT_EQ(surface, (hmi::AutotileCell{2, 3}));  // droite + bas + gauche = bord haut
    EXPECT_EQ(inside, (hmi::AutotileCell{3, 3}));   // les quatre voisins = interieur plein
    EXPECT_FALSE(surface == inside);
}

/**
 * @brief Les blocs poussables se raccordent aux murs.
 * \castest{<b>Le voisinage est celui de la solidite, pas du type de tuile.</b><br/>
 * \tcat Unitaire · Raccords automatiques<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Placer un solide et, a sa droite, un bloc poussable.<br/>
 * \tattendu Le solide compte le bloc comme voisin solide.
 * }
 */
TEST(TileAutotileTest, RaccordEntreTypesSolidesDifferents) {
    core::TileMap tiles = emptyGrid();
    tiles.setTile(2, 2, core::TileType::Solid);
    tiles.setTile(3, 2, core::TileType::Block);

    // Se raccorder au seul type identique laisserait une couture visible partout ou un bloc jouxte
    // un mur, alors que les deux forment visuellement la meme matiere.
    EXPECT_TRUE((hmi::solidNeighborMask(tiles, 2, 2) & hmi::NEIGHBOR_RIGHT) != 0);
}

/**
 * @brief La case representative est l'interieur plein.
 * \castest{<b>La case representative d'une planche est son interieur plein.</b><br/>
 * \tcat Unitaire · Raccords automatiques<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Demander la case representative de la planche.<br/>
 * \tattendu Elle est celle du masque a quatre voisins, et non la case zero.
 * }
 */
TEST(TileAutotileTest, CaseRepresentativeEstLInterieurPlein) {
    const hmi::AutotileCell full = hmi::autotileCell(hmi::NEIGHBOR_UP | hmi::NEIGHBOR_RIGHT |
                                                     hmi::NEIGHBOR_DOWN | hmi::NEIGHBOR_LEFT);

    EXPECT_EQ(hmi::autotileRepresentativeCell(), full);
    EXPECT_FALSE(hmi::autotileRepresentativeCell() == (hmi::AutotileCell{0, 0}));
}

/**
 * @brief Les bits hors des quatre voisins sont ignores.
 * \castest{<b>Un masque portant des bits parasites reste dans la planche.</b><br/>
 * \tcat Unitaire · Raccords automatiques<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Demander la case d'un masque dont les bits de poids fort sont a 1.<br/>
 * \tattendu La case est celle des quatre bits de poids faible, sans depassement de table.
 * }
 */
TEST(TileAutotileTest, BitsParasitesIgnores) {
    constexpr std::uint8_t noisy = 0xF0 | hmi::NEIGHBOR_UP;

    EXPECT_EQ(hmi::autotileCell(noisy), hmi::autotileCell(hmi::NEIGHBOR_UP));
}

/**
 * @brief Chacun des seize masques produit une clé de libellé distincte (atelier pixel art, LOT-54
 *        TACHE-08) : aucune case de la planche ne partage le libellé d'une autre.
 * \castest{<b>Les seize masques produisent seize cles de libelle distinctes.</b><br/>
 * \tcat Unitaire · Raccords automatiques<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Produire la cle de libelle de chacun des seize masques.<br/>
 * \tattendu Les seize cles sont deux a deux differentes.
 * }
 */
TEST(TileAutotileTest, SeizeMasquesProduisentSeizeClesDeLibelleDistinctes) {
    std::set<std::string_view> keys;
    for (int mask = 0; mask < hmi::AUTOTILE_CONFIGURATION_COUNT; ++mask) {
        keys.insert(hmi::autotileConfigurationLabelKey(static_cast<std::uint8_t>(mask)));
    }
    EXPECT_EQ(keys.size(), static_cast<std::size_t>(hmi::AUTOTILE_CONFIGURATION_COUNT));
}

/**
 * @brief Les bits parasites d'un masque de libellé sont ignorés, comme pour `autotileCell`.
 * \castest{<b>Un masque de libelle portant des bits parasites reste dans la table.</b><br/>
 * \tcat Unitaire · Raccords automatiques<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Demander le libelle d'un masque dont les bits de poids fort sont a 1.<br/>
 * \tattendu Le libelle est celui des quatre bits de poids faible.
 * }
 */
TEST(TileAutotileTest, LibelleIgnoreLesBitsParasites) {
    constexpr std::uint8_t noisy = 0xF0 | hmi::NEIGHBOR_UP;
    EXPECT_EQ(hmi::autotileConfigurationLabelKey(noisy),
              hmi::autotileConfigurationLabelKey(hmi::NEIGHBOR_UP));
}

/**
 * @brief L'assemblage 3×3 démonstratif place l'intérieur plein (les quatre voisins internes
 *        solides) au centre, et couvre coins/bords/intérieur — un panorama représentatif des
 *        seize configurations, pas seulement un sous-ensemble.
 * \castest{<b>L'assemblage 3x3 place l'interieur plein au centre.</b><br/>
 * \tcat Unitaire · Raccords automatiques<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Produire les neuf masques de l'assemblage.<br/>2. Verifier la case centrale et la
 * diversite des masques.<br/>
 * \tattendu Le masque central vaut le masque plein (quatre voisins) ; les neuf masques ne sont pas
 * tous identiques.
 * }
 */
TEST(TileAutotileTest, AssemblageTroisParTroisPlaceLInterieurPleinAuCentre) {
    const std::array<std::uint8_t, 9> masks = hmi::autotileAssemblyMasks();

    constexpr std::uint8_t fullMask =
        hmi::NEIGHBOR_UP | hmi::NEIGHBOR_RIGHT | hmi::NEIGHBOR_DOWN | hmi::NEIGHBOR_LEFT;
    EXPECT_EQ(masks[4], fullMask) << "case centrale (index 4 = ligne 1, colonne 1)";

    const std::set<std::uint8_t> distinctMasks(masks.begin(), masks.end());
    EXPECT_GT(distinctMasks.size(), 1U) << "l'assemblage doit couvrir plusieurs configurations";
}

/**
 * @brief Chaque masque de l'assemblage 3×3 résout, via la table canonique, une case valide de la
 *        planche 4×4 — jamais une seconde table qui pourrait diverger.
 * \castest{<b>Chaque masque de l'assemblage resout une case valide de la planche.</b><br/>
 * \tcat Unitaire · Raccords automatiques<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Resoudre la case de chacun des neuf masques via autotileCell.<br/>
 * \tattendu Chaque case tient dans la planche 4x4 (colonne et ligne entre 0 et 3).
 * }
 */
TEST(TileAutotileTest, AssemblageResoutDesCasesValidesViaLaTableCanonique) {
    for (const std::uint8_t mask : hmi::autotileAssemblyMasks()) {
        const hmi::AutotileCell cell = hmi::autotileCell(mask);
        EXPECT_GE(cell.column, 0);
        EXPECT_LT(cell.column, hmi::AUTOTILE_SHEET_SIDE);
        EXPECT_GE(cell.row, 0);
        EXPECT_LT(cell.row, hmi::AUTOTILE_SHEET_SIDE);
    }
}
