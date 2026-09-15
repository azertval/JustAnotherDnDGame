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

/**
 * @brief Identité des trois clips historiques du personnage, **côté présentation seulement**.
 *
 * Distinct de `core::AnimationClip` (donnée générique, `LOT-46`) : l'atlas procédural reste le
 * repli et la **référence** visuelle du personnage (`LOT-39`), peint « en dur » à partir de trois
 * poses connues à la compilation — `Core` n'a, depuis `LOT-46`, plus aucune connaissance de ces
 * trois clips en particulier (`EX-ARCH-012`). L'ordre des valeurs est celui de
 * `core::playerClipSet()` (`Core/Ecs/Systems/AnimationSystem.h`, `PLAYER_CLIP_IDLE`/`_RUN`/
 * `_JUMP`) : c'est `HMI` (`GameSession::refreshPlayerSprite`) qui fait le pont entre l'index de
 * clip résolu par `Core` et cette énumération.
 */
enum class PlayerClipKind { Idle, Run, Jump };

/// Nombre d'images du clip `Idle`. Seule source de vérité (`LOT-18`, migré côté `HMI` par `LOT-46`
/// puisque seul l'atlas procédural en a encore besoin pour son placement fixe).
constexpr int PLAYER_IDLE_FRAME_COUNT = 2;
/// Nombre d'images du clip `Run`.
constexpr int PLAYER_RUN_FRAME_COUNT = 4;
/// Nombre d'images du clip `Jump` (pose unique, jamais animée).
constexpr int PLAYER_JUMP_FRAME_COUNT = 1;

/**
 * @brief Index à plat (0-based) d'une image dans la grille sous les tuiles : `Idle`
 *        (0..PLAYER_IDLE_FRAME_COUNT-1), puis `Run`, puis `Jump`.
 *
 * **Seule source de vérité** de cet ordre, partagée par `buildProceduralAtlasImage` (placement
 * des images) et par `TextureAtlas::playerFrameRegion` (résolution de la même région) — les
 * dupliquer aux deux endroits risquerait de les laisser diverger silencieusement.
 * @param clip       Clip d'animation du personnage.
 * @param frameIndex Index de l'image dans le clip (0-based).
 * @return L'index à plat de cette image dans la grille.
 */
[[nodiscard]] int flatPlayerFrameIndex(PlayerClipKind clip, int frameIndex);

/// Pixels d'un atlas généré en mémoire, au format `R8G8B8A8_UNORM` (ordre mémoire R,G,B,A).
struct ProceduralAtlasImage {
    int width = 0;
    int height = 0;
    /// Taille `width * height`, ligne par ligne (haut en bas), pixel `pack(r,g,b,a)`.
    std::vector<std::uint32_t> pixels;
};

/**
 * @brief Génère, en mémoire, l'atlas procédural historique (grille de tuiles + images du
 *        personnage), de façon **déterministe**.
 *
 * Logique pure (`EX-NFR-010`), sans dépendance Direct3D/Qt : utilisée à la fois par
 * `hmi::TextureAtlas` (repli si l'asset fichier est absent/illisible, `EX-NFR-040`) et par l'export
 * ponctuel de l'atlas de base en fichier (`--export-atlas`, cf. `main.cpp`), pour que les deux
 * chemins restent l'unique source de vérité de ce contenu.
 * @return L'image générée (dimensions et pixels).
 */
[[nodiscard]] ProceduralAtlasImage buildProceduralAtlasImage();

}  // namespace hmi
