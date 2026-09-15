// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

/**
 * @file test_dialogue_screen.cpp
 * @brief Tests unitaires de ce que l'écran de dialogue affiche (`LOT-15`) : réplique, réponses,
 *        jet annoncé puis restitué, refus, fin.
 */

#include <set>
#include <string>
#include <string_view>
#include <vector>

#include <gtest/gtest.h>

#include "Core/Gameplay/WorldFlags.h"
#include "Core/Math/DeterministicRandom.h"
#include "Core/Rpg/Check.h"
#include "Core/Rpg/Dialogue.h"
#include "HMI/Presentation/DialogueScreen.h"

namespace {

/// Un catalogue d'essai qui rend la cle entre crochets : ce qui s'affiche dit d'ou il vient.
std::string cle(std::string_view key) {
    if (key == "dialogue.check.summary") {
        return "%1 : %2 contre %3 -- %4";
    }
    return "<" + std::string(key) + ">";
}

class Auditeur final : public core::DialogueListener {
public:
    Auditeur(std::set<std::string> langues, int bonus)
        : _langues(std::move(langues)), _bonus(bonus) {}
    [[nodiscard]] bool speaks(std::string_view l) const override {
        return _langues.contains(std::string(l));
    }
    [[nodiscard]] std::vector<core::Modifier> skillModifiers(std::string_view) const override {
        return {{"essai", _bonus}};
    }
    void receiveItem(std::string_view, int) override {}

private:
    std::set<std::string> _langues;
    int _bonus;
};

core::DialogueGraph grapheDEssai() {
    const core::DialogueLoad lu = core::readDialogue(
        R"({"id":"garde","name":"Garde","source":"original",)"
        R"("speaker":{"languages":["common"],"attitude":"hostile"},"start":"halte","nodes":[)"
        R"({"id":"halte","type":"line","choices":[{"id":"negocier","next":"jet"},)"
        R"({"id":"partir","next":"fin"}]},)"
        R"({"id":"jet","type":"check","skill":"animal-handling","difficulty":"facile",)"
        R"("success":"passe","failure":"passe"},)"
        R"({"id":"passe","type":"line","attitude":"friendly","next":"fin"},)"
        R"({"id":"fin","type":"end"}]})",
        "garde.json");
    return *lu.graph;
}

core::DifficultyScale echelle() {
    core::DifficultyScale e;
    e.tiers.push_back({"facile", "Facile", 10});
    return e;
}

}  // namespace

/**
 * @brief Une réplique en attente s'affiche traduite, et une réponse qui mène à un jet l'annonce.
 * \castest{<b>L'ecran de dialogue annonce le jet avant le choix.</b><br/>
 * \tcat Unitaire · Interface<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Ouvrir un dialogue de garde hostile.<br/>2. Lire les valeurs de l'ecran.<br/>
 * \tattendu Nom, attitude hostile et replique par leur cle ; deux reponses dans l'ordre ; la
 * premiere porte « [competence] » avec la cle de lexique a tiret bas ; aucune restitution de jet ;
 * l'ecran n'est pas termine.
 * }
 */
TEST(DialogueScreenTest, UneRepliqueAnnonceLeJetDeSaReponse) {
    const core::DialogueGraph graphe = grapheDEssai();
    const core::DifficultyScale e = echelle();
    core::WorldFlags drapeaux;
    Auditeur auditeur({"common"}, 30);
    core::DeterministicRandom hasard(5);
    core::DialogueRunner runner(graphe, drapeaux, auditeur, e, hasard);
    ASSERT_EQ(runner.start(), core::DialogueState::AwaitingChoice);

    const hmi::DialogueScreenValues v = hmi::dialogueScreenValues(runner, cle);
    EXPECT_EQ(v.speakerName, "<dialogue.garde.speaker>");
    EXPECT_EQ(v.attitude, "<dialogue.attitude.hostile>");
    EXPECT_EQ(v.line, "<dialogue.garde.halte>");
    ASSERT_EQ(v.replies.size(), 2U);
    EXPECT_EQ(v.replies[0].id, "negocier");
    EXPECT_EQ(v.replies[0].label, "<dialogue.garde.halte.negocier>");
    EXPECT_EQ(v.replies[0].value, "[<rpg.skill.animal_handling>]");
    EXPECT_EQ(v.replies[1].value, "");
    EXPECT_TRUE(v.checkOutcome.empty());
    EXPECT_FALSE(v.finished);
}

/**
 * @brief Le jet joué par une réponse se restitue sur la réplique qui en découle, et seulement
 *        sur celle-là.
 * \castest{<b>Le jet se restitue sur la replique qui suit, puis s'efface.</b><br/>
 * \tcat Unitaire · Interface<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Negocier (bonus +30 contre 10).<br/>2. Lire les valeurs.<br/>3. Continuer jusqu'a la
 * fin.<br/>
 * \tattendu Apres le jet : attitude amicale, restitution « competence : total contre 10 --
 * reussite », une reponse « continuer ». A la fin : termine, restitution effacee, une seule
 * reponse « quitter ».
 * }
 */
TEST(DialogueScreenTest, LeJetSeRestitueSurLaRepliqueQuiSuit) {
    const core::DialogueGraph graphe = grapheDEssai();
    const core::DifficultyScale e = echelle();
    core::WorldFlags drapeaux;
    Auditeur auditeur({"common"}, 30);
    core::DeterministicRandom hasard(5);
    core::DialogueRunner runner(graphe, drapeaux, auditeur, e, hasard);
    ASSERT_EQ(runner.start(), core::DialogueState::AwaitingChoice);
    ASSERT_EQ(runner.choose("negocier"), core::ChoiceResult::Advanced);

    const hmi::DialogueScreenValues apres = hmi::dialogueScreenValues(runner, cle);
    EXPECT_EQ(apres.attitude, "<dialogue.attitude.friendly>");
    const std::string total = std::to_string(runner.lastCheck()->result.total);
    EXPECT_EQ(apres.checkOutcome,
              "<rpg.skill.animal_handling> : " + total + " contre 10 -- <dialogue.check.success>");
    ASSERT_EQ(apres.replies.size(), 1U);
    EXPECT_EQ(apres.replies[0].id, "continue");
    EXPECT_EQ(apres.replies[0].label, "<dialogue.continue>");

    ASSERT_EQ(runner.choose("continue"), core::ChoiceResult::Advanced);
    const hmi::DialogueScreenValues fin = hmi::dialogueScreenValues(runner, cle);
    EXPECT_TRUE(fin.finished);
    EXPECT_TRUE(fin.checkOutcome.empty());
    ASSERT_EQ(fin.replies.size(), 1U);
    EXPECT_EQ(fin.replies[0].id, std::string(hmi::DIALOGUE_LEAVE_REPLY));
}

/**
 * @brief Un refus faute de langue commune montre le refus, et seulement « Quitter ».
 * \castest{<b>L'ecran montre le refus faute de langue commune.</b><br/>
 * \tcat Unitaire · Interface<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Ouvrir le dialogue du garde avec un interlocuteur qui ne parle que l'elfique.<br/>
 * 2. Lire les valeurs.<br/>
 * \tattendu Replique = cle du refus ; une seule reponse « quitter » ; l'ecran ne se referme pas de
 * lui-meme, pour que le refus se lise.
 * }
 */
TEST(DialogueScreenTest, UnRefusMontreLeRefusEtQuitter) {
    const core::DialogueGraph graphe = grapheDEssai();
    const core::DifficultyScale e = echelle();
    core::WorldFlags drapeaux;
    Auditeur elfe({"elvish"}, 0);
    core::DeterministicRandom hasard(5);
    core::DialogueRunner runner(graphe, drapeaux, elfe, e, hasard);
    ASSERT_EQ(runner.start(), core::DialogueState::Refused);

    const hmi::DialogueScreenValues v = hmi::dialogueScreenValues(runner, cle);
    EXPECT_EQ(v.line, "<dialogue.refused>");
    EXPECT_FALSE(v.finished);
    ASSERT_EQ(v.replies.size(), 1U);
    EXPECT_EQ(v.replies[0].id, std::string(hmi::DIALOGUE_LEAVE_REPLY));
    EXPECT_EQ(v.replies[0].label, "<dialogue.leave>");
}
