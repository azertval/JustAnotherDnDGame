// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <string>

#include "HMI/Graphics/ProceduralAtlas.h"

/**
 * @file HMI/Graphics/MissingTexture.h
 * @brief Repli visible (damier magenta) pour toute texture attendue mais absente (`EX-NFR-040`).
 */

namespace hmi {

/// Côté, en pixels, du damier de repli généré par défaut (une case, `EX-ARCH-021`).
inline constexpr int MISSING_TEXTURE_SIZE = 16;

/// Côté, en pixels, d'un carreau du damier : quatre carreaux par côté, motif lisible même réduit.
inline constexpr int MISSING_TEXTURE_CHECKER_SIZE = 4;

/**
 * @brief Génère, en mémoire, la texture de repli « asset manquant » : un damier magenta/noir.
 *
 * Sœur de `hmi::buildProceduralAtlasImage` : logique **pure** et **déterministe**, sans dépendance
 * Direct3D (`EX-NFR-010`). Le repli est volontairement **opaque** et magenta : il doit être
 * impossible de le confondre avec un asset réel (aucune palette du jeu n'utilise le magenta) ni
 * avec un trou de rendu (une zone transparente passerait inaperçue). Un asset manquant doit se
 * voir, pas se deviner.
 * @param size Côté de l'image générée, en pixels (> 0 ; `MISSING_TEXTURE_SIZE` par défaut).
 * @return L'image générée (dimensions et pixels `R8G8B8A8_UNORM`).
 */
[[nodiscard]] ProceduralAtlasImage buildMissingTextureImage(int size = MISSING_TEXTURE_SIZE);

/**
 * @brief Message d'avertissement émis lorsqu'un asset attendu est introuvable ou refusé.
 *
 * Fonction **pure**, séparée de la journalisation elle-même pour être assertable sans GPU : le
 * message doit **nommer l'asset attendu**, seule information qui permette à l'auteur de savoir
 * quoi créer.
 * @param fileName Nom logique de l'asset attendu.
 * @return Le message d'avertissement, en français.
 */
[[nodiscard]] std::string missingTextureWarning(const std::string& fileName);

}  // namespace hmi
