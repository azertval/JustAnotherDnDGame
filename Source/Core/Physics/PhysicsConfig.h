// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

/**
 * @file Core/Physics/PhysicsConfig.h
 * @brief Constantes de réglage de la physique du personnage (données pures).
 */

namespace core {

/**
 * @brief Tolérance géométrique des routines de balayage, en unités monde.
 *
 * Empêche qu'un simple **effleurement** de frontière — un bord exactement posé sur une
 * limite de case — compte comme un chevauchement. Partagée par `sweepX`/`sweepY`
 * (`SweptCollision`) et par la résolution AABB/AABB : les deux doivent s'accorder sur ce
 * qu'est un contact, sans quoi une même position serait bloquante pour l'une et libre pour
 * l'autre. Ce n'est pas un réglage de ressenti, d'où sa place ici et non dans
 * `PhysicsConfig` : la modifier ne change pas le jeu, elle change la définition du contact.
 */
inline constexpr float COLLISION_SKIN = 1e-4F;

/**
 * @brief Paramètres réglables de la physique du personnage, regroupés pour faciliter le *tuning*.
 *
 * Donnée pure (`EX-ARCH-011`) : le système de déplacement la lit, ne la modifie pas. Les valeurs
 * par défaut sont des points de départ **à affiner par tests** (le ressenti « game feel » est
 * marqué ⚠️ dans la spec `gameplay.md`). Unités monde : une tuile = 1 unité, `y` vers le bas.
 */
struct PhysicsConfig {
    /// Vitesse horizontale à pleine intention, en unités/seconde (`EX-GP-010`). ⚠️ à affiner.
    float moveSpeed = 3.0f;
    /// Accélération de la gravité, en unités/seconde². ⚠️ à affiner au ressenti.
    float gravity = 50.0f;
    /// Coefficient de traînée verticale en chute, en 1/s (`EX-GP-019`). La vitesse terminale
    /// **émerge** de l'équilibre poids/traînée (poids = masse × gravité effective) plutôt que
    /// d'être plafonnée : à masse par défaut (1,0), `90 / 3.6 = 25` unités/s, continuité avec
    /// l'ancien plafond fixe. ⚠️ à affiner.
    float fallDragCoefficient = 3.6f;
    /// Vitesse verticale de l'impulsion de saut, en unités/seconde (montée). Avec `gravity = 50`,
    /// ~2,25 tuiles de hauteur / apex ~0,3 s. ⚠️ à affiner (viser ~2,5 tuiles, apex ~0,35 s).
    float jumpSpeed = 15.0f;
    /// Fenêtre de *coyote time*, en secondes : saut permis peu après avoir quitté un bord. ⚠️.
    float coyoteTime = 0.08f;
    /// Fenêtre de *jump buffering*, en secondes : saut pré-appuyé honoré à l'atterrissage. ⚠️.
    float jumpBufferTime = 0.12f;
    /// Fraction de vitesse ascendante **conservée** au relâchement du saut (hauteur variable) :
    /// 0 = coupe nette, 1 = pas de coupe. ⚠️ à affiner (~0,5).
    float jumpCutFactor = 0.5f;
    /// Nombre de sauts **aériens** supplémentaires (double/multi saut, `EX-GP-015`). Défaut 1.
    int airJumps = 1;
    /// Vitesse de descente **plafonnée** le long d'un mur (wall slide), en unités/s. ⚠️.
    float wallSlideSpeed = 4.0f;
    /// Composante horizontale de l'impulsion de **wall jump** (éjection opposée au mur), unités/s.
    float wallJumpSpeedX = 12.0f;
    /// Composante verticale (montée) de l'impulsion de **wall jump**, en unités/s.
    float wallJumpSpeedY = 14.0f;
    /// Durée (secondes) de verrouillage du contrôle horizontal après un wall jump. ⚠️.
    float wallJumpLockTime = 0.18f;
    /// Vitesse de la ruée de **dash**, en unités/s (`EX-GP-017`). ⚠️.
    float dashSpeed = 15.0f;
    /// Durée (secondes) du dash ; distance ≈ dashSpeed × dashDuration. ⚠️.
    float dashDuration = 0.15f;
    /// Nombre de dashs utilisables entre deux contacts avec le sol (`EX-GP-017`, `EX-GP-055`).
    /// Défaut 1 : un seul dash par saut, le comportement historique.
    int dashCharges = 1;
    /// Multiplicateur de gravité en **chute** (> 1) : la chute est plus rapide que la montée
    /// (`EX-GP-018`). ⚠️ à affiner (~1,8).
    float fallGravityMultiplier = 1.8f;
    /// Seuil de vitesse verticale (unités/s) sous lequel on est « à l'apex » (flottement). ⚠️.
    float apexThreshold = 4.0f;
    /// Multiplicateur de gravité **près de l'apex** (< 1) : contrôle flottant au sommet. ⚠️.
    float apexGravityMultiplier = 0.5f;
    /// Multiplicateur de gravité **supplémentaire** en chute quand « bas » est maintenu
    /// (*fast-fall*, > 1). ⚠️ à affiner (~1,6).
    float fastFallMultiplier = 1.6f;
    /// Seuil (secondes) de maintien de la direction opposée pour armer un dash boosté
    /// (`EX-GP-056`). ⚠️ à affiner.
    float dashChargeHoldTime = 0.25f;
    /// Multiplicateur de vitesse d'un dash **boosté** (`EX-GP-056`), appliqué à `dashSpeed`. ⚠️.
    float dashBoostSpeedMultiplier = 1.5f;
    /// Multiplicateur de durée d'un dash **boosté** (`EX-GP-056`), appliqué à `dashDuration`. ⚠️.
    float dashBoostDurationMultiplier = 1.3f;
    /// Vitesse de chute imposée pendant un **ground pound** (`EX-GP-058`), en unités/s ; nettement
    /// au-delà de la vitesse terminale normale pour un impact net. ⚠️ à affiner.
    float groundPoundSpeed = 30.0f;
    /// Durée (secondes) du verrou horizontal après un **jump-cancel** de dash (`EX-GP-061`) :
    /// pendant ce temps, la vitesse horizontale conservée du dash n'est pas écrasée par le contrôle
    /// normal (même rôle que `wallJumpLockTime`). ⚠️.
    float dashJumpLockTime = 0.12f;
    /// Fenêtre (secondes) pendant laquelle des jump-cancels rapprochés cumulent un bonus de vitesse
    /// (`EX-GP-061`). ⚠️.
    float comboWindowTime = 0.3f;
    /// Bonus de vitesse (unités/s) ajouté par enchaînement de jump-cancels rapprochés
    /// (`EX-GP-061`), multiplié par `Player::comboChainCount`. ⚠️.
    float comboSpeedBonus = 2.0f;
    /// Plafond (unités/s) du bonus cumulé de combo, quel que soit le nombre d'enchaînements
    /// (`EX-GP-061`) — protège l'équilibrage des niveaux. ⚠️.
    float comboSpeedCap = 8.0f;
    /// Fenêtre (secondes) pendant laquelle un saut hérite de la vitesse d'une poussée renforcée
    /// récente (`EX-GP-057`/`EX-GP-061`). ⚠️.
    float pushMomentumWindowTime = 0.25f;
    /// Fraction de la vitesse d'un bloc poussé (poussée renforcée) héritée par le saut suivant, si
    /// déclenché dans `pushMomentumWindowTime` (`EX-GP-061`). ⚠️.
    float momentumCarryRatio = 0.5f;
};

}  // namespace core
