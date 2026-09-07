// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>
#include <vector>

/**
 * @file HMI/Interface/CharacterSheetPlate.h
 * @brief La table de la **planche gravée** de la fiche de personnage (`LOT-38`, `EX-IHM-090`) :
 *        où chaque intitulé et chaque valeur se posent sur le trait du livre.
 */

namespace hmi {

/// Un rectangle en **unités de planche** — celles de la maquette, 816 × 1056, jamais des pixels.
/// La planche est dessinée à la taille qu'on lui donne ; ces coordonnées ne changent pas avec elle.
struct PlateRect {
    double x = 0.0;
    double y = 0.0;
    double width = 0.0;
    double height = 0.0;

    [[nodiscard]] bool isEmpty() const noexcept {
        return width <= 0.0 || height <= 0.0;
    }

    [[nodiscard]] friend bool operator==(const PlateRect&, const PlateRect&) noexcept = default;
};

/// Rôle d'un champ sur la planche. C'est le **rôle gravé**, pas un style : la feuille de style le
/// traduit en corps et en couleur, et deux champs de même rôle se lisent de la même façon.
enum class PlateRole {
    Name,      ///< Le nom du personnage, dans son grand cartouche.
    Band,      ///< Bandeau d'identité : classe, historique, espèce, alignement...
    Dial,      ///< Cadran : inspiration, bonus de maîtrise, initiative, classe d'armure.
    Plaque,    ///< Plaque gravée : vitesse, perception passive, points de vie.
    Ability,   ///< Modificateur de caractéristique, au centre d'un grand anneau.
    Score,     ///< Valeur de caractéristique, dans le petit disque accolé.
    Skill,     ///< Une des dix-huit lignes du livre des compétences.
    Save,      ///< Un des six onglets de jets de sauvegarde.
    Banner,    ///< Titre de section gravé sur un bandeau.
    Prose,     ///< Zone d'écriture libre : traits, idéaux, liens, défauts.
    Table,     ///< Cellule du tableau d'attaques.
    Portrait,  ///< L'anneau central de la roue, où le portrait se pose.
};

/// Alignement horizontal du texte dans son rectangle.
enum class PlateAlign { Left, Center };

/**
 * @brief Un champ de la planche : ce qu'on efface de la gravure, et ce qu'on repose à la place.
 *
 * Les rectangles sont **les mêmes des deux côtés**. `scripts/build_character_sheet_plate.py` lit
 * `erase` pour retirer le lettrage anglais du tracé ; cet écran lit `label` et `value` pour poser
 * l'intitulé traduit et la valeur au même endroit. Une seule table, donc : il est impossible
 * d'effacer un intitulé sans le reposer, ni de reposer un intitulé sur une gravure restée en place.
 */
struct PlateField {
    std::string id;        ///< Identifiant du champ, unique dans la table.
    std::string labelKey;  ///< Clé de traduction de l'intitulé. Vide : ce champ n'en porte pas.
    /// Identifiant sous lequel la valeur arrive (`hmi::characterSheetValues`). **Vide** veut dire :
    /// ce champ existe sur la feuille, il est à l'écran, et rien ne l'alimente encore — il garde
    /// son tiret cadratin. La colonne des identifiants vides EST le périmètre restant.
    std::string valueId;
    PlateRole role = PlateRole::Band;
    PlateAlign align = PlateAlign::Center;
    PlateRect label;  ///< Où l'intitulé traduit se pose. Vide : pas d'intitulé à l'écran.
    PlateRect value;  ///< Où la valeur se pose. Vide : ce champ n'affiche pas de valeur.
    /// Rectangles à **repeindre en parchemin** avant d'écrire. La découpe vectorielle retire le
    /// lettrage de la quasi-totalité de la planche ; sur une poignée de cartouches, les lettres
    /// sont *évidées dans* leur propre fond, et les retirer boucherait le fond au lieu de les
    /// effacer. Là, le fond plat est simplement repeint — invisible, puisqu'il est plat.
    std::vector<PlateRect> cover;
};

/// La planche entière : la taille de la maquette, et ses champs.
struct CharacterSheetPlateLayout {
    double pageWidth = 816.0;
    double pageHeight = 1056.0;
    std::vector<PlateField> fields;

    [[nodiscard]] bool isValid() const noexcept {
        return pageWidth > 0.0 && pageHeight > 0.0 && !fields.empty();
    }
};

/**
 * @brief Lit la table de la planche (`Assets/UI/character-sheet-plate.json`).
 *
 * Logique **pure** : une chaîne entre, une table sort, aucun accès disque ni widget
 * (`EX-NFR-010`). C'est ce qui permet de vérifier par test que la table couvre bien les dix-huit
 * compétences et les six caractéristiques, et qu'aucun champ ne nomme une valeur que
 * `hmi::characterSheetValues` ne produit pas — sans ouvrir de fenêtre.
 *
 * @param json Contenu du fichier.
 * @return La table. Un fichier illisible rend une table **invalide** (`isValid()` faux) plutôt
 *         qu'une exception : l'écran affiche alors la planche nue, ce qui se voit, plutôt que de
 *         faire tomber l'application (`EX-NFR-040`).
 */
[[nodiscard]] CharacterSheetPlateLayout parseCharacterSheetPlate(const std::string& json);

}  // namespace hmi
