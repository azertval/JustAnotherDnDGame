// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "HMI/Graphics/FollowCamera.h"

#include <algorithm>
#include <cmath>

namespace hmi {

namespace {

// Lissage exponentiel (temps de reponse constant, independant du pas) vers target : converge sans
// jamais l'atteindre exactement en temps fini -- c'est precisement ce qui rend une inversion
// d'anticipation "progressive" plutot que la faire sauter d'une valeur a l'autre.
float exponentialApproach(float current, float target, float timeConstant, float dt) noexcept {
    if (timeConstant <= 0.0f) {
        return target;
    }
    const float factor = 1.0f - std::exp(-dt / timeConstant);
    return current + (target - current) * factor;
}

// Ramene une coordonnee dans les limites [levelMin, levelMax] du niveau sur UN axe, avec la marge
// du cadrage (viewHalfExtent) -- sauf si le niveau est plus etroit que le cadrage sur cet axe, cas
// ou la camera est CENTREE plutot que bornee (sinon elle collerait a un bord, epic.md).
float clampAxis(float value, float levelMin, float levelSize, float viewHalfExtent) noexcept {
    if (levelSize <= viewHalfExtent * 2.0f) {
        return levelMin + levelSize * 0.5f;
    }
    return std::clamp(value, levelMin + viewHalfExtent, levelMin + levelSize - viewHalfExtent);
}

}  // namespace

FollowCameraState advanceFollowCamera(const FollowCameraState& previous,
                                      core::Vector2 characterPosition, core::Vector2 facing,
                                      const core::Rect& levelBounds, core::Vector2 viewHalfExtent,
                                      float fixedDelta) noexcept {
    if (!previous.initialized) {
        // Premier pas apres un chargement : demarre directement SUR le personnage (borne), sans
        // lissage depuis une position par defaut arbitraire qui produirait un glissement visible
        // a l'apparition.
        const core::Vector2 start{clampAxis(characterPosition.x, levelBounds.position.x,
                                            levelBounds.size.x, viewHalfExtent.x),
                                  clampAxis(characterPosition.y, levelBounds.position.y,
                                            levelBounds.size.y, viewHalfExtent.y)};
        return FollowCameraState{
            .anchor = characterPosition, .anticipation = {}, .center = start, .initialized = true};
    }

    // Zone morte : l'ancre ne bouge que si le personnage en sort, tout juste assez pour le
    // ramener au bord du rectangle -- pas de saut au centre.
    core::Vector2 anchor = previous.anchor;
    if (characterPosition.x < anchor.x - FOLLOW_DEAD_ZONE_HALF_WIDTH_UNITS) {
        anchor.x = characterPosition.x + FOLLOW_DEAD_ZONE_HALF_WIDTH_UNITS;
    } else if (characterPosition.x > anchor.x + FOLLOW_DEAD_ZONE_HALF_WIDTH_UNITS) {
        anchor.x = characterPosition.x - FOLLOW_DEAD_ZONE_HALF_WIDTH_UNITS;
    }
    if (characterPosition.y < anchor.y - FOLLOW_DEAD_ZONE_HALF_HEIGHT_UNITS) {
        anchor.y = characterPosition.y + FOLLOW_DEAD_ZONE_HALF_HEIGHT_UNITS;
    } else if (characterPosition.y > anchor.y + FOLLOW_DEAD_ZONE_HALF_HEIGHT_UNITS) {
        anchor.y = characterPosition.y - FOLLOW_DEAD_ZONE_HALF_HEIGHT_UNITS;
    }

    // Anticipation : la cible tourne PROGRESSIVEMENT avec la direction de marche (son propre
    // lissage, plus lent que le lissage principal ci-dessous) ; a l'arret, conserve la derniere
    // direction plutot que de revenir a zero -- rien ne justifie de "regarder derriere" un
    // personnage immobile.
    //
    // Un VECTEUR depuis le LOT-07, sur les deux axes : anticiper seulement a gauche et a droite
    // etait un reste du jeu de plateforme, ou l'on ne courait que lateralement. En vue de dessus,
    // marcher vers le haut est un deplacement comme un autre (EX-EXP-001), et la camera doit le
    // devancer de la meme facon.
    const core::Vector2 desired =
        facing.lengthSquared() > 0.0f ? facing.normalized() : previous.anticipation;
    const core::Vector2 anticipation{
        exponentialApproach(previous.anticipation.x, desired.x,
                            FOLLOW_ANTICIPATION_TIME_CONSTANT_SECONDS, fixedDelta),
        exponentialApproach(previous.anticipation.y, desired.y,
                            FOLLOW_ANTICIPATION_TIME_CONSTANT_SECONDS, fixedDelta)};

    const core::Vector2 target = anchor + anticipation * FOLLOW_ANTICIPATION_DISTANCE_UNITS;

    // Lissage vers la cible, cadence sur le pas fixe (EX-REN-021) : @p fixedDelta est TOUJOURS le
    // pas de simulation, jamais un delta de rendu -- sinon la caméra se comporterait différemment
    // à 60 et à 144 Hz (tache-02, piège documenté).
    const core::Vector2 smoothed{
        exponentialApproach(previous.center.x, target.x, FOLLOW_SMOOTHING_TIME_CONSTANT_SECONDS,
                            fixedDelta),
        exponentialApproach(previous.center.y, target.y, FOLLOW_SMOOTHING_TIME_CONSTANT_SECONDS,
                            fixedDelta)};

    const core::Vector2 bounded{
        clampAxis(smoothed.x, levelBounds.position.x, levelBounds.size.x, viewHalfExtent.x),
        clampAxis(smoothed.y, levelBounds.position.y, levelBounds.size.y, viewHalfExtent.y)};

    return FollowCameraState{
        .anchor = anchor, .anticipation = anticipation, .center = bounded, .initialized = true};
}

}  // namespace hmi
