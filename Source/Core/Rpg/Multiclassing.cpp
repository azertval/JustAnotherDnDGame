// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Core/Rpg/Multiclassing.h"

#include <algorithm>

namespace core {

namespace {

// Niveau de personnage maximal : la table des emplacements s'arrete la, et un niveau de lanceur
// au-dela n'aurait aucune ligne a designer.
constexpr int NIVEAU_MAXIMAL = 20;

// La contribution d'UNE classe, arrondie a l'inferieur. L'arrondi se fait ici, par classe, et
// jamais sur le total : un paladin 3/rodeur 3 donne 1 + 1 = 2, pas 6 / 2 = 3.
[[nodiscard]] int contribution(const ClassLevel& classe) {
    if (classe.level <= 0) {
        return 0;
    }
    switch (classe.progression) {
        case CasterProgression::Full:
            return classe.level;
        case CasterProgression::Half:
            return classe.level / 2;
        case CasterProgression::Third:
            return classe.level / 3;
        case CasterProgression::Pact:
            // La magie de pacte n'entre pas dans cette somme (EX-RPG-052) : ses emplacements sont
            // peu nombreux, toujours au niveau maximal, et recuperes au repos COURT.
            return 0;
        case CasterProgression::None:
            return 0;
    }
    return 0;  // inatteignable : le switch ci-dessus est exhaustif.
}

}  // namespace

int multiclassCasterLevel(std::span<const ClassLevel> classes) {
    int total = 0;
    for (const ClassLevel& classe : classes) {
        total += contribution(classe);
    }
    return std::min(total, NIVEAU_MAXIMAL);
}

std::string_view casterProgressionName(CasterProgression progression) {
    // switch exhaustif sans default : une progression ajoutee sans nom casse la compilation.
    switch (progression) {
        case CasterProgression::None:
            return "none";
        case CasterProgression::Third:
            return "third";
        case CasterProgression::Half:
            return "half";
        case CasterProgression::Full:
            return "full";
        case CasterProgression::Pact:
            return "pact";
    }
    return {};
}

std::optional<CasterProgression> parseCasterProgression(std::string_view name) {
    for (const CasterProgression progression :
         {CasterProgression::None, CasterProgression::Third, CasterProgression::Half,
          CasterProgression::Full, CasterProgression::Pact}) {
        if (casterProgressionName(progression) == name) {
            return progression;
        }
    }
    return std::nullopt;
}

}  // namespace core
