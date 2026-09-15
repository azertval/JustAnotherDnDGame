// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <cstdint>
#include <string_view>

#include <DirectXMath.h>

#include "Core/Ecs/Components/Sprite.h"  // core::Color
#include "HMI/Graphics/ComposedScene.h"
#include "HMI/Graphics/ProceduralFont.h"  // hmi::FontMetrics
#include "HMI/Graphics/RenderLayer.h"

/**
 * @file HMI/Graphics/TextRenderer.h
 * @brief Composition d'une chaîne en quads sur le calque `UI`, en espace écran (`LOT-52`).
 */

namespace hmi {

class BitmapFont;

/// Ancrage horizontal d'un texte composé (`hmi::composeText`).
enum class TextHorizontalAnchor { Left, Center, Right };

/// Ancrage vertical d'un texte composé (`hmi::composeText`).
enum class TextVerticalAnchor { Top, Middle, Bottom };

/// Point d'ancrage d'un texte : quel coin/bord de sa boîte englobante coïncide avec la position
/// passée à `hmi::composeText` — centrer un texte sans devoir le mesurer soi-même.
struct TextAnchor {
    TextHorizontalAnchor horizontal = TextHorizontalAnchor::Left;
    TextVerticalAnchor vertical = TextVerticalAnchor::Top;
};

/**
 * @brief Construit la projection **écran → clip** du calque `UI`, indépendante de `Camera2D`.
 *
 * Le HUD ne doit ni tourner ni changer de taille avec le zoom de la caméra du monde
 * (`EX-ARCH-022`) : cette projection ne dépend que des dimensions du viewport, en pixels écran
 * (origine haut-gauche, `Y` vers le bas — même convention que le reste du rendu).
 * @param viewportWidth  Largeur de la surface de rendu, en pixels.
 * @param viewportHeight Hauteur de la surface de rendu, en pixels.
 * @return La matrice de projection, à passer à `hmi::submitComposedScene` pour la passe `UI`.
 */
[[nodiscard]] DirectX::XMFLOAT4X4 screenProjectionMatrix(int viewportWidth,
                                                         int viewportHeight) noexcept;

/**
 * @brief Compose une chaîne en quads sur le calque `UI`, ancrée à une position **écran**.
 *
 * Émet un `hmi::SpriteQuad` par point de code couvert par `metrics` (substitution déterministe
 * d'un caractère non couvert, `EX-NFR-040` — jamais un caractère ignoré silencieusement, sauf
 * l'espace, jamais dessinée : elle n'a aucun pixel visible, l'émettre serait un quad pour rien).
 * Positions arrondies au pixel écran entier (netteté, `EX-ARCH-022`) ; un retour à la ligne (`\n`)
 * revient à l'abscisse d'ancrage, une `lineHeight` plus bas.
 *
 * @warning `scene` **ne doit avoir aucun cadrage de culling actif**
 *          (`hmi::ComposedScene::isCullingEnabled() == false`, l'état par défaut) : le texte est
 *          en espace écran, sans position monde — le tester contre un cadrage caméra (unités
 *          monde) écarterait systématiquement les quads composés ici (`EX-NFR-005` ne s'applique
 *          pas au calque `UI`). Composer le HUD dans une `ComposedScene` **dédiée**, jamais
 *          partagée avec la scène monde soumise par `hmi::SpriteRenderer`.
 * @param scene     Scène composée, remplie (pas vidée : la composition peut être cumulative).
 * @param metrics   Métriques de la police utilisée (`hmi::BitmapFont::metrics`).
 * @param texture   Texture de l'atlas de glyphes (`hmi::BitmapFont::textureHandle`).
 * @param textureWidth  Largeur de l'atlas de glyphes, en pixels (normalisation UV).
 * @param textureHeight Hauteur de l'atlas de glyphes, en pixels (normalisation UV).
 * @param text      Texte à composer, encodé en UTF-8.
 * @param x         Abscisse écran du point d'ancrage, en pixels.
 * @param y         Ordonnée écran du point d'ancrage, en pixels.
 * @param scale     Facteur d'échelle appliqué aux glyphes (entier recommandé, netteté pixel art).
 * @param tint      Teinte appliquée aux quads (le nuanceur multiplie texture × couleur).
 * @param anchor    Point de la boîte englobante du texte qui coïncide avec (@p x, @p y).
 * @param sortOrder Tri fin à l'intérieur du calque `UI` et de la texture de police.
 */
void composeText(ComposedScene& scene, const FontMetrics& metrics, TextureHandle texture,
                 int textureWidth, int textureHeight, std::string_view text, float x, float y,
                 float scale, const core::Color& tint, TextAnchor anchor = {},
                 std::int32_t sortOrder = 0);

/**
 * @brief Raccourci de `hmi::composeText` prenant directement une `hmi::BitmapFont`.
 * @param scene  Scène composée (mêmes contraintes que la surcharge ci-dessus).
 * @param font   Police chargée (fournit métriques et texture).
 * @param text   Texte à composer, en UTF-8.
 * @param x      Abscisse écran du point d'ancrage, en pixels.
 * @param y      Ordonnée écran du point d'ancrage, en pixels.
 * @param scale  Facteur d'échelle appliqué aux glyphes.
 * @param tint   Teinte appliquée aux quads.
 * @param anchor Point de la boîte englobante du texte qui coïncide avec (@p x, @p y).
 * @param sortOrder Tri fin à l'intérieur du calque `UI` et de la texture de police.
 */
void composeText(ComposedScene& scene, const BitmapFont& font, std::string_view text, float x,
                 float y, float scale, const core::Color& tint, TextAnchor anchor = {},
                 std::int32_t sortOrder = 0);

}  // namespace hmi
