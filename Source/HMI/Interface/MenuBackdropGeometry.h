// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <vector>

/**
 * @file HMI/Interface/MenuBackdropGeometry.h
 * @brief Décor du menu principal, en pavés (`LOT-68`, `EX-IHM-070`).
 *
 * Logique **pure** (aucune dépendance Qt/GPU), testable hors instance d'application
 * (`EX-NFR-010`) — même patron que `hmi::iconGeometry` et `hmi::parchmentFrameStrokes` : cette
 * fonction décide *quoi* dessiner, `hmi::MainMenu` décide *comment* le peindre en résolvant chaque
 * rôle depuis les jetons de la portée identité.
 *
 * Aucune image livrée, pour la même raison que le cadre : un décor en PNG figerait ses couleurs
 * hors des jetons (`EX-IHM-051`) et ne suivrait ni le facteur d'agrandissement ni la taille de la
 * fenêtre.
 *
 * Le décor est **déterministe** : les étoiles occupent des positions fixées par une table, jamais
 * tirées au sort. Un ciel qui changerait à chaque ouverture du menu se remarquerait, et rendrait
 * toute capture d'écran de non-régression inutilisable.
 */

namespace hmi {

/// Rôle de couleur d'un pavé du décor, résolu depuis les jetons au moment du rendu.
enum class BackdropRole {
    SkyHigh,     ///< Haut du ciel, la valeur la plus sombre.
    SkyMid,      ///< Bande intermédiaire.
    SkyLow,      ///< Bande proche de l'horizon, la plus claire du ciel.
    Star,        ///< Étoile vive.
    StarDim,     ///< Étoile en retrait.
    Moon,        ///< Disque lunaire.
    MoonCrater,  ///< Cratère, une nuance sous le disque.
    Silhouette,  ///< Bâtiments lointains, à l'horizon.
    Hill,        ///< Collines de plan intermédiaire.
    Ground,      ///< Sol de premier plan.
    GroundEdge,  ///< Liseré supérieur du sol, qui lui donne son épaisseur.
    Veil,        ///< Voile de lisibilité, posé par bandes sous le texte.
};

/// Un pavé plein, en pixels, relatif au coin haut-gauche de la surface décorée.
struct BackdropQuad {
    BackdropRole role;
    int x;
    int y;
    int width;
    int height;

    [[nodiscard]] friend bool operator==(const BackdropQuad&,
                                         const BackdropQuad&) noexcept = default;
};

/// Part de la largeur couverte par le voile de lisibilité, sous le titre et les entrées de menu.
inline constexpr float BACKDROP_VEIL_WIDTH_RATIO = 0.55f;

/// Nombre de bandes du voile. Un **dégradé** serait plus doux, mais la charte du `LOT-66` pose ses
/// lavis en passes plates, à la manière des feuilles de Tanares : des paliers francs. La raison
/// invoquée jusque-là — le pixel art ne s'accommode pas d'une transition continue — est tombée
/// avec l'identité pixel art ; le choix, lui, reste celui de la charte.
inline constexpr int BACKDROP_VEIL_STEPS = 6;

/**
 * @brief Pavés du décor, dans l'**ordre de dessin** (les derniers recouvrent les premiers).
 *
 * Les proportions suivent la surface : le décor se recompose à toute taille de fenêtre plutôt que
 * d'être calé sur une définition de référence. Le pas de la grille suit @p scale, de sorte que les
 * blocs du sol restent alignés sur celui du jeu.
 *
 * @param width  Largeur de la surface, en pixels.
 * @param height Hauteur de la surface, en pixels.
 * @param scale  Facteur d'agrandissement entier (`hmi::identityScaleFor`).
 * @return Les pavés à peindre ; vide si la surface est trop petite pour porter un décor.
 */
[[nodiscard]] std::vector<BackdropQuad> menuBackdropQuads(int width, int height, int scale);

}  // namespace hmi
