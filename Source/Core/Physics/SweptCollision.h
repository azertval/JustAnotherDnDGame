// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0

#pragma once

#include "Core/Math/Vector2.h"
#include "Core/Physics/Aabb.h"

/**
 * @file Core/Physics/SweptCollision.h
 * @brief Collision **continue** (balayage) d'une boîte AABB contre une grille de tuiles solides.
 */

namespace core {

class TileMap;

/**
 * @brief Résultat d'un balayage : où la boîte s'arrête et sur quels axes elle a été bloquée.
 */
struct SweepResult {
    /// Nouveau coin haut-gauche de la boîte après résolution (déplacement éventuellement écourté).
    Vector2 position{};

    /**
     * @brief Indicateur de blocage **par axe** (grille alignée aux axes).
     *
     * Pour chaque composante : `0` si l'axe est resté libre ; une valeur non nulle si un contact
     * a bloqué le mouvement sur cet axe, de **signe** égal au sens « surface → boîte » (un sol,
     * situé sous la boîte avec `y` vers le bas, donne `normal.y < 0`). Ce n'est donc pas un
     * vecteur unitaire mais un **indicateur directionnel par axe**, que la physique (TACHE-03)
     * lit pour annuler la composante de vitesse correspondante et déduire l'appui au sol.
     */
    Vector2 normal{};

    /// `true` si au moins un contact bloquant a eu lieu pendant le balayage.
    bool hit = false;
};

/**
 * @brief Déplace @p box de @p delta en s'arrêtant au premier contact avec une tuile **solide**,
 *        puis en **glissant** le long de la surface pour le déplacement résiduel.
 *
 * Collision **continue** (`EX-GP-014`) : quelle que soit la vitesse, la boîte ne **traverse
 * jamais** une tuile solide sur un pas. Méthode : **balayage par axe** (X puis Y), chaque axe
 * parcourant *toutes* les cellules entre la position courante et la cible (donc aucun tunneling),
 * puis **calant** la position sur la coordonnée exacte du mur (clamp direct, sans dérive
 * flottante). La passe Y part de la position X déjà résolue, ce qui produit le **glissement** le
 * long des murs. Ordre X→Y conventionnel et déterministe (`EX-NFR-002`). Fonction **pure** :
 * aucun état, aucun aléatoire.
 *
 * Précondition : @p box ne **chevauche** aucune tuile solide au départ (état non pénétrant) — la
 * physique appelante garantit cet invariant pas après pas.
 *
 * @param box   Boîte mobile avant déplacement (coin haut-gauche = position de l'entité).
 * @param delta Déplacement voulu sur le pas, en unités monde (`velocity * dt`).
 * @param tiles Grille de collision : `tiles.isSolid(colonne, ligne)` désigne les tuiles bloquantes.
 * @return La position finale, l'indicateur de blocage par axe et l'occurrence d'un contact.
 */
[[nodiscard]] SweepResult sweepAabb(const Aabb& box, const Vector2& delta, const TileMap& tiles);

}  // namespace core
