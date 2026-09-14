// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Core/Rpg/Dialogue.h"

#include <algorithm>
#include <map>
#include <set>
#include <system_error>
#include <utility>
#include <variant>

#include "Core/Data/JsonDocument.h"
#include "Core/Gameplay/WorldFlags.h"
#include "Core/Rpg/Ability.h"
#include "Core/Rpg/CharacterSheet.h"
#include "Core/Rpg/Inventory.h"
#include "Core/Rpg/Skill.h"

namespace core {

namespace {

// Un dialogue est une donnee, pas un document de format : pas de champ `version`.
constexpr int SANS_GARDE_DE_VERSION = 0;

using Json = nlohmann::json;

/// Rassemble les erreurs d'un document, chacune prefixee de son origine et de son noeud.
class Rapport {
public:
    explicit Rapport(std::string_view origine) : _origine(origine) {}

    void document(const std::string& message) {
        _erreurs.push_back(_origine + " : " + message);
    }
    void noeud(std::string_view noeud, const std::string& message) {
        _erreurs.push_back(_origine + " : noeud '" + std::string(noeud) + "' : " + message);
    }
    [[nodiscard]] bool vide() const noexcept {
        return _erreurs.empty();
    }
    [[nodiscard]] std::vector<std::string> extraire() {
        return std::move(_erreurs);
    }

private:
    std::string _origine;
    std::vector<std::string> _erreurs;
};

[[nodiscard]] std::optional<std::string> texte(const Json& objet, const char* champ) {
    const auto trouve = objet.find(champ);
    if (trouve == objet.end() || !trouve->is_string() || trouve->get<std::string>().empty()) {
        return std::nullopt;
    }
    return trouve->get<std::string>();
}

[[nodiscard]] std::optional<DialogueAttitude> attitudeDepuis(std::string_view mot) {
    if (mot == "friendly") {
        return DialogueAttitude::Friendly;
    }
    if (mot == "indifferent") {
        return DialogueAttitude::Indifferent;
    }
    if (mot == "hostile") {
        return DialogueAttitude::Hostile;
    }
    return std::nullopt;
}

[[nodiscard]] std::optional<DialogueNodeKind> natureDepuis(std::string_view mot) {
    if (mot == "line") {
        return DialogueNodeKind::Line;
    }
    if (mot == "condition") {
        return DialogueNodeKind::Condition;
    }
    if (mot == "action") {
        return DialogueNodeKind::Action;
    }
    if (mot == "check") {
        return DialogueNodeKind::Check;
    }
    if (mot == "end") {
        return DialogueNodeKind::End;
    }
    return std::nullopt;
}

/// Lit `{ "flag": ..., "isSet": ... }`. `isSet` absent vaut vrai : « si le drapeau est leve ».
[[nodiscard]] std::optional<FlagCondition> conditionDepuis(const Json& objet) {
    if (!objet.is_object()) {
        return std::nullopt;
    }
    const auto drapeau = texte(objet, "flag");
    if (!drapeau) {
        return std::nullopt;
    }
    FlagCondition condition;
    condition.flag = *drapeau;
    if (const auto attendu = objet.find("isSet"); attendu != objet.end()) {
        if (!attendu->is_boolean()) {
            return std::nullopt;
        }
        condition.expected = attendu->get<bool>();
    }
    return condition;
}

/// Exige un champ texte, et le nomme s'il manque.
[[nodiscard]] std::string exiger(const Json& objet, const char* champ, std::string_view noeud,
                                 Rapport& rapport) {
    auto valeur = texte(objet, champ);
    if (!valeur) {
        rapport.noeud(noeud, std::string("champ '") + champ + "' absent ou vide.");
        return {};
    }
    return std::move(*valeur);
}

void lireReplique(const Json& brut, DialogueNode& noeud, Rapport& rapport) {
    const auto reponses = brut.find("choices");
    const bool aSuite = brut.contains("next");
    if (reponses != brut.end()) {
        if (aSuite) {
            rapport.noeud(noeud.id, "une replique a des reponses OU une suite, pas les deux.");
        }
        if (!reponses->is_array()) {
            rapport.noeud(noeud.id, "'choices' n'est pas un tableau.");
            return;
        }
        if (reponses->empty()) {
            // Le « choix vide » du critere d'acceptation : le joueur resterait devant une replique
            // sans rien a repondre.
            rapport.noeud(noeud.id, "choix vide : 'choices' ne propose aucune reponse.");
            return;
        }
        std::set<std::string> vus;
        bool uneSansCondition = false;
        for (const Json& reponse : *reponses) {
            if (!reponse.is_object()) {
                rapport.noeud(noeud.id, "une reponse n'est pas un objet.");
                continue;
            }
            DialogueChoice choix;
            const auto id = texte(reponse, "id");
            if (!id) {
                rapport.noeud(noeud.id, "choix vide : une reponse n'a pas d'identifiant.");
                continue;
            }
            choix.id = *id;
            if (!vus.insert(choix.id).second) {
                rapport.noeud(noeud.id, "reponse '" + choix.id + "' en double.");
            }
            if (choix.id == DIALOGUE_CONTINUE_CHOICE) {
                rapport.noeud(noeud.id,
                              "l'identifiant 'continue' est reserve a la reponse implicite.");
            }
            choix.next = exiger(reponse, "next", noeud.id + "' / reponse '" + choix.id, rapport);
            if (const auto condition = reponse.find("condition"); condition != reponse.end()) {
                choix.condition = conditionDepuis(*condition);
                if (!choix.condition) {
                    rapport.noeud(noeud.id,
                                  "reponse '" + choix.id +
                                      "' : condition sans 'flag' ou 'isSet' non booleen.");
                }
            } else {
                uneSansCondition = true;
            }
            noeud.choices.push_back(std::move(choix));
        }
        if (!noeud.choices.empty() && !uneSansCondition) {
            // Des drapeaux qui les masqueraient toutes laisseraient une replique sans issue, et
            // cela ne se decouvrirait qu'en jeu, dans l'etat de monde precis qui la produit.
            rapport.noeud(noeud.id,
                          "toutes les reponses sont conditionnelles : il en faut une toujours "
                          "proposee.");
        }
    } else if (aSuite) {
        noeud.next = exiger(brut, "next", noeud.id, rapport);
    } else {
        rapport.noeud(noeud.id, "replique sans reponses ni suite ('choices' ou 'next').");
    }
    if (const auto attitude = brut.find("attitude"); attitude != brut.end()) {
        const auto lue =
            attitude->is_string() ? attitudeDepuis(attitude->get<std::string>()) : std::nullopt;
        if (!lue) {
            rapport.noeud(noeud.id, "attitude inconnue (friendly, indifferent, hostile).");
        }
        noeud.attitude = lue;
    }
}

void lireCondition(const Json& brut, DialogueNode& noeud, Rapport& rapport) {
    const auto condition = conditionDepuis(brut);
    if (!condition) {
        rapport.noeud(noeud.id, "condition sans 'flag', ou 'isSet' non booleen.");
    } else {
        noeud.condition = *condition;
    }
    noeud.whenTrue = exiger(brut, "then", noeud.id, rapport);
    noeud.whenFalse = exiger(brut, "else", noeud.id, rapport);
}

void lireAction(const Json& brut, DialogueNode& noeud, Rapport& rapport) {
    const auto effets = brut.find("actions");
    if (effets == brut.end() || !effets->is_array() || effets->empty()) {
        rapport.noeud(noeud.id, "'actions' absent, vide ou non tableau.");
    } else {
        for (const Json& effet : *effets) {
            const auto type = effet.is_object() ? texte(effet, "type") : std::nullopt;
            DialogueAction action;
            const char* champ = nullptr;
            if (type == "setFlag") {
                action.kind = DialogueActionKind::SetFlag;
                champ = "flag";
            } else if (type == "clearFlag") {
                action.kind = DialogueActionKind::ClearFlag;
                champ = "flag";
            } else if (type == "giveItem") {
                action.kind = DialogueActionKind::GiveItem;
                champ = "item";
            } else if (type == "startQuest") {
                action.kind = DialogueActionKind::StartQuest;
                champ = "quest";
            } else {
                rapport.noeud(noeud.id,
                              "action de type inconnu (setFlag, clearFlag, giveItem, "
                              "startQuest).");
                continue;
            }
            action.target = exiger(effet, champ, noeud.id, rapport);
            if (action.kind == DialogueActionKind::GiveItem) {
                if (const auto quantite = effet.find("quantity"); quantite != effet.end()) {
                    if (!quantite->is_number_integer() || quantite->get<int>() < 1) {
                        rapport.noeud(noeud.id, "'quantity' doit etre un entier positif.");
                    } else {
                        action.quantity = quantite->get<int>();
                    }
                }
            }
            noeud.actions.push_back(std::move(action));
        }
    }
    noeud.next = exiger(brut, "next", noeud.id, rapport);
}

void lireJet(const Json& brut, DialogueNode& noeud, Rapport& rapport) {
    noeud.skill = exiger(brut, "skill", noeud.id, rapport);
    if (const auto seuil = brut.find("difficulty");
        seuil != brut.end() && seuil->is_number_integer()) {
        // Un nombre nu est le defaut que `EX-DND-021` interdit : il ne dit pas ce qu'il vaut, et
        // regler l'equilibre demanderait de relire chaque dialogue.
        rapport.noeud(noeud.id,
                      "'difficulty' nomme un degre de rules/difficulty.json, jamais un "
                      "nombre.");
    } else {
        noeud.difficulty = exiger(brut, "difficulty", noeud.id, rapport);
    }
    noeud.onSuccess = exiger(brut, "success", noeud.id, rapport);
    noeud.onFailure = exiger(brut, "failure", noeud.id, rapport);
}

/// Les cibles d'un noeud, dans l'ordre de la donnee.
[[nodiscard]] std::vector<const std::string*> ciblesDe(const DialogueNode& noeud) {
    std::vector<const std::string*> cibles;
    switch (noeud.kind) {
        case DialogueNodeKind::Line:
            if (noeud.choices.empty()) {
                cibles.push_back(&noeud.next);
            }
            for (const DialogueChoice& choix : noeud.choices) {
                cibles.push_back(&choix.next);
            }
            break;
        case DialogueNodeKind::Condition:
            cibles.push_back(&noeud.whenTrue);
            cibles.push_back(&noeud.whenFalse);
            break;
        case DialogueNodeKind::Action:
            cibles.push_back(&noeud.next);
            break;
        case DialogueNodeKind::Check:
            cibles.push_back(&noeud.onSuccess);
            cibles.push_back(&noeud.onFailure);
            break;
        case DialogueNodeKind::End:
            break;
    }
    return cibles;
}

/// Un noeud ou le joueur CHOISIT : le seul par lequel une boucle a le droit de passer.
[[nodiscard]] bool estUnArret(const DialogueNode& noeud) {
    return noeud.kind == DialogueNodeKind::Line && !noeud.choices.empty();
}

/**
 * Les controles de graphe, sur un graphe dont chaque noeud est lu et chaque cible existe. Les
 * faire sur un graphe incomplet produirait des orphelins et des impasses qui ne sont que l'ombre
 * d'une cible mal orthographiee.
 */
void controlerLeGraphe(const DialogueGraph& graphe, Rapport& rapport) {
    std::map<std::string, std::size_t, std::less<>> indices;
    for (std::size_t i = 0; i < graphe.nodes.size(); ++i) {
        indices.emplace(graphe.nodes[i].id, i);
    }
    const auto indiceDe = [&indices](const std::string& id) { return indices.find(id)->second; };

    // Orphelins : ce que rien n'atteint depuis l'entree.
    std::vector<bool> atteint(graphe.nodes.size(), false);
    std::vector<std::size_t> pile{indiceDe(graphe.start)};
    atteint[pile.back()] = true;
    while (!pile.empty()) {
        const std::size_t i = pile.back();
        pile.pop_back();
        for (const std::string* cible : ciblesDe(graphe.nodes[i])) {
            const std::size_t j = indiceDe(*cible);
            if (!atteint[j]) {
                atteint[j] = true;
                pile.push_back(j);
            }
        }
    }
    for (std::size_t i = 0; i < graphe.nodes.size(); ++i) {
        if (!atteint[i]) {
            rapport.noeud(graphe.nodes[i].id,
                          "orphelin : aucun chemin ne l'atteint depuis '" + graphe.start + "'.");
        }
    }

    // Cycles non intentionnels : une boucle qui ne traverse aucun arret. Parcours en profondeur
    // restreint aux noeuds qui ne sont pas des arrets -- une arete vers un arret termine le chemin.
    enum class Couleur { Blanc, Gris, Noir };
    std::vector<Couleur> couleurs(graphe.nodes.size(), Couleur::Blanc);
    std::vector<std::size_t> chemin;
    std::set<std::string> dejaSignales;
    const std::function<void(std::size_t)> visiter = [&](std::size_t i) {
        couleurs[i] = Couleur::Gris;
        chemin.push_back(i);
        for (const std::string* cible : ciblesDe(graphe.nodes[i])) {
            const std::size_t j = indiceDe(*cible);
            if (estUnArret(graphe.nodes[j])) {
                continue;
            }
            if (couleurs[j] == Couleur::Gris) {
                const auto debut = std::ranges::find(chemin, j);
                std::string trace;
                for (auto k = debut; k != chemin.end(); ++k) {
                    trace += graphe.nodes[*k].id + " -> ";
                }
                trace += graphe.nodes[j].id;
                if (dejaSignales.insert(graphe.nodes[j].id).second) {
                    rapport.noeud(graphe.nodes[j].id,
                                  "cycle non intentionnel, sans reponse a donner : " + trace + ".");
                }
            } else if (couleurs[j] == Couleur::Blanc) {
                visiter(j);
            }
        }
        chemin.pop_back();
        couleurs[i] = Couleur::Noir;
    };
    for (std::size_t i = 0; i < graphe.nodes.size(); ++i) {
        if (couleurs[i] == Couleur::Blanc && !estUnArret(graphe.nodes[i])) {
            visiter(i);
        }
    }

    // Impasses : les noeuds atteints d'ou aucune fin n'est atteignable. Parcours inverse depuis
    // les fins.
    std::vector<std::vector<std::size_t>> predecesseurs(graphe.nodes.size());
    for (std::size_t i = 0; i < graphe.nodes.size(); ++i) {
        for (const std::string* cible : ciblesDe(graphe.nodes[i])) {
            predecesseurs[indiceDe(*cible)].push_back(i);
        }
    }
    std::vector<bool> menaUneFin(graphe.nodes.size(), false);
    for (std::size_t i = 0; i < graphe.nodes.size(); ++i) {
        if (graphe.nodes[i].kind == DialogueNodeKind::End) {
            menaUneFin[i] = true;
            pile.push_back(i);
        }
    }
    while (!pile.empty()) {
        const std::size_t i = pile.back();
        pile.pop_back();
        for (const std::size_t j : predecesseurs[i]) {
            if (!menaUneFin[j]) {
                menaUneFin[j] = true;
                pile.push_back(j);
            }
        }
    }
    for (std::size_t i = 0; i < graphe.nodes.size(); ++i) {
        if (atteint[i] && !menaUneFin[i]) {
            rapport.noeud(graphe.nodes[i].id, "impasse : aucune fin n'est atteignable d'ici.");
        }
    }
}

[[nodiscard]] std::vector<std::filesystem::path> fichiersJson(const std::filesystem::path& dossier,
                                                              std::error_code& code) {
    std::vector<std::filesystem::path> fichiers;
    for (const auto& entree : std::filesystem::directory_iterator(dossier, code)) {
        if (entree.is_regular_file(code) && entree.path().extension() == ".json") {
            fichiers.push_back(entree.path());
        }
    }
    std::ranges::sort(fichiers);
    return fichiers;
}

}  // namespace

// ---------------------------------------------------------------------------------------------

bool FlagCondition::holds(const WorldFlags& flags) const {
    return flags.isSet(flag) == expected;
}

const DialogueNode* DialogueGraph::find(std::string_view nodeId) const {
    const auto trouve = std::ranges::find(nodes, nodeId, &DialogueNode::id);
    return trouve == nodes.end() ? nullptr : &*trouve;
}

std::string dialogueSpeakerKey(std::string_view dialogueId) {
    return "dialogue." + std::string(dialogueId) + ".speaker";
}

std::string dialogueLineKey(std::string_view dialogueId, std::string_view nodeId) {
    return "dialogue." + std::string(dialogueId) + '.' + std::string(nodeId);
}

std::string dialogueChoiceKey(std::string_view dialogueId, std::string_view nodeId,
                              std::string_view choiceId) {
    return dialogueLineKey(dialogueId, nodeId) + '.' + std::string(choiceId);
}

std::string_view dialogueAttitudeName(DialogueAttitude attitude) noexcept {
    switch (attitude) {
        case DialogueAttitude::Friendly:
            return "friendly";
        case DialogueAttitude::Indifferent:
            return "indifferent";
        case DialogueAttitude::Hostile:
            return "hostile";
    }
    return "indifferent";
}

std::string dialogueAttitudeKey(DialogueAttitude attitude) {
    return "dialogue.attitude." + std::string(dialogueAttitudeName(attitude));
}

std::vector<std::string> dialogueTextKeys(const DialogueGraph& graph) {
    std::vector<std::string> cles;
    const auto ajouter = [&cles](std::string cle) {
        if (std::ranges::find(cles, cle) == cles.end()) {
            cles.push_back(std::move(cle));
        }
    };
    ajouter(dialogueSpeakerKey(graph.id));
    ajouter(dialogueAttitudeKey(graph.attitude));
    for (const DialogueNode& noeud : graph.nodes) {
        if (noeud.kind != DialogueNodeKind::Line) {
            continue;
        }
        ajouter(dialogueLineKey(graph.id, noeud.id));
        if (noeud.attitude) {
            ajouter(dialogueAttitudeKey(*noeud.attitude));
        }
        if (noeud.choices.empty()) {
            ajouter(std::string(DIALOGUE_CONTINUE_KEY));
        }
        for (const DialogueChoice& choix : noeud.choices) {
            ajouter(dialogueChoiceKey(graph.id, noeud.id, choix.id));
        }
    }
    return cles;
}

std::string questStartedFlag(std::string_view questId) {
    return "quest/" + std::string(questId) + "/started";
}

// ---------------------------------------------------------------------------------------------

DialogueLoad readDialogue(std::string_view json, std::string_view origin) {
    DialogueLoad resultat;
    Rapport rapport(origin);

    const JsonDocument document = readJsonObject(json, SANS_GARDE_DE_VERSION, origin);
    if (!document.ok()) {
        resultat.errors.push_back(std::string(origin) + " : " + document.message);
        return resultat;
    }
    const Json& racine = document.root;

    DialogueGraph graphe;
    if (const auto id = texte(racine, "id")) {
        graphe.id = *id;
    } else {
        rapport.document("champ 'id' absent ou vide.");
    }
    if (const auto entree = texte(racine, "start")) {
        graphe.start = *entree;
    } else {
        rapport.document("champ 'start' absent ou vide.");
    }

    const auto interlocuteur = racine.find("speaker");
    if (interlocuteur == racine.end() || !interlocuteur->is_object()) {
        rapport.document("champ 'speaker' absent ou non objet.");
    } else {
        const auto langues = interlocuteur->find("languages");
        if (langues == interlocuteur->end() || !langues->is_array() || langues->empty()) {
            // Sans langue declaree, « refuse faute de langue commune » ne se deciderait jamais :
            // le PNJ parlerait a tout le monde, ce que `EX-RPG-042` interdit de supposer.
            rapport.document(
                "'speaker.languages' absent ou vide : un PNJ parle au moins une "
                "langue.");
        } else {
            for (const Json& langue : *langues) {
                if (langue.is_string() && !langue.get<std::string>().empty()) {
                    graphe.speakerLanguages.push_back(langue.get<std::string>());
                } else {
                    rapport.document(
                        "'speaker.languages' contient une valeur qui n'est pas un "
                        "identifiant.");
                }
            }
        }
        if (const auto attitude = interlocuteur->find("attitude");
            attitude != interlocuteur->end()) {
            const auto lue =
                attitude->is_string() ? attitudeDepuis(attitude->get<std::string>()) : std::nullopt;
            if (lue) {
                graphe.attitude = *lue;
            } else {
                rapport.document("'speaker.attitude' inconnue (friendly, indifferent, hostile).");
            }
        }
    }

    const auto noeuds = racine.find("nodes");
    if (noeuds == racine.end() || !noeuds->is_array() || noeuds->empty()) {
        rapport.document("champ 'nodes' absent, vide ou non tableau.");
    } else {
        std::set<std::string> vus;
        for (const Json& brut : *noeuds) {
            if (!brut.is_object()) {
                rapport.document("un noeud n'est pas un objet.");
                continue;
            }
            DialogueNode noeud;
            if (const auto id = texte(brut, "id")) {
                noeud.id = *id;
            } else {
                rapport.document("un noeud n'a pas d'identifiant.");
                continue;
            }
            if (!vus.insert(noeud.id).second) {
                rapport.noeud(noeud.id, "identifiant en double.");
                continue;
            }
            const auto type = texte(brut, "type");
            const auto nature = type ? natureDepuis(*type) : std::nullopt;
            if (!nature) {
                rapport.noeud(noeud.id, "type inconnu (line, condition, action, check, end) : '" +
                                            type.value_or("") + "'.");
                continue;
            }
            noeud.kind = *nature;
            switch (noeud.kind) {
                case DialogueNodeKind::Line:
                    lireReplique(brut, noeud, rapport);
                    break;
                case DialogueNodeKind::Condition:
                    lireCondition(brut, noeud, rapport);
                    break;
                case DialogueNodeKind::Action:
                    lireAction(brut, noeud, rapport);
                    break;
                case DialogueNodeKind::Check:
                    lireJet(brut, noeud, rapport);
                    break;
                case DialogueNodeKind::End:
                    break;
            }
            graphe.nodes.push_back(std::move(noeud));
        }
    }

    // Les cibles, une fois tous les noeuds connus.
    if (!graphe.start.empty() && !graphe.nodes.empty() && graphe.find(graphe.start) == nullptr) {
        rapport.document("noeud cible inconnu : l'entree 'start' nomme '" + graphe.start + "'.");
    }
    for (const DialogueNode& noeud : graphe.nodes) {
        for (const std::string* cible : ciblesDe(noeud)) {
            if (!cible->empty() && graphe.find(*cible) == nullptr) {
                rapport.noeud(noeud.id, "noeud cible inconnu : '" + *cible + "'.");
            }
        }
    }
    const bool fin = std::ranges::any_of(
        graphe.nodes, [](const DialogueNode& n) { return n.kind == DialogueNodeKind::End; });
    if (!graphe.nodes.empty() && !fin) {
        rapport.document("aucun noeud 'end' : la conversation ne pourrait pas se terminer.");
    }

    if (rapport.vide()) {
        controlerLeGraphe(graphe, rapport);
    }
    if (!rapport.vide()) {
        resultat.errors = rapport.extraire();
        return resultat;
    }
    resultat.graph = std::move(graphe);
    return resultat;
}

DialogueLoad loadDialogue(const std::filesystem::path& path) {
    const JsonDocument lu = readJsonObjectFromFile(path, SANS_GARDE_DE_VERSION);
    if (!lu.ok()) {
        DialogueLoad echec;
        echec.errors.push_back(path.string() + " : " + lu.message);
        return echec;
    }
    return readDialogue(lu.root.dump(), path.string());
}

std::vector<std::string> validateDialogueReferences(const DialogueGraph& graph,
                                                    const DialogueReferences& references) {
    std::vector<std::string> erreurs;
    const auto signaler = [&](std::string_view noeud, const std::string& message) {
        erreurs.push_back("dialogue '" + graph.id + "' : noeud '" + std::string(noeud) +
                          "' : " + message);
    };
    if (references.languageExists) {
        for (const std::string& langue : graph.speakerLanguages) {
            if (!references.languageExists(langue)) {
                erreurs.push_back("dialogue '" + graph.id + "' : langue inconnue '" + langue +
                                  "'.");
            }
        }
    }
    for (const DialogueNode& noeud : graph.nodes) {
        if (noeud.kind == DialogueNodeKind::Check) {
            if (references.skills != nullptr && references.skills->find(noeud.skill) == nullptr) {
                signaler(noeud.id, "competence inconnue '" + noeud.skill + "'.");
            }
            if (references.difficulty != nullptr &&
                references.difficulty->find(noeud.difficulty) == nullptr) {
                signaler(noeud.id, "degre de difficulte inconnu '" + noeud.difficulty + "'.");
            }
        }
        if (noeud.kind == DialogueNodeKind::Action && references.itemExists) {
            for (const DialogueAction& action : noeud.actions) {
                if (action.kind == DialogueActionKind::GiveItem &&
                    !references.itemExists(action.target)) {
                    signaler(noeud.id, "objet inconnu '" + action.target + "'.");
                }
            }
        }
    }
    return erreurs;
}

const DialogueGraph* DialogueCatalog::find(std::string_view id) const {
    const auto trouve = std::ranges::find(dialogues, id, &DialogueGraph::id);
    return trouve == dialogues.end() ? nullptr : &*trouve;
}

DialogueCatalog loadDialogues(const std::filesystem::path& directory) {
    DialogueCatalog catalogue;
    std::error_code code;
    if (!std::filesystem::is_directory(directory, code)) {
        catalogue.errors.push_back(directory.string() + " : dossier absent ou illisible.");
        return catalogue;
    }
    for (const std::filesystem::path& chemin : fichiersJson(directory, code)) {
        DialogueLoad lu = loadDialogue(chemin);
        catalogue.errors.insert(catalogue.errors.end(), lu.errors.begin(), lu.errors.end());
        if (!lu.graph) {
            continue;
        }
        if (lu.graph->id != chemin.stem().string()) {
            // Une carte nomme un dialogue par son identifiant ; un fichier qui en porte un autre
            // se chargerait et ne s'ouvrirait jamais.
            catalogue.errors.push_back(chemin.string() + " : l'identifiant '" + lu.graph->id +
                                       "' doit etre le nom du fichier.");
            continue;
        }
        catalogue.dialogues.push_back(std::move(*lu.graph));
    }
    return catalogue;
}

// ---------------------------------------------------------------------------------------------

CharacterListener::CharacterListener(const CharacterSheet& sheet, Inventory& inventory,
                                     const ExperienceTable& experience, const SkillCatalog& skills)
    : _sheet(sheet), _inventory(inventory), _experience(experience), _skills(skills) {}

bool CharacterListener::speaks(std::string_view languageId) const {
    return _sheet.languages.contains(std::string(languageId));
}

std::vector<Modifier> CharacterListener::skillModifiers(std::string_view skillId) const {
    std::vector<Modifier> modificateurs;
    const SkillDefinition* competence = _skills.find(skillId);
    if (competence == nullptr) {
        return modificateurs;
    }
    // Detaille, et non le total de `skillModifier` : « +3 (charisma) + 2 (maitrise) » se
    // restitue, « +5 » ne dit pas d'ou il vient (EX-DND-003). La regle reste la sienne.
    const SkillCheckModifier total = skillModifier(_sheet, _experience, _skills, skillId);
    const int caracteristique = _sheet.modifier(competence->ability);
    modificateurs.push_back({std::string(abilityName(competence->ability)), caracteristique});
    if (total.proficient) {
        modificateurs.push_back({"maitrise", total.value - caracteristique});
    }
    return modificateurs;
}

void CharacterListener::receiveItem(std::string_view itemId, int quantity) {
    addToBackpack(_inventory, std::string(itemId), quantity);
}

// ---------------------------------------------------------------------------------------------

DialogueRunner::DialogueRunner(const DialogueGraph& graph, WorldFlags& flags,
                               DialogueListener& listener, const DifficultyScale& difficulty,
                               DeterministicRandom& random)
    : _graph(graph), _flags(flags), _listener(listener), _difficulty(difficulty), _random(random) {}

DialogueState DialogueRunner::start() {
    if (_state != DialogueState::NotStarted) {
        return _state;
    }
    const bool commune = std::ranges::any_of(
        _graph.speakerLanguages, [this](const std::string& l) { return _listener.speaks(l); });
    if (!commune) {
        // Refuse AVANT tout noeud : une conversation qui n'a pas lieu ne pose aucun drapeau, sans
        // quoi un PNJ qu'on ne comprend pas ferait avancer une quete.
        std::string langues;
        for (const std::string& l : _graph.speakerLanguages) {
            langues += (langues.empty() ? "" : ", ") + l;
        }
        _journal.push_back("refus : aucune langue commune (" + langues + ")");
        _state = DialogueState::Refused;
        return _state;
    }
    _journal.push_back("debut : " + _graph.id);
    _automaticSteps = 0;
    advanceTo(_graph.start);
    return _state;
}

ChoiceResult DialogueRunner::choose(std::string_view choiceId) {
    if (_state != DialogueState::AwaitingChoice || _current == nullptr) {
        return ChoiceResult::NotAwaiting;
    }
    std::string suite;
    if (_current->choices.empty()) {
        if (choiceId != DIALOGUE_CONTINUE_CHOICE) {
            return ChoiceResult::Unavailable;
        }
        suite = _current->next;
    } else {
        const auto choix = std::ranges::find(_current->choices, choiceId, &DialogueChoice::id);
        // La condition est REEVALUEE : entre l'affichage et le geste, un drapeau a pu changer, et
        // l'ecran ne doit pas pouvoir faire passer une reponse que la donnee n'offre plus.
        if (choix == _current->choices.end() ||
            (choix->condition && !choix->condition->holds(_flags))) {
            return ChoiceResult::Unavailable;
        }
        suite = choix->next;
    }
    _journal.push_back("reponse : " + std::string(choiceId));
    _automaticSteps = 0;
    _lastCheck.reset();
    advanceTo(suite);
    return ChoiceResult::Advanced;
}

void DialogueRunner::advanceTo(const std::string& nodeId) {
    std::string id = nodeId;
    while (true) {
        const DialogueNode* noeud = _graph.find(id);
        // La validation rend ces deux gardes inatteignables pour un graphe charge ; un graphe
        // construit a la main peut les atteindre, et il vaut mieux finir que boucler ou planter.
        if (noeud == nullptr) {
            _journal.push_back("erreur : noeud inconnu '" + id + "', fin forcee");
            _current = nullptr;
            _state = DialogueState::Ended;
            return;
        }
        if (++_automaticSteps > _graph.nodes.size() + 1) {
            _journal.push_back("erreur : boucle sans reponse, fin forcee");
            _current = nullptr;
            _state = DialogueState::Ended;
            return;
        }
        switch (noeud->kind) {
            case DialogueNodeKind::Line:
                _journal.push_back("replique : " + noeud->id);
                _current = noeud;
                _state = DialogueState::AwaitingChoice;
                return;
            case DialogueNodeKind::End:
                _journal.push_back("fin : " + noeud->id);
                _current = nullptr;
                _state = DialogueState::Ended;
                return;
            case DialogueNodeKind::Condition: {
                const bool tient = noeud->condition.holds(_flags);
                _journal.push_back("condition : " + noeud->id + " (" + noeud->condition.flag +
                                   (tient ? ") tient" : ") ne tient pas"));
                id = tient ? noeud->whenTrue : noeud->whenFalse;
                break;
            }
            case DialogueNodeKind::Action:
                _journal.push_back("action : " + noeud->id);
                for (const DialogueAction& action : noeud->actions) {
                    apply(action);
                }
                id = noeud->next;
                break;
            case DialogueNodeKind::Check:
                runCheck(*noeud);
                id = (_lastCheck && _lastCheck->nodeId == noeud->id &&
                      _lastCheck->result.succeeded())
                         ? noeud->onSuccess
                         : noeud->onFailure;
                break;
        }
    }
}

void DialogueRunner::apply(const DialogueAction& action) {
    switch (action.kind) {
        case DialogueActionKind::SetFlag:
            _flags.set(action.target);
            _journal.push_back("drapeau pose : " + action.target);
            break;
        case DialogueActionKind::ClearFlag:
            _flags.clear(action.target);
            _journal.push_back("drapeau retire : " + action.target);
            break;
        case DialogueActionKind::GiveItem:
            _listener.receiveItem(action.target, action.quantity);
            _journal.push_back("objet donne : " + action.target + " x" +
                               std::to_string(action.quantity));
            break;
        case DialogueActionKind::StartQuest:
            _flags.set(questStartedFlag(action.target));
            _journal.push_back("quete demarree : " + action.target);
            break;
    }
}

void DialogueRunner::runCheck(const DialogueNode& node) {
    const DifficultyTier* degre = _difficulty.find(node.difficulty);
    if (degre == nullptr) {
        // Un degre inconnu ne se jette pas contre 0 -- tout reussirait. L'echec est la suite
        // prudente, et le journal le dit.
        _lastCheck.reset();
        _journal.push_back("jet : " + node.id + " -- degre inconnu '" + node.difficulty +
                           "', echec");
        return;
    }
    const std::vector<Modifier> modificateurs = _listener.skillModifiers(node.skill);
    DialogueCheck jet;
    jet.nodeId = node.id;
    jet.skill = node.skill;
    jet.difficulty = node.difficulty;
    jet.result = rollCheck(degre->dc, modificateurs, RollStance::Normal, _random);
    _journal.push_back("jet : " + node.id + " (" + node.skill + ") " + jet.result.describe());
    _lastCheck = std::move(jet);
}

const DialogueNode* DialogueRunner::currentLine() const {
    return _state == DialogueState::AwaitingChoice ? _current : nullptr;
}

std::string DialogueRunner::lineKey() const {
    const DialogueNode* ligne = currentLine();
    return ligne == nullptr ? std::string{} : dialogueLineKey(_graph.id, ligne->id);
}

DialogueAttitude DialogueRunner::attitude() const {
    const DialogueNode* ligne = currentLine();
    return (ligne != nullptr && ligne->attitude) ? *ligne->attitude : _graph.attitude;
}

std::vector<AvailableChoice> DialogueRunner::choices() const {
    std::vector<AvailableChoice> proposees;
    const DialogueNode* ligne = currentLine();
    if (ligne == nullptr) {
        return proposees;
    }
    const auto competenceJetee = [this](const std::string& cible) {
        const DialogueNode* suite = _graph.find(cible);
        return (suite != nullptr && suite->kind == DialogueNodeKind::Check) ? suite->skill
                                                                            : std::string{};
    };
    if (ligne->choices.empty()) {
        proposees.push_back({std::string(DIALOGUE_CONTINUE_CHOICE),
                             std::string(DIALOGUE_CONTINUE_KEY), competenceJetee(ligne->next)});
        return proposees;
    }
    for (const DialogueChoice& choix : ligne->choices) {
        if (choix.condition && !choix.condition->holds(_flags)) {
            continue;
        }
        proposees.push_back({choix.id, dialogueChoiceKey(_graph.id, ligne->id, choix.id),
                             competenceJetee(choix.next)});
    }
    return proposees;
}

// ---------------------------------------------------------------------------------------------

std::optional<DialogueTrigger> dialogueTriggerFor(const MapEntity& entity) {
    if (entity.type != NPC_ENTITY_TYPE) {
        return std::nullopt;
    }
    const auto trouve = entity.properties.find(std::string(NPC_DIALOGUE_PROPERTY));
    if (trouve == entity.properties.end()) {
        return std::nullopt;
    }
    const std::string* dialogue = std::get_if<std::string>(&trouve->second);
    if (dialogue == nullptr || dialogue->empty()) {
        return std::nullopt;
    }
    return DialogueTrigger{*dialogue, entity.position};
}

}  // namespace core
