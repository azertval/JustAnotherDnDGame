// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <optional>
#include <string>

#include "HMI/Input/InputState.h"

/**
 * @file HMI/Input/KeyName.h
 * @brief Nom affichable d'une touche et capture générique « prochaine touche pressée » (`LOT-29`).
 */

namespace hmi {

/**
 * @brief Nom lisible d'une touche, pour l'affichage dans les écrans de remappage (`LOT-29`) et le
 *        panneau d'aide de l'éditeur (`EX-EDIT-015`).
 * @param key Touche à nommer (n'importe quel code, pas seulement les énumérateurs nommés).
 * @return Un nom court : celui des touches nommées de `Key` (flèches, Espace, Maj…), le caractère
 *         lui-même pour une lettre/un chiffre non nommé explicitement, sinon un repli hexadécimal.
 */
[[nodiscard]] std::string keyDisplayName(Key key);

/**
 * @brief Cherche la première touche pressée cette frame, pour la capture d'un remappage.
 *
 * Scrute l'ensemble des codes suivis par `InputState` (aucune touche nommée par `Key` n'est
 * privilégiée : n'importe quelle touche du clavier peut être liée à une action). `Échap` et
 * `Entrée` sont **exclus** — réservés à la navigation globale, jamais assignables — leur pression
 * seule ne renvoie rien, la capture doit rester ouverte.
 * @param input État des entrées de la frame.
 * @return La touche capturée, ou `std::nullopt` si aucune touche assignable n'est pressée.
 */
[[nodiscard]] std::optional<Key> capturedKey(const InputState& input);

}  // namespace hmi
