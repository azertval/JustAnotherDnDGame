# LOT-19 — Grille tactique et déplacement {#lot-19}

> Statut : **à faire**.
> Prérequis : [LOT-18](@ref lot-18).

## Objectif

Poser la grille de combat, l'occupation des cases, et le calcul du déplacement par budget.

## Ce qui existe déjà — à ne pas réécrire

**`core::GridDistanceField`** (`Source/Core/World/GridDistanceField.h`) : champ de distances de
plus court chemin sur la grille, BFS 4-voisins **multi-source**, lectures `O(1)`, avec sentinelle
d'inatteignabilité. Il vient du solveur d'IA du dépôt d'origine ; le `LOT-01` l'a **délibérément
sauvé** de la purge et déplacé dans `Core/World/` **pour ce lot précis**.

C'est exactement le calcul « quelles cases puis-je atteindre avec ce budget de déplacement ».

## Périmètre

- `Source/Core/Combat/BattleGrid.{h,cpp}` : occupation des cases, terrain difficile, obstacles
  **issus de la couche collision** du format v3 (`LOT-04`).
- Portée de déplacement via `GridDistanceField`, budget dérivé de la vitesse de la fiche
  (`LOT-13`), à l'échelle 1 case = 1,5 m figée au `LOT-12`.
- `Pathfinding.{h,cpp}` : A* **déterministe**, avec départage stable des égalités.

## Pourquoi le déterminisme du chemin n'est pas négociable

Deux chemins de même coût existent presque toujours sur une grille. Si le départage dépend de
l'ordre d'itération d'un conteneur non ordonné, l'IA (`LOT-23`) devient irreproductible et
**aucun test de combat ne tient**. Le départage doit être une règle explicite (par exemple : plus
petit indice de case), pas un hasard d'implémentation.

## Exigences couvertes

`EX-CBT-*` (grille, portée de déplacement, chemin déterministe).

## Critères d'acceptation

- Même entrée → **même chemin**, systématiquement.
- L'ensemble des cases atteignables correspond **exactement** au budget de déplacement, ni une de
  plus ni une de moins (tests aux bornes).
- Deux créatures ne partagent **jamais** une case.
- Le terrain difficile double le coût, et la portée s'en trouve réduite en conséquence.
