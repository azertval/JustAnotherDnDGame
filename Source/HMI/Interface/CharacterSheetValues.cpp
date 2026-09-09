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

        // Les langues viennent de l'ESPECE, qui n'en porte que les identifiants. Le nom lisible
        // vient du CATALOGUE des langues : « common » est une cle, et l'afficher tel quel la
        // ferait passer pour une donnee. Sans catalogue, rien n'est publie -- le tiret cadratin
        // dit alors la verite, la ou l'identifiant brut serait un mensonge lisible.
        if (espece != nullptr && !espece->languages.empty() && context.languages != nullptr) {
            std::string langues;
            std::size_t rang = 0;
            for (const std::string& langue : espece->languages) {
                const core::LanguageDefinition* const connue = context.languages->find(langue);
                if (connue == nullptr) {
                    // Une langue absente du catalogue est PASSEE, pas devinee : un test verifie
                    // que les especes livrees n'en accordent aucune qui manque, et le silence ici
                    // ne masque donc rien -- il refuse seulement d'ecrire une cle a l'ecran.
                    continue;
                }
                if (!langues.empty()) {
                    langues += ", ";
                }
                langues += connue->name;
                // Et chaque langue A PART, indexee : la planche en pose une par ligne. Lui faire
                // redecouper « commun, elfique » serait defaire la-bas ce qu'on assemble ici, et
                // la premiere langue dont le nom porte une virgule le dirait.
                valeurs["sheet.language." + std::to_string(rang)] = connue->name;
                ++rang;
            }
            if (rang > 0) {
                valeurs["sheet.languages"] = langues;
                valeurs["sheet.language.count"] = std::to_string(rang);
            }
        }

        // Le don d'historique : son nom et son texte, separement. Une seule chaine obligerait la
        // planche a la redecouper pour mettre le nom en titre.
        if (historique != nullptr && historique->feature.has_value()) {
            valeurs["sheet.background_feature"] = historique->feature->name;
            valeurs["sheet.background_feature_text"] = historique->feature->text;
        }

        // --- Ce que l'ESPECE accorde -----------------------------------------------------------
        //
        // Les traits d'espece portent leur nom ET leur texte, en francais, dans le catalogue :
        // c'est la seule des trois sources qui soit complete. Chacun sort indexe, nom et texte
        // separes, parce que la planche met le nom en titre et le texte dessous.
        if (espece != nullptr) {
            std::size_t rang = 0;
            for (const core::NamedTrait& trait : espece->traits) {
                const std::string prefixe = "sheet.species_trait." + std::to_string(rang);
                valeurs[prefixe + ".name"] = trait.name;
                valeurs[prefixe + ".text"] = trait.text;
                ++rang;
            }
            valeurs["sheet.species_trait.count"] = std::to_string(rang);
        }

        // --- Ce que l'HISTORIQUE accorde -------------------------------------------------------
        //
        // Les maitrises de competences accordees par l'historique. Le nom lisible vient du
        // catalogue des competences, jamais de l'identifiant : « animal-handling » affiche tel
        // quel se lirait comme une donnee. Sans catalogue, aucune n'est publiee.
        if (historique != nullptr && context.skills != nullptr) {
            std::size_t rang = 0;
            for (const std::string& identifiant : historique->skillProficiencies) {
                const core::SkillDefinition* const connue = context.skills->find(identifiant);
                if (connue == nullptr) {
                    continue;
                }
                valeurs["sheet.background_skill." + std::to_string(rang)] = connue->name;
                ++rang;
            }
            valeurs["sheet.background_skill.count"] = std::to_string(rang);
        }

        // --- Ce que la CLASSE accorde ----------------------------------------------------------
        //
        // La progression de classe nomme ses aptitudes par IDENTIFIANT, et n'en porte ni le
        // libelle ni le texte : aucun catalogue d'aptitudes n'existe encore. On publie donc
        // l'identifiant TEL QUEL, avec le niveau ou il s'obtient, et la planche l'affiche comme ce
        // qu'il est -- une cle en attente de son catalogue. Lui inventer un nom ici ferait croire
        // que la donnee le contient, ce qui est le contraire de ce que le tiret cadratin promet.
        //
        // Seuls les niveaux DEJA ATTEINTS sont publies : annoncer l'aptitude du niveau 9 a un
        // personnage de niveau 3 lui ferait lire comme acquis ce qui ne l'est pas.
        if (classe != nullptr) {
            std::size_t rang = 0;
            for (const core::ClassLevel& palier : classe->progression) {
                if (palier.level > fiche.level) {
                    continue;
                }
                for (const std::string& aptitude : palier.features) {
                    const std::string prefixe = "sheet.class_feature." + std::to_string(rang);
                    valeurs[prefixe + ".id"] = aptitude;
                    valeurs[prefixe + ".level"] = std::to_string(palier.level);
                    ++rang;
                }
            }
            valeurs["sheet.class_feature.count"] = std::to_string(rang);
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
            const int sauvegarde =
                core::savingThrowModifier(fiche, *context.experience, caracteristique);
            valeurs[std::string("sheet.save.") + suffixe] = signe(sauvegarde);
            // La maitrise SE VOIT sur la planche, sous forme de pastille pleine. Elle se lit ici a
            // l'ecart entre la sauvegarde et le modificateur nu : une sauvegarde maitrisee ajoute
            // le bonus de maitrise, une autre ne l'ajoute pas. Le jour ou un don ajoutera un autre
            // bonus a une sauvegarde, cet ecart cessera de signifier la maitrise -- et c'est alors
            // la REGLE qui devra la publier, pas cette soustraction.
            valeurs[std::string("sheet.save.") + suffixe + ".proficient"] =
                (sauvegarde != fiche.modifier(caracteristique)) ? "1" : "";
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
