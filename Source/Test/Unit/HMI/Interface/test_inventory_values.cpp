// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_inventory_values.cpp
 * @brief Tests unitaires des valeurs affichées par l'équipement (`LOT-14`, `LOT-38`). Logique
 *        pure, sans Qt ni fichier.
 */

#include <map>
#include <string>

#include <gtest/gtest.h>

#include "HMI/Interface/InventoryValues.h"

namespace {

/// Un catalogue réduit aux quatre objets du sac d'essai. Écrit ici, et non lu dans la donnée
/// livrée : un test qui ouvre les catalogues vérifie deux choses à la fois, et tombe le jour où la
/// donnée bouge — pour la mauvaise raison.
[[nodiscard]] core::ItemCatalog cataloguesDObjets() {
    core::ItemCatalog catalogue;
    catalogue.items = {
        {.id = "torch", .name = "Torche"},
        {.id = "rope-hemp", .name = "Corde en chanvre (15 m)"},
        {.id = "rations", .name = "Rations (1 jour)"},
        {.id = "waterskin", .name = "Gourde (pleine)"},
    };
    return catalogue;
}

[[nodiscard]] core::Inventory sacDEssai() {
    core::Inventory sac;
    sac.backpack = {{.itemId = "torch", .quantity = 5},
                    {.itemId = "rope-hemp", .quantity = 1},
                    {.itemId = "rations", .quantity = 4},
                    // Une pile VIDE, qui doit être sautée : une ligne « Gourde ×0 » à l'écran se
                    // lirait comme un objet possédé, alors qu'il ne l'est pas.
                    {.itemId = "waterskin", .quantity = 0}};
    sac.purseCopper = 4275;
    return sac;
}

}  // namespace

TEST(InventoryValuesTest, LeSacSortPileParPileNomEtQuantiteSepares) {
    const core::Inventory sac = sacDEssai();
    const core::ItemCatalog objets = cataloguesDObjets();
    const std::map<std::string, std::string> valeurs =
        hmi::inventoryValues({.inventory = &sac, .lookup = {.items = &objets}});

    // Trois piles, pas quatre : celle dont la quantité est nulle n'existe pas.
    EXPECT_EQ(valeurs.at("inventory.backpack.count"), "3");
    EXPECT_EQ(valeurs.at("inventory.backpack.0.name"), "Torche");
    EXPECT_EQ(valeurs.at("inventory.backpack.0.quantity"), "×5");
    EXPECT_EQ(valeurs.at("inventory.backpack.2.name"), "Rations (1 jour)");
    EXPECT_EQ(valeurs.at("inventory.backpack.2.quantity"), "×4");
    EXPECT_EQ(valeurs.count("inventory.backpack.3.name"), 0U);
}

TEST(InventoryValuesTest, LaQuantiteEstPubliueMemeQuandElleVautUn) {
    const core::Inventory sac = sacDEssai();
    const core::ItemCatalog objets = cataloguesDObjets();
    const std::map<std::string, std::string> valeurs =
        hmi::inventoryValues({.inventory = &sac, .lookup = {.items = &objets}});

    // La planche aligne une colonne de quantités : une case vide y ferait un trou, là où « ×1 »
    // dit « un seul ». C'est la forme en UNE chaîne, elle, qui omet le « ×1 ».
    EXPECT_EQ(valeurs.at("inventory.backpack.1.quantity"), "×1");
    EXPECT_EQ(valeurs.at("inventory.backpack"),
              "Torche ×5\nCorde en chanvre (15 m)\nRations (1 jour) ×4");
}

TEST(InventoryValuesTest, UnSacVideCompteZeroPileEtNonAucune) {
    core::Inventory sac;
    sac.purseCopper = 0;
    const std::map<std::string, std::string> valeurs = hmi::inventoryValues({.inventory = &sac});

    // La clé est écrite dans TOUS les cas : « zéro pile » est une vérité, et son absence
    // obligerait la planche à distinguer « sac vide » de « valeurs pas encore posées ».
    EXPECT_EQ(valeurs.at("inventory.backpack.count"), "0");
    EXPECT_EQ(valeurs.at("inventory.backpack"), "—");
}

TEST(InventoryValuesTest, SansInventaireAucuneValeurNEstProduite) {
    const std::map<std::string, std::string> valeurs = hmi::inventoryValues({});
    EXPECT_TRUE(valeurs.empty());
}

TEST(InventoryValuesTest, LaBourseSeRepartitEnOrArgentEtCuivre) {
    const core::Inventory sac = sacDEssai();
    const std::map<std::string, std::string> valeurs = hmi::inventoryValues({.inventory = &sac});
    // 4275 pièces de cuivre : 42 or, 7 argent, 5 cuivre.
    EXPECT_EQ(valeurs.at("inventory.purse"), "42 po 7 pa 5 pc");
}

TEST(InventoryValuesTest, UnObjetInconnuDuCatalogueGardeSonIdentifiant) {
    core::Inventory sac;
    sac.backpack = {{.itemId = "objet-fantome", .quantity = 2}};
    const std::map<std::string, std::string> valeurs = hmi::inventoryValues({.inventory = &sac});

    // L'identifiant, et non le tiret : un objet que le catalogue ignore doit se VOIR. Le tiret le
    // ferait passer pour un emplacement vide, ce qui est le contraire de la vérité.
    EXPECT_EQ(valeurs.at("inventory.backpack.0.name"), "objet-fantome");
}
