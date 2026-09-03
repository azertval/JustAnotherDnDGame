// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <memory>

#include "Core/Ecs/AnimationClip.h"
#include "Core/Ecs/Components/Animation.h"
#include "Core/Ecs/ISystem.h"

/**
 * @file Core/Ecs/Systems/AnimationSystem.h
 * @brief Système d'animation générique, piloté par données (`LOT-46`).
 */

namespace core {

/**
 * @brief Jeu de clips du personnage, migré tel quel depuis l'ancien dispositif figé (`LOT-18`),
 *        étendu aux états aériens/dash livrés par `LOT-10`/`LOT-11` (`LOT-48` TACHE-02).
 *
 * Sept clips nommés : « idle », « run », « jump » (mêmes durées/nombres d'images qu'avant
 * `LOT-46`, `LOT-46` TACHE-04), plus « fall », « land », « wallslide », « dash » (`LOT-48`). Ces
 * quatre nouveaux clips n'ont pas de pose procédurale dédiée (`hmi::PlayerClipKind` reste limité à
 * trois poses) : c'est `HMI` (`hmi::resolveDeclaredPlayerClip`) qui les fait retomber sur le clip
 * le plus proche déclaré, aussi bien pour l'atlas procédural que pour une spritesheet externe
 * partielle — `Core` n'a pas à le savoir (`EX-ARCH-012`).
 *
 * Instance **partagée** (même `shared_ptr` renvoyé à chaque appel) : tous les personnages d'une
 * session utilisent le même jeu de clips, immuable.
 * @return Le jeu de clips du personnage.
 */
[[nodiscard]] std::shared_ptr<const ClipSet> playerClipSet();

/// Index du clip « idle » dans `core::playerClipSet()` — ordre **stable**, seule source de vérité
/// partagée avec `hmi::TextureAtlas::playerFrameRegion` (traduction en région d'atlas, `LOT-46`
/// TACHE-04) : ne pas réordonner sans mettre à jour `HMI` en conséquence.
constexpr int PLAYER_CLIP_IDLE = 0;
/// Index du clip « run » dans `core::playerClipSet()` (voir `PLAYER_CLIP_IDLE`).
constexpr int PLAYER_CLIP_RUN = 1;
/// Index du clip « jump » dans `core::playerClipSet()` (voir `PLAYER_CLIP_IDLE`).
constexpr int PLAYER_CLIP_JUMP = 2;
/// Index du clip « fall » (chute, distincte du saut, `LOT-48`) dans `core::playerClipSet()`.
constexpr int PLAYER_CLIP_FALL = 3;
/// Index du clip « land » (atterrissage, transition jouée une fois, `LOT-48`) dans
/// `core::playerClipSet()`.
constexpr int PLAYER_CLIP_LAND = 4;
/// Index du clip « wallslide » (glissade murale, `LOT-48`) dans `core::playerClipSet()`.
constexpr int PLAYER_CLIP_WALLSLIDE = 5;
/// Index du clip « dash » dans `core::playerClipSet()` (voir `PLAYER_CLIP_IDLE`).
constexpr int PLAYER_CLIP_DASH = 6;

/**
 * @brief Fait progresser l'animation de toute entité portant `core::Animation`, et projette
 *        l'état physique du personnage sur son clip cible.
 *
 * Deux responsabilités distinctes (`LOT-46` TACHE-02) :
 * - **projection**, spécifique au personnage : pour chaque entité `Actor` + `Velocity` +
 *   `Animation`, détermine le clip actif d'après la **norme** de la vitesse (`LOT-06`,
 *   les deux axes comptant pareil) : au-delà du seuil `MOVING_THRESHOLD`, « run » ; en deçà,
 *   « idle ». Les clips aériens du platformer restent **déclarés** — les poses procédurales et les
 *   spritesheets externes s'y accrochent — mais plus rien ne les sélectionne, faute d'un état qui
 *   puisse les justifier ; c'est le `LOT-08` qui refera ce vocabulaire pour le RPG ;
 * - **progression**, générale : pour **toute** entité portant `core::Animation` (personnage,
 *   tuiles animées, `LOT-46` TACHE-05), avance l'image courante selon la durée du clip résolu,
 *   boucle ou bascule sur le clip suivant en fin de clip joué une fois (`core::ClipEndMode::
 *   OneShot`).
 *
 * Un changement de clip réinitialise immédiatement l'image à 0 (pas de transition progressive).
 * Toute la logique vit ici ; les composants restent des **données pures** (`EX-ARCH-011`).
 * Déterministe au pas fixe (`EX-NFR-002`), testable sans GPU (`EX-NFR-010`).
 *
 * Doit s'exécuter **après** le système de déplacement dans la boucle d'un écran : il lit la
 * vitesse calculée par le déplacement pour le **même** pas.
 */
class AnimationSystem : public ISystem {
public:
    /**
     * @brief Applique un pas de simulation d'animation à toutes les entités animées.
     * @param world      Monde ECS.
     * @param fixedDelta Durée du pas de simulation, en secondes.
     */
    void update(World& world, float fixedDelta) override;
};

/**
 * @brief Fait progresser une seule animation d'un pas fixe (image courante, bouclage/bascule).
 *
 * Extrait de `AnimationSystem::update` pour être réutilisable **hors ECS** : `LOT-46` TACHE-05
 * anime des tuiles partageant un même asset (eau, lave) via une horloge unique par asset plutôt
 * qu'une entité par case, précisément pour éviter de répéter cette progression une fois par case.
 * Sans effet si `animation.clips` est nul ou vide (`EX-NFR-040`).
 * @param animation  État d'animation à faire progresser.
 * @param fixedDelta Durée du pas de simulation, en secondes.
 */
void advanceAnimation(Animation& animation, float fixedDelta);

}  // namespace core
