# LOT-26 — Butin, marchands, économie {#lot-26}

> Statut : **à faire**.
> Prérequis : [LOT-14](@ref lot-14), [LOT-15](@ref lot-15).

## Objectif

Boucler la boucle économique : gagner du butin, le vendre, acheter mieux.

## Périmètre

- `Source/Core/Rpg/LootTable.{h,cpp}` : tables de butin en **JSON**, tirées avec le hasard
  déterministe du `LOT-12` — un coffre rouvert après rechargement de sauvegarde doit donner le
  **même** contenu, sinon le joueur peut relancer jusqu'au bon tirage.
- Or et valeur des objets.
- `Shop.{h,cpp}` : achat, vente, marge du marchand, stock.
- IHM marchand, sur le système de design existant.

## Le piège du tirage

Un butin tiré au moment de l'ouverture, avec une graine liée à l'instant, se re-tire différemment à
chaque chargement de sauvegarde. La graine doit dériver de l'**identité du coffre** et de l'état de
la partie, pas de l'horloge — `deriveSeed(baseSeed, step, entityId)` existe pour cela.

## Exigences couvertes

`EX-RPG-*`, `EX-INV-*`.

## Critères d'acceptation

- Un coffre donne le **même** butin après sauvegarde et rechargement.
- Vendre puis racheter un objet ne crée ni ne détruit de valeur au-delà de la marge annoncée.
- Le stock d'un marchand se comporte de façon définie quand il est épuisé.
