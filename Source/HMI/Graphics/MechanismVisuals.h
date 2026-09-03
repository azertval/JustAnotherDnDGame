// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <optional>
#include <set>
#include <string>
#include <vector>

#include "Core/Levels/TileType.h"
#include "HMI/Graphics/AnimationCatalog.h"  // core::Animation, hmi::AnimationDescription
#include "HMI/Graphics/RenderMode.h"

/**
 * @file HMI/Graphics/MechanismVisuals.h
 * @brief Correspondance état logique d'un mécanisme → nom de clip d'animation (`LOT-47` TACHE-01).
 */

namespace hmi {

/// Noms de clip conventionnels, documentés dans `Source/Elements/Assets/README.md` — c'est le
/// contrat qu'un auteur d'asset doit respecter pour que son animation soit reconnue.
inline constexpr const char* MECHANISM_CLIP_DOOR_CLOSED = "closed";
inline constexpr const char* MECHANISM_CLIP_DOOR_OPENING = "opening";
inline constexpr const char* MECHANISM_CLIP_DOOR_OPEN = "open";
inline constexpr const char* MECHANISM_CLIP_DOOR_CLOSING = "closing";
inline constexpr const char* MECHANISM_CLIP_SWITCH_INACTIVE = "inactive";
inline constexpr const char* MECHANISM_CLIP_SWITCH_ACTIVE = "active";
inline constexpr const char* MECHANISM_CLIP_PLATE_RELEASED = "released";
inline constexpr const char* MECHANISM_CLIP_PLATE_PRESSED = "pressed";
inline constexpr const char* MECHANISM_CLIP_KEY_PRESENT = "present";
inline constexpr const char* MECHANISM_CLIP_KEY_COLLECTED = "collected";
inline constexpr const char* MECHANISM_CLIP_LOCKED_DOOR_CLOSED = "closed";
inline constexpr const char* MECHANISM_CLIP_LOCKED_DOOR_OPEN = "open";

/**
 * @brief Indique si un type de tuile est un **mécanisme à état**, dont l'apparence dépend d'une
 *        donnée lue en lecture seule dans `Core` (`EX-ARCH-012`).
 * @param type Type de tuile.
 * @return true pour `Door`, `Switch`, `PressurePlate`, `Key`, `LockedDoor` (`EX-GP-023`,
 *         `LOT-63`) ; false pour tout autre type
 *         (aucune demande de clip à produire).
 */
[[nodiscard]] bool isStatefulMechanism(core::TileType type) noexcept;

/**
 * @brief Nom du clip d'**état cible** d'un mécanisme, d'après son type et son état logique courant.
 *
 * Fonction **pure** : ni GPU, ni Qt, ni ECS — seulement la table de correspondance (`LOT-47`
 * TACHE-01). Un type sans état renvoie `std::nullopt` : c'est à l'appelant de ne rien demander,
 * plutôt qu'à cette fonction d'inventer une réponse.
 * @param type   Type de tuile.
 * @param active État logique courant : porte ouverte, interrupteur/plaque actionné, danger commuté
 *               actif. Sans effet pour les types sans état (un seul clip, l'état
 *               est porté par la position, pas par ce booléen).
 * @return Le nom du clip d'état cible, ou `std::nullopt` si @p type n'est pas un mécanisme à état.
 */
[[nodiscard]] std::optional<std::string> mechanismTargetClip(core::TileType type,
                                                             bool active) noexcept;

/**
 * @brief Nom du clip de **transition** joué une fois lors d'un changement d'état (`LOT-47`
 *        TACHE-02), s'il y a lieu.
 *
 * Seule `Door` transitionne visiblement (`closed → opening → open`, `open → closing → closed`) ;
 * les autres familles basculent directement sur leur clip d'état cible (`mechanismTargetClip`).
 * Fonction **pure**, ne consulte aucun catalogue d'asset : c'est à l'appelant de vérifier que le
 * clip retourné existe réellement dans le jeu de clips de l'asset assigné, et de replier sur
 * `mechanismTargetClip` sinon (`LOT-47` TACHE-02, repli sur clip manquant).
 * @param type       Type de tuile.
 * @param wasActive  État logique au pas précédent.
 * @param isActive   État logique au pas courant.
 * @return Le nom du clip de transition, ou `std::nullopt` si @p type ne transitionne pas ou que
 *         l'état n'a pas changé.
 */
[[nodiscard]] std::optional<std::string> mechanismTransitionClip(core::TileType type,
                                                                 bool wasActive,
                                                                 bool isActive) noexcept;

/**
 * @brief Noms de clip attendus par la correspondance pour un type de mécanisme, dans l'ordre du
 *        tableau ci-dessus (utilisé par le diagnostic du panneau « Textures », `LOT-47` TACHE-04).
 * @param type Type de tuile.
 * @return Les noms de clip attendus, dans un ordre stable ; vide si @p type n'est pas un mécanisme
 *         à état.
 */
[[nodiscard]] std::vector<std::string> mechanismExpectedClips(core::TileType type);

/**
 * @brief État de présentation d'un mécanisme animé, par **instance** (`LOT-47` TACHE-02).
 *
 * Horloge d'animation propre à cette tuile, indépendante de l'horloge partagée par asset
 * (`LOT-46` TACHE-05, en phase pour toute tuile décorative d'un même asset) : deux portes du même
 * asset peuvent être dans des états différents, elles ne doivent donc pas partager leur image
 * courante. `previousActive` porte l'état logique du pas précédent, pour détecter un changement
 * (front) sans redéclencher la transition tant que l'état ne bouge pas ; `initialized` distingue le
 * tout premier calcul (rejoint l'état cible directement, sans transition) des suivants.
 */
struct MechanismVisualState {
    core::Animation animation;
    bool previousActive = false;
    bool initialized = false;
};

/**
 * @brief Fait progresser l'animation d'**une** instance de mécanisme d'un pas fixe : détecte un
 *        changement d'état, choisit le clip approprié (transition ou état cible, avec repli sur
 *        clip manquant), avance l'image courante, et renvoie la région à afficher (`LOT-47`
 *        TACHE-02).
 *
 * Logique **pure** (`core::Animation`/`core::ClipSet`/`hmi::AnimationDescription` uniquement,
 * aucun GPU/Qt/ECS), testable en isolation. Ne redéclenche **jamais** de transition tant que
 * l'état logique ne change pas — comparaison au pas précédent, portée par @p state. Au tout
 * premier appel pour une instance (`state.initialized == false`, typiquement juste après un
 * chargement/rechargement de niveau), rejoint directement le clip d'état cible, **sans**
 * transition. Un clip de transition absent bascule directement sur le clip d'état cible, sans
 * avertissement (cas légitime) ; un clip d'**état** absent journalise un avertissement, **une
 * seule fois** par combinaison (asset, clip), puis replie sur le premier clip disponible.
 * @param state              État de présentation de cette instance, muté en place.
 * @param description        Description de l'asset effectivement lié à cette instance.
 * @param type                Type de tuile du mécanisme.
 * @param active              État logique courant (voir
 *                            `mechanismTargetClip`).
 * @param assetKey            Identifiant de l'asset (typiquement son chemin), pour dédoublonner le
 *                            journal d'avertissements.
 * @param fixedDelta          Durée du pas de simulation, en secondes.
 * @param warnedMissingClips  Combinaisons (asset, clip d'état) déjà signalées manquantes, mutées en
 *                            place — un seul message par combinaison pour toute la session.
 * @return La région de la spritesheet à afficher pour l'image courante.
 */
[[nodiscard]] core::AtlasRegion advanceMechanismVisual(MechanismVisualState& state,
                                                       const AnimationDescription& description,
                                                       core::TileType type, bool active,
                                                       const std::string& assetKey,
                                                       float fixedDelta,
                                                       std::set<std::string>& warnedMissingClips);

/**
 * @brief Alpha de **diagnostic**, réservé au mode Physique (`LOT-47` TACHE-03).
 *
 * Le mode Physique n'a pas d'assets animés et doit conserver un moyen de distinguer un mécanisme
 * actif d'un mécanisme inactif ; le mode Texture, lui, montre désormais l'état au clip, jamais à
 * l'alpha — la teinte y redevient un pur ajustement colorimétrique (`EX-REN-046`). Fonction
 * **pure**, testable sans GPU : `hmi::GameSession` ne fait que lui fournir le mode courant, l'état
 * et la paire d'alpha propre à la famille de mécanisme concernée (porte : ouverte plus
 * transparente ; danger : actif plus opaque — deux sens opposés, jamais une formule unique).
 * @param mode          Mode de rendu courant.
 * @param active        État logique du mécanisme.
 * @param activeAlpha   Alpha appliqué quand @p active vaut `true`, en mode Physique.
 * @param inactiveAlpha Alpha appliqué quand @p active vaut `false`, en mode Physique.
 * @return `1.0f` en mode Texture (toujours) ; @p activeAlpha ou @p inactiveAlpha en mode Physique.
 */
[[nodiscard]] float mechanismDiagnosticAlpha(RenderMode mode, bool active, float activeAlpha,
                                             float inactiveAlpha) noexcept;

}  // namespace hmi
