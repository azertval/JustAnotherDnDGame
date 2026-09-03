# LOT-16 — Quêtes et drapeaux de monde {#lot-16}

> Statut : **à faire**.
> Prérequis : [LOT-15](@ref lot-15) (les dialogues déclenchent et font avancer les quêtes).

## Objectif

Suivre l'avancement du joueur par des **drapeaux persistants**, et en donner une lecture dans un
journal de quêtes.

## Périmètre

- `Source/Core/Rpg/WorldFlags.{h,cpp}` : ensemble de drapeaux nommés, posés et lus par les
  dialogues, les entités de carte et les rencontres. C'est le **substrat** : coffre déjà ouvert
  (`LOT-10`), ennemi vaincu (`LOT-18`), PNJ déjà rencontré.
- `Quest.{h,cpp}` : étapes, conditions d'avancement, récompenses. Défini en **JSON**.
- Journal de quêtes (contenu ; l'écran vient au `LOT-24` ou avec l'IHM RPG).
- Persistance dans `SaveGame` (`LOT-17`).

## Note de conception

Les drapeaux de monde et les quêtes sont deux niveaux du même mécanisme : une quête **lit** des
drapeaux pour décider de son avancement, elle n'a pas d'état propre au-delà de son étape courante.
Cette séparation évite le piège classique où l'état du monde existe en double — une fois dans les
entités, une fois dans les quêtes — et diverge.

## Exigences couvertes

`EX-RPG-*`.

## Critères d'acceptation

- Une quête à trois étapes se déclenche, progresse et se termine.
- Elle **survit à une sauvegarde et un rechargement** à n'importe laquelle de ses étapes.
- Un drapeau posé par un dialogue est lu par une entité de carte, et inversement.
