// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <array>
#include <cstdint>
#include <vector>

#include "Core/Levels/TileType.h"

/**
 * @file HMI/Graphics/TileSilhouette.h
 * @brief Détourage des tuiles dont la matière n'occupe pas toute la case.
 */

namespace hmi {

/**
 * @brief Types de tuile dont la matière n'occupe **pas** toute la case, et qui doivent donc être
 *        détourées à l'affichage plutôt que dessinées en carré plein.
 *
 * @note **Actuellement vide.** Le `LOT-01` a retiré les pentes, arrondis et concaves du jeu de
 *       plateforme, seuls types qui peuplaient cette liste. Le mécanisme est conservé — il est
 *       partagé par l'atlas procédural, le détourage des skins (`hmi::TextureCache`) et les
 *       vignettes de la palette (`hmi::PaletteAppearance`), et le vocabulaire de tuiles du RPG
 *       (`LOT-08` : falaises, bords d'eau, ponts) le repeuplera. Le vider plutôt que le supprimer
 *       évite de reconstruire trois chaînes d'appel qui fonctionnent.
 */
inline constexpr std::array<core::TileType, 0> SILHOUETTE_TILE_TYPES{};

/// Nombre de types de tuile à silhouette (voir `SILHOUETTE_TILE_TYPES`).
inline constexpr int SILHOUETTE_TILE_TYPE_COUNT =
    static_cast<int>(SILHOUETTE_TILE_TYPES.size());

/**
 * @brief Indique si @p type a une silhouette, c'est-à-dire une matière qui n'occupe pas toute la
 *        case et qu'il faut détourer à l'affichage.
 * @param type Type de tuile.
 * @return `true` si @p type figure dans `SILHOUETTE_TILE_TYPES` — donc jamais tant que cette liste
 *         est vide.
 */
[[nodiscard]] bool hasSilhouette(core::TileType type) noexcept;

/**
 * @brief Indique si le pixel (@p localX, @p localY) d'une case appartient à la matière de @p type.
 *
 * Point de vérité **unique** de la silhouette, partagé par l'atlas procédural et le détourage des
 * skins : deux implémentations de la même forme finiraient par diverger, et un skin découpé
 * autrement que l'atlas se verrait immédiatement.
 * @param type     Type de tuile.
 * @param localX   Abscisse du pixel dans la case, 0-based.
 * @param localY   Ordonnée du pixel dans la case, 0-based.
 * @param tileSize Côté de la case, en pixels.
 * @return `true` si le pixel est dans la matière. Un type **sans** silhouette remplit toute la
 *         case : la réponse est alors toujours `true`.
 */
[[nodiscard]] bool isInsideSilhouette(core::TileType type, int localX, int localY,
                                      int tileSize) noexcept;

/**
 * @brief Applique la silhouette d'un type à une image carrée, en rendant transparent l'extérieur.
 *
 * L'artiste peint un carré plein ; le moteur le découpe à la forme exacte de la hitbox. Le
 * résultat est mis en cache (`hmi::TextureCache`) plutôt que recalculé à chaque image.
 *
 * Sans effet si @p type n'a pas de silhouette, ou si l'image n'est pas carrée — un skin destiné à
 * un type carré ne doit pas être découpé par accident.
 * @param type   Type de tuile dont on applique la silhouette.
 * @param width  Largeur de l'image, en pixels.
 * @param height Hauteur de l'image, en pixels.
 * @param pixels Pixels `R8G8B8A8_UNORM`, modifiés sur place ; taille attendue `width * height`.
 */
void applySilhouetteMask(core::TileType type, int width, int height,
                         std::vector<std::uint32_t>& pixels);

}  // namespace hmi
