// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_game_mode.cpp
 * @brief Tests unitaires du mode de jeu : l'ordre des passes du pas fixe (`EX-ARCH-002`, LOT-05).
 *
 * `hmi::GameSession` exige un atlas, un lot de sprites et une police : impossible à instancier
 * sans fenêtre. C'est précisément ce que l'extraction corrige — l'**ordre des passes** vit
 * désormais dans un objet qui ne parle que de `core::`, et se vérifie contre une implémentation
 * d'essai qui enregistre la séquence des appels sans rien simuler.
 */

#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "Core/Levels/LevelOutcome.h"
#include "Core/Physics/PlayerInput.h"
#include "HMI/Game/ExplorationMode.h"
#include "HMI/Game/IGameMode.h"

namespace {

// Enregistre la sequence des passes appelees, sans rien simuler. L'issue renvoyee par
// evaluateOutcome() est pilotee par le test.
class RecordingPasses final : public hmi::IGameModePasses {
public:
    explicit RecordingPasses(core::LevelOutcome outcome = core::LevelOutcome::Playing)
        : _outcome(outcome) {}

    void snapshotPreviousPositions() override {
        _calls.emplace_back("snapshotPreviousPositions");
    }
    void advanceParticles(float) override {
        _calls.emplace_back("advanceParticles");
    }
    void advanceScreenShake(float) override {
        _calls.emplace_back("advanceScreenShake");
    }
    void moveCharacter(const core::PlayerInput&, float) override {
        _calls.emplace_back("moveCharacter");
    }
    void advanceAnimations(float) override {
        _calls.emplace_back("advanceAnimations");
    }
    void updateCamera(float) override {
        _calls.emplace_back("updateCamera");
    }
    void updateMechanisms(const core::PlayerInput&) override {
        _calls.emplace_back("updateMechanisms");
    }
    void detectEvents() override {
        _calls.emplace_back("detectEvents");
    }
    void updateMechanismVisuals(float) override {
        _calls.emplace_back("updateMechanismVisuals");
    }
    core::LevelOutcome evaluateOutcome() override {
        _calls.emplace_back("evaluateOutcome");
        return _outcome;
    }
    void onLevelLost() override {
        _calls.emplace_back("onLevelLost");
    }

    [[nodiscard]] const std::vector<std::string>& calls() const noexcept {
        return _calls;
    }
    void clear() {
        _calls.clear();
    }

private:
    std::vector<std::string> _calls;
    core::LevelOutcome _outcome;
};

// La liste annoncee par le mode, en chaines comparables a ce que RecordingPasses enregistre.
std::vector<std::string> announcedOrder(const hmi::IGameMode& mode) {
    std::vector<std::string> names;
    for (const std::string_view name : mode.passOrder()) {
        names.emplace_back(name);
    }
    return names;
}

constexpr float FIXED_DELTA = 1.0f / 60.0f;

}  // namespace

/**
 * @brief L'ordre réellement appelé par le mode d'exploration est **exactement** celui qu'il
 * annonce pour les diagnostics (`EX-ARCH-002`).
 * \castest{<b>L'ordre des passes appele est celui annonce.</b><br/>
 * \tcat Unitaire · Mode de jeu<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Avancer le mode d'exploration d'un pas fixe sur des passes qui enregistrent leurs
 * appels.<br/>2. Comparer la sequence enregistree a passOrder().<br/>
 * \tattendu Les deux listes sont identiques, dans le meme ordre.
 * }
 */
TEST(ModeDeJeuTest, OrdreAppeleIdentiqueALOrdreAnnonce) {
    hmi::ExplorationMode mode;
    RecordingPasses passes;

    const core::LevelOutcome outcome = mode.step(passes, core::PlayerInput{}, FIXED_DELTA);

    EXPECT_EQ(outcome, core::LevelOutcome::Playing);
    EXPECT_EQ(passes.calls(), announcedOrder(mode));
}

/**
 * @brief L'ordre des passes ne dépend ni du pas ni de l'intention d'entrée : 600 pas produisent
 * 600 fois la même séquence (`EX-ARCH-002`).
 * \castest{<b>600 pas produisent 600 fois la meme sequence de passes.</b><br/>
 * \tcat Unitaire · Mode de jeu<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Avancer le mode de 600 pas fixes, avec une intention qui change a chaque pas.<br/>2.
 * Comparer la sequence de chaque pas a celle du premier.<br/>
 * \tattendu Les 600 sequences sont identiques.
 * }
 */
TEST(ModeDeJeuTest, SequenceStableSur600Pas) {
    hmi::ExplorationMode mode;
    const std::vector<std::string> expected = announcedOrder(mode);

    for (int step = 0; step < 600; ++step) {
        RecordingPasses passes;
        core::PlayerInput input;
        // L'intention varie d'un pas a l'autre : l'ordre des passes, lui, ne doit pas en dependre.
        input.interactPressed = (step % 3) == 0;
        mode.step(passes, input, FIXED_DELTA);
        ASSERT_EQ(passes.calls(), expected) << "pas " << step;
    }
}

/**
 * @brief Sur un échec, les conséquences (éclatement, secousse, rechargement) sont une passe à
 * part, appelée **après** l'évaluation de l'issue (`EX-ARCH-002`).
 * \castest{<b>Un echec declenche ses consequences apres l'evaluation.</b><br/>
 * \tcat Unitaire · Mode de jeu<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Avancer le mode d'un pas sur des passes qui repondent Lost a l'evaluation.<br/>
 * \tattendu onLevelLost est la derniere passe appelee, juste apres evaluateOutcome.
 * }
 */
TEST(ModeDeJeuTest, EchecDeclencheSesConsequencesApresLEvaluation) {
    hmi::ExplorationMode mode;
    RecordingPasses passes(core::LevelOutcome::Lost);

    const core::LevelOutcome outcome = mode.step(passes, core::PlayerInput{}, FIXED_DELTA);

    EXPECT_EQ(outcome, core::LevelOutcome::Lost);
    ASSERT_GE(passes.calls().size(), 2u);
    EXPECT_EQ(passes.calls().back(), "onLevelLost");
    EXPECT_EQ(passes.calls()[passes.calls().size() - 2], "evaluateOutcome");
}

/**
 * @brief Une réussite ne déclenche **pas** les conséquences d'un échec : l'appelant décide quoi
 * faire d'un niveau gagné (`EX-ARCH-002`).
 * \castest{<b>Une reussite ne declenche pas les consequences d'un echec.</b><br/>
 * \tcat Unitaire · Mode de jeu<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Avancer le mode d'un pas sur des passes qui repondent Won a l'evaluation.<br/>
 * \tattendu onLevelLost n'est jamais appele, et la sequence est celle annoncee.
 * }
 */
TEST(ModeDeJeuTest, ReussiteNeDeclenchePasLesConsequencesDUnEchec) {
    hmi::ExplorationMode mode;
    RecordingPasses passes(core::LevelOutcome::Won);

    const core::LevelOutcome outcome = mode.step(passes, core::PlayerInput{}, FIXED_DELTA);

    EXPECT_EQ(outcome, core::LevelOutcome::Won);
    EXPECT_EQ(passes.calls(), announcedOrder(mode));
}

/**
 * @brief Le mode d'exploration est **sans état** : ses crochets de bascule n'appellent aucune
 * passe, et il se nomme pour les diagnostics (`EX-ARCH-002`).
 * \castest{<b>Le mode d'exploration est sans etat et se nomme.</b><br/>
 * \tcat Unitaire · Mode de jeu<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Charger puis decharger le mode.<br/>
 * \tattendu Aucune passe appelee, et le mode se nomme 'exploration'.
 * }
 */
TEST(ModeDeJeuTest, ModeSansEtatEtNomme) {
    hmi::ExplorationMode mode;
    RecordingPasses passes;

    mode.onLoad(passes);
    mode.onUnload(passes);

    EXPECT_TRUE(passes.calls().empty());
    EXPECT_EQ(mode.name(), "exploration");
}
