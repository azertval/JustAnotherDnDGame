// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <string>

/**
 * @file HMI/Editor/LevelNameValidation.h
 * @brief Validation d'un nom de niveau saisi dans l'éditeur (`EX-EDIT-009`).
 */

namespace hmi {

/**
 * @brief Indique si @p name peut servir de nom de niveau (donc de nom de fichier Windows).
 *
 * Liste noire **minimale** (pas une liste blanche restrictive : les accents et l'Unicode restent
 * autorisés, `EX-NFR-010` n'impose rien de plus) : un nom valide n'est pas vide une fois les
 * espaces de bord retirés, et ne contient aucun des caractères interdits par le système de
 * fichiers Windows (antislash, barre oblique, deux-points, astérisque, point d'interrogation,
 * guillemet droit, chevrons ouvrant/fermant, barre verticale).
 * @param name Nom saisi, tel quel (non retiré de ses espaces de bord).
 * @return `true` si @p name peut être utilisé comme nom de niveau.
 */
[[nodiscard]] bool isValidLevelName(const std::string& name);

/**
 * @brief Retire les espaces de bord de @p name (le nom réellement utilisé une fois confirmé).
 * @param name Nom saisi.
 * @return @p name sans ses espaces de début/fin.
 */
[[nodiscard]] std::string trimLevelName(const std::string& name);

}  // namespace hmi
