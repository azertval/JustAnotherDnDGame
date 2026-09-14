// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Game/DialogueMode.h"

namespace hmi {

std::string_view DialogueMode::name() const {
    return "dialogue";
}

std::vector<std::string_view> DialogueMode::passOrder() const {
    // Doit rester le miroir exact de step() ci-dessous : un test compare cette liste a la sequence
    // reellement appelee.
    return {"snapshotPreviousPositions", "advanceParticles", "advanceScreenShake",
            "advanceAnimations", "updateCamera"};
}

void DialogueMode::onLoad(IGameModePasses& passes) {
    // Rien a preparer : la conversation est ouverte par core::DialogueRunner::start, qui ne depend
    // d'aucune passe.
    (void)passes;
}

core::LevelOutcome DialogueMode::step(IGameModePasses& passes, const core::PlayerInput& input,
                                      float fixedDelta) {
    // L'intention du joueur choisit une REPONSE (core::DialogueRunner::choose) ; elle ne deplace
    // personne. L'ignorer ici est ce qui GELE le monde.
    (void)input;

    passes.snapshotPreviousPositions();
    // La scene reste visible derriere le parchemin : elle respire, elle ne joue pas.
    passes.advanceParticles(fixedDelta);
    passes.advanceScreenShake(fixedDelta);
    passes.advanceAnimations(fixedDelta);
    passes.updateCamera(fixedDelta);

    // Rien de ce qu'on dit ne fait perdre le niveau.
    return core::LevelOutcome::Playing;
}

void DialogueMode::onUnload(IGameModePasses& passes) {
    // Le retour a l'exploration n'a rien a restituer : le personnage n'a pas bouge, et ce que la
    // conversation a change vit dans les drapeaux de monde.
    (void)passes;
}

}  // namespace hmi
