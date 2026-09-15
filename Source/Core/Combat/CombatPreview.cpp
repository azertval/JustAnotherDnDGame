// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "Core/Combat/CombatPreview.h"

#include "Core/Combat/EnemyAi.h"
#include "Core/Combat/LineOfSight.h"

namespace core {

std::optional<AttackPreview> previewAttack(const ArenaSession& session, CombatantId target,
                                           std::size_t attackIndex) {
    const CombatState& combat = session.combat();
    const std::optional<CombatantId> actif = combat.activeCombatant();
    const std::vector<AttackProfile>* attaques =
        actif.has_value() ? session.attacks(*actif) : nullptr;
    const Combatant* cible = combat.find(target);
    if (!actif.has_value() || attaques == nullptr || attackIndex >= attaques->size() ||
        cible == nullptr) {
        return std::nullopt;
    }
    const AttackProfile& profil = (*attaques)[attackIndex];
    AttackPreview apercu{.check = checkTarget(combat, *actif, target, profil),
                         .attackIndex = attackIndex,
                         .label = profil.label};
    // Les memes sources que resolveAttack, dans le meme ordre : la grille, puis la session.
    const AttackCircumstances grille = attackCircumstances(combat, *actif, target, profil);
    const AttackCircumstances session_ = session.circumstancesAgainst(*actif, target, profil);
    for (const AttackCircumstances* source : {&grille, &session_}) {
        apercu.advantages.insert(apercu.advantages.end(), source->advantages.begin(),
                                 source->advantages.end());
        apercu.disadvantages.insert(apercu.disadvantages.end(), source->disadvantages.begin(),
                                    source->disadvantages.end());
    }
    apercu.stance = rollStance(static_cast<int>(apercu.advantages.size()),
                               static_cast<int>(apercu.disadvantages.size()));
    apercu.cover = coverBetween(combat, *actif, target);
    apercu.armorClass = cible->profile.armorClass + coverBonus(apercu.cover);
    apercu.requiredRoll = requiredRoll(apercu.armorClass, attackBonusOf(profil));
    if (apercu.check == TargetCheck::Valid) {
        apercu.hitChance = hitChance(apercu.requiredRoll, apercu.stance, profil.criticalThreshold);
        apercu.expectedDamage = expectedDamage(profil, apercu.armorClass, apercu.stance);
    }
    return apercu;
}

std::optional<std::size_t> firstValidAttack(const ArenaSession& session, CombatantId target) {
    const std::optional<CombatantId> actif = session.combat().activeCombatant();
    const std::vector<AttackProfile>* attaques =
        actif.has_value() ? session.attacks(*actif) : nullptr;
    if (attaques == nullptr) {
        return std::nullopt;
    }
    for (std::size_t i = 0; i < attaques->size(); ++i) {
        if (checkTarget(session.combat(), *actif, target, (*attaques)[i]) == TargetCheck::Valid) {
            return i;
        }
    }
    return std::nullopt;
}

MovePreview previewMove(const ArenaSession& session, GridPosition destination) {
    MovePreview apercu;
    const std::optional<ReachableArea> zone = session.combat().reachableArea();
    if (!zone.has_value()) {
        return apercu;
    }
    apercu.path = zone->pathTo(destination);
    apercu.movementLeft = zone->budget() - (apercu.path.has_value() ? apercu.path->cost : 0);
    if (apercu.path.has_value()) {
        apercu.opportunities = session.previewOpportunities(destination);
    }
    return apercu;
}

}  // namespace core
