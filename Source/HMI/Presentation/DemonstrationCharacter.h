// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <map>
#include <string>
#include <utility>
#include <vector>

/**
 * @file HMI/Presentation/DemonstrationCharacter.h
 * @brief Le personnage de démonstration, chargé **une fois** pour tous les écrans (`LOT-86`).
 */

namespace hmi {

/// Ce qu'un chargement du personnage de démonstration produit, prêt à être publié au QML.
struct DemonstrationCharacter {
    /// Valeurs de la fiche, indexées (`sheet.hit_points`, `sheet.skill.athletics`…).
    std::map<std::string, std::string> sheet;
    /// Valeurs de l'inventaire, indexées (`inventory.slot.head`, `inventory.purse`…).
    std::map<std::string, std::string> inventory;
    /// Compétences du catalogue de règles : identifiant et nom lisible, dans l'ordre du catalogue.
    std::vector<std::pair<std::string, std::string>> skills;
};

/**
 * @brief Charge le personnage de démonstration livré en donnée, et calcule ses deux tables.
 *
 * **ÉCHAFAUDAGE, et il est écrit comme tel.** Il n'existe encore ni groupe ni sauvegarde d'où
 * tirer un personnage réel, et des écrans qui n'affichent aucun personnage ne se relisent pas. Le
 * jour où une partie en fournira un, c'est **cette fonction** qui change — pas les écrans, qui ne
 * consomment que des valeurs nommées, d'où qu'elles viennent.
 *
 * **Pourquoi une fonction partagée.** La fiche et l'inventaire décrivent le **même** personnage :
 * charger deux fois les huit catalogues aurait doublé le travail et, surtout, permis aux deux
 * écrans de diverger — la classe d'armure de la fiche vient de ce que l'inventaire contient, et
 * deux chargements séparés auraient pu ne pas voir le même équipement.
 *
 * Une donnée manquante n'interrompt rien : l'erreur est journalisée en nommant son fichier
 * (`EX-CNT-010`) et les tables sont partielles. Un écran partiel garde ses tirets, ce qui est la
 * vérité — et vaut mieux qu'un écran vide.
 */
/// @note Nommee `loadDemonstrationValues` et non `…Character` : les vues-modeles exposent au
/// QML une methode `loadDemonstrationCharacter`, et deux noms identiques auraient fait que la
/// methode s'appelle elle-meme -- la recherche de nom trouve le membre avant la fonction libre.
[[nodiscard]] DemonstrationCharacter loadDemonstrationValues();

}  // namespace hmi
