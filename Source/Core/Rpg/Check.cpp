// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Core/Rpg/Check.h"

#include <algorithm>

namespace core {

CheckResult rollCheck(int target, std::span<const Modifier> modifiers, RollStance stance,
                      DeterministicRandom& random) {
    CheckResult resultat;
    resultat.target = target;
    resultat.stance = stance;

    // Les DEUX des sont conserves en cas d'avantage ou de desavantage, pas seulement celui qui est
    // retenu : c'est ce que le joueur veut voir pour comprendre ce que son avantage lui a rapporte
    // (EX-DND-003).
    resultat.dice.push_back(random.nextInt(1, D20_FACES));
    if (stance != RollStance::Normal) {
        resultat.dice.push_back(random.nextInt(1, D20_FACES));
    }

    switch (stance) {
        case RollStance::Advantage:
            resultat.keptDie = *std::ranges::max_element(resultat.dice);
            break;
        case RollStance::Disadvantage:
            resultat.keptDie = *std::ranges::min_element(resultat.dice);
            break;
        case RollStance::Normal:
            resultat.keptDie = resultat.dice.front();
            break;
    }

    resultat.modifiers.assign(modifiers.begin(), modifiers.end());
    resultat.total = resultat.keptDie;
    for (const Modifier& modificateur : resultat.modifiers) {
        resultat.total += modificateur.value;
    }
    return resultat;
}

std::string_view rollStanceName(RollStance stance) noexcept {
    // switch exhaustif sans default.
    switch (stance) {
        case RollStance::Normal:
            return "normal";
        case RollStance::Advantage:
            return "avantage";
        case RollStance::Disadvantage:
            return "desavantage";
    }
    return {};
}

std::string CheckResult::describe() const {
    std::string texte = "d20";
    if (stance != RollStance::Normal) {
        texte += " (";
        texte += rollStanceName(stance);
        texte += " : ";
        bool premier = true;
        for (const int de : dice) {
            if (!premier) {
                texte += ", ";
            }
            texte += std::to_string(de);
            premier = false;
        }
        texte += ')';
    }
    texte += " = " + std::to_string(keptDie);
    for (const Modifier& modificateur : modifiers) {
        texte += modificateur.value >= 0 ? " + " : " - ";
        texte += std::to_string(modificateur.value >= 0 ? modificateur.value : -modificateur.value);
        texte += " (" + modificateur.source + ')';
    }
    texte += " = " + std::to_string(total);
    texte += total >= target ? " >= " : " < ";
    texte += std::to_string(target);
    texte += total >= target ? " : reussite" : " : echec";
    if (isNaturalTwenty()) {
        texte += " (20 naturel)";
    } else if (isNaturalOne()) {
        texte += " (1 naturel)";
    }
    return texte;
}

}  // namespace core
