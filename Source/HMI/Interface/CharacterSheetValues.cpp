// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Interface/CharacterSheetValues.h"

#include <array>
#include <string>

namespace hmi {
namespace {

using core::Ability;

/// Les six caractéristiques et le suffixe d'identifiant sous lequel l'ossature les attend.
constexpr std::array<std::pair<Ability, const char*>, 6> ABILITIES = {{
    {Ability::Strength, "strength"},
    {Ability::Dexterity, "dexterity"},
    {Ability::Constitution, "constitution"},
    {Ability::Intelligence, "intelligence"},
    {Ability::Wisdom, "wisdom"},
    {Ability::Charisma, "charisma"},
}};

/// @return Un modificateur avec son signe — `+3`, `-1`, `+0`. Le signe n'est pas une coquetterie :
///         un `3` nu se lit comme une valeur de caractéristique, et les deux se côtoient sur la
///         même ligne de la feuille.
[[nodiscard]] std::string signe(int valeur) {
    return (valeur >= 0 ? "+" : "") + std::to_string(valeur);
}

/// @return La vitesse en mètres, sans décimale inutile : « 9 m », jamais « 9.000000 m ».
[[nodiscard]] std::string metres(float valeur) {
    const int entier = static_cast<int>(valeur);
    std::string texte = std::to_string(entier);
    const float reste = valeur - static_cast<float>(entier);
    if (reste > 0.05F) {
        texte += ',' + std::to_string(static_cast<int>(reste * 10.0F + 0.5F));
    }
    return texte + " m";
}

}  // namespace

std::map<std::string, std::string> characterSheetValues(const CharacterSheetContext& context) {
    std::map<std::string, std::string> valeurs;
    if (context.sheet == nullptr) {
        // Aucune fiche : une table VIDE, jamais des zéros. L'écran garde ses tirets, et un tiret
        // dit « on ne sait pas » là où un zéro affirmerait « rien ».
        return valeurs;
    }
    const core::CharacterSheet& fiche = *context.sheet;

    const auto nomOuTiret = [&context](const std::string& nom) {
        return nom.empty() ? context.emptyMark : nom;
    };

    valeurs["sheet.name"] = nomOuTiret(fiche.name);

    // Les trois choix qui ont construit la fiche portent des IDENTIFIANTS ; leur nom lisible vit
    // dans le catalogue. Un identifiant inconnu du catalogue rend le tiret plutôt que l'identifiant
    // brut : `human-variant` affiché tel quel se lirait comme une donnée, alors que c'est une clé.
    if (context.options != nullptr) {
        const core::Species* const espece = context.options->findSpecies(fiche.speciesId);
        const core::PlayableClass* const classe = context.options->findClass(fiche.classId);
        const core::Background* const historique =
            context.options->findBackground(fiche.backgroundId);
        valeurs["sheet.species"] = espece != nullptr ? espece->name : context.emptyMark;
        valeurs["sheet.class"] = classe != nullptr ? classe->name : context.emptyMark;
        valeurs["sheet.background"] = historique != nullptr ? historique->name : context.emptyMark;
        if (classe != nullptr && classe->hitDie > 0) {
            valeurs["sheet.hit_dice"] =
                std::to_string(fiche.level) + "d" + std::to_string(classe->hitDie);
        }
    }

    valeurs["sheet.level"] = std::to_string(fiche.level);
    // « Classe et niveau » est UN champ sur la planche, et deux dans le modèle.
    if (const auto classe = valeurs.find("sheet.class"); classe != valeurs.end()) {
        valeurs["sheet.class_and_level"] = classe->second + " " + std::to_string(fiche.level);
    }
    valeurs["sheet.experience"] = std::to_string(fiche.experiencePoints);
    if (!context.characterId.empty()) {
        // La CLE, jamais un chemin (EX-CNT-040) : le jour ou l'on range les illustrations
        // autrement, c'est un dossier qui bouge, pas une fiche qui casse.
        valeurs["sheet.portrait"] = "character/" + context.characterId;
    }
    valeurs["sheet.armor_class"] = std::to_string(fiche.armorClass);
    valeurs["sheet.hit_points_max"] = std::to_string(fiche.maximumHitPoints);
    // Les points de vie courants se lisent CONTRE leur maximum : « 12 » seul ne dit pas si le
    // personnage va bien.
    valeurs["sheet.hit_points"] =
        std::to_string(fiche.currentHitPoints) + " / " + std::to_string(fiche.maximumHitPoints);
    // Et les courants A PART. Une jauge a besoin d'un rapport, pas d'une phrase : lui faire
    // redecouper « 27 / 32 » serait defaire ici ce qu'on vient de faire deux lignes plus haut.
    valeurs["sheet.hit_points_current"] = std::to_string(fiche.currentHitPoints);
    valeurs["sheet.speed"] = metres(fiche.speedMeters);

    // La classe d'armure et la vitesse dependent de ce qui est PORTE (LOT-14), et la fiche les a
    // calculees avant d'avoir un inventaire. Elles sont donc reprises des statistiques derivees
    // quand on en fournit -- recalculees depuis l'equipement, jamais accumulees.
    if (context.derived != nullptr) {
        valeurs["sheet.armor_class"] = std::to_string(context.derived->armorClass);
        valeurs["sheet.speed"] = metres(context.derived->speedMeters);
    }
    valeurs["sheet.initiative"] = signe(fiche.modifier(Ability::Dexterity));

    for (const auto& [caracteristique, suffixe] : ABILITIES) {
        const std::string racine = std::string("sheet.ability.") + suffixe;
        // La roue de la planche 1 porte les deux SÉPARÉMENT : le modificateur dans le grand
        // disque, la valeur dans le petit disque accolé. Une seule chaîne « 15 (+2) » obligerait
        // le peintre à la redécouper, c'est-à-dire à défaire ici ce qu'on aurait fait là-bas.
        valeurs[racine + ".score"] = std::to_string(fiche.ability(caracteristique));
        valeurs[racine + ".modifier"] = signe(fiche.modifier(caracteristique));
        // Et la ligne combinée, pour un affichage en liste (écran d'ATH, cible de combat).
        valeurs[racine] = valeurs[racine + ".score"] + " (" + valeurs[racine + ".modifier"] + ")";
    }

    if (context.experience != nullptr) {
        valeurs["sheet.proficiency_bonus"] =
            signe(core::proficiencyBonus(fiche, *context.experience));
        // Le seuil du niveau SUIVANT : une jauge de progression sans borne haute ne progresse vers
        // rien. Au dernier niveau le seuil ne bouge plus et la jauge est pleine, ce qui est exact
        // -- il n'y a alors plus rien a atteindre.
        valeurs["sheet.experience_next"] =
            std::to_string(context.experience->thresholdAt(fiche.level + 1));
        for (const auto& [caracteristique, suffixe] : ABILITIES) {
            valeurs[std::string("sheet.save.") + suffixe] =
                signe(core::savingThrowModifier(fiche, *context.experience, caracteristique));
        }
    }

    if (context.experience != nullptr && context.skills != nullptr) {
        for (const core::SkillDefinition& competence : context.skills->skills) {
            const core::SkillCheckModifier modificateur =
                core::skillModifier(fiche, *context.experience, *context.skills, competence.id);
            if (!modificateur.found) {
                continue;  // le catalogue ne la connaît pas : ne rien afficher plutôt qu'un zéro.
            }
            // La maîtrise se voit : c'est l'information que la pastille cochée porte sur la
            // feuille, et le seul moyen de la rendre dans une ligne de texte.
            const std::string racine = std::string("sheet.skill.") + competence.id;
            valeurs[racine] = signe(modificateur.value) + (modificateur.proficient ? " •" : "");
            // La maitrise, EN PLUS, sous forme de drapeau. La pastille de la planche est une
            // forme, pas un caractere : lui faire chercher un point dans une chaine reviendrait a
            // reparser ce que l'on vient d'ecrire, et le premier changement de signe la casserait.
            valeurs[racine + ".proficient"] = modificateur.proficient ? "1" : "";
        }
        // Perception passive : 10 + le modificateur de Perception, la règle du livre. Elle est
        // dérivée, jamais stockée -- deux valeurs qui doivent s'accorder finissent par diverger.
        const core::SkillCheckModifier perception =
            core::skillModifier(fiche, *context.experience, *context.skills, "perception");
        if (perception.found) {
            valeurs["sheet.passive_perception"] = std::to_string(10 + perception.value);
        }
    }

    return valeurs;
}

}  // namespace hmi
