// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

/**
 * @file HMI/Interface/IdentityScale.h
 * @brief Facteur d'agrandissement **entier** des écrans du jeu (`LOT-66`, `EX-IHM-070`).
 *
 * Logique **pure** (aucune dépendance Qt/GPU), testable hors instance d'application
 * (`EX-NFR-010`) — compilée à la fois dans `JustAnotherDnDGame` et directement dans `UnitTests`,
 * comme `HMI/Interface/DesignTokens.cpp`. Même découpage que `hmi::thumbnailPixelSize` : cette
 * fonction décide *de combien* agrandir, la couche Qt (`hmi::applyStyleSheet`) décide *quoi*
 * multiplier.
 *
 * @par Pourquoi le facteur reste entier après la sortie du pixel art
 * Le `LOT-68` l'exigeait pour une raison qui n'existe plus : le filtrage au plus proche voisin ne
 * sait pas rendre une bordure d'un pixel et demi. La charte parchemin du `LOT-66` peint des traits
 * anticrénelés, que rien n'oblige à tomber sur la grille. Le facteur reste néanmoins entier pour
 * une **seconde** raison, elle intacte : toutes les grandeurs d'identité passent par la feuille de
 * style, dont les longueurs sont des **entiers de pixels**. À 1,5×, un trait d'une unité et un
 * filet d'une unité s'arrondissent tous deux à 2 px — l'encadrement perd sa réserve de parchemin
 * et se lit comme une bordure épaisse, sans qu'aucune erreur ne soit levée.
 *
 * Une échelle fractionnaire ne serait donc pas *floue* : elle serait **fausse**, et silencieuse.
 */

namespace hmi {

/// Hauteur, en pixels **logiques**, de la maquette à l'échelle 1. Les écrans sont dessinés pour
/// cette hauteur ; tout le reste en est un multiple entier.
inline constexpr int IDENTITY_BASE_HEIGHT = 360;

/// Plus grand facteur admis. Au-delà, les écrans cessent de gagner en lisibilité et commencent à
/// perdre en contenu visible : trois entrées de menu par écran ne rendent service à personne.
inline constexpr int IDENTITY_MAX_SCALE = 3;

/**
 * @brief Facteur d'agrandissement des écrans du jeu pour une hauteur de fenêtre donnée.
 *
 * La hauteur attendue est celle en pixels **logiques**, pas en pixels réels : la mise à l'échelle
 * du système d'exploitation est appliquée par Qt **par-dessus** ce facteur, sur une disposition
 * exprimée en unités logiques. Multiplier une seconde fois par `devicePixelRatio` ici produirait
 * une interface deux fois trop grande sur un écran réglé à 200 %.
 *
 * Le résultat est toujours un entier d'au moins 1, même pour une fenêtre plus petite que la
 * maquette : une fenêtre étroite doit rogner l'écran, jamais le rendre à une échelle fractionnaire.
 *
 * @param windowLogicalHeight Hauteur de la fenêtre, en pixels logiques.
 * @return Le facteur entier, dans `[1, IDENTITY_MAX_SCALE]`.
 */
[[nodiscard]] int identityScaleFor(int windowLogicalHeight) noexcept;

/**
 * @brief Facteur d'agrandissement **borné par la zone d'affichage réellement disponible**
 *        (`EX-IHM-081`).
 *
 * `identityScaleFor` seule ne connaît que la fenêtre, et c'est insuffisant : le facteur grossit les
 * grandeurs d'habillage, qui grossissent la taille minimale des écrans, qui grossit celle de la
 * fenêtre — laquelle relance le calcul un cran plus haut. Rien ne redescend jamais, la fenêtre ne
 * pouvant pas passer sous son propre minimum : c'est un **cliquet**, et il n'a d'issue que si le
 * facteur cesse de dépendre d'une hauteur que lui-même fait croître.
 *
 * La zone disponible, elle, ne dépend de rien que l'application décide. La borner ferme la boucle :
 * une fenêtre que Qt a été contraint d'agrandir au-delà de l'écran ne gagne plus de facteur pour
 * autant.
 *
 * @param windowLogicalHeight    Hauteur de la fenêtre, en pixels logiques.
 * @param availableLogicalHeight Hauteur utile de l'écran hébergeant la fenêtre (hors barre des
 *                               tâches), en pixels logiques. Une valeur nulle ou négative signifie
 *                               « écran inconnu » et laisse la seule hauteur de fenêtre décider.
 * @return Le facteur entier, dans `[1, IDENTITY_MAX_SCALE]`, jamais supérieur à celui que la zone
 *         disponible admet.
 */
[[nodiscard]] int identityScaleForDisplay(int windowLogicalHeight,
                                          int availableLogicalHeight) noexcept;

}  // namespace hmi
