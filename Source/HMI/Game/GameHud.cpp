// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Game/GameHud.h"

#include "HMI/Localization/Localization.h"

namespace hmi {

// Lignes a afficher pour l'affichage tete haute d'un pas de simulation donne (voir en-tete).
std::vector<std::string> gameHudLines(const std::string& levelName,
                                      const Localization& localization, bool overlappingKey) {
    std::vector<std::string> lines;

    // Invite « Interagir » (EX-GP-023, EX-CTRL-022, LOT-65 TACHE-07) : affichee EN PREMIER, et
    // seulement au contact d'une cle non ramassee. Ramasser une cle exige le contact ET l'action
    // « Interagir », la seule entree du jeu qu'aucun autre tableau ne demande -- sans invite, un
    // joueur qui l'ignore reste bloque devant la porte verrouillee sans aucun retour, ce que la
    // conception des niveaux interdit (niveaux.md Sec. 3, « aucune situation sans issue »). Le
    // tutoriel restant « sans texte », c'est la seule exception, et elle est contextuelle : rien
    // ne s'affiche tant que le personnage ne touche pas une cle.
    if (overlappingKey) {
        lines.push_back(localization.text("hud.interact_prompt"));
    }

    lines.push_back(levelName);
    return lines;
}

}  // namespace hmi
