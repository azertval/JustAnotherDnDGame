# LOT-09 — Graphe de cartes et transitions {#lot-09}

> Statut : **à faire**.
> Prérequis : [LOT-04](@ref lot-04), [LOT-06](@ref lot-06).

## Objectif

Remplacer la **séquence linéaire** de tableaux héritée par un **monde de cartes connectées**, où
chaque carte a des sorties nommées vers d'autres cartes, et où l'on peut revenir sur ses pas.

## Le problème

`core::LevelSequence` est une **liste ordonnée** : tableau 1, puis 2, puis 3. C'est la structure
d'un jeu de niveaux discrets, pas d'un monde. Un RPG a besoin d'un **graphe** : un village ouvre
sur une forêt et sur une auberge, la forêt ramène au village, le donjon a trois entrées.

## Périmètre

- `Source/Core/World/WorldGraph.{h,cpp}` : cartes, portails, points d'arrivée **nommés**. Un
  portail référence `(carte cible, nom du point d'arrivée)` — jamais des coordonnées brutes, qui
  se désynchroniseraient au moindre redimensionnement de la carte cible.
- Chargement de carte **à chaud**, sans repasser par un écran de sélection.
- Transition visuelle (fondu) côté `hmi::ScreenFlow`.
- **Validation au chargement** : un portail dont la carte ou le point d'arrivée n'existe pas est
  une erreur explicite, pas un plantage à la traversée (`EX-NFR-040`).

`LevelSequence` disparaît ou devient un cas dégénéré du graphe — à trancher à l'implémentation.

## Exigences couvertes

`EX-EXP-*` (portails, points d'arrivée nommés), `EX-LVL-*` (validation du graphe).

## Critères d'acceptation

- A → B → A ramène le personnage **au bon point d'arrivée**, pas à l'entrée par défaut de A.
- Un portail orphelin (carte inconnue, point d'arrivée inconnu) est refusé au chargement avec un
  message exploitable.
- Test headless d'un parcours de cinq cartes, aller et retour.
- L'état de la carte quittée (coffres ouverts, ennemis vaincus) est **conservé** au retour — c'est
  ce qui distingue un monde d'une séquence de tableaux. Persisté au `LOT-17`.
