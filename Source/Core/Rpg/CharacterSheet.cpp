// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Core/Rpg/CharacterSheet.h"

#include <algorithm>
#include <utility>

#include "Core/Data/JsonDocument.h"
#include "Core/Rpg/Scale.h"

namespace core {

namespace {

// La table d'experience ne porte pas de champ `version` : c'est une donnee de regle, pas un
// document de format.
constexpr int SANS_GARDE_DE_VERSION = 0;

// Le gain de points de vie d'un niveau ne descend jamais sous 1. Une Constitution desastreuse fait
// gagner peu de points de vie ; elle n'en fait pas perdre.
constexpr int GAIN_MINIMAL_PAR_NIVEAU = 1;

}  // namespace

CharacterCreationRules loadCharacterCreationRules(const std::filesystem::path& path) {
    CharacterCreationRules regles;
    const JsonDocument document = readJsonObjectFromFile(path, SANS_GARDE_DE_VERSION);
    if (!document.ok()) {
        regles.errors.push_back(document.message);
        return regles;
    }
    for (const auto& [champ, sortie] :
         {std::pair{"unarmoredArmorClass", &regles.unarmoredArmorClass},
          std::pair{"maximumAbilityScore", &regles.maximumAbilityScore}}) {
        const auto trouve = document.root.find(champ);
        if (trouve == document.root.end() || !trouve->is_number_integer()) {
            // Une regle absente ne se devine pas : lui donner une valeur par defaut la ferait
            // passer pour une regle du jeu, alors que ce serait une valeur inventee par le
            // chargeur.
            regles.errors.push_back(path.string() + " : champ '" + champ +
                                    "' absent ou non entier.");
            continue;
        }
        *sortie = trouve->get<int>();
    }
    return regles;
}

// -- Table d'experience -----------------------------------------------------------------------

int ExperienceTable::levelFor(int experiencePoints) const {
    int atteint = 0;
    for (const ExperienceLevel& ligne : levels) {
        if (experiencePoints >= ligne.experience) {
            atteint = std::max(atteint, ligne.level);
        }
    }
    return atteint;
}

int ExperienceTable::proficiencyBonusAt(int level) const {
    const auto trouve = std::ranges::find(levels, level, &ExperienceLevel::level);
    return trouve == levels.end() ? 0 : trouve->proficiencyBonus;
}

int ExperienceTable::thresholdAt(int level) const {
    const auto trouve = std::ranges::find(levels, level, &ExperienceLevel::level);
    return trouve == levels.end() ? 0 : trouve->experience;
}

int ExperienceTable::maximumLevel() const {
    int maximum = 0;
    for (const ExperienceLevel& ligne : levels) {
        maximum = std::max(maximum, ligne.level);
    }
    return maximum;
}

ExperienceTable loadExperienceTable(const std::filesystem::path& path) {
    ExperienceTable table;
    const JsonDocument document = readJsonObjectFromFile(path, SANS_GARDE_DE_VERSION);
    if (!document.ok()) {
        // Le message porte deja le fichier et la ligne (EX-CNT-010).
        table.errors.push_back(document.message);
        return table;
    }
    const auto niveaux = document.root.find("levels");
    if (niveaux == document.root.end() || !niveaux->is_array()) {
        table.errors.push_back(path.string() + " : champ 'levels' absent ou non tableau.");
        return table;
    }
    for (const auto& element : *niveaux) {
        if (!element.is_object()) {
            continue;
        }
        const auto niveau = element.find("level");
        const auto px = element.find("experience");
        const auto bonus = element.find("proficiencyBonus");
        if (niveau == element.end() || !niveau->is_number_integer() || px == element.end() ||
            !px->is_number_integer() || bonus == element.end() || !bonus->is_number_integer()) {
            table.errors.push_back(path.string() + " : ligne de table incomplete.");
            continue;
        }
        table.levels.push_back({niveau->get<int>(), px->get<int>(), bonus->get<int>()});
    }
    std::ranges::sort(table.levels, {}, &ExperienceLevel::level);
    return table;
}

LoadedCharacterSheet loadCharacterSheet(const std::filesystem::path& path,
                                        const CharacterOptions& options,
                                        const CharacterCreationRules& rules,
                                        const ExperienceTable& table) {
    LoadedCharacterSheet resultat;
    const JsonDocument document = readJsonObjectFromFile(path, SANS_GARDE_DE_VERSION);
    if (!document.ok()) {
        resultat.errors.push_back(document.message);
        return resultat;
    }

    const auto texte = [&document](const char* champ) {
        const auto trouve = document.root.find(champ);
        return (trouve != document.root.end() && trouve->is_string()) ? trouve->get<std::string>()
                                                                      : std::string{};
    };

    // Les six valeurs de BASE, avant augmentation d'espece. Les six sont exigees : une fiche a
    // cinq caracteristiques n'existe pas, et laisser la sixieme a zero donnerait un modificateur
    // de -5 que rien ne signalerait.
    std::array<int, 6> base{};
    const auto caracteristiques = document.root.find("baseAbilities");
    if (caracteristiques == document.root.end() || !caracteristiques->is_object()) {
        resultat.errors.push_back(path.string() + " : champ 'baseAbilities' absent ou non objet.");
    } else {
        for (const Ability caracteristique : allAbilities()) {
            const std::string nom{abilityName(caracteristique)};
            const auto valeur = caracteristiques->find(nom);
            if (valeur == caracteristiques->end() || !valeur->is_number_integer()) {
                resultat.errors.push_back(path.string() + " : caracteristique '" + nom +
                                          "' absente.");
                continue;
            }
            base[static_cast<std::size_t>(caracteristique)] = valeur->get<int>();
        }
    }

    // Les trois choix sont resolus DANS LE CATALOGUE, et un identifiant inconnu est signale : une
    // fiche qui reference une espece absente s'afficherait sans vitesse ni augmentation, ce qui
    // ressemble a un personnage faible et non a une donnee fausse.
    const std::string especeId = texte("speciesId");
    const std::string classeId = texte("classId");
    const std::string historiqueId = texte("backgroundId");
    const Species* const espece = options.findSpecies(especeId);
    const PlayableClass* const classe = options.findClass(classeId);
    const Background* const historique = options.findBackground(historiqueId);
    if (espece == nullptr) {
        resultat.errors.push_back(path.string() + " : espece inconnue '" + especeId + "'.");
    }
    if (classe == nullptr) {
        resultat.errors.push_back(path.string() + " : classe inconnue '" + classeId + "'.");
    }
    if (historique == nullptr) {
        resultat.errors.push_back(path.string() + " : historique inconnu '" + historiqueId + "'.");
    }

    // La fiche est CONSTRUITE, jamais recopiee : points de vie, classe d'armure, valeurs finales
    // et seuil d'experience sont derives par la regle (LOT-13). Les ecrire dans le fichier en
    // ferait une seconde source, qui differerait de la premiere au premier ajustement de regle.
    resultat.sheet =
        buildCharacterSheet(texte("name"), base, espece, classe, historique, rules, table);

    const auto niveau = document.root.find("level");
    if (niveau != document.root.end() && niveau->is_number_integer() && classe != nullptr) {
        // Monter par l'EXPERIENCE, et non en posant le niveau : c'est le meme chemin que celui
        // qu'une partie empruntera, donc les memes points de vie et le meme bonus de maitrise.
        const int cible = niveau->get<int>();
        const int seuil = table.thresholdAt(cible);
        if (seuil > resultat.sheet.experiencePoints) {
            gainExperience(resultat.sheet, table, classe->hitDie,
                           seuil - resultat.sheet.experiencePoints);
        }
    }

    const auto competences = document.root.find("skillProficiencies");
    if (competences != document.root.end() && competences->is_array()) {
        for (const auto& element : *competences) {
            if (element.is_string()) {
                resultat.sheet.skillProficiencies.insert(element.get<std::string>());
            }
        }
    }

    return resultat;
}

// -- Fiche ---------------------------------------------------------------------------------------

float CharacterSheet::speedInTiles() const {
    return tilesFromMeters(speedMeters);
}

int proficiencyBonus(const CharacterSheet& sheet, const ExperienceTable& table) {
    return table.proficiencyBonusAt(sheet.level);
}

int savingThrowModifier(const CharacterSheet& sheet, const ExperienceTable& table, Ability which) {
    const int maitrise =
        sheet.savingThrowProficiencies.contains(which) ? proficiencyBonus(sheet, table) : 0;
    return sheet.modifier(which) + maitrise;
}

SkillCheckModifier skillModifier(const CharacterSheet& sheet, const ExperienceTable& table,
                                 const SkillCatalog& catalog, std::string_view skillId) {
    const SkillDefinition* competence = catalog.find(skillId);
    if (competence == nullptr) {
        // Une competence inconnue ne se devine pas : renvoyer un modificateur nu SANS le dire
        // laisserait croire a une maitrise absente plutot qu'a une donnee manquante.
        return {};
    }
    SkillCheckModifier resultat;
    resultat.found = true;
    resultat.proficient = sheet.skillProficiencies.contains(std::string{skillId});
    resultat.value = sheet.modifier(competence->ability) +
                     (resultat.proficient ? proficiencyBonus(sheet, table) : 0);
    return resultat;
}

int maximumHitPointsFor(int hitDie, int level, int constitutionModifier) {
    if (hitDie <= 0 || level <= 0) {
        return 0;
    }
    // Niveau 1 : le MAXIMUM du de. La regle du livre, et la raison pour laquelle un magicien de
    // niveau 1 n'a pas 3 points de vie.
    int total = hitDie + constitutionModifier;
    // Niveaux suivants : la valeur fixe de la classe, << la valeur moyenne (arrondie au superieur)
    // du de >> (Basic Rules p. 11) -- soit (de / 2) + 1 pour tout de pair, et tous le sont.
    const int moyenne = (hitDie / 2) + 1;
    for (int niveau = 2; niveau <= level; ++niveau) {
        total += std::max(moyenne + constitutionModifier, GAIN_MINIMAL_PAR_NIVEAU);
    }
    return std::max(total, GAIN_MINIMAL_PAR_NIVEAU);
}

LevelUpResult gainExperience(CharacterSheet& sheet, const ExperienceTable& table, int hitDie,
                             int amount) {
    LevelUpResult resultat;
    resultat.previousLevel = sheet.level;
    resultat.newLevel = sheet.level;
    resultat.previousProficiencyBonus = table.proficiencyBonusAt(sheet.level);
    resultat.newProficiencyBonus = resultat.previousProficiencyBonus;
    if (amount <= 0) {
        // Perdre de l'experience n'est pas une regle de ce jeu. L'accepter en silence ferait
        // DESCENDRE un personnage de niveau, et le defaut passerait pour de l'equilibrage.
        return resultat;
    }

    sheet.experiencePoints += amount;
    const int atteint = std::min(table.levelFor(sheet.experiencePoints), table.maximumLevel());
    if (atteint <= sheet.level) {
        return resultat;
    }

    const int avant = sheet.maximumHitPoints;
    sheet.level = atteint;
    sheet.maximumHitPoints =
        maximumHitPointsFor(hitDie, sheet.level, sheet.modifier(Ability::Constitution));
    // Les points de vie COURANTS montent du meme gain, pas jusqu'au maximum : monter de niveau
    // n'est pas un soin, et rendre toute sa vie a un personnage blesse ferait de la montee de
    // niveau une potion gratuite.
    sheet.currentHitPoints += sheet.maximumHitPoints - avant;

    resultat.newLevel = sheet.level;
    resultat.hitPointsGained = sheet.maximumHitPoints - avant;
    resultat.newProficiencyBonus = table.proficiencyBonusAt(sheet.level);
    return resultat;
}

CharacterSheet buildCharacterSheet(std::string name, const std::array<int, 6>& baseAbilities,
                                   const Species* species, const PlayableClass* playableClass,
                                   const Background* background,
                                   const CharacterCreationRules& rules,
                                   const ExperienceTable& table) {
    CharacterSheet fiche;
    fiche.name = std::move(name);
    fiche.abilities = baseAbilities;

    if (species != nullptr) {
        fiche.speciesId = species->id;
        fiche.speedMeters = species->speed;
        for (const Ability caracteristique : allAbilities()) {
            fiche.abilities[static_cast<std::size_t>(caracteristique)] = abilityScoreWith(
                *species, caracteristique, baseAbilities[static_cast<std::size_t>(caracteristique)],
                rules.maximumAbilityScore);
        }
    }

    if (playableClass != nullptr) {
        fiche.classId = playableClass->id;
        for (const Ability caracteristique : playableClass->savingThrowProficiencies) {
            fiche.savingThrowProficiencies.insert(caracteristique);
        }
        fiche.maximumHitPoints = maximumHitPointsFor(playableClass->hitDie, fiche.level,
                                                     fiche.modifier(Ability::Constitution));
    }
    fiche.currentHitPoints = fiche.maximumHitPoints;

    if (background != nullptr) {
        fiche.backgroundId = background->id;
        for (const std::string& competence : background->skillProficiencies) {
            fiche.skillProficiencies.insert(competence);
        }
    }

    // La classe d'armure sans armure, LUE DANS LA DONNEE, plus le modificateur de Dexterite.
    // L'armure portee la remplacera au LOT-14, qui livre les armures.
    fiche.armorClass = rules.unarmoredArmorClass + fiche.modifier(Ability::Dexterity);
    fiche.experiencePoints = table.thresholdAt(fiche.level);
    return fiche;
}

}  // namespace core
