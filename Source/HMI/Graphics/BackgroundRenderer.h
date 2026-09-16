// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include "HMI/Graphics/ComposedScene.h"
#include "HMI/Graphics/RenderMode.h"

/**
 * @file HMI/Graphics/BackgroundRenderer.h
 * @brief Composition du fond de niveau sur le calque `RenderLayer::Background` (`LOT-44`,
 *        `EX-REN-044`).
 */

namespace hmi {

/// Recadrage UV d'un fond, calculé pour préserver son ratio d'aspect (`LOT-44`).
struct BackgroundFit {
    float u0 = 0.0f;
    float v0 = 0.0f;
    float u1 = 1.0f;
    float v1 = 1.0f;
};

/**
 * @brief Calcule le recadrage UV d'une image pour couvrir un rectangle cible sans déformation.
 *
 * Fonction **pure**, testable sans GPU (`EX-NFR-010`) : ratio d'aspect **préservé**, recadrage par
 * le **centre** plutôt qu'étirement libre (décision de cadrage `LOT-44`) — l'image déborde sur la
 * dimension excédentaire. Équivalent du motif CSS `background-size: cover`, exprimé en
 * coordonnées UV plutôt qu'en pixels écran. Aucun recadrage (UV `[0,1]` complet) quand les deux
 * ratios coïncident.
 * @param imageWidth   Largeur de l'image source, en pixels (> 0).
 * @param imageHeight  Hauteur de l'image source, en pixels (> 0).
 * @param targetWidth  Largeur du rectangle à couvrir, unités cohérentes avec @p targetHeight (> 0).
 * @param targetHeight Hauteur du rectangle à couvrir (> 0).
 * @return Les UV `[u0,v0]`-`[u1,v1]` à échantillonner dans l'image.
 */
[[nodiscard]] BackgroundFit computeBackgroundFit(float imageWidth, float imageHeight,
                                                 float targetWidth, float targetHeight) noexcept;

/**
 * @brief Texture de fond déjà résolue, prête à être composée.
 *
 * Sépare la **résolution** (accès disque/GPU via `hmi::TextureCache`,
 * `hmi::resolveBackgroundTexture`, déclarée dans `HMI/Graphics/SpriteRenderer.h` aux côtés de
 * `hmi::sceneTextures`) de la **composition** (`composeBackground`, pure) — même patron que
 * `hmi::SceneTextures` vis-à-vis de `hmi::composeWorldSprites` : c'est ce qui rend la composition
 * testable sans GPU (`EX-NFR-004`).
 */
struct BackgroundTexture {
    /// Texture à lier : l'asset réel si trouvé, le damier de repli sinon. `nullptr` si aucun fond
    /// n'est désigné pour le niveau (état normal, à ne pas confondre avec un fond introuvable).
    TextureHandle texture = nullptr;
    /// Largeur de la texture, en pixels.
    int width = 0;
    /// Hauteur de la texture, en pixels.
    int height = 0;
};

/**
 * @brief Compose le fond du niveau sur `RenderLayer::Background`, s'il y a lieu.
 *
 * Fonction **pure** (aucun accès disque/GPU), testable via `hmi::QuadRecorder` (`EX-NFR-004`).
 * Trois cas distincts (`LOT-44`), à ne pas confondre :
 * - `background.texture == nullptr` (aucun fond désigné) → **rien** n'est émis, aucune anomalie ;
 * - fond désigné et chargeable → un quad est émis, étiré sur les bornes du niveau, ratio préservé
 *   (`computeBackgroundFit`) ;
 * - fond désigné mais introuvable/invalide → `background.texture` porte déjà le damier de repli
 *   (résolu par `resolveBackgroundTexture`) : un quad de repli est émis identiquement.
 *
 * Rien n'est jamais émis en mode `RenderMode::Physique` (`EX-REN-046`) : la lecture des collisions
 * reste sans distraction. Le quad, en espace **niveau**, est soumis au culling par boîte
 * englobante comme toute autre primitive de `hmi::ComposedScene` (`EX-NFR-005`).
 * @param scene       Scène composée, cible de l'ajout.
 * @param background  Texture de fond déjà résolue (`resolveBackgroundTexture`).
 * @param levelWidth  Largeur du niveau, en unités monde (une case = une unité).
 * @param levelHeight Hauteur du niveau, en unités monde.
 * @param mode        Mode de rendu courant ; sans effet hors `RenderMode::Texture`.
 */
void composeBackground(ComposedScene& scene, const BackgroundTexture& background, int levelWidth,
                       int levelHeight, RenderMode mode);

}  // namespace hmi
