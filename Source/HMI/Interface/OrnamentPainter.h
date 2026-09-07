// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <vector>

#include "HMI/Interface/ParchmentOrnaments.h"

class QPainter;
class QPoint;

/**
 * @file HMI/Interface/OrnamentPainter.h
 * @brief Peintre unique des ornements tracés (`LOT-76`, `EX-IHM-075`).
 *
 * Couche **Qt** au-dessus de `hmi::ParchmentOrnaments` : la géométrie décide *quoi* dessiner, ce
 * peintre résout chaque rôle depuis les jetons de la portée **identité** — jamais depuis ceux du
 * châssis d'édition, qui suivent le thème clair/sombre (`EX-IHM-054`).
 *
 * Une **seule** fonction, appelée par tous les ornements, pour la raison qui a déjà valu au
 * fleuron de focus d'être tracé une seule fois (`LOT-66`) : deux peintres séparés dérivent l'un de
 * l'autre à la première retouche, et le joueur voit deux cabochons différents sur deux écrans sans
 * comprendre pourquoi.
 */

namespace hmi {

/**
 * @brief Peint les formes de @p shapes, translatées de @p origin.
 *
 * L'anticrénelage est **actif** : toutes ces formes sont obliques — facettes de gemme, doigts
 * d'aile, chevrons — et sont les seules de l'habillage à ne pas suivre la grille de pixels. Sans
 * lui, elles redeviendraient l'escalier du pixel art dont la charte est sortie. C'est l'inverse du
 * choix fait pour les bandes de `hmi::ParchmentPanel`, qui sont toutes alignées sur la grille et
 * qu'un anticrénelage coudrait d'une demi-teinte claire à chaque jointure.
 *
 * Les réglages du @p painter (crayon, brosse, indices de rendu) sont **restaurés** à la sortie :
 * ce peintre est appelé au milieu du `paintEvent` d'autrui.
 */
void paintOrnaments(QPainter& painter, const std::vector<OrnamentShape>& shapes,
                    const QPoint& origin);

}  // namespace hmi
