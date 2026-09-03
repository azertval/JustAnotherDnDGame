// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Game/ExplorationMode.h"

namespace hmi {

std::string_view ExplorationMode::name() const {
    return "exploration";
}

std::vector<std::string_view> ExplorationMode::passOrder() const {
    // Doit rester le miroir exact de step() ci-dessous : un test compare cette liste a la sequence
    // reellement appelee, pour qu'un ordre modifie sans mettre la liste a jour echoue plutot que
    // de mentir aux diagnostics.
    return {"snapshotPreviousPositions", "advanceParticles", "advanceScreenShake", "moveCharacter",
            "advanceAnimations",         "updateCamera",     "updateMechanisms",   "detectEvents",
            "updateMechanismVisuals",    "evaluateOutcome"};
}

void ExplorationMode::onLoad(IGameModePasses& passes) {
    // Rien a preparer : l'etat d'entree d'un niveau (personnage a l'entree, mecanismes et budgets
    // remis) est pose par le chargement de l'orchestrateur, comme avant l'extraction. Le crochet
    // existe pour les modes qui en auront besoin -- le combat, qui devra rouler l'initiative avant
    // le premier pas (LOT-20).
    (void)passes;
}

core::LevelOutcome ExplorationMode::step(IGameModePasses& passes, const core::PlayerInput& input,
                                         float fixedDelta) {
    // Interpolation (EX-ARCH-031) : fige la position COURANTE de chaque entite mobile comme sa
    // position "precedente" AVANT que ce pas ne la modifie.
    passes.snapshotPreviousPositions();

    // Particules puis secousse d'ecran (LOT-53) : on avance ce qui a ete emis aux pas precedents
    // AVANT que ce pas n'en emette de nouvelles -- age puis emet, jamais l'inverse.
    passes.advanceParticles(fixedDelta);
    passes.advanceScreenShake(fixedDelta);

    // Deplacement du personnage, puis animations (EX-REN-012) et tuiles animees : meme pas fixe
    // que tout ce qui precede, jamais le rythme du rendu (EX-NFR-002).
    passes.moveCharacter(input, fixedDelta);
    passes.advanceAnimations(fixedDelta);

    // Camera : selon le mode de cadrage resolu du niveau (LOT-64). Apres le deplacement, dont elle
    // lit le resultat sans jamais l'ecrire (EX-ARCH-012).
    passes.updateCamera(fixedDelta);

    // Mecanismes, puis les evenements que leurs transitions produisent, puis leur apparence : la
    // detection vient APRES la resolution, sans quoi elle releverait l'etat du pas precedent.
    passes.updateMechanisms(input);
    passes.detectEvents();
    passes.updateMechanismVisuals(fixedDelta);

    // Issue du niveau. Sur echec, les consequences (eclatement, secousse, rechargement) sont une
    // passe a part : l'evaluation ne doit rien changer a ce qu'elle observe.
    const core::LevelOutcome outcome = passes.evaluateOutcome();
    if (outcome == core::LevelOutcome::Lost) {
        passes.onLevelLost();
    }
    return outcome;
}

void ExplorationMode::onUnload(IGameModePasses& passes) {
    // Rien a defaire : le mode est sans etat. Symetrique de onLoad, et pour la meme raison.
    (void)passes;
}

}  // namespace hmi
