// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#include "HMI/Graphics/TileAutotile.h"

#include <array>

namespace hmi {

namespace {

// Correspondance masque -> case de la planche, une entree par configuration (index = masque).
//
// Convention de lecture : la case dessine la silhouette de la tuile, donc les cotes ou la MATIERE
// s'arrete. Un bit a 1 = voisin solide = aucun bord a dessiner de ce cote.
//
// L'index de la planche est simplement le masque lu en base 4 (colonne = masque % 4, ligne =
// masque / 4). La table est neanmoins ecrite en clair plutot que calculee : elle documente ce que
// l'auteur d'une planche doit dessiner dans chaque case, ce qu'une formule ne dirait pas. C'est
// aussi ce qui la rend verifiable configuration par configuration.
constexpr std::array<AutotileCell, AUTOTILE_CONFIGURATION_COUNT> CELL_BY_MASK{{
    // 0  ---- : aucun voisin solide -> tuile isolee, bordee sur ses quatre cotes.
    AutotileCell{.column = 0, .row = 0},
    // 1  U--- : voisin en haut -> extremite basse d'une colonne (ouverte vers le haut).
    AutotileCell{.column = 1, .row = 0},
    // 2  -R-- : voisin a droite -> extremite gauche d'une rangee.
    AutotileCell{.column = 2, .row = 0},
    // 3  UR-- : haut + droite -> coin exterieur bas-gauche.
    AutotileCell{.column = 3, .row = 0},
    // 4  --D- : voisin en bas -> extremite haute d'une colonne.
    AutotileCell{.column = 0, .row = 1},
    // 5  U-D- : haut + bas -> segment vertical de colonne (bords a gauche et a droite).
    AutotileCell{.column = 1, .row = 1},
    // 6  -RD- : droite + bas -> coin exterieur haut-gauche.
    AutotileCell{.column = 2, .row = 1},
    // 7  URD- : haut + droite + bas -> bord gauche d'une masse.
    AutotileCell{.column = 3, .row = 1},
    // 8  ---L : voisin a gauche -> extremite droite d'une rangee.
    AutotileCell{.column = 0, .row = 2},
    // 9  U--L : haut + gauche -> coin exterieur bas-droite.
    AutotileCell{.column = 1, .row = 2},
    // 10 -R-L : gauche + droite -> segment horizontal (bords en haut et en bas).
    AutotileCell{.column = 2, .row = 2},
    // 11 UR-L : haut + droite + gauche -> bord bas d'une masse.
    AutotileCell{.column = 3, .row = 2},
    // 12 --DL : bas + gauche -> coin exterieur haut-droite.
    AutotileCell{.column = 0, .row = 3},
    // 13 U-DL : haut + bas + gauche -> bord droit d'une masse.
    AutotileCell{.column = 1, .row = 3},
    // 14 -RDL : droite + bas + gauche -> bord haut d'une masse (le dessus d'une plateforme).
    AutotileCell{.column = 2, .row = 3},
    // 15 URDL : les quatre voisins solides -> interieur plein, invisible depuis l'exterieur.
    AutotileCell{.column = 3, .row = 3},
}};

// Interieur plein : masque dont les quatre bits sont a 1.
constexpr std::uint8_t FULL_MASK = NEIGHBOR_UP | NEIGHBOR_RIGHT | NEIGHBOR_DOWN | NEIGHBOR_LEFT;

// Vrai si la case designee est solide, l'exterieur de la grille comptant comme solide (voir la
// convention documentee dans l'en-tete).
[[nodiscard]] bool isSolidOrOutside(const core::TileMap& tiles, int column, int row) noexcept {
    if (!tiles.inBounds(column, row)) {
        return true;
    }
    return tiles.isSolid(column, row);
}

}  // namespace

std::uint8_t solidNeighborMask(const core::TileMap& tiles, int column, int row) noexcept {
    std::uint8_t mask = 0;
    if (isSolidOrOutside(tiles, column, row - 1)) {
        mask |= NEIGHBOR_UP;
    }
    if (isSolidOrOutside(tiles, column + 1, row)) {
        mask |= NEIGHBOR_RIGHT;
    }
    if (isSolidOrOutside(tiles, column, row + 1)) {
        mask |= NEIGHBOR_DOWN;
    }
    if (isSolidOrOutside(tiles, column - 1, row)) {
        mask |= NEIGHBOR_LEFT;
    }
    return mask;
}

AutotileCell autotileCell(std::uint8_t mask) noexcept {
    // Seuls les quatre bits de poids faible portent du sens : masquer plutot que supposer
    // l'appelant discipline evite un depassement de table sur une valeur inattendue.
    return CELL_BY_MASK[mask & FULL_MASK];
}

AutotileCell autotileRepresentativeCell() noexcept {
    return CELL_BY_MASK[FULL_MASK];
}

std::string_view autotileConfigurationLabelKey(std::uint8_t mask) noexcept {
    // Une cle par masque (0-15), meme ordre et meme sens que CELL_BY_MASK ci-dessus : lire les
    // deux tables cote a cote est ce qui garantit qu'elles ne divergent jamais silencieusement.
    static constexpr std::array<std::string_view, AUTOTILE_CONFIGURATION_COUNT> KEYS{{
        "autotile.config.0",
        "autotile.config.1",
        "autotile.config.2",
        "autotile.config.3",
        "autotile.config.4",
        "autotile.config.5",
        "autotile.config.6",
        "autotile.config.7",
        "autotile.config.8",
        "autotile.config.9",
        "autotile.config.10",
        "autotile.config.11",
        "autotile.config.12",
        "autotile.config.13",
        "autotile.config.14",
        "autotile.config.15",
    }};
    return KEYS[mask & FULL_MASK];
}

std::array<std::uint8_t, 9> autotileAssemblyMasks() noexcept {
    std::array<std::uint8_t, 9> masks{};
    for (int row = 0; row < 3; ++row) {
        for (int column = 0; column < 3; ++column) {
            std::uint8_t mask = 0;
            // Bloc 3x3 plein et flottant : un voisin ne compte solide que s'il appartient lui
            // aussi au bloc (jamais l'exterieur, a l'inverse de solidNeighborMask) -- c'est ce qui
            // montre les quatre bords ET l'interieur dans un seul assemblage compact.
            if (row > 0) {
                mask |= NEIGHBOR_UP;
            }
            if (column < 2) {
                mask |= NEIGHBOR_RIGHT;
            }
            if (row < 2) {
                mask |= NEIGHBOR_DOWN;
            }
            if (column > 0) {
                mask |= NEIGHBOR_LEFT;
            }
            masks[(static_cast<std::size_t>(row) * 3) + static_cast<std::size_t>(column)] = mask;
        }
    }
    return masks;
}

}  // namespace hmi
