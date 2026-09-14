// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "Core/Rpg/Inventory.h"

/**
 * @file HMI/Presentation/InventoryScreen.h
 * @brief Ce que l'écran d'inventaire de la charte v2 montre et fait (`LOT-87`, T3.5).
 */

namespace hmi {

/**
 * @brief Les familles d'objets que les onglets de l'inventaire filtrent.
 *
 * Tirées des **données** et non de la maquette : les catalogues distinguent les armes et armures
 * (`EquipmentCatalog`) et, parmi les objets, le matériel (`gear`), les outils (`tool`) et les
 * montures (`mount`). La maquette montre « Consommables » et « Divers » ; aucun objet ne porte ces
 * catégories, et un onglet qui ne trierait rien serait inopérant (`EX-IHM-072`).
 */
enum class ItemFamily {
    All,        ///< Aucun filtre.
    Equipment,  ///< Armes, armures et boucliers.
    Gear,       ///< Matériel d'aventurier (`gear`).
    Tools,      ///< Outils (`tool`).
    Other,      ///< Tout le reste (`mount`, catégorie inconnue, identifiant inconnu).
};

/// Une case de la grille : une pile du sac.
struct InventoryCell {
    std::string itemId;
    std::string name;  ///< Nom du catalogue, ou l'identifiant s'il est inconnu.
    int quantity = 1;
    ItemFamily family = ItemFamily::Other;
};

/// La fiche d'un objet sélectionné.
struct ItemSheet {
    std::string name;
    std::string kind;    ///< « Arme de guerre », « Armure légère », « Matériel »…
    std::string damage;  ///< « 1d8 tranchant », vide si l'objet n'en inflige pas.
    std::string armor;   ///< « CA 11 + Dex », « +2 », vide si ce n'est pas une armure.
    std::string weight;  ///< « 1,5 kg ».
    std::string text;    ///< Propriétés ou description, telles que le catalogue les écrit.
    bool equippable = false;
};

/// @return La famille de @p itemId, cherchée dans les catalogues.
[[nodiscard]] ItemFamily familyOf(std::string_view itemId, const core::ItemLookup& lookup);

/// @return Les piles du sac appartenant à @p filter (toutes pour `All`), dans l'ordre du sac.
[[nodiscard]] std::vector<InventoryCell> backpackCells(const core::Inventory& inventory,
                                                       const core::ItemLookup& lookup,
                                                       ItemFamily filter);

/// @return La fiche de @p itemId. Un identifiant inconnu rend une fiche à son nom, sans détail.
[[nodiscard]] ItemSheet itemSheet(std::string_view itemId, const core::ItemLookup& lookup);

/**
 * @brief L'emplacement où un objet s'équipe **naturellement**.
 *
 * Arme à distance : `Ranged` ; arme de corps à corps : `MainHand` ; bouclier : `OffHand` ; armure :
 * `Torso`. Un objet du catalogue général n'a pas d'emplacement déductible — une amulette et une
 * torche y ont la même catégorie (`gear`) — et rend `std::nullopt` : l'écran ne propose alors pas
 * « Équiper », plutôt que de ranger une torche au cou.
 */
[[nodiscard]] std::optional<core::EquipmentSlot> naturalSlot(std::string_view itemId,
                                                             const core::ItemLookup& lookup);

/**
 * @brief Équipe un exemplaire du sac à son emplacement naturel.
 *
 * L'exemplaire quitte le sac ; ce que l'emplacement portait **y revient** (`core::equip` le rend) :
 * rien ne se perd dans un échange. Les statistiques dérivées ne sont pas touchées ici — elles se
 * recalculent depuis l'inventaire (`core::derivedStatsFor`), jamais par ajout.
 *
 * @return Vrai si l'objet a été équipé ; faux s'il n'est pas dans le sac ou ne s'équipe pas.
 */
bool equipFromBackpack(core::Inventory& inventory, std::string_view itemId,
                       const core::ItemLookup& lookup);

/// @brief Retire ce que @p slot porte et le range dans le sac. @return Faux si l'emplacement est
/// vide.
bool unequipToBackpack(core::Inventory& inventory, core::EquipmentSlot slot);

/// @brief Jette un exemplaire de @p itemId du sac. @return Faux s'il n'y est pas.
bool dropFromBackpack(core::Inventory& inventory, std::string_view itemId);

/// @brief Trie le sac par nom (catalogue), à nom égal par identifiant : un ordre stable et lisible.
void sortBackpack(core::Inventory& inventory, const core::ItemLookup& lookup);

}  // namespace hmi
