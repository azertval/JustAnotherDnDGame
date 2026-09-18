// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <DirectXMath.h>

#include "HMI/Graphics/ComposedScene.h"
#include "HMI/Graphics/SpriteBatch.h"

/**
 * @file HMI/Graphics/SpriteRenderer.h
 * @brief Soumission d'une scène composée au pipeline de dessin.
 */

namespace hmi {

/**
 * @brief Soumet une scène composée au pipeline de dessin, une passe par groupe de texture.
 *
 * Seul endroit du rendu qui reconvertit une `hmi::TextureHandle` en ressource Direct3D : c'est la
 * **frontière** entre la composition (pure, testable sans GPU) et la soumission. Émet un
 * `SpriteBatch::begin/end` par groupe **contigu** de même texture, dans l'ordre de la scène — donc
 * dans l'ordre des calques, que `ComposedScene::sort()` a rendu prioritaire (`EX-REN-043`). Le
 * contrat public de `hmi::SpriteBatch` est strictement inchangé.
 * @param batch      Pipeline de quads texturés (non possédé).
 * @param projection Matrice de projection monde → clip (fournie par la caméra).
 * @param scene      Scène **déjà triée** (`ComposedScene::sort()`).
 */
void submitComposedScene(SpriteBatch& batch, const DirectX::XMFLOAT4X4& projection,
                         const ComposedScene& scene);

/**
 * @brief Construit la projection **écran → clip**, indépendante de `Camera2D`.
 *
 * Pour ce qui se dessine en pixels d'écran plutôt qu'en unités monde (la galerie des assets) : la
 * projection ne dépend que des dimensions de la surface (origine haut-gauche, `Y` vers le bas —
 * même convention que le reste du rendu).
 * @param viewportWidth  Largeur de la surface de rendu, en pixels.
 * @param viewportHeight Hauteur de la surface de rendu, en pixels.
 */
[[nodiscard]] DirectX::XMFLOAT4X4 screenProjectionMatrix(int viewportWidth,
                                                         int viewportHeight) noexcept;

}  // namespace hmi
