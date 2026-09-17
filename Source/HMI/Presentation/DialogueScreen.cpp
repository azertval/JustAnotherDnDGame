// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "HMI/Presentation/DialogueScreen.h"

#include <algorithm>
#include <array>
#include <cstddef>

namespace hmi {

namespace {

// Remplace %1..%4 dans un gabarit traduit. L'ordre des mots change d'une langue a l'autre ; un
// gabarit par langue le permet, une concatenation en C++ l'interdirait.
[[nodiscard]] std::string remplir(std::string gabarit, const std::array<std::string, 4>& valeurs) {
    for (std::size_t i = 0; i < valeurs.size(); ++i) {
        const std::string marque = "%" + std::to_string(i + 1);
        for (std::size_t position = gabarit.find(marque); position != std::string::npos;
             position = gabarit.find(marque, position + valeurs[i].size())) {
            gabarit.replace(position, marque.size(), valeurs[i]);
        }
    }
    return gabarit;
}

}  // namespace

std::string skillLabelKey(std::string_view skillId) {
    std::string cle = "rpg.skill." + std::string(skillId);
    std::ranges::replace(cle, '-', '_');
    return cle;
}

DialogueScreenValues dialogueScreenValues(const core::DialogueRunner& runner,
                                          const TextLookup& text) {
    DialogueScreenValues valeurs;
    const core::DialogueGraph& graphe = runner.graph();
    valeurs.speakerName = text(core::dialogueSpeakerKey(graphe.id));
    valeurs.attitude = text(core::dialogueAttitudeKey(runner.attitude()));

    const auto quitter = [&text]() {
        return DialogueReply{
            .id = std::string(DIALOGUE_LEAVE_REPLY), .label = text("dialogue.leave"), .value = {}};
    };

    switch (runner.state()) {
        case core::DialogueState::Refused:
            valeurs.line = text("dialogue.refused");
            valeurs.replies.push_back(quitter());
            return valeurs;
        case core::DialogueState::Ended:
        case core::DialogueState::NotStarted:
            valeurs.finished = runner.state() == core::DialogueState::Ended;
            valeurs.replies.push_back(quitter());
            return valeurs;
        case core::DialogueState::AwaitingChoice:
            break;
    }

    valeurs.line = text(runner.lineKey());
    for (const core::AvailableChoice& choix : runner.choices()) {
        DialogueReply reponse{.id = choix.id, .label = text(choix.textKey), .value = {}};
        if (!choix.checkSkill.empty()) {
            // Annonce le jet AVANT qu'on le choisisse, comme une table l'annonce : un joueur qui
            // decouvre apres coup qu'il jouait sa Persuasion n'a pas choisi, il a subi.
            reponse.value = "[" + text(skillLabelKey(choix.checkSkill)) + "]";
        }
        valeurs.replies.push_back(std::move(reponse));
    }

    if (const auto& jet = runner.lastCheck()) {
        valeurs.checkOutcome = remplir(
            text("dialogue.check.summary"),
            {text(skillLabelKey(jet->skill)), std::to_string(jet->result.total),
             std::to_string(jet->result.target),
             text(jet->result.succeeded() ? "dialogue.check.success" : "dialogue.check.failure")});
    }
    return valeurs;
}

}  // namespace hmi
