// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include "Core/Levels/CameraFraming.h"
#include "Core/Math/Rect.h"
#include "Core/Math/Vector2.h"

/**
 * @file HMI/Graphics/Parallax.h
 * @brief Décalage de parallaxe, bornage d'un plan et décision d'activation (`EX-DEC-043`, LOT-69).
 *
 * Le facteur n'est plus une constante par couche : il est **porté par le plan** (`core::Plane`),
 * réglable niveau par niveau, et **par axe**. Ces fonctions ne connaissent que des nombres — elles
 * ignorent d'où ils viennent.
 */

namespace hmi {

/// Facteur **neutre** : le contenu est solidaire du niveau et ne subit aucun décalage.
inline constexpr float PARALLAX_FACTOR_NONE = 1.0f;

/**
 * @brief Calcule la position de **rendu** d'un contenu, décalée par sa parallaxe.
 *
 * Fonction **pure**, sans état (LOT-49 TACHE-03) : le décalage est calculé **relativement au
 * centre de la salle courante** (@p cameraBounds, le rectangle cadré par la caméra — solidaire de
 * la salle, `EX-REN-015`) et non en espace niveau absolu. Formulation : la position de rendu est
 * le centre de la salle, plus le déplacement du contenu **par rapport à ce centre**, mis à
 * l'échelle du facteur — de sorte qu'à facteur `1.0` (`PARALLAX_FACTOR_NONE`) la position est
 * **inchangée**, et que deux contenus à la même position relative dans deux salles différentes se
 * retrouvent au même endroit à l'écran, quelle que soit la position absolue de chaque salle.
 *
 * C'est le seul comportement cohérent avec une caméra à coupure nette entre salles
 * (`EX-REN-015`, `LOT-32`) : un décalage absolu en espace niveau ferait sauter le contenu à chaque
 * bascule (artefact visible) ; ce décalage-ci se replace au moment exact où toute l'image change
 * déjà (invisible). Un décalage absolu a été envisagé au cadrage du `LOT-69`, au motif qu'un plan
 * continu n'a pas de repère ponctuel ; il a été écarté pour cette raison précise.
 * @param modelPosition Position en espace **modèle**, en unités monde — **jamais modifiée** par
 *                      cette fonction : la parallaxe est purement visuelle (`EX-ARCH-012`).
 * @param factor        Facteur de parallaxe **par axe** (`core::Plane::parallaxX`/`parallaxY`).
 * @param cameraBounds  Rectangle cadré par la caméra (`hmi::Camera2D::visibleBounds`), en unités
 *                      monde.
 * @return La position à utiliser pour le rendu (et le culling) de ce contenu.
 */
[[nodiscard]] core::Vector2 parallaxRenderPosition(core::Vector2 modelPosition,
                                                   core::Vector2 factor,
                                                   const core::Rect& cameraBounds) noexcept;

/**
 * @brief Inverse de `parallaxRenderPosition` : retrouve la position **modèle** équivalente à un
 * point exprimé en espace de rendu (`LOT-50` TACHE-02).
 *
 * Nécessaire à l'éditeur : le curseur converti par `hmi::Camera2D::screenToWorld` est comparable
 * à la position de **rendu** d'un contenu parallaxé (c'est elle qui occupe l'écran), jamais
 * directement à sa position modèle dès que le facteur diffère de `1.0`. Cette fonction ramène donc
 * un point d'écran dans l'espace modèle, pour que ce qu'on manipule reste visuellement « collé »
 * au curseur quel que soit son facteur de parallaxe.
 * @param renderPosition Position en espace de rendu, en unités monde.
 * @param factor         Facteur de parallaxe **par axe**.
 * @param cameraBounds   Même rectangle caméra que celui utilisé pour `parallaxRenderPosition`.
 * @return La position modèle correspondante ; l'axe est laissé **inchangé** si son facteur est nul
 *         (robustesse — un facteur nul n'est pas inversible).
 */
[[nodiscard]] core::Vector2 parallaxModelPosition(core::Vector2 renderPosition,
                                                  core::Vector2 factor,
                                                  const core::Rect& cameraBounds) noexcept;

/**
 * @brief Borne le décalage d'un plan pour qu'il **couvre toujours** le cadrage.
 *
 * Un plan couvre exactement le niveau ; le translater découvre donc son bord, et une bande vide
 * apparaîtrait au bout du décalage. L'alternative « que l'artiste peigne des marges » est un
 * piège : la marge nécessaire dépend du facteur **et** de la taille de salle — personne ne la
 * calculera juste, et l'erreur ne se verrait qu'au bord d'un niveau, en jeu.
 *
 * Le bornage est fait **par axe**, indépendamment. Si le plan est plus **petit** que le cadrage sur
 * un axe (aucune translation ne peut le couvrir), le décalage y est ramené à zéro : le plan reste
 * aligné sur le niveau, ce qui est le moindre mal et reste stable.
 * @param offset       Décalage souhaité, en unités monde.
 * @param planeBounds  Rectangle couvert par le plan **avant** décalage (typiquement le niveau).
 * @param cameraBounds Rectangle cadré par la caméra, à couvrir.
 * @return Le décalage effectivement applicable.
 */
[[nodiscard]] core::Vector2 clampPlaneOffset(core::Vector2 offset, const core::Rect& planeBounds,
                                             const core::Rect& cameraBounds) noexcept;

/**
 * @brief Décide si la parallaxe des plans s'applique, selon le mode de cadrage et le drapeau du
 *        niveau (`EX-DEC-043`, `EX-REN-016`).
 *
 * **Table pure**, plutôt qu'une condition disséminée dans la session de jeu :
 *
 * - **`Follow` → active.** Seul mode où la caméra défile, donc le seul où la parallaxe se lit.
 *   Inverse la décision du `LOT-64`, qui l'avait coupée parce qu'un **décor** est un objet collé au
 *   contenu et paraissait « suivre » la caméra ; un plan est un fond, l'argument tombe.
 * - **`PerRoom` → active.** Formule relative à la salle, donc sans saut à la bascule
 *   (`EX-REN-015`).
 * - **`WholeLevel` → neutralisée.** La caméra ne bouge pas : le décalage serait une constante, donc
 *   un simple désalignement du plan par rapport aux tuiles.
 *
 * Le drapeau est **par niveau**, mais la neutralisation en `WholeLevel` est une règle du moteur :
 * un level designer qui coche « parallaxe » sur un niveau `WholeLevel` ne verrait rien. L'interface
 * grise donc la case dans ce mode et l'explique (`TACHE-08`), sinon c'est un défaut perçu.
 * @param mode      Mode de cadrage du niveau.
 * @param levelFlag Drapeau `parallax` déclaré par le niveau (`core::Level::parallaxEnabled`).
 * @return `true` si le décalage doit être appliqué aux plans.
 */
[[nodiscard]] constexpr bool planeParallaxActive(core::CameraFramingMode mode,
                                                 bool levelFlag) noexcept {
    if (!levelFlag) {
        return false;
    }
    return mode != core::CameraFramingMode::WholeLevel;
}

/**
 * @brief Arrondit une position monde au pixel écran le plus proche, pour un zoom pixel art net.
 *
 * Un décalage de parallaxe fractionnaire produirait une image floue ou tremblante : le zoom de la
 * caméra est entier quand c'est possible (`EX-ARCH-022`), mais un décalage de parallaxe
 * quelconque, lui, ne l'est pas — cette fonction referme cet écart, **après** application de la
 * parallaxe **et** de son bornage. Fonction **pure**, sans dépendance GPU.
 * @param worldPosition     Position à arrondir, en unités monde.
 * @param pixelsPerWorldUnit Échelle courante (`hmi::Camera2D::PIXELS_PER_UNIT * zoom`) ; @p
 *                      worldPosition est renvoyée inchangée si cette valeur n'est pas strictement
 *                      positive (robustesse).
 * @return La position arrondie au pixel écran entier le plus proche.
 */
[[nodiscard]] core::Vector2 roundToScreenPixel(core::Vector2 worldPosition,
                                               float pixelsPerWorldUnit) noexcept;

}  // namespace hmi
