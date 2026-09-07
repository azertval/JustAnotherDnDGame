// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <vector>

/**
 * @file HMI/Interface/ParchmentFrame.h
 * @brief Géométrie de l'encadrement de parchemin des écrans du jeu (`LOT-66`, `EX-IHM-070`).
 *
 * Logique **pure** (aucune dépendance Qt/GPU), testable hors instance d'application
 * (`EX-NFR-010`) — même patron que `hmi::iconGeometry` : ces fonctions décident *quoi* dessiner,
 * `hmi::ParchmentPanel` (Qt) décide *comment* le peindre, en résolvant chaque rôle depuis les
 * jetons de la portée identité.
 *
 * Aucun fichier d'image n'est livré. Un cadre 9 tranches en PNG figerait ses couleurs hors des
 * jetons (`EX-IHM-051`) et devrait être réexporté à chaque changement de palette ; tracé par code,
 * il suit la palette sans qu'on y pense — exactement le raisonnement qui a mené aux icônes
 * vectorielles du `LOT-56`.
 */

namespace hmi {

/// Rôle de couleur d'une bande, résolu depuis les jetons au moment du rendu.
enum class ParchmentRole {
    Field,     ///< Champ intérieur, le parchemin lui-même (`surface`).
    Edge,      ///< Trait extérieur d'encre (`frameEdge`).
    Ornament,  ///< Filet ornemental doré, en retrait du trait (`frameOrnament`).
    Shadow,    ///< Ombre du filet sur le champ (`frameShadow`).
};

/// Une bande pleine, en pixels, relative au coin haut-gauche du widget.
struct ParchmentStroke {
    ParchmentRole role;
    int x;
    int y;
    int width;
    int height;

    [[nodiscard]] friend bool operator==(const ParchmentStroke&,
                                         const ParchmentStroke&) noexcept = default;
};

/// Un sommet, en pixels, relatif au coin haut-gauche du carré englobant.
struct ParchmentPoint {
    int x;
    int y;

    [[nodiscard]] friend bool operator==(const ParchmentPoint&,
                                         const ParchmentPoint&) noexcept = default;
};

/// Épaisseur totale de l'encadrement, en **unités de maquette** : trait, réserve de parchemin,
/// filet. C'est la réserve du milieu qui fait l'encadrement — sans elle, trait et filet se
/// touchent et l'ensemble se lit comme une bordure épaisse de deux tons, pas comme un cadre.
inline constexpr int PARCHMENT_FRAME_UNITS = 3;

/**
 * @brief Bandes composant un encadrement, dans l'**ordre de dessin** (les dernières recouvrent les
 *        premières).
 *
 * Les coins ne sont **pas entaillés** : ils portent un cabochon doré, carré de trois unités posé
 * par-dessus le trait. C'est ce détail qui distingue l'encadrement d'une simple bordure à deux
 * traits — et c'est exactement ce que le pixel art faisait à l'envers, en *retirant* de la matière
 * aux angles au lieu d'en ajouter.
 *
 * Toutes les bandes retournées sont contenues dans `[0, width] x [0, height]`, y compris lorsque le
 * cadre est trop petit pour porter son encadrement : dans ce cas l'épaisseur se réduit, elle ne
 * déborde jamais. Un cadre plus petit que six unités dans l'une de ses dimensions ne rend que son
 * champ — mieux vaut un panneau nu qu'un encadrement écrasé, illisible et faux.
 *
 * @param width  Largeur du cadre, en pixels.
 * @param height Hauteur du cadre, en pixels.
 * @param scale  Facteur d'agrandissement entier (`hmi::identityScaleFor`) : trait, réserve et
 *               filet valent chacun une unité de maquette, donc @p scale pixels réels.
 * @return Les bandes à peindre ; vide si @p width ou @p height est nul ou négatif.
 */
[[nodiscard]] std::vector<ParchmentStroke> parchmentFrameStrokes(int width, int height, int scale);

/**
 * @brief Côté du **cabochon d'angle** d'un encadrement de @p width × @p height, en pixels.
 *
 * Exposé pour que le peintre pose au même endroit, et à la même taille, l'ornement tracé du
 * `LOT-76` (`hmi::cabochonShapes`). Le déduire de l'ordre des bandes rendues par
 * `parchmentFrameStrokes` marcherait aujourd'hui et casserait à la première bande ajoutée, sans
 * qu'aucune erreur ne le dise : le cabochon se poserait ailleurs que le carré qu'il habille.
 *
 * @return Le côté du carré d'angle ; **zéro** quand le cadre est trop petit pour porter un
 *         encadrement, cas où `parchmentFrameStrokes` ne rend que son champ.
 */
[[nodiscard]] int parchmentFrameCorner(int width, int height, int scale);

/// Facteur de débord du **cabochon** sur le carré d'angle qu'il habille (`LOT-76`).
///
/// Un cabochon exactement à la taille du carré disparaît : il fait alors l'épaisseur de
/// l'encadrement, et son octogone se confond avec le filet qui le borde. Sur les feuilles du
/// corpus, la pierre mesure plus du double du filet — elle est *posée sur* l'angle, pas
/// *encastrée dedans*. Deux fois, donc, et pas trois : au-delà elle mordrait sur le champ.
inline constexpr int PARCHMENT_CABOCHON_FACTOR = 2;

/**
 * @brief Sommets du **fleuron de focus** — une pointe pleine tournée vers la droite, inscrite dans
 *        un carré de côté @p size (`LOT-66`, `EX-IHM-071`).
 *
 * Un polygone, et non plus des pavés : la charte parchemin peint anticrénelé, et le triangle en
 * escalier du pixel art n'a plus lieu d'être. La pointe est **échancrée** à l'arrière — quatre
 * sommets, pas trois — pour qu'elle se lise comme une marque de position et non comme le curseur
 * de saisie d'un champ de texte.
 *
 * @param size Côté du carré englobant, en pixels. En dessous de quatre pixels, le fleuron est vide
 *             plutôt que réduit à une tache : une marque qu'on ne reconnaît pas ne désigne rien.
 * @return Les sommets, dans l'ordre du tracé, relatifs au coin haut-gauche du carré.
 */
[[nodiscard]] std::vector<ParchmentPoint> focusFleuronPoints(int size);

}  // namespace hmi
