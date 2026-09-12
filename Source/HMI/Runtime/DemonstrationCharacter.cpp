// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Runtime/DemonstrationCharacter.h"

#include <filesystem>

#include "Core/Rpg/CharacterOptions.h"
#include "Core/Rpg/CharacterSheet.h"
#include "Core/Rpg/Equipment.h"
#include "Core/Rpg/Inventory.h"
#include "Core/Rpg/Skill.h"
#include "HMI/HmiLog.h"
#include "HMI/Platform/ExecutableDirectory.h"
#include "HMI/Presentation/CharacterSheetValues.h"
#include "HMI/Presentation/InventoryValues.h"

namespace hmi {
namespace {

/// Fichier du personnage de démonstration, dans `Rpg/characters/`.
constexpr const char* DEMONSTRATION_CHARACTER_FILE = "demonstration-brenna.json";

/// Le même signe que les autres écrans posent sur un champ sans source.
constexpr const char* EMPTY_MARK = "—";

}  // namespace

DemonstrationCharacter loadDemonstrationValues() {
    const std::filesystem::path rpg = executableDirectory() / "Rpg";

    const core::CharacterOptions options =
        core::loadCharacterOptions(rpg / "species", rpg / "backgrounds", rpg / "classes");
    const core::SkillCatalog skills = core::loadSkills(rpg / "skills");
    const core::ExperienceTable experience =
        core::loadExperienceTable(rpg / "rules" / "experience.json");
    const core::CharacterCreationRules rules =
        core::loadCharacterCreationRules(rpg / "rules" / "character-creation.json");

    const core::LoadedCharacterSheet loaded = core::loadCharacterSheet(
        rpg / "characters" / DEMONSTRATION_CHARACTER_FILE, options, rules, experience);
    for (const std::string& error : loaded.errors) {
        // Journalise et poursuit : une fiche partielle vaut mieux qu'un écran vide, et l'erreur
        // nomme son fichier (EX-CNT-010).
        HMI_LOG_WARNING("Personnage de demonstration : " + error);
    }

    const core::ItemCatalog items = core::loadItems(rpg / "items");
    const core::EquipmentCatalog equipment = core::loadEquipment(rpg / "weapons", rpg / "armors");
    const core::EncumbranceRules encumbrance =
        core::loadEncumbranceRules(rpg / "rules" / "encumbrance.json");
    for (const std::string& error : items.errors) {
        HMI_LOG_WARNING("Catalogue d'objets : " + error);
    }
    const core::ItemLookup lookup{.items = &items, .equipment = &equipment};
    for (const std::string& unknown : core::unknownIds(loaded.inventory, lookup)) {
        // Un identifiant que rien ne porte ne pèse rien et s'affiche tel quel : le dire vaut mieux
        // que de peser faux en silence (EX-CNT-010).
        HMI_LOG_WARNING("Inventaire de demonstration : objet inconnu '" + unknown + "'.");
    }

    // Les statistiques dérivées sont RECALCULÉES ici, jamais retenues : c'est ce qui les empêche
    // de dériver quand on équipe et retire dans le désordre (LOT-14).
    const core::DerivedStats derived =
        core::derivedStatsFor(loaded.sheet, loaded.inventory, lookup, rules, encumbrance);

    DemonstrationCharacter result;
    // L'inventaire D'ABORD, parce que la fiche en dépend : sa classe d'armure et sa vitesse
    // viennent de ce qui est porté, pas de la construction du personnage.
    result.inventory = inventoryValues({.inventory = &loaded.inventory,
                                        .lookup = lookup,
                                        .derived = derived,
                                        .emptyMark = EMPTY_MARK});
    result.sheet = characterSheetValues({.sheet = &loaded.sheet,
                                         .options = &options,
                                         .experience = &experience,
                                         .skills = &skills,
                                         .derived = &derived,
                                         .emptyMark = EMPTY_MARK});

    result.skills.reserve(skills.skills.size());
    for (const core::SkillDefinition& skill : skills.skills) {
        result.skills.emplace_back(skill.id, skill.name);
    }
    return result;
}

}  // namespace hmi
