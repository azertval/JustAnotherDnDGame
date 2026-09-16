// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "Core/Math/Vector2.h"

/**
 * @file HMI/Graphics/PlayerSprite.h
 * @brief Logique pure de l'habillage du personnage par spritesheet externe (`LOT-48`).
 */

namespace hmi {

/// Sous-dossier de la spritesheet du personnage, relatif au dossier d'assets (`LOT-48`).
inline const std::string PLAYER_SUBDIRECTORY = "Player/";
/// Nom logique de la spritesheet du personnage (`LOT-48`).
inline const std::string PLAYER_SHEET_FILE_NAME = "player.png";

/**
 * @brief Position et taille du quad d'affichage du personnage, en unités monde, relatives au coin
 *        haut-gauche de sa hitbox (`Transform::position`).
 */
struct PlayerSpriteQuad {
    /// Décalage du coin haut-gauche de l'image depuis celui de la hitbox.
    core::Vector2 offset{};
    /// Taille de l'image à l'écran, en unités monde.
    core::Vector2 size{};
};

/**
 * @brief Calcule le quad d'affichage du personnage à partir de la taille de l'image et de la
 *        hitbox, par ancrage **centre-bas** (`LOT-48` TACHE-01).
 *
 * Fonction **pure**, testable sans GPU (`EX-NFR-010`) : le centre-bas de l'image est aligné sur le
 * centre-bas de la hitbox (`core::playerSize()`), indépendamment de la taille de l'image — une
 * image plus grande ou plus large que la hitbox ne déplace donc jamais celle-ci (`EX-ARCH-012`,
 * la hitbox elle-même n'est **jamais** lue ni modifiée ici, seulement sa taille). L'ancrage est
 * **symétrique** horizontalement : le retournement (`LOT-48` TACHE-03) n'a besoin d'aucune
 * correction de position, seulement d'un échange des coordonnées de texture.
 * @param imageSizePixels Taille de l'image à afficher (une image de la spritesheet, ou de l'image
 *                        procédurale, toujours carrée), en pixels.
 * @param hitboxSize      Taille de la boîte de collision (`core::playerSize()`), en unités monde.
 * @return Le décalage et la taille du quad, en unités monde, relatifs au coin haut-gauche de la
 *         hitbox.
 */
[[nodiscard]] PlayerSpriteQuad computePlayerSpriteQuad(core::Vector2 imageSizePixels,
                                                       core::Vector2 hitboxSize);

/// Noms des clips que l'atlas procédural sait dessiner (`Idle`/`Run`/`Jump`, `LOT-18`) : seule
/// source de vérité, partagée avec `hmi::PlayerClipKind` — traitée comme une « spritesheet »
/// n'en déclarant que trois par `hmi::resolveDeclaredPlayerClip`.
[[nodiscard]] const std::vector<std::string>& proceduralPlayerClipNames();

/**
 * @brief Résout un nom de clip demandé vers le plus proche **déclaré** dans un jeu partiel
 *        (`LOT-48` TACHE-01/TACHE-02).
 *
 * Une spritesheet externe — ou l'atlas procédural, qui n'en déclare que trois
 * (`hmi::proceduralPlayerClipNames`) — peut ne pas fournir tous les clips que
 * `core::AnimationSystem` projette (`fall`, `land`, `wallslide`, `dash`). Chaîne de repli
 * **fixe**, documentée ici comme seule source de vérité : `fall → jump`, `land → idle`,
 * `wallslide → jump`, `dash → run`, `run → idle`, `jump → idle` ; `idle` est le dernier recours
 * (toujours supposé déclaré). Fonction **pure**, testable sans GPU.
 * @param declaredNames Noms de clips effectivement fournis par la source consultée.
 * @param requested     Nom de clip demandé (résolu par `core::AnimationSystem`).
 * @return Le nom du clip à effectivement afficher : @p requested s'il est déclaré, sinon le
 *         premier nom de la chaîne de repli qui l'est ; `"idle"` en dernier recours.
 */
[[nodiscard]] std::string resolveDeclaredPlayerClip(const std::vector<std::string>& declaredNames,
                                                    std::string_view requested);

}  // namespace hmi
