// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Game/GameHud.h"

#include "HMI/Localization/Localization.h"

namespace hmi {

namespace {

// Remplace la premiere occurrence de "%1" par value (pas de dependance Qt : GameSession/GameHud
// restent utilisables hors interface, meme discipline que le reste de Core/Game).
std::string formatCount(const std::string& templateText, int value) {
    const std::string placeholder = "%1";
    const std::size_t position = templateText.find(placeholder);
    if (position == std::string::npos) {
        return templateText;
    }
    std::string result = templateText;
    result.replace(position, placeholder.size(), std::to_string(value));
    return result;
}

}  // namespace

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
