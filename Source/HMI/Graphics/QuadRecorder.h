// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include <string>
#include <vector>

#include "HMI/Graphics/ComposedScene.h"

/**
 * @file HMI/Graphics/QuadRecorder.h
 * @brief Capture et inspection des primitives composées pour une image (`EX-NFR-004`).
 */

namespace hmi {

/**
 * @brief Capture la liste des primitives d'une image et répond aux questions qu'un test se pose.
 *
 * Outil de **vérification et de diagnostic**, jamais un détour du rendu : la production compose
 * dans une `hmi::ComposedScene` et la soumet directement ; le recorder ne fait qu'en **copier** le
 * résultat, à la demande, hors du chemin de dessin. Il n'y a donc aucun coût en production.
 *
 * Il existe parce que l'habillage empile six couches dont
 * l'ordre, la priorité de résolution et l'isolement sont exactement le genre de règle qui casse
 * silencieusement, et dont les critères d'acceptation (« rendu identique », « ordre de calque
 * correct ») seraient sinon des vérifications à l'œil. Les prédicats offerts ici — ordre des
 * calques, contiguïté des groupes de texture, dénombrement par calque, présence d'une primitive à
 * une position — sont la formulation **assertable** de ces critères, réutilisable par tous les
 * lots suivants (`EX-NFR-004`, `EX-NFR-010`).
 *
 * On assertionne des **listes de primitives**, jamais des pixels : c'est précisément ce qui rend la
 * vérification possible sans GPU.
 */
class QuadRecorder {
public:
    /**
     * @brief Capture l'état d'une scène composée (copie : la scène peut ensuite être réutilisée).
     * @param scene Scène dont on capture les primitives et les compteurs.
     */
    void record(const ComposedScene& scene);

    /// Oublie la capture courante.
    void clear() noexcept;

    /// @return Les primitives capturées, dans leur ordre de soumission.
    [[nodiscard]] const std::vector<ComposedQuad>& quads() const noexcept {
        return _quads;
    }

    /// @return Le nombre de primitives capturées.
    [[nodiscard]] std::size_t size() const noexcept {
        return _quads.size();
    }

    /// @return Les compteurs de l'image capturée (composées, écartées, soumises, passes).
    [[nodiscard]] const SceneStatistics& statistics() const noexcept {
        return _statistics;
    }

    /**
     * @brief Vérifie que l'ordre des calques est respecté (`EX-REN-014`).
     * @return `true` si aucune primitive n'est soumise avant une primitive d'un calque
     *         **inférieur**, quel que soit l'ordre de composition.
     */
    [[nodiscard]] bool isLayerOrderRespected() const;

    /**
     * @brief Vérifie que chaque texture forme un seul groupe **contigu** par calque
     *        (`EX-REN-043`).
     *
     * C'est la propriété qui garantit qu'une passe `begin/end` par groupe suffit : une texture qui
     * réapparaîtrait plus loin **dans le même calque** imposerait une passe de plus, et signalerait
     * un tri instable. La contiguïté ne vaut qu'à calque constant : une même texture utilisée sur
     * deux calques différents produit légitimement deux passes, le calque primant sur la texture.
     * @return `true` si aucune texture n'apparaît dans deux groupes distincts d'un même calque.
     */
    [[nodiscard]] bool areTextureGroupsContiguous() const;

    /**
     * @brief Suite des calques rencontrés, sans répétition consécutive.
     * @return Les calques dans l'ordre de première apparition contiguë (ex. `Tile`, `Player`).
     */
    [[nodiscard]] std::vector<RenderLayer> layerSequence() const;

    /**
     * @brief Suite des textures liées, sans répétition consécutive : une entrée par passe
     *        `begin/end`.
     * @return Les identités de texture, dans l'ordre des passes.
     */
    [[nodiscard]] std::vector<TextureHandle> textureSequence() const;

    /**
     * @brief Nombre de primitives capturées sur un calque donné.
     * @param layer Calque compté.
     * @return Le nombre de primitives de ce calque.
     */
    [[nodiscard]] int countOnLayer(RenderLayer layer) const;

    /**
     * @brief Nombre de primitives capturées liées à une texture donnée.
     * @param texture Texture comptée.
     * @return Le nombre de primitives utilisant cette texture.
     */
    [[nodiscard]] int countWithTexture(TextureHandle texture) const;

    /**
     * @brief Cherche un rectangle composé à une position monde donnée.
     * @param x         Abscisse attendue du coin haut-gauche, en unités monde.
     * @param y         Ordonnée attendue du coin haut-gauche, en unités monde.
     * @param tolerance Écart absolu toléré sur chaque axe (comparaison de flottants).
     * @return `true` si au moins un `QuadKind::Sprite` capturé est à cette position.
     */
    [[nodiscard]] bool containsSpriteAt(float x, float y, float tolerance = 1e-4f) const;

    /**
     * @brief Résumé lisible de la capture, à joindre au message d'échec d'une assertion.
     * @return Une ligne par passe : calque, texture et nombre de primitives.
     */
    [[nodiscard]] std::string describe() const;

private:
    std::vector<ComposedQuad> _quads;
    SceneStatistics _statistics;
};

}  // namespace hmi
