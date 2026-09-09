// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Presentation/CharacterSheetModel.h"

#include <QVector>
#include <array>
#include <filesystem>
#include <string>

#include "Core/Rpg/Ability.h"
#include "Core/Rpg/CharacterOptions.h"
#include "Core/Rpg/CharacterSheet.h"
#include "Core/Rpg/Equipment.h"
#include "Core/Rpg/Inventory.h"
#include "Core/Rpg/Skill.h"
#include "HMI/HmiLog.h"
#include "HMI/Localization/Localization.h"
#include "HMI/Platform/ExecutableDirectory.h"
#include "HMI/Presentation/CharacterSheetValues.h"

namespace hmi {
namespace {

/// Fichier du personnage de démonstration, dans `Rpg/characters/`.
constexpr const char* DEMONSTRATION_CHARACTER_FILE = "demonstration-brenna.json";

/// Tiret cadratin : à l'écran, « inconnu » et « pas encore alimenté » se ressemblent, et rien ne
/// gagne à les distinguer par deux signes différents.
constexpr const char* EMPTY_MARK = "—";

/// Les six caractéristiques, dans l'ordre où une feuille de personnage les présente.
constexpr std::array<core::Ability, 6> ABILITIES{
    core::Ability::Strength,     core::Ability::Dexterity, core::Ability::Constitution,
    core::Ability::Intelligence, core::Ability::Wisdom,    core::Ability::Charisma,
};

[[nodiscard]] QString toQt(const std::string& text) {
    return QString::fromStdString(text);
}

/// @return La valeur de @p key dans @p values, ou une chaîne vide.
[[nodiscard]] QString lookup(const std::map<std::string, std::string>& values,
                             const std::string& key) {
    const auto found = values.find(key);
    return found == values.end() ? QString() : toQt(found->second);
}

}  // namespace

CharacterSheetModel::CharacterSheetModel(QObject* parent)
    : QObject(parent), _abilities(this), _savingThrows(this), _skills(this) {}

QString CharacterSheetModel::value(const char* key) const {
    const auto found = _values.find(key);
    if (found == _values.end() || found->second.empty()) {
        return QString::fromUtf8(EMPTY_MARK);
    }
    return toQt(found->second);
}

void CharacterSheetModel::loadDemonstrationCharacter() {
    const std::filesystem::path rpg = executableDirectory() / "Rpg";

    // Les noms français des six caractéristiques sont une DONNÉE, pas une constante de code : ils
    // viennent du lexique des règles (`rpg.glossary.csv`), qui garantit une seule traduction par
    // terme dans tout le jeu. Les écrire ici en dur aurait créé un deuxième vocabulaire, et c'est
    // exactement ce que le lexique existe pour empêcher.
    Localization labels(executableDirectory() / "Localization");
    if (!labels.loadDefaultLanguage("fr")) {
        // Non bloquant, mais loin d'être anodin : sans catalogue, `text()` rend la CLÉ, et l'écran
        // afficherait « rpg.ability.strength » là où on attend « Force ». Le dire vaut mieux que
        // de laisser chercher d'où vient un libellé technique (EX-NFR-040).
        HMI_LOG_WARNING(
            "Catalogue de traduction introuvable : les libelles de la fiche resteront techniques.");
    }

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
        HMI_LOG_WARNING("Fiche de demonstration : " + error);
    }

    // Ce que le personnage PORTE (LOT-14) : la classe d'armure et la vitesse en dépendent, et
    // elles sont RECALCULÉES ici plutôt que retenues — c'est ce qui les empêche de dériver quand
    // on équipe et retire dans le désordre.
    const core::ItemCatalog items = core::loadItems(rpg / "items");
    const core::EquipmentCatalog equipment = core::loadEquipment(rpg / "weapons", rpg / "armors");
    const core::EncumbranceRules encumbrance =
        core::loadEncumbranceRules(rpg / "rules" / "encumbrance.json");
    const core::ItemLookup lookupTables{.items = &items, .equipment = &equipment};
    const core::DerivedStats derived =
        core::derivedStatsFor(loaded.sheet, loaded.inventory, lookupTables, rules, encumbrance);

    _values = characterSheetValues({.sheet = &loaded.sheet,
                                    .options = &options,
                                    .experience = &experience,
                                    .skills = &skills,
                                    .derived = &derived,
                                    .emptyMark = EMPTY_MARK});

    // Les trois listes répétitives. Leurs libellés viennent des catalogues de règles, jamais d'une
    // table écrite ici : un nom de compétence a déjà une source de vérité, en donnée.
    QVector<SheetRow> abilityRows;
    QVector<SheetRow> saveRows;
    abilityRows.reserve(static_cast<qsizetype>(ABILITIES.size()));
    saveRows.reserve(static_cast<qsizetype>(ABILITIES.size()));
    for (const core::Ability ability : ABILITIES) {
        const std::string id(core::abilityName(ability));
        const QString label = toQt(labels.text("rpg.ability." + id));
        abilityRows.append(SheetRow{
            .id = toQt(id), .label = label, .value = lookup(_values, "sheet.ability." + id)});
        saveRows.append(
            SheetRow{.id = toQt(id), .label = label, .value = lookup(_values, "sheet.save." + id)});
    }
    _abilities.setRows(std::move(abilityRows));
    _savingThrows.setRows(std::move(saveRows));

    QVector<SheetRow> skillRows;
    skillRows.reserve(static_cast<qsizetype>(skills.skills.size()));
    for (const core::SkillDefinition& skill : skills.skills) {
        skillRows.append(SheetRow{.id = toQt(skill.id),
                                  .label = toQt(skill.name),
                                  .value = lookup(_values, "sheet.skill." + skill.id)});
    }
    _skills.setRows(std::move(skillRows));

    emit changed();
}

}  // namespace hmi
