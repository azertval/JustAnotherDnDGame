// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <array>
#include <cstddef>

/**
 * @file HMI/Interface/AbilityWheelGeometry.h
 * @brief La **roue des caractéristiques** de la fiche : où se posent le portrait et les six
 *        médaillons.
 *
 * Logique **pure** (aucune dépendance Qt), testable hors instance d'application (`EX-NFR-010`) —
 * même patron que `hmi::ParchmentOrnaments` et `hmi::iconGeometry` : cette table décide *où*, le
 * peintre Qt décide *comment*.
 *
 * ## Ce que la planche du corpus dispose, et que la grille avait perdu
 *
 * La feuille vierge de Tanares ne range pas les six caractéristiques en tableau : elle les pose en
 * **médaillons ronds sur un arc**, autour d'un grand cercle vide cerclé d'entrelacs. Ce cercle est
 * l'emplacement du portrait, et c'est cette composition — pas les valeurs qu'elle porte — qui fait
 * reconnaître la feuille au premier coup d'œil.
 *
 * L'arc est **ouvert en bas**, et l'ouverture n'est pas un reste : c'est là que le nom du
 * personnage se pose. Fermer l'arc en répartissant les six sur un cercle complet rendrait la roue
 * plus régulière et lui retirerait sa raison d'être — il n'y aurait plus où écrire qui l'on est.
 *
 * ## Pourquoi des rapports, et non des pixels
 *
 * Toutes les longueurs ci-dessous se déduisent du côté de la roue. Écrire « 195 » quelque part
 * obligerait à réécrire six autres nombres le jour où la fenêtre change de taille, et la première
 * fois qu'on en oublierait un, les médaillons cesseraient de toucher leur arc — sans qu'aucune
 * erreur ne soit levée, parce qu'un arc et un médaillon ne se parlent pas.
 */

namespace hmi {

/**
 * @brief Les six sièges de la roue, **dans l'ordre de la planche** : on part du bas à gauche, on
 *        monte par la gauche, on redescend par la droite.
 *
 * Cet ordre est relevé, pas choisi. Il est aussi celui de `core::Ability` (`LOT-12`), ce qui n'est
 * pas une coïncidence : les deux viennent de la même règle.
 */
enum class WheelSeat {
    Strength,      ///< Bas à gauche.
    Dexterity,     ///< Milieu gauche.
    Constitution,  ///< Haut à gauche.
    Intelligence,  ///< Haut à droite.
    Wisdom,        ///< Milieu droit.
    Charisma,      ///< Bas à droite.
};

/// Nombre de sièges. Six, et la règle n'en connaît pas d'autre.
inline constexpr std::size_t ABILITY_SEAT_COUNT = 6;

/**
 * @brief Côté minimal, en pixels, sous lequel la roue n'est plus lisible.
 *
 * Ce plancher n'est pas esthétique. Sous cette taille, le médaillon tombe à moins de vingt pixels
 * et ne tient plus deux chiffres et le cabochon de son modificateur ; la roue afficherait alors des
 * valeurs illisibles à la place d'un tableau qui, lui, resterait lisible. Même raison que
 * `hmi::ORNAMENT_MINIMUM_SIZE` : une marque qu'on ne reconnaît pas ne désigne rien.
 */
inline constexpr int ABILITY_WHEEL_MINIMUM_SIZE = 200;

/// Un disque, en pixels relatifs au coin haut-gauche de la roue. En flottants : un siège tombe
/// rarement sur un pixel entier, et l'arrondir décale le médaillon hors de son arc.
struct WheelDisc {
    float centerX = 0.0F;
    float centerY = 0.0F;
    float radius = 0.0F;

    [[nodiscard]] friend bool operator==(const WheelDisc&, const WheelDisc&) noexcept = default;
};

/**
 * @brief Tout ce qu'il faut pour peindre la roue, et rien de plus.
 *
 * Les angles sont en **degrés depuis le haut, sens horaire** — la convention de lecture de la
 * planche, et non celle de Qt (qui compte depuis trois heures, en sens inverse). La conversion est
 * le travail du peintre : la faire ici obligerait à relire la table à l'envers pour vérifier
 * qu'un siège est bien là où la planche le met.
 */
struct AbilityWheelLayout {
    /// Le portrait, au centre.
    WheelDisc portrait{};
    /// Les six médaillons, indexés par `WheelSeat`.
    std::array<WheelDisc, ABILITY_SEAT_COUNT> seats{};
    /// Les fleurons posés **entre** deux médaillons voisins, sur le même arc : cinq intervalles
    /// pour six sièges. Ils sont ici et non dans le peintre parce qu'un ornement placé à vue
    /// dérive du jour où l'arc bouge, et que rien ne le signale — les deux ne se parlent pas.
    std::array<WheelDisc, ABILITY_SEAT_COUNT - 1> fleurons{};
    /// Rayon de l'anneau d'entrelacs, entre le portrait et les médaillons.
    float ringRadius = 0.0F;
    /// Rayon de l'arc qui relie les six sièges : celui sur lequel leurs centres sont posés.
    float arcRadius = 0.0F;
    /// Début de l'arc, en degrés depuis le haut, sens horaire. Négatif : à gauche du sommet.
    float arcStartDegrees = 0.0F;
    /// Ouverture balayée par l'arc. Ce qui reste des 360 est l'ouverture du bas, celle du nom.
    float arcSweepDegrees = 0.0F;
    /// Faux si le côté demandé est sous `ABILITY_WHEEL_MINIMUM_SIZE` : rien n'est alors à peindre,
    /// et l'appelant doit se rabattre sur une disposition en lignes.
    bool valid = false;
};

/**
 * @brief Compose la roue pour un carré de côté @p side.
 *
 * @return La disposition ; `valid == false` et tout à zéro sous `ABILITY_WHEEL_MINIMUM_SIZE`.
 */
[[nodiscard]] AbilityWheelLayout abilityWheelLayout(int side) noexcept;

/// @return Le siège de la roue pour l'indice @p index (0 à 5), dans l'ordre de la planche.
[[nodiscard]] WheelSeat wheelSeatAt(std::size_t index) noexcept;

}  // namespace hmi
