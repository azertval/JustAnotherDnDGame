// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <cstdint>
#include <vector>

/**
 * @file HMI/Graphics/ProceduralAtlas.h
 * @brief Génération procédurale (CPU, sans GPU) des pixels de l'atlas de repli.
 */

namespace hmi {

/// Pixels d'un atlas généré en mémoire, au format `R8G8B8A8_UNORM` (ordre mémoire R,G,B,A).
struct ProceduralAtlasImage {
    int width = 0;
    int height = 0;
    /// Taille `width * height`, ligne par ligne (haut en bas), pixel `pack(r,g,b,a)`.
    std::vector<std::uint32_t> pixels;
};

/**
 * @brief Génère, en mémoire, l'atlas procédural (grille de tuiles colorées), de façon
 *        **déterministe**.
 *
 * Logique pure (`EX-NFR-010`), sans dépendance GPU/Qt : unique source des pixels de
 * `hmi::TextureAtlas` et des vignettes de la palette de l'éditeur (`hmi::PalettePanel`).
 * @return L'image générée (dimensions et pixels).
 */
[[nodiscard]] ProceduralAtlasImage buildProceduralAtlasImage();

}  // namespace hmi
