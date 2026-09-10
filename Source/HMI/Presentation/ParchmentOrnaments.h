// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <vector>

/**
 * @file HMI/Presentation/ParchmentOrnaments.h
 * @brief Ornements d'habillage des écrans du jeu, **tracés** (`LOT-76`, `EX-IHM-075`).
 *
 * Logique **pure** (aucune dépendance Qt/GPU), testable hors instance d'application
 * (`EX-NFR-010`) — même patron que `hmi::iconGeometry` et `hmi::parchmentFrameStrokes` : ces
 * fonctions décident *quoi* dessiner, les peintres Qt décident *comment*.
 *
 * ## Tracé, et pas extrait
 *
 * Les feuilles de personnage du corpus portent ces ornements peints, à 300 ppp, et il aurait été
 * plus court de les découper en PNG. Trois choses l'interdisent, et c'est la raison d'être de ce
 * module :
 *
 * - **une image ne s'étire pas honnêtement.** Un cabochon posé sur un panneau bas s'ovalise, ou
 *   mange le tiers de sa hauteur ; un bandeau étiré déforme ses ailes. Un ornement tracé se
 *   *redessine* à la taille demandée — c'est le sens du mot « dynamique » ici ;
 * - **une image fige ses couleurs hors des jetons** (`EX-IHM-051`), et devrait être réexportée à
 *   chaque retouche de palette. C'est l'argument du [LOT-66](@ref lot-66), et il n'a pas changé :
 *   chaque forme ci-dessous porte un **rôle**, jamais une teinte ;
 * - **une image ne suit pas le facteur d'agrandissement.** L'identité du jeu se met à l'échelle par
 *   entiers (`EX-IHM-081`) ; un tracé y est net à tous les facteurs, un bitmap à un seul.
 *
 * Ce que le corpus apporte reste entier : les **proportions** et les **teintes** sont relevées sur
 * `Character_Sheets_Tanares.pdf`, jamais choisies à vue (`EX-IHM-070`). Le grenat du cabochon est
 * la dominante quantifiée des pixels rouges de deux angles opposés de la planche, qui donnent la
 * même valeur ; elle vit dans `hmi::identityTokens().color.gem`.
 */

namespace hmi {

/// Rôle de couleur d'une forme, résolu depuis les jetons de la portée identité au moment du rendu.
enum class OrnamentRole {
    Edge,           ///< Trait extérieur d'encre (`frameEdge`).
    Ornament,       ///< Or du serti, des filets et des ailes (`frameOrnament`).
    OrnamentLight,  ///< Éclat d'or : facette éclairée, reflet (`accentHover`).
    Gem,            ///< Facette éclairée de la gemme (`gem`).
    GemShadow,      ///< Facette en profondeur, fond de plaque (`gemShadow`).
};

/// Un sommet, en pixels, relatif au coin haut-gauche de la forme. En flottants : une facette de
/// gemme tombe rarement sur un pixel entier, et l'arrondir déforme l'angle plutôt que la position.
struct OrnamentPoint {
    float x;
    float y;

    [[nodiscard]] friend bool operator==(const OrnamentPoint&,
                                         const OrnamentPoint&) noexcept = default;
};

/// Une forme d'ornement : un polygone fermé, rempli ou tracé, d'un seul rôle.
struct OrnamentShape {
    std::vector<OrnamentPoint> points;
    OrnamentRole role = OrnamentRole::Ornament;
    bool filled = true;  ///< Rempli ; sinon tracé au filet d'une unité d'agrandissement.
};

/// Côté minimal, en pixels, sous lequel un cabochon n'est plus qu'une tache. Six sommets d'octogone
/// dans moins de huit pixels ne dessinent plus un serti, et une marque qu'on ne reconnaît pas ne
/// désigne rien — même raison que le plancher du fleuron de focus (`hmi::focusFleuronPoints`).
inline constexpr int ORNAMENT_MINIMUM_SIZE = 8;

/**
 * @brief Cabochon d'angle : un grenat taillé, serti d'or, inscrit dans un carré de côté @p size.
 *
 * C'est l'ornement que les feuilles posent aux quatre angles de chaque panneau. Il remplace le
 * carré d'or plein du [LOT-66](@ref lot-66) — qui n'était pas faux, seulement muet : un aplat de
 * trois unités ne dit pas qu'il y a une pierre.
 *
 * @param size Côté du carré englobant, en pixels.
 * @return Les formes, dans l'**ordre de dessin** (les dernières recouvrent) ; vide sous
 *         `ORNAMENT_MINIMUM_SIZE`.
 */
[[nodiscard]] std::vector<OrnamentShape> cabochonShapes(int size);

/**
 * @brief Bandeau de titre : une plaque de grenat à chevrons, entre deux ailes d'or.
 *
 * Les ailes ne s'étirent **pas** avec la largeur : leur envergure suit la **hauteur** du bandeau,
 * comme dans le livre, et c'est le seul point où l'on peut se tromper en le traçant. Un bandeau
 * deux fois plus long doit porter une plaque deux fois plus longue et les mêmes ailes ; les faire
 * grandir avec la largeur donne, sur un titre long, deux ailes de chauve-souris démesurées de part
 * et d'autre d'une plaque restée fine.
 *
 * Quand la largeur ne suffit plus à porter les deux ailes et une plaque lisible, ce sont les
 * **ailes** qui cèdent — d'abord en se réduisant, puis, sous `ORNAMENT_MINIMUM_SIZE`, en
 * disparaissant. Jamais la plaque : c'est elle qui porte le titre, et un titre rogné pour loger
 * des ailes entières n'a plus de raison d'avoir un bandeau. Un moignon de six pixels, lui, ne se
 * lit pas comme une aile mais comme une bavure.
 *
 * @param width  Largeur totale du bandeau, en pixels.
 * @param height Hauteur totale, en pixels.
 * @return Les formes, dans l'ordre de dessin ; vide si le bandeau est dégénéré.
 */
[[nodiscard]] std::vector<OrnamentShape> titleBannerShapes(int width, int height);

/// Envergure d'une aile pour un bandeau de @p height pixels de haut — exposée pour que la mise en
/// page réserve la place du titre **entre** les ailes plutôt que par-dessus.
[[nodiscard]] int bannerWingSpan(int height);

}  // namespace hmi
