// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "Core/Math/Rect.h"
#include "Core/Math/Vector2.h"

/**
 * @file HMI/Graphics/FollowCamera.h
 * @brief Caméra de suivi du personnage : zone morte, anticipation, lissage, bornage
 *        (`EX-REN-016`, LOT-64 TACHE-02).
 */

namespace hmi {

/// Demi-taille de la zone morte, en unités monde : le personnage se déplace librement dans ce
/// rectangle centré sur le point suivi sans faire bouger la caméra -- ce qui supprime le
/// tremblement permanent d'une caméra qui collerait exactement au personnage. Deux constantes
/// scalaires plutôt qu'un `core::Vector2` : son constructeur n'est pas `constexpr`.
inline constexpr float FOLLOW_DEAD_ZONE_HALF_WIDTH_UNITS = 1.5f;
inline constexpr float FOLLOW_DEAD_ZONE_HALF_HEIGHT_UNITS = 1.0f;
/// Distance d'anticipation dans le sens du déplacement, en unités monde.
inline constexpr float FOLLOW_ANTICIPATION_DISTANCE_UNITS = 2.5f;
/// Temps de réponse (lissage exponentiel) du centre retenu vers sa cible, en secondes.
inline constexpr float FOLLOW_SMOOTHING_TIME_CONSTANT_SECONDS = 0.15f;
/// Temps de réponse de l'inversion de l'anticipation au changement de sens, en secondes -- plus
/// lent que le lissage principal, pour que l'inversion se voie comme un mouvement, pas un saut.
inline constexpr float FOLLOW_ANTICIPATION_TIME_CONSTANT_SECONDS = 0.25f;

/**
 * @brief État déterministe de la caméra de suivi, avancé une fois par **pas fixe**.
 *
 * Donnée pure (`EX-NFR-002`), sans horloge système : `center` est la valeur **déjà bornée** aux
 * limites du niveau, prête à l'usage (mais pas encore alignée au pixel -- l'échelle de rendu, qui
 * dépend du zoom courant, n'est connue qu'à l'affichage, cf. `hmi::roundToScreenPixel`).
 */
struct FollowCameraState {
    /// Point suivi par la zone morte (non lissé, non aligné) : ne bouge que lorsque le personnage
    /// sort du rectangle centré dessus.
    core::Vector2 anchor{};
    /// Direction d'anticipation courante, lissée dans `[-1, 1]` (0 = aucune anticipation).
    float anticipationSign = 0.0f;
    /// Centre résultant de ce pas : cible (ancre + anticipation) lissée puis bornée au niveau.
    core::Vector2 center{};
    /// Faux avant le tout premier appel : le centre démarre alors directement sur le personnage
    /// (borné), sans lissage depuis une position par défaut arbitraire.
    bool initialized = false;
};

/**
 * @brief Avance la caméra de suivi d'un pas fixe.
 *
 * Fonction **pure** : mêmes entrées, même résultat (`EX-NFR-002`, testable sans GPU). N'affecte
 * jamais la simulation (`EX-ARCH-012`) -- elle ne fait que lire une position déjà simulée.
 * @param previous           État du pas précédent (`FollowCameraState{}` par défaut au premier
 *                           appel après un chargement de niveau).
 * @param characterPosition  Position simulée du personnage (centre de sa boîte), en unités monde.
 * @param movementDirection  Sens du déplacement du personnage (`core::Actor::facing`, négatif à
 *                           gauche, positif à droite) ; `0` conserve la dernière anticipation
 *                           (immobile, rien à réviser).
 * @param levelBounds        Rectangle du niveau, en unités monde (coin haut-gauche + dimensions).
 * @param viewHalfExtent     Demi-largeur/demi-hauteur de la zone visible par la caméra, en unités
 *                           monde -- détermine la marge de bornage aux limites du niveau.
 * @param fixedDelta         Durée du pas fixe, en secondes (`EX-REN-021`) -- jamais un delta de
 *                           rendu, sous peine de dépendre de la fréquence d'affichage.
 * @return Le nouvel état, dont `center` est prêt à l'usage (borné, pas encore aligné au pixel).
 */
[[nodiscard]] FollowCameraState advanceFollowCamera(
    const FollowCameraState& previous, core::Vector2 characterPosition, float movementDirection,
    const core::Rect& levelBounds, core::Vector2 viewHalfExtent, float fixedDelta) noexcept;

}  // namespace hmi
