// SPDX-FileCopyrightText: 2026 Valentin Eloy
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Core/Gameplay/Interaction.h"

#include <cmath>
#include <limits>

#include "Core/Gameplay/WorldFlags.h"
#include "Core/Levels/TileMap.h"
#include "Core/Levels/TileType.h"

namespace core {

namespace {

// Le centre d'une case, en unites monde ou une case vaut 1. Sert au departage : la distance se
// mesure au CENTRE de la case visee, pas a son coin, sinon deux candidats symetriques autour du
// centre departageraient sur un arrondi.
[[nodiscard]] Vector2 centreDe(GridPosition cellule) {
    return {static_cast<float>(cellule.column) + 0.5F, static_cast<float>(cellule.row) + 0.5F};
}

[[nodiscard]] float distanceCarree(Vector2 gauche, Vector2 droite) {
    const float dx = gauche.x - droite.x;
    const float dy = gauche.y - droite.y;
    return (dx * dx) + (dy * dy);
}

}  // namespace

GridPosition aimedCell(GridPosition from, Vector2 facing) {
    const float horizontal = std::abs(facing.x);
    const float vertical = std::abs(facing.y);
    if (horizontal <= 0.0F && vertical <= 0.0F) {
        // Orientation nulle : le personnage ne vise rien. Rendre une voisine arbitraire ferait
        // ouvrir un coffre qu'il ne regarde pas.
        return from;
    }
    // La direction DOMINANTE, jamais une diagonale : un personnage qui regarde a 30 degres vise la
    // case de droite. Viser en diagonale rendrait la cible imprevisible a la manette analogique.
    // A egalite exacte, l'horizontale l'emporte -- il faut un depart, et celui-la est ecrit.
    if (horizontal >= vertical) {
        return {from.column + (facing.x >= 0.0F ? 1 : -1), from.row};
    }
    return {from.column, from.row + (facing.y >= 0.0F ? 1 : -1)};
}

InteractionTarget findInteractionTarget(GridPosition from, Vector2 facing, const TileMap& map,
                                        const std::vector<InteractionCandidate>& candidates,
                                        const WorldFlags& flags) {
    InteractionTarget resultat;
    resultat.aimedCell = aimedCell(from, facing);
    if (resultat.aimedCell == from) {
        return resultat;  // Orientation nulle.
    }
    if (!map.inBounds(resultat.aimedCell.column, resultat.aimedCell.row)) {
        return resultat;
    }
    // L'interaction ne traverse pas un mur. La case visee est adjacente, si bien qu'il n'y a rien
    // entre elle et le personnage : c'est la case ELLE-MEME qui doit etre traversable.
    if (map.isSolid(resultat.aimedCell.column, resultat.aimedCell.row)) {
        return resultat;
    }

    const Vector2 centre = centreDe(resultat.aimedCell);
    float meilleure = std::numeric_limits<float>::max();
    for (const InteractionCandidate& candidat : candidates) {
        if (candidat.interactable == nullptr) {
            continue;
        }
        if (candidat.interactable->position != resultat.aimedCell) {
            continue;
        }
        if (candidat.interactable->isConsumable() &&
            flags.isSet(candidat.interactable->consumedFlag)) {
            // Un coffre vide n'est plus une cible : continuer a l'afficher comme telle promettrait
            // au joueur quelque chose qui n'arrivera pas.
            continue;
        }
        const float distance = distanceCarree(centreDe(candidat.interactable->position), centre);
        // Strictement inferieur : a distance egale, le PREMIER l'emporte, donc le plus petit
        // indice. Sans ce depart, deux candidats sur la meme case donneraient tantot l'un tantot
        // l'autre selon l'ordre de parcours de l'ECS, qui n'est pas stable.
        if (distance < meilleure) {
            meilleure = distance;
            resultat.interactable = candidat.interactable;
            resultat.index = candidat.index;
        }
    }
    return resultat;
}

InteractionOutcome interact(const InteractionTarget& target, WorldFlags& flags) {
    InteractionOutcome resultat;
    if (!target.found()) {
        return resultat;
    }
    resultat.happened = true;
    resultat.type = target.interactable->type;
    resultat.promptKey = target.interactable->promptKey;
    if (target.interactable->isConsumable()) {
        // `set` dit si le fait etait NEUF. Demander d'abord puis ecrire ensuite laisserait entre
        // les deux appels une fenetre ou un second appel donnerait le butin une seconde fois.
        resultat.consumed = flags.set(target.interactable->consumedFlag);
    }
    return resultat;
}

}  // namespace core
