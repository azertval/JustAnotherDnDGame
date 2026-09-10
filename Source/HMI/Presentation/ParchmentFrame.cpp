// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Presentation/ParchmentFrame.h"

#include <algorithm>

namespace hmi {

namespace {

/// Ajoute les quatre bandes d'un rectangle creux d'epaisseur @p thickness, en retrait de @p inset
/// des bords. Les bandes horizontales couvrent toute la largeur : les angles sont donc peints DEUX
/// fois, ce qui est sans effet (meme couleur) et evite quatre cas particuliers.
void addRing(std::vector<ParchmentStroke>& strokes, ParchmentRole role, int width, int height,
             int inset, int thickness) {
    strokes.push_back({role, inset, inset, width - (2 * inset), thickness});
    strokes.push_back({role, inset, height - inset - thickness, width - (2 * inset), thickness});
    strokes.push_back({role, inset, inset, thickness, height - (2 * inset)});
    strokes.push_back({role, width - inset - thickness, inset, thickness, height - (2 * inset)});
}

/// Cote d'une unite de maquette, en pixels, pour un cadre donne. Jamais assez large pour que les
/// deux bords opposes se chevauchent : au-dela, `width - 2 * inset` deviendrait negatif et les
/// bandes sortiraient du widget au lieu de s'y reduire.
[[nodiscard]] int frameUnit(int width, int height, int scale) {
    if (width <= 0 || height <= 0) {
        return 0;
    }
    const int maximumUnit = std::min(width, height) / (2 * PARCHMENT_FRAME_UNITS);
    return std::min(std::max(scale, 1), maximumUnit);
}

}  // namespace

int parchmentFrameCorner(int width, int height, int scale) {
    return PARCHMENT_FRAME_UNITS * std::max(0, frameUnit(width, height, scale));
}

std::vector<ParchmentStroke> parchmentFrameStrokes(int width, int height, int scale) {
    if (width <= 0 || height <= 0) {
        return {};
    }

    std::vector<ParchmentStroke> strokes;
    // Le champ d'abord : tout le reste se pose dessus. Le peindre integralement, plutot que le seul
    // interieur de l'encadrement, evite qu'un arrondi d'epaisseur laisse une bande de pixels non
    // peints -- que Qt rendrait par ce qui se trouve derriere, jamais par du parchemin.
    strokes.push_back({ParchmentRole::Field, 0, 0, width, height});

    const int unit = frameUnit(width, height, scale);
    if (unit <= 0) {
        // Trop petit pour un encadrement honnete : un champ nu, pas un cadre ecrase.
        return strokes;
    }

    addRing(strokes, ParchmentRole::Edge, width, height, 0, unit);
    addRing(strokes, ParchmentRole::Ornament, width, height, 2 * unit, unit);
    // L'ombre se pose JUSTE A L'INTERIEUR du filet, jamais entre le trait et lui : c'est le filet
    // qui est en relief sur le champ, et une ombre placee en amont comblerait la reserve de
    // parchemin -- la seule chose qui fasse lire les deux traits comme un encadrement.
    if (std::min(width, height) >= 2 * ((PARCHMENT_FRAME_UNITS + 1) * unit)) {
        addRing(strokes, ParchmentRole::Shadow, width, height, PARCHMENT_FRAME_UNITS * unit, unit);
    }

    // Cabochons d'angle, poses en DERNIER : ils recouvrent le trait et la reserve aux quatre
    // angles, ce qui donne l'encadrement dore d'une feuille de personnage. Trois unites de cote,
    // soit exactement l'epaisseur de l'encadrement.
    const int corner = PARCHMENT_FRAME_UNITS * unit;
    for (const int x : {0, width - corner}) {
        for (const int y : {0, height - corner}) {
            strokes.push_back({ParchmentRole::Ornament, x, y, corner, corner});
        }
    }
    return strokes;
}

std::vector<ParchmentPoint> focusFleuronPoints(int size) {
    // Sous quatre pixels, l'echancrure arriere ne fait plus qu'un pixel de profondeur : la marque
    // cesse d'etre reconnaissable, et une marque qu'on ne reconnait pas ne designe rien.
    if (size < 4) {
        return {};
    }
    const int half = size / 2;
    const int notch = size / 4;
    return {{0, 0}, {size, half}, {0, size}, {notch, half}};
}

}  // namespace hmi
