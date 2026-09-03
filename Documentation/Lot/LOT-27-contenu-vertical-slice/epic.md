# LOT-27 — Contenu du *vertical slice* {#lot-27}

> Statut : **à faire**.
> Prérequis : phases B, C et D complètes.

## Objectif

Produire le contenu jouable qui démontre la boucle entière : *« un personnage explore une carte
top-down, parle à un PNJ, déclenche une rencontre, gagne un combat tactique au d20 »*.

## Périmètre

- Un **village** : quelques bâtiments, trois PNJ, un marchand, un point de départ.
- Un **donjon** : trois salles, deux rencontres, un coffre gardé.
- Une **quête principale** reliant les deux, avec dialogues à choix et au moins un jet de compétence.
- Deux types d'**ennemis** avec des profils d'IA distincts (mêlée agressive, distance prudente).
- Un **combat scénarisé** déclenché par un dialogue.

Tout se produit **dans l'éditeur** (`LOT-11`), pas en écrivant du JSON à la main : c'est le test
grandeur nature de l'outil, et la preuve qu'un non-développeur pourrait le faire (`EX-VIS-006`).

## Outillage

`scripts/check_world_graph.py` remplace le `check_demo_sequence.py` hérité, retiré au `LOT-01`.
Il valide, en CI :

- le **graphe de cartes** : aucun portail orphelin, aucune carte inatteignable ;
- les **références de dialogue et de quête** : chaque nœud cible, chaque drapeau, chaque
  récompense existe ;
- que chaque zone de rencontre est un **terrain tactique valide** (contrainte du `LOT-11`, née de
  la décision « combat sur la carte »).

## Exigences couvertes

`EX-RPG-*`, `EX-LVL-*`, et concrétisation de `EX-VIS-001` à `EX-VIS-005`.

## Critères d'acceptation

- La boucle complète se joue de bout en bout.
- Un **test système** la rejoue en headless, du départ à la victoire du combat.
- `check_world_graph.py` vert.
- Le contenu se recharge après sauvegarde à n'importe quel point de la boucle.
