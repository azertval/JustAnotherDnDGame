// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Core/Rpg/CharacterOptions.h"

#include <algorithm>
#include <set>
#include <system_error>

#include "Core/Data/JsonDocument.h"
#include "Core/Rpg/RpgEnumNames.h"

namespace core {

namespace {

// Les entrees de catalogue ne portent pas de champ `version` : ce sont des donnees, pas des
// documents de format. `0` desactive la garde de version, comme pour le bestiaire (LOT-33).
constexpr int SANS_GARDE_DE_VERSION = 0;

[[nodiscard]] std::string lireTexte(const nlohmann::json& objet, const char* champ) {
    const auto trouve = objet.find(champ);
    return (trouve != objet.end() && trouve->is_string()) ? trouve->get<std::string>()
                                                          : std::string{};
}

[[nodiscard]] std::vector<std::string> lireTextes(const nlohmann::json& objet, const char* champ) {
    std::vector<std::string> valeurs;
    const auto trouve = objet.find(champ);
    if (trouve == objet.end() || !trouve->is_array()) {
        return valeurs;
    }
    for (const auto& element : *trouve) {
        if (element.is_string()) {
            valeurs.push_back(element.get<std::string>());
        }
    }
    return valeurs;
}

void lireTraits(const nlohmann::json& objet, const char* champ, std::vector<NamedTrait>& sortie) {
    const auto trouve = objet.find(champ);
    if (trouve == objet.end() || !trouve->is_array()) {
        return;
    }
    for (const auto& element : *trouve) {
        if (element.is_object()) {
            sortie.push_back({lireTexte(element, "name"), lireTexte(element, "text")});
        }
    }
}

// Une caracteristique inconnue est SIGNALEE, jamais ignoree : c'est le scenario d'EX-CNT-011, ou
// la donnee nomme ce que le moteur ne connait pas, la valeur tombe dans un cas par defaut, et le
// personnage sort avec un modificateur faux que rien n'annonce.
[[nodiscard]] std::vector<Ability> lireCaracteristiques(const nlohmann::json& objet,
                                                        const char* champ,
                                                        const std::string& fichier,
                                                        std::vector<std::string>& erreurs) {
    std::vector<Ability> valeurs;
    for (const std::string& nom : lireTextes(objet, champ)) {
        const std::optional<Ability> lue = parseAbility(nom);
        if (lue.has_value()) {
            valeurs.push_back(*lue);
        } else {
            erreurs.push_back(fichier + " : " + champ + " '" + nom +
                              "' inconnue du moteur. Une caracteristique ignoree fausse un "
                              "modificateur sans qu'aucun message ne le dise.");
        }
    }
    return valeurs;
}

void lireStatut(const nlohmann::json& objet, ProvisionalStatus& sortie) {
    const auto trouve = objet.find("status");
    if (trouve == objet.end() || !trouve->is_object()) {
        return;
    }
    const auto provisoire = trouve->find("provisoire");
    sortie.provisional =
        provisoire != trouve->end() && provisoire->is_boolean() && provisoire->get<bool>();
    sortie.reason = lireTexte(*trouve, "raison");
    sortie.removalCriterion = lireTexte(*trouve, "retraitSi");
}

// Balaie un dossier et applique `lecteur` a chaque document. Le dossier est BALAYE, jamais
// enumere dans le code : une liste de noms ecrite en C++ serait une seconde source de verite.
template <typename Lecteur>
void balayer(const std::filesystem::path& dossier, std::vector<std::string>& erreurs,
             Lecteur lecteur) {
    std::error_code code;
    if (!std::filesystem::is_directory(dossier, code)) {
        // Un dossier absent n'est PAS un catalogue vide : les deux se ressemblent a l'execution,
        // et les confondre fait chercher le defaut du mauvais cote pendant longtemps.
        erreurs.push_back(dossier.string() + " : dossier absent ou illisible.");
        return;
    }
    std::vector<std::filesystem::path> fichiers;
    for (const auto& entree : std::filesystem::directory_iterator(dossier, code)) {
        if (entree.is_regular_file(code) && entree.path().extension() == ".json") {
            fichiers.push_back(entree.path());
        }
    }
    std::ranges::sort(fichiers);
    for (const std::filesystem::path& chemin : fichiers) {
        const JsonDocument document = readJsonObjectFromFile(chemin, SANS_GARDE_DE_VERSION);
        if (!document.ok()) {
            // Le message porte deja le fichier et la ligne (EX-CNT-010).
            erreurs.push_back(document.message);
            continue;
        }
        lecteur(document.root, chemin.filename().string());
    }
}

}  // namespace

const ClassLevel* PlayableClass::atLevel(int level) const {
    const auto trouve = std::ranges::find(progression, level, &ClassLevel::level);
    return trouve == progression.end() ? nullptr : &*trouve;
}

const Species* CharacterOptions::findSpecies(std::string_view id) const {
    const auto trouve = std::ranges::find(species, id, &Species::id);
    return trouve == species.end() ? nullptr : &*trouve;
}

const Background* CharacterOptions::findBackground(std::string_view id) const {
    const auto trouve = std::ranges::find(backgrounds, id, &Background::id);
    return trouve == backgrounds.end() ? nullptr : &*trouve;
}

const PlayableClass* CharacterOptions::findClass(std::string_view id) const {
    const auto trouve = std::ranges::find(classes, id, &PlayableClass::id);
    return trouve == classes.end() ? nullptr : &*trouve;
}

std::vector<std::string> CharacterOptions::requiredMechanisms() const {
    std::set<std::string> uniques;
    for (const Species& espece : species) {
        uniques.insert(espece.requiredMechanisms.begin(), espece.requiredMechanisms.end());
    }
    return {uniques.begin(), uniques.end()};
}

std::vector<std::string> CharacterOptions::provisionalClassIds() const {
    std::vector<std::string> identifiants;
    for (const PlayableClass& classe : classes) {
        if (classe.status.provisional) {
            identifiants.push_back(classe.id);
        }
    }
    return identifiants;
}

int abilityScoreWith(const Species& species, Ability which, int baseScore, int maximumScore) {
    return std::min(baseScore + species.increase(which), maximumScore);
}

CharacterOptions loadCharacterOptions(const std::filesystem::path& speciesDir,
                                      const std::filesystem::path& backgroundsDir,
                                      const std::filesystem::path& classesDir) {
    CharacterOptions options;

    balayer(
        speciesDir, options.errors,
        [&options](const nlohmann::json& racine, const std::string& fichier) {
            Species espece;
            espece.id = lireTexte(racine, "id");
            espece.name = lireTexte(racine, "name");
            espece.source = lireTexte(racine, "source");
            espece.parentSpecies = lireTexte(racine, "parentSpecies");
            const std::optional<CreatureSize> taille = parseCreatureSize(lireTexte(racine, "size"));
            if (!taille.has_value()) {
                options.errors.push_back(fichier + " : taille inconnue du moteur.");
                return;
            }
            espece.size = *taille;
            const auto vitesse = racine.find("speed");
            if (vitesse == racine.end() || !vitesse->is_number()) {
                options.errors.push_back(fichier + " : champ 'speed' absent ou non numerique.");
                return;
            }
            espece.speed = vitesse->get<float>();
            if (const auto increases = racine.find("abilityScoreIncrease");
                increases != racine.end() && increases->is_object()) {
                for (const auto& [nom, valeur] : increases->items()) {
                    const std::optional<Ability> lue = parseAbility(nom);
                    if (!lue.has_value() || !valeur.is_number_integer()) {
                        options.errors.push_back(fichier + " : augmentation '" + nom +
                                                 "' inconnue du moteur.");
                        continue;
                    }
                    espece.abilityScoreIncrease[static_cast<std::size_t>(*lue)] = valeur.get<int>();
                }
            }
            espece.languages = lireTextes(racine, "languages");
            espece.requiredMechanisms = lireTextes(racine, "mecanismesRequis");
            lireTraits(racine, "traits", espece.traits);
            options.species.push_back(std::move(espece));
        });

    balayer(backgroundsDir, options.errors,
            [&options](const nlohmann::json& racine, const std::string&) {
                Background historique;
                historique.id = lireTexte(racine, "id");
                historique.name = lireTexte(racine, "name");
                historique.source = lireTexte(racine, "source");
                historique.skillProficiencies = lireTextes(racine, "skillProficiencies");
                historique.text = lireTexte(racine, "text");
                if (const auto langues = racine.find("languageCount");
                    langues != racine.end() && langues->is_number_integer()) {
                    historique.languageCount = langues->get<int>();
                }
                if (const auto capacite = racine.find("feature");
                    capacite != racine.end() && capacite->is_object()) {
                    historique.feature =
                        NamedTrait{lireTexte(*capacite, "name"), lireTexte(*capacite, "text")};
                }
                options.backgrounds.push_back(std::move(historique));
            });

    balayer(classesDir, options.errors,
            [&options](const nlohmann::json& racine, const std::string& fichier) {
                PlayableClass classe;
                classe.id = lireTexte(racine, "id");
                classe.name = lireTexte(racine, "name");
                classe.source = lireTexte(racine, "source");
                const auto de = racine.find("hitDie");
                if (de == racine.end() || !de->is_number_integer()) {
                    options.errors.push_back(fichier +
                                             " : champ 'hitDie' absent ou non entier. Le de de "
                                             "vie decide des points de vie a chaque niveau.");
                    return;
                }
                classe.hitDie = de->get<int>();
                classe.primaryAbility =
                    lireCaracteristiques(racine, "primaryAbility", fichier, options.errors);
                classe.savingThrowProficiencies = lireCaracteristiques(
                    racine, "savingThrowProficiencies", fichier, options.errors);
                lireStatut(racine, classe.status);
                if (const auto progression = racine.find("progression");
                    progression != racine.end() && progression->is_array()) {
                    for (const auto& element : *progression) {
                        if (!element.is_object()) {
                            continue;
                        }
                        ClassLevel niveau;
                        const auto valeur = element.find("level");
                        const auto bonus = element.find("proficiencyBonus");
                        if (valeur == element.end() || !valeur->is_number_integer() ||
                            bonus == element.end() || !bonus->is_number_integer()) {
                            options.errors.push_back(fichier +
                                                     " : ligne de progression sans niveau ni "
                                                     "bonus de maitrise.");
                            continue;
                        }
                        niveau.level = valeur->get<int>();
                        niveau.proficiencyBonus = bonus->get<int>();
                        niveau.features = lireTextes(element, "features");
                        classe.progression.push_back(std::move(niveau));
                    }
                }
                std::ranges::sort(classe.progression, {}, &ClassLevel::level);
                options.classes.push_back(std::move(classe));
            });

    std::ranges::sort(options.species, {}, &Species::id);
    std::ranges::sort(options.backgrounds, {}, &Background::id);
    std::ranges::sort(options.classes, {}, &PlayableClass::id);
    return options;
}

}  // namespace core
