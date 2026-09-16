// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

/**
 * @file test_inventory_screen.cpp
 * @brief Tests unitaires de la logique de l'écran d'inventaire (LOT-87, T3.5).
 */

#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "Core/Rpg/Equipment.h"
#include "Core/Rpg/Inventory.h"
#include "HMI/Presentation/InventoryScreen.h"

namespace {

/// Des catalogues minimaux, construits en mémoire : une épée, un arc, une armure, un bouclier,
/// une torche (matériel) et des outils de voleur.
struct Catalogs {
    core::ItemCatalog items;
    core::EquipmentCatalog equipment;

    Catalogs() {
        items.items.push_back(
            core::Item{.id = "torche", .name = "Torche", .category = "gear", .weightGrams = 500});
        items.items.push_back(core::Item{.id = "outils-de-voleur",
                                         .name = "Outils de voleur",
                                         .category = "tool",
                                         .weightGrams = 500});
        equipment.weapons.push_back(core::Weapon{.id = "epee-longue",
                                                 .name = "Épée longue",
                                                 .category = "martial",
                                                 .weightGrams = 1500});
        equipment.weapons.push_back(
            core::Weapon{.id = "dague", .name = "Dague", .category = "simple", .weightGrams = 500});
        equipment.weapons.push_back(core::Weapon{
            .id = "arc-long", .name = "Arc long", .category = "martial", .ranged = true});
        equipment.armors.push_back(core::Armor{.id = "cuir",
                                               .name = "Armure de cuir",
                                               .category = core::ArmorCategory::Light,
                                               .baseArmorClass = 11,
                                               .dexterityBonus = true});
        equipment.armors.push_back(core::Armor{.id = "bouclier",
                                               .name = "Bouclier",
                                               .category = core::ArmorCategory::Shield,
                                               .baseArmorClass = 2});
    }

    [[nodiscard]] core::ItemLookup lookup() const {
        return core::ItemLookup{.items = &items, .equipment = &equipment};
    }
};

[[nodiscard]] int quantityOf(const core::Inventory& inventory, const std::string& id) {
    for (const core::InventoryStack& stack : inventory.backpack) {
        if (stack.itemId == id) {
            return stack.quantity;
        }
    }
    return 0;
}

}  // namespace

/**
 * @brief Les onglets filtrent le sac par famille d'objet, tirée des catalogues.
 * \castest{<b>Les onglets de l'inventaire filtrent le sac par famille.</b><br/>
 * \tcat Unitaire · Inventaire<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Remplir un sac d'une épée, de torches et d'outils.<br/>
 * 2. Demander les cases de chaque filtre.<br/>
 * \tattendu Tous : trois cases ; Équipement : l'épée ; Matériel : les torches (quantité 5) ;
 * Outils : les outils.
 * }
 */
TEST(InventoryScreenTest, OngletsFiltrentParFamille) {
    const Catalogs catalogs;
    core::Inventory inventory;
    core::addToBackpack(inventory, "epee-longue", 1);
    core::addToBackpack(inventory, "torche", 5);
    core::addToBackpack(inventory, "outils-de-voleur", 1);

    EXPECT_EQ(hmi::backpackCells(inventory, catalogs.lookup(), hmi::ItemFamily::All).size(), 3U);
    const auto equipment =
        hmi::backpackCells(inventory, catalogs.lookup(), hmi::ItemFamily::Equipment);
    ASSERT_EQ(equipment.size(), 1U);
    EXPECT_EQ(equipment[0].name, "Épée longue");
    const auto gear = hmi::backpackCells(inventory, catalogs.lookup(), hmi::ItemFamily::Gear);
    ASSERT_EQ(gear.size(), 1U);
    EXPECT_EQ(gear[0].quantity, 5);
    EXPECT_EQ(hmi::backpackCells(inventory, catalogs.lookup(), hmi::ItemFamily::Tools).size(), 1U);
}

/**
 * @brief Équiper depuis le sac échange avec ce que l'emplacement portait, sans rien perdre.
 * \castest{<b>Équiper depuis l'inventaire rend l'objet remplacé au sac.</b><br/>
 * \tcat Unitaire · Inventaire<br/>
 * \tcrit Critique<br/>
 * \tetapes 1. Porter une dague en main directrice, avoir une épée longue dans le sac.<br/>
 * 2. Équiper l'épée depuis le sac.<br/>
 * \tattendu L'épée est en main directrice, la dague est dans le sac, l'épée n'y est plus.
 * }
 */
TEST(InventoryScreenTest, EquiperEchangeAvecLEmplacement) {
    const Catalogs catalogs;
    core::Inventory inventory;
    core::equip(inventory, core::EquipmentSlot::MainHand, "dague");
    core::addToBackpack(inventory, "epee-longue", 1);

    ASSERT_TRUE(hmi::equipFromBackpack(inventory, "epee-longue", catalogs.lookup()));
    EXPECT_EQ(inventory.at(core::EquipmentSlot::MainHand), "epee-longue");
    EXPECT_EQ(quantityOf(inventory, "dague"), 1);
    EXPECT_EQ(quantityOf(inventory, "epee-longue"), 0);
}

/**
 * @brief Chaque objet d'équipement va à son emplacement naturel ; le matériel ne s'équipe pas.
 * \castest{<b>Un objet s'équipe à son emplacement naturel, ou pas du tout.</b><br/>
 * \tcat Unitaire · Inventaire<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Demander l'emplacement naturel d'un arc, d'une épée, d'une armure, d'un bouclier,
 * d'une torche.<br/>
 * 2. Tenter d'équiper la torche.<br/>
 * \tattendu Distance, main directrice, torse, main secondaire ; aucun pour la torche, qui reste
 * dans le sac.
 * }
 */
TEST(InventoryScreenTest, EmplacementNaturel) {
    const Catalogs catalogs;
    EXPECT_EQ(hmi::naturalSlot("arc-long", catalogs.lookup()), core::EquipmentSlot::Ranged);
    EXPECT_EQ(hmi::naturalSlot("epee-longue", catalogs.lookup()), core::EquipmentSlot::MainHand);
    EXPECT_EQ(hmi::naturalSlot("cuir", catalogs.lookup()), core::EquipmentSlot::Torso);
    EXPECT_EQ(hmi::naturalSlot("bouclier", catalogs.lookup()), core::EquipmentSlot::OffHand);
    EXPECT_FALSE(hmi::naturalSlot("torche", catalogs.lookup()).has_value());

    core::Inventory inventory;
    core::addToBackpack(inventory, "torche", 1);
    EXPECT_FALSE(hmi::equipFromBackpack(inventory, "torche", catalogs.lookup()));
    EXPECT_EQ(quantityOf(inventory, "torche"), 1);
}

/**
 * @brief Retirer range au sac ; jeter retire un seul exemplaire.
 * \castest{<b>Retirer un objet le range au sac, jeter en retire un exemplaire.</b><br/>
 * \tcat Unitaire · Inventaire<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Retirer le bouclier porté.<br/>
 * 2. Jeter une torche d'une pile de trois.<br/>
 * 3. Retirer un emplacement vide.<br/>
 * \tattendu Le bouclier est dans le sac, il reste deux torches, retirer le vide échoue.
 * }
 */
TEST(InventoryScreenTest, RetirerEtJeter) {
    core::Inventory inventory;
    core::equip(inventory, core::EquipmentSlot::OffHand, "bouclier");
    core::addToBackpack(inventory, "torche", 3);

    ASSERT_TRUE(hmi::unequipToBackpack(inventory, core::EquipmentSlot::OffHand));
    EXPECT_FALSE(inventory.isEquipped(core::EquipmentSlot::OffHand));
    EXPECT_EQ(quantityOf(inventory, "bouclier"), 1);

    ASSERT_TRUE(hmi::dropFromBackpack(inventory, "torche"));
    EXPECT_EQ(quantityOf(inventory, "torche"), 2);

    EXPECT_FALSE(hmi::unequipToBackpack(inventory, core::EquipmentSlot::Head));
}

/**
 * @brief Trier range le sac par nom du catalogue.
 * \castest{<b>Trier l'inventaire range le sac par nom.</b><br/>
 * \tcat Unitaire · Inventaire<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Remplir un sac de torches, d'une épée et d'une dague, dans cet ordre.<br/>
 * 2. Trier.<br/>
 * \tattendu Dague, Torche, Épée longue — l'ordre des noms tels que le catalogue les écrit.
 * }
 */
TEST(InventoryScreenTest, TrierParNom) {
    const Catalogs catalogs;
    core::Inventory inventory;
    core::addToBackpack(inventory, "torche", 2);
    core::addToBackpack(inventory, "epee-longue", 1);
    core::addToBackpack(inventory, "dague", 1);

    hmi::sortBackpack(inventory, catalogs.lookup());
    ASSERT_EQ(inventory.backpack.size(), 3U);
    EXPECT_EQ(inventory.backpack[0].itemId, "dague");
    EXPECT_EQ(inventory.backpack[1].itemId, "torche");
    EXPECT_EQ(inventory.backpack[2].itemId, "epee-longue");
}

/**
 * @brief La fiche d'un objet décrit sa nature, sa protection et son poids.
 * \castest{<b>La fiche d'un objet de l'inventaire décrit sa nature et son poids.</b><br/>
 * \tcat Unitaire · Inventaire<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Demander la fiche de l'armure de cuir, du bouclier et de la torche.<br/>
 * \tattendu Armure légère « CA 11 + Dex » équipable ; bouclier « +2 » ; torche « Matériel »,
 * « 0,5 kg », non équipable.
 * }
 */
TEST(InventoryScreenTest, FicheDObjet) {
    const Catalogs catalogs;
    const hmi::ItemSheet armor = hmi::itemSheet("cuir", catalogs.lookup());
    EXPECT_EQ(armor.kind, "Armure légère");
    EXPECT_EQ(armor.armor, "CA 11 + Dex");
    EXPECT_TRUE(armor.equippable);
    EXPECT_EQ(hmi::itemSheet("bouclier", catalogs.lookup()).armor, "+2");
    const hmi::ItemSheet torch = hmi::itemSheet("torche", catalogs.lookup());
    EXPECT_EQ(torch.kind, "Matériel");
    EXPECT_EQ(torch.weight, "0,5 kg");
    EXPECT_FALSE(torch.equippable);
}
