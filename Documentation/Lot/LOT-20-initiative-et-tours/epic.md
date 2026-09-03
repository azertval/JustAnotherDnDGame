# LOT-20 — Initiative et tour par tour {#lot-20}

> Statut : **à faire**.
> Prérequis : [LOT-12](@ref lot-12) (jet d'initiative), [LOT-19](@ref lot-19).

## Objectif

Ordonnancer les combattants par initiative et structurer le tour : mouvement, action, action bonus,
réaction.

## Périmètre

- `Source/Core/Combat/TurnOrder.{h,cpp}` : ordre d'initiative, **multi-alliés dès maintenant**
  (décision de cadrage : un héros au départ, quatre à terme — l'ordre n'a aucune raison de
  supposer un seul allié, même si le contenu n'en propose qu'un).
- `CombatState.{h,cpp}` : machine à états **explicite** — début de round, tour actif, fin de tour,
  fin de combat. Pas de drapeaux épars : un état nommé, des transitions nommées.
- Égalités d'initiative : départage déterministe et documenté.
- Entrée et sortie d'un combattant **en cours** de combat (renfort, fuite).

## Les trois fins

Un combat se termine de trois façons, et les trois doivent être couvertes :

- **victoire** : tous les ennemis à 0 PV ;
- **défaite** : tous les alliés à 0 PV ;
- **fuite** : les alliés quittent la zone.

Une machine à états qui n'en couvre que deux laisse un combat qui ne finit jamais — le pire des
défauts, parce qu'il bloque le joueur sans message d'erreur.

## Exigences couvertes

`EX-CBT-*` (initiative, structure du tour, conditions de fin).

## Critères d'acceptation

- Un combat à cinq combattants se déroule **en headless** du premier round à une condition de fin.
- Les **trois** conditions de fin sont couvertes par un test chacune.
- Un test monte **quatre alliés** : rien dans l'ordre d'initiative ni dans la machine à états ne
  suppose un héros unique.
- Rejeu à graine fixe strictement reproductible, ordre d'initiative compris.
