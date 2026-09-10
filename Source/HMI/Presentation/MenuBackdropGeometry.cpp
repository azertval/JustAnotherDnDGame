// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Presentation/MenuBackdropGeometry.h"

#include <algorithm>
#include <array>
#include <utility>

namespace hmi {

namespace {

/// Etoiles : positions en FRACTION de la surface, fixees une fois pour toutes. Deterministe par
/// construction -- un ciel tire au sort changerait a chaque ouverture du menu.
struct StarPosition {
    float x;
    float y;
    bool bright;
};

constexpr std::array<StarPosition, 9> STARS{{
    {.x = 0.075f, .y = 0.09f, .bright = true},
    {.x = 0.225f, .y = 0.18f, .bright = false},
    {.x = 0.325f, .y = 0.07f, .bright = true},
    {.x = 0.425f, .y = 0.33f, .bright = false},
    {.x = 0.500f, .y = 0.22f, .bright = false},
    {.x = 0.575f, .y = 0.11f, .bright = true},
    {.x = 0.750f, .y = 0.29f, .bright = false},
    {.x = 0.850f, .y = 0.15f, .bright = true},
    {.x = 0.925f, .y = 0.35f, .bright = false},
}};

/// Silhouettes de l'horizon : largeur et hauteur en pas de grille, lues en boucle sur la largeur.
constexpr std::array<std::pair<int, int>, 14> SILHOUETTES{{
    {3, 2},
    {1, 5},
    {2, 3},
    {4, 1},
    {1, 4},
    {1, 3},
    {5, 2},
    {3, 3},
    {1, 5},
    {7, 2},
    {2, 4},
    {4, 1},
    {1, 3},
    {5, 2},
}};

/// Collines de plan intermediaire : largeur et hauteur en pas de grille.
constexpr std::array<std::pair<int, int>, 5> HILLS{{{8, 2}, {6, 3}, {10, 1}, {7, 4}, {9, 2}}};

/// Disque lunaire, en rangees de 16 : depart et largeur de chaque rangee. Un cercle trace en pixels
/// entiers, jamais un rayon arrondi -- un border-radius produirait un bord anticrenele.
constexpr std::array<std::pair<int, int>, 16> MOON_ROWS{{
    {5, 6},
    {3, 10},
    {2, 12},
    {1, 14},
    {1, 14},
    {0, 16},
    {0, 16},
    {0, 16},
    {0, 16},
    {0, 16},
    {0, 16},
    {1, 14},
    {1, 14},
    {2, 12},
    {3, 10},
    {5, 6},
}};

}  // namespace

std::vector<BackdropQuad> menuBackdropQuads(int width, int height, int scale) {
    // Pas de grille : celui du jeu (32 px de tuile) multiplie par le facteur, mais ramene a une
    // valeur qui laisse au moins quelques colonnes sur une fenetre etroite.
    const int step = std::max(1, std::min(32 * std::max(scale, 1), std::max(1, width / 20)));
    if (width < step * 4 || height < step * 4) {
        return {};  // surface trop petite : un decor y serait illisible, mieux vaut aucun.
    }

    std::vector<BackdropQuad> quads;

    // ---- Ciel : trois bandes franches, de la plus sombre en haut a la plus claire a l'horizon.
    const int horizon = (height * 62) / 100;
    const int bandHeight = std::max(step, horizon / 6);
    quads.push_back({BackdropRole::SkyHigh, 0, 0, width, horizon});
    quads.push_back({BackdropRole::SkyMid, 0, horizon - (2 * bandHeight), width, bandHeight});
    quads.push_back({BackdropRole::SkyLow, 0, horizon - bandHeight, width, bandHeight});

    // ---- Etoiles : carres d'un pas de grille reduit, jamais des points anticreneles.
    const int starSize = std::max(1, step / 8);
    for (const StarPosition& star : STARS) {
        const int y = static_cast<int>(static_cast<float>(horizon) * star.y);
        quads.push_back({star.bright ? BackdropRole::Star : BackdropRole::StarDim,
                         static_cast<int>(static_cast<float>(width) * star.x), y, starSize,
                         starSize});
    }

    // ---- Lune, en haut a droite.
    const int moonPixel = std::max(1, step / 4);
    const int moonSize = moonPixel * 16;
    const int moonX = width - moonSize - (2 * step);
    const int moonY = step;
    if (moonX > 0) {
        for (std::size_t row = 0; row < MOON_ROWS.size(); ++row) {
            const auto [startCell, widthCells] = MOON_ROWS[row];
            quads.push_back({BackdropRole::Moon, moonX + (startCell * moonPixel),
                             moonY + (static_cast<int>(row) * moonPixel), widthCells * moonPixel,
                             moonPixel});
        }
        // Trois crateres, poses sur des rangees pleines pour ne jamais deborder du disque.
        quads.push_back({BackdropRole::MoonCrater, moonX + (4 * moonPixel), moonY + (5 * moonPixel),
                         2 * moonPixel, 2 * moonPixel});
        quads.push_back({BackdropRole::MoonCrater, moonX + (9 * moonPixel), moonY + (7 * moonPixel),
                         3 * moonPixel, 2 * moonPixel});
        quads.push_back({BackdropRole::MoonCrater, moonX + (6 * moonPixel),
                         moonY + (10 * moonPixel), 2 * moonPixel, moonPixel});
    }

    // ---- Silhouettes de l'horizon, posees SUR la ligne d'horizon.
    int x = 0;
    std::size_t index = 0;
    while (x < width) {
        const auto [cellsWide, cellsHigh] = SILHOUETTES[index % SILHOUETTES.size()];
        const int blockWidth = cellsWide * step;
        const int blockHeight = cellsHigh * step;
        quads.push_back(
            {BackdropRole::Silhouette, x, horizon - blockHeight, blockWidth, blockHeight});
        x += blockWidth;
        ++index;
    }

    // ---- Collines : plan intermediaire, entre l'horizon et le sol.
    const int groundTop = height - std::max(step * 2, height / 6);
    x = 0;
    index = 0;
    while (x < width) {
        const auto [cellsWide, cellsHigh] = HILLS[index % HILLS.size()];
        const int blockWidth = cellsWide * step;
        const int blockHeight = cellsHigh * step;
        quads.push_back({BackdropRole::Hill, x, groundTop - blockHeight, blockWidth, blockHeight});
        x += blockWidth;
        ++index;
    }

    // ---- Sol de premier plan, et son lisere superieur.
    quads.push_back({BackdropRole::Ground, 0, groundTop, width, height - groundTop});
    quads.push_back({BackdropRole::GroundEdge, 0, groundTop, width, std::max(1, step / 4)});

    // ---- Voile de lisibilite : des BANDES et non un degrade. Chacune couvre une part decroissante
    // de la largeur, ce qui produit un fondu par paliers francs, compatible avec la grille.
    const int veilWidth = static_cast<int>(static_cast<float>(width) * BACKDROP_VEIL_WIDTH_RATIO);
    for (int band = 0; band < BACKDROP_VEIL_STEPS; ++band) {
        const int bandWidth = (veilWidth * (BACKDROP_VEIL_STEPS - band)) / BACKDROP_VEIL_STEPS;
        if (bandWidth <= 0) {
            continue;
        }
        quads.push_back({BackdropRole::Veil, 0, 0, bandWidth, height});
    }

    return quads;
}

}  // namespace hmi
