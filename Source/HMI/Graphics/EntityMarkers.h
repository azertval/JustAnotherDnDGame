// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

/**
 * @file HMI/Graphics/EntityMarkers.h
 * @brief Le marqueur d'une entité de carte : sa clé d'asset et ses pixels (`LOT-11`, `LOT-39`).
 */

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace core {
struct MarkerImage;
}

namespace hmi {

/// @brief Famille de clé d'asset des marqueurs d'entité : `marker/<type>`.
inline constexpr std::string_view ENTITY_MARKER_FAMILY = "marker";

/// @brief Identifiant retenu quand le type d'entité ne laisse **aucun** caractère utilisable (type
///        vide, ou fait uniquement de ponctuation) : `marker/inconnu`.
inline constexpr std::string_view ENTITY_MARKER_UNKNOWN_ID = "inconnu";

/// @brief Côté d'un marqueur d'entité, en pixels : **une case** (`Camera2D::PIXELS_PER_UNIT`).
///        Le marqueur occupe exactement la case de l'entité, comme une tuile.
inline constexpr int ENTITY_MARKER_SIZE_PIXELS = 16;

/**
 * @brief La clé d'asset du marqueur d'un type d'entité de carte.
 *
 * Un type d'entité est libre (`core::MapEntity::type`) et s'écrit en `camelCase` (`spawnPoint`,
 * `arenaEntry`) ; une clé d'asset n'admet que minuscules, chiffres et tirets simples
 * (`core::isValidAssetKey`). La règle de conversion, nommée ici parce que l'éditeur et l'essai
 * immédiat doivent donner **le même** marqueur au même type :
 *
 * - une **majuscule** ouvre un nouveau mot quand elle suit une minuscule ou un chiffre, ou quand
 *   elle précède une minuscule après une autre majuscule (`NPCGuard` → `npc-guard`) ; elle est
 *   écrite en minuscule ;
 * - `-`, `_` et l'espace **séparent** deux mots ;
 * - tout autre caractère (ponctuation, non-ASCII) est **ignoré** ;
 * - les tirets en tête, en fin et répétés sont supprimés ;
 * - un résultat vide donne `marker/inconnu` (`ENTITY_MARKER_UNKNOWN_ID`).
 *
 * La clé rendue est donc **toujours** valide : un type mal écrit se voit avec un marqueur plutôt
 * que de disparaître (`EX-NFR-040`). Deux types qui ne diffèrent que par la ponctuation partagent
 * leur marqueur — assumé : c'est une faute de frappe, pas deux familles.
 */
[[nodiscard]] std::string entityMarkerKey(std::string_view entityType);

/**
 * @brief Les pixels d'un marqueur au format que `hmi::createTexture` attend : un `std::uint32_t`
 *        par pixel, `R | G<<8 | B<<16 | A<<24` (RGBA8 petit-boutiste), ligne par ligne depuis le
 *        haut.
 * @return `width * height` pixels, ou un vecteur vide pour une image vide.
 */
[[nodiscard]] std::vector<std::uint32_t> markerPixelsRgba8(const core::MarkerImage& image);

}  // namespace hmi
