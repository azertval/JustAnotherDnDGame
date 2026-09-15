// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

/**
 * @file HMI/Graphics/Quad.h
 * @brief Primitives de dessin 2D (quads texturés), **indépendantes de Direct3D**.
 *
 * Ces structures décrivent *quoi* dessiner, jamais *comment* : elles ne référencent aucune
 * ressource Direct3D et sont donc manipulables par la **composition** du rendu
 * (`hmi::ComposedScene`), qui doit rester testable sans GPU (`EX-NFR-004`). `hmi::SpriteBatch` les
 * consomme côté soumission ; son contrat public est inchangé (il les expose toujours via
 * `HMI/Graphics/SpriteBatch.h`, qui inclut ce fichier).
 */

namespace hmi {

/**
 * @brief Un quad texturé à dessiner : rectangle en **unités monde**, région de texture
 *        (coordonnées normalisées) et teinte.
 *
 * Le coin (`x`, `y`) est haut-gauche **avant rotation**, l'axe Y va vers le bas (convention du
 * projet). Les coordonnées de texture `u,v` sont normalisées dans [0, 1] ; la conversion depuis
 * une région d'atlas en pixels est faite en amont (par le rendu des sprites). `rotation` (radians,
 * `LOT-50` TACHE-02) tourne le rectangle autour de **son propre centre** — nul par défaut, donc
 * sans coût ni différence visuelle pour les quads alignés aux axes (tuiles, personnage).
 */
struct SpriteQuad {
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
    float rotation = 0.0f;
    float u0 = 0.0f;
    float v0 = 0.0f;
    float u1 = 1.0f;
    float v1 = 1.0f;
    float r = 1.0f;
    float g = 1.0f;
    float b = 1.0f;
    float a = 1.0f;
};

/**
 * @brief Un segment épais à dessiner : deux extrémités en **unités monde**, une épaisseur
 *        (perpendiculaire au segment, unités monde) et une teinte.
 *
 * Contrairement à `SpriteQuad` (rectangle toujours aligné aux axes), ce quad peut être **orienté**
 * dans n'importe quelle direction — utilisé pour les liens de mécanismes (`LOT-37`, flèches
 * déclencheur → cible). Mêmes conventions que `SpriteQuad` (Y vers le bas, UV normalisées).
 */
struct LineQuad {
    float ax = 0.0f;
    float ay = 0.0f;
    float bx = 0.0f;
    float by = 0.0f;
    float thickness = 0.0f;
    float u0 = 0.0f;
    float v0 = 0.0f;
    float u1 = 1.0f;
    float v1 = 1.0f;
    float r = 1.0f;
    float g = 1.0f;
    float b = 1.0f;
    float a = 1.0f;
};

}  // namespace hmi
