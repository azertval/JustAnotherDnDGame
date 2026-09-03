# LOT-10 — Entités de carte et interaction {#lot-10}

> Statut : **à faire**.
> Prérequis : [LOT-04](@ref lot-04) (couche `objects`), [LOT-06](@ref lot-06) (orientation du
> personnage), [LOT-08](@ref lot-08).

## Objectif

Peupler les cartes d'entités qui ne sont **pas des tuiles** — PNJ, coffres, panneaux, portails,
déclencheurs — et permettre au joueur d'interagir avec elles.

## Pourquoi des entités et pas des tuiles

Une tuile, c'est un type par case et rien d'autre : la grille ne porte aucune métadonnée. Le
platformer avait contourné cela avec des vecteurs annexes indexés par position — le `LOT-01` en a
retiré quatre. Un PNJ a un nom, un dialogue, un inventaire ; un coffre a un contenu et un état
« déjà ouvert ». Cela demande une **liste d'objets à propriétés**, ce que la couche `objects` du
format v3 fournit.

## Périmètre

- Instanciation ECS depuis la couche `objects` (`LevelScene`), une entité par objet, avec ses
  propriétés libres.
- **Cible d'interaction** : la case devant l'orientation du personnage. `core::PlayerInput` porte
  déjà `interactPressed/Held/Released` — rien à câbler côté entrée.
- Invite visuelle quand une cible est à portée (le joueur doit savoir qu'il *peut* interagir).
- Coffre : contenu, état consommé. Panneau : texte localisé.

## Le piège

Un coffre ouvert deux fois ne doit donner son butin **qu'une fois**, et cet état doit survivre au
départ puis au retour sur la carte (`LOT-09`) et à la sauvegarde (`LOT-17`). C'est un drapeau de
monde, pas un booléen local à l'entité rechargée.

## Exigences couvertes

`EX-EXP-*` (interaction, portée, priorité de cible), `EX-RPG-*` (coffre, panneau).

## Critères d'acceptation

- Ouvrir un coffre deux fois ne donne le butin qu'une fois, y compris après aller-retour de carte.
- L'interaction **ne traverse pas un mur**.
- Quand deux cibles sont à portée, celle désignée est déterministe et prévisible (la plus proche du
  centre de la case visée).
