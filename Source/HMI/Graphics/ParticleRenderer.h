// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include "HMI/Graphics/ComposedScene.h"
#include "HMI/Graphics/RenderMode.h"
#include "HMI/Graphics/TileAppearance.h"

/**
 * @file HMI/Graphics/ParticleRenderer.h
 * @brief Rendu des particules du personnage, sur les calques `Object`/`Foreground` (`LOT-53`
 *        TACHE-03, `EX-REN-008`).
 */

namespace core {
class World;
}  // namespace core

namespace hmi {

/// Côté du carré teinté d'une particule, en unités monde (`LOT-53` TACHE-03 : « un simple carré
/// teinté suffit pour un premier jet », pas d'asset dédié).
inline constexpr float PARTICLE_QUAD_SIZE = 0.12f;

/**
 * @brief Compose un quad par particule vivante, teinté selon la durée de vie restante et le
 *        calque de son effet.
 *
 * Parcourt les entités portant un `core::Particle` (`LOT-53` TACHE-01/02) et empile un
 * `hmi::SpriteQuad` centré sur chacune, échantillonnant une région **opaque unie** de l'atlas
 * (`hmi::TextureAtlas::tile(0, 0)`, même patron que les segments unis de `hmi::DraftRenderer`) :
 * aucun asset dédié n'est introduit, la teinte porte entièrement l'apparence de l'effet.
 * L'opacité vaut `life / maxLife` (`core::Particle`) : une particule fraîchement émise est pleine,
 * elle s'estompe jusqu'à disparaître avec la simulation. Le calque dépend de l'effet
 * (`core::ParticleKind`) : une traînée de dash passe **derrière** le personnage
 * (`RenderLayer::Object`), une bouffée de poussière ou un éclat de mort **devant**
 * (`RenderLayer::Foreground`), conformément au périmètre du lot.
 *
 * Lecture **seule** de l'ECS (`EX-ARCH-012`), sans effet en dehors de `RenderMode::Texture`
 * (`EX-REN-046`) — le mode Physique reste la lecture nue des collisions, sans effet visuel.
 * @param scene    Scène à remplir (non vidée : la composition peut être cumulative).
 * @param world    Monde dont on lit `core::Particle`.
 * @param mode     Mode de rendu courant.
 * @param textures Textures liables (seul `hmi::SceneTextures::atlas` est utilisé).
 */
void composeParticles(ComposedScene& scene, core::World& world, RenderMode mode,
                      const SceneTextures& textures);

}  // namespace hmi
