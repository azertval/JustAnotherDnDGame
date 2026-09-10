// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Game/CombatMode.h"

namespace hmi {

std::string_view CombatMode::name() const {
    return "combat";
}

std::vector<std::string_view> CombatMode::passOrder() const {
    // Doit rester le miroir exact de step() ci-dessous : un test compare cette liste a la sequence
    // reellement appelee, pour qu'un ordre modifie sans mettre la liste a jour echoue plutot que
    // de mentir aux diagnostics.
    return {"snapshotPreviousPositions", "advanceParticles", "advanceScreenShake",
            "advanceAnimations", "updateCamera"};
}

void CombatMode::onLoad(IGameModePasses& passes) {
    // Rien a preparer : le montage de la rencontre a eu lieu AVANT la bascule de mode
    // (core::beginEncounter), et l'initiative arrive au LOT-20. Le crochet reste, parce que c'est
    // ici que le premier tour se roulera.
    (void)passes;
}

core::LevelOutcome CombatMode::step(IGameModePasses& passes, const core::PlayerInput& input,
                                    float fixedDelta) {
    // L'intention du joueur n'est pas consommee par un deplacement libre : elle alimentera le
    // budget de deplacement du tour (LOT-19). La recevoir sans l'employer est deliberé -- la
    // signature est celle du mode, et l'ignorer ici est ce qui GELE le monde.
    (void)input;

    // Interpolation (EX-ARCH-031) : meme raison qu'en exploration, et meme place -- avant tout ce
    // qui pourrait bouger.
    passes.snapshotPreviousPositions();

    // Le combat reste une SCENE : ce qui a ete emis avant la bascule finit de vivre, et un
    // combattant immobile respire.
    passes.advanceParticles(fixedDelta);
    passes.advanceScreenShake(fixedDelta);
    passes.advanceAnimations(fixedDelta);
    passes.updateCamera(fixedDelta);

    // Pas d'evaluation d'issue de NIVEAU : tomber a zero point de vie est une issue du COMBAT
    // (core::CombatOutcome), et l'evaluer ici rechargerait le niveau au lieu d'ouvrir l'agonie.
    return core::LevelOutcome::Playing;
}

void CombatMode::onUnload(IGameModePasses& passes) {
    // La restitution de l'exploration n'a pas lieu ici : elle est rendue par `core::endEncounter`,
    // qui est pur et se verifie sans fenetre. Ce crochet ne sert qu'a ce qui tient a l'affichage.
    (void)passes;
}

}  // namespace hmi
