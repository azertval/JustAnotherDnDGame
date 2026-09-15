// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <array>
#include <cstdint>
#include <string_view>

#include "Core/Levels/TileMap.h"

/**
 * @file HMI/Graphics/TileAutotile.h
 * @brief Raccords automatiques : voisinage solide → case d'une planche 4×4 (`EX-EDIT-025`).
 */

namespace hmi {

/**
 * @brief Bits du masque de voisinage, un par voisin **orthogonal** solide.
 *
 * Quatre voisins et non huit : seize configurations couvrent bords et coins concaves, et
 * demandent à l'auteur une planche de seize cases plutôt que quarante-sept — proportionné à un
 * jeu dont les assets sont dessinés à la main. Le champ `mode` de `hmi::SkinEntry` permettra
 * d'ajouter un `bitmask47` sans refonte si ce rendu s'avère insuffisant.
 */
enum NeighborBits : std::uint8_t {
    NEIGHBOR_UP = 1u << 0,     ///< Voisin du dessus solide.
    NEIGHBOR_RIGHT = 1u << 1,  ///< Voisin de droite solide.
    NEIGHBOR_DOWN = 1u << 2,   ///< Voisin du dessous solide.
    NEIGHBOR_LEFT = 1u << 3,   ///< Voisin de gauche solide.
};

/// Nombre de cases par côté de la planche de raccords (4×4 = seize configurations).
inline constexpr int AUTOTILE_SHEET_SIDE = 4;

/// Nombre de configurations de voisinage distinctes.
inline constexpr int AUTOTILE_CONFIGURATION_COUNT = AUTOTILE_SHEET_SIDE * AUTOTILE_SHEET_SIDE;

/// Position d'une case dans la planche de raccords, en cases (origine haut-gauche).
struct AutotileCell {
    int column = 0;
    int row = 0;

    [[nodiscard]] friend bool operator==(const AutotileCell& lhs,
                                         const AutotileCell& rhs) noexcept {
        return lhs.column == rhs.column && lhs.row == rhs.row;
    }
};

/**
 * @brief Calcule le masque de voisinage solide d'une case de la grille.
 *
 * **L'extérieur de la grille compte comme solide.** Convention explicite et non implicite : un mur
 * de bordure afficherait sinon un contour sur sa face invisible, hors du niveau, ce qui donne
 * l'impression que le décor s'arrête en plein vide. Le revers — un niveau ouvert sur l'extérieur
 * ne montre pas de bord franc à sa limite — est sans conséquence, la caméra ne dépassant pas les
 * bornes du niveau.
 *
 * **Le voisinage est celui de la solidité, pas du type** : `Solid` et les trois tailles de `Block`
 * se raccordent entre eux (`core::isSolid`). Se raccorder au seul type identique laisserait une
 * couture visible partout où un bloc jouxte un mur, alors que les deux forment visuellement la
 * même matière. Les pentes et arrondis ne sont jamais solides : ils ne participent donc pas au
 * voisinage et restent en mode `single`, avec leur masque de silhouette.
 *
 * La grille passée doit être celle **du niveau**, jamais la grille de collision d'un
 * `core::MechanismController` : une porte qui s'ouvre ne doit pas changer l'habillage des murs
 * qui l'entourent.
 *
 * Fonction **pure**, testable sans GPU (`EX-NFR-010`).
 * @param tiles  Grille du niveau.
 * @param column Colonne de la case examinée.
 * @param row    Ligne de la case examinée.
 * @return Le masque des quatre voisins solides (combinaison de `hmi::NeighborBits`, 0 à 15).
 */
[[nodiscard]] std::uint8_t solidNeighborMask(const core::TileMap& tiles, int column,
                                             int row) noexcept;

/**
 * @brief Case de la planche 4×4 correspondant à un masque de voisinage.
 *
 * La correspondance est une constante figée, documentée configuration par configuration dans
 * l'implémentation, et vérifiée exhaustivement sur les seize valeurs.
 * @param mask Masque de voisinage (seuls les quatre bits de poids faible sont lus).
 * @return La case à échantillonner dans la planche.
 */
[[nodiscard]] AutotileCell autotileCell(std::uint8_t mask) noexcept;

/**
 * @brief Case représentative d'une planche : l'**intérieur plein** (quatre voisins solides).
 *
 * Utilisée là où une seule image doit résumer un skin à raccords — la vignette de la palette de
 * l'éditeur (`EX-EDIT-027`). L'intérieur plein est le motif de fond du skin ; la case zéro, elle,
 * serait la tuile isolée, un cas particulier peu représentatif.
 * @return La case d'intérieur plein.
 */
[[nodiscard]] AutotileCell autotileRepresentativeCell() noexcept;

/**
 * @brief Clé de traduction décrivant la configuration de voisinage d'un masque, pour l'atelier
 *        pixel art (`LOT-54` TACHE-08, `EX-REN-033`).
 *
 * Les seize valeurs correspondent **exactement** aux seize entrées de la table canonique
 * `CELL_BY_MASK` (`TileAutotile.cpp`) — dessiner une planche sans savoir ce que représente chaque
 * case est une source d'erreurs garantie ; ce n'est jamais une seconde table, seulement sa
 * description en langage naturel.
 * @param mask Masque de voisinage (seuls les quatre bits de poids faible sont lus).
 * @return La clé (existe dans les deux catalogues, `fr.lang`/`en.lang`).
 */
[[nodiscard]] std::string_view autotileConfigurationLabelKey(std::uint8_t mask) noexcept;

/**
 * @brief Masques des neuf cases d'un assemblage 3×3 démonstratif (`LOT-54` TACHE-08).
 *
 * Un bloc 3×3 **plein**, flottant (aucun voisin hors du bloc n'est considéré solide) : chaque case
 * ne voit que ses voisines internes, ce qui donne un panorama représentatif — coins, bords et
 * intérieur — de la façon dont la planche en cours se raccorde à elle-même. C'est l'assemblage,
 * pas les repères, qui répond à « le résultat tient-il ? ».
 * @return Les neuf masques, en ordre de lecture (ligne par ligne, gauche à droite, haut en bas).
 */
[[nodiscard]] std::array<std::uint8_t, 9> autotileAssemblyMasks() noexcept;

}  // namespace hmi
