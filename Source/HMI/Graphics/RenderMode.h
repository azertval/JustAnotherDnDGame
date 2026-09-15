// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <string>

/**
 * @file HMI/Graphics/RenderMode.h
 * @brief Mode de rendu Physique/Texture et sa persistance (`EX-REN-046`).
 */

namespace hmi {

/**
 * @brief Mode de rendu de la scène : lecture directe des collisions, ou habillage.
 *
 * Deux valeurs, jamais trois : la superposition et le contrôle **fin par calque** relèvent de
 * `LOT-51` et sont délibérément une autre notion. La bascule est purement **visuelle** — elle ne
 * touche ni la simulation, ni la scène ECS, seulement la résolution de l'apparence
 * (`hmi::resolveTileAppearance`).
 */
enum class RenderMode {
    /// Couleur plate par type de tuile : la géométrie de collision se lit directement à l'écran.
    Physique,
    /// Habillage par textures, construit lot après lot à partir de `LOT-42`. Tant qu'aucun skin
    /// n'existe, tout s'affiche avec le damier de repli — c'est le comportement attendu, pas un
    /// défaut (`EX-NFR-040`).
    Texture,
};

/**
 * @brief Mode appliqué au premier lancement et à défaut de préférence exploitable.
 *
 * `Texture` dans **toutes** les configurations de build (`EX-REN-046`) : deux binaires du même
 * code ne doivent jamais afficher un rendu différent par défaut, sous peine de rendre ambiguë
 * toute capture d'écran ou vérification visuelle.
 */
inline constexpr RenderMode DEFAULT_RENDER_MODE = RenderMode::Texture;

/**
 * @brief Nom persisté d'un mode de rendu (valeur écrite dans les préférences).
 * @param mode Mode à nommer.
 * @return Le nom stable du mode (chaîne statique, jamais nulle).
 */
[[nodiscard]] const char* renderModeName(RenderMode mode) noexcept;

/**
 * @brief Mode de rendu correspondant à un nom persisté.
 *
 * Fonction **pure** (aucune dépendance Qt ni GPU), donc testable en isolation : une préférence
 * absente, vide ou corrompue est un cas **attendu**, pas une erreur — elle retombe silencieusement
 * sur `hmi::DEFAULT_RENDER_MODE` plutôt que de refuser de démarrer.
 * @param name Nom lu dans les préférences (comparaison insensible à la casse).
 * @return Le mode correspondant, ou `hmi::DEFAULT_RENDER_MODE` si @p name n'en désigne aucun.
 */
[[nodiscard]] RenderMode renderModeFromName(const std::string& name) noexcept;

}  // namespace hmi
