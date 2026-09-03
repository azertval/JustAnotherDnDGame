// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>
#include <vector>

/**
 * @file HMI/Game/GameHud.h
 * @brief Choix du contenu de l'affichage tête haute : invite contextuelle, nom du tableau
 *        (`LOT-52` TACHE-03, `EX-IHM-003`).
 */

namespace hmi {

class Localization;

/**
 * @brief Lignes à afficher pour l'affichage tête haute d'un pas de simulation donné.
 *
 * Fonction **pure** (`EX-NFR-010`) : ne lit que ce qu'on lui passe, aucun accès GPU/fichier. C'est
 * ce qui rend le contenu du HUD assertable sans rendu — la brique de texte (`hmi::composeText`,
 * TACHE-02) n'a plus qu'à afficher ce que cette fonction décide.
 *
 * **Les budgets de sauts et de dashs ont disparu avec eux** (`LOT-06`) : ils comptaient les
 * ressources d'un personnage de plateforme, que le déplacement en vue de dessus n'a plus. Ne
 * reste que ce qui a encore un sens — l'invite contextuelle et le nom du tableau.
 * @param levelName    Nom du tableau en cours (`core::Level::name`), affiché tel quel (donnée, pas
 *                     un libellé traduit).
 * @param localization Catalogue de traduction des libellés (`EX-REN-033`) — aucune chaîne en dur,
 *                     y compris les libellés courts.
 * @param overlappingKey Le personnage touche-t-il une **clé non ramassée** (`EX-GP-023`) ? Si oui,
 *                     une invite rappelle l'action « Interagir » (`EX-CTRL-022`). Ramasser une clé
 *                     exige le contact **et** cette action — la seule entrée du jeu qu'aucun autre
 *                     tableau ne demande. Sans invite, un joueur qui l'ignore reste bloqué devant
 *                     la porte verrouillée sans aucun retour, ce qu'interdit la conception des
 *                     niveaux (`niveaux.md`, Sec. 3). Le tutoriel restant « sans texte », c'est la
 *                     seule exception, et elle est **contextuelle**. Faux par défaut : un tableau
 *                     sans clé n'affiche jamais rien.
 * @return Les lignes à afficher, dans l'ordre : invite « Interagir » le cas échéant, puis le nom
 *         du tableau.
 */
[[nodiscard]] std::vector<std::string> gameHudLines(const std::string& levelName,
                                                    const Localization& localization,
                                                    bool overlappingKey = false);

}  // namespace hmi
