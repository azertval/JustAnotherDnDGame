// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>
#include <vector>

#include "HMI/Interface/DesignTokens.h"

/**
 * @file HMI/Interface/KeyHintText.h
 * @brief Rappels de touches en bas des écrans du jeu (`LOT-68`, `EX-IHM-070`).
 *
 * Logique **pure** (aucune dépendance Qt/GPU), testable hors instance d'application
 * (`EX-NFR-010`). Produit le texte enrichi d'une ligne « touche → action », les touches rendues
 * comme des **capuchons** : un fond, une bordure, du rembourrage. C'est ce qui les distingue du
 * libellé qui les suit, sans quoi la ligne se lit comme une phrase et non comme une aide.
 *
 * Un texte enrichi plutôt qu'un widget par touche : la ligne est purement décorative, et une
 * douzaine de widgets vides coûteraient plus cher à construire qu'à lire.
 */

namespace hmi {

/// Une paire touche → action, déjà **traduite** par l'appelant (`EX-REN-033`).
struct KeyHint {
    std::string key;     ///< Ce qui s'affiche dans le capuchon (ex. « ÉCHAP », « A », « ↑↓ »).
    std::string action;  ///< Ce que la touche fait.
};

/**
 * @brief Compose la ligne de rappels en texte enrichi.
 *
 * Les couleurs viennent des **jetons** et ne sont jamais littérales (`EX-IHM-051`) : la ligne suit
 * la palette comme le reste de l'écran.
 *
 * @param hints  Rappels à afficher, dans l'ordre. Une liste vide produit une chaîne vide, jamais
 *               un cadre orphelin.
 * @param tokens Jetons de la portée à employer (identité pour les écrans du jeu).
 * @param scale  Facteur d'agrandissement entier (`hmi::identityScaleFor`), appliqué au rembourrage
 * et à la taille du texte.
 * @return Le fragment HTML à poser dans un libellé.
 */
[[nodiscard]] std::string keyHintText(const std::vector<KeyHint>& hints, const DesignTokens& tokens,
                                      int scale);

}  // namespace hmi
