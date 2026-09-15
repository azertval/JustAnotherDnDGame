// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <QIcon>

#include "HMI/Interface/IconGeometry.h"

/**
 * @file HMI/Interface/ThemeIcons.h
 * @brief Rendu Qt des icônes de l'IHM, depuis leur géométrie pure (`LOT-56` TACHE-04).
 */

namespace hmi {

struct DesignTokens;

/**
 * @brief Peint une icône à la taille demandée, recolorée depuis les jetons.
 * @param id        Icône à peindre (`hmi::iconGeometry`).
 * @param pixelSize Côté du carré, en pixels **réels** (déjà multiplié par le facteur d'échelle
 *                  d'affichage par l'appelant, `LOT-56` TACHE-05) : le tracé reste net à toute
 *                  échelle.
 * @param tokens    Jetons de la portée courante (châssis d'édition) : résout les rôles de couleur
 *                  de la géométrie, de sorte que l'icône suive le thème actif (TACHE-06).
 * @return Une `QIcon` couvrant l'unique taille demandée.
 */
[[nodiscard]] QIcon themeIcon(IconId id, int pixelSize, const DesignTokens& tokens);

}  // namespace hmi
