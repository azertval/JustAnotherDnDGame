# LOT-25 — Sorts et capacités de classe {#lot-25}

> Statut : **à faire**.
> Prérequis : [LOT-13](@ref lot-13), [LOT-21](@ref lot-21), [LOT-22](@ref lot-22).

## Objectif

Ajouter les sorts — emplacements, incantation, concentration — et les capacités propres aux
classes.

## Périmètre

- `Source/Core/Rpg/Spell.{h,cpp}` : école, niveau, portée, durée, composantes, effet.
- `SpellSlots` : emplacements par niveau, consommés à l'incantation, restaurés au repos.
- `Concentration` : un seul sort concentré à la fois ; il tombe si le lanceur subit des dégâts
  (jet de sauvegarde) ou en incante un autre.
- Catalogue de sorts en **JSON** (`EX-VIS-007`).
- Branchement sur les gabarits d'effet de zone du `LOT-22` — aucune géométrie nouvelle.
- Effets **hors combat** aussi : un sort d'utilité en exploration passe par le même catalogue.

## Note de conception

C'est le lot où la tentation d'écrire des règles en dur est la plus forte, parce que chaque sort a
sa particularité. Y céder rend l'équilibrage impossible : un sort qui se règle en recompilant ne se
règle pas. Un sort est une **donnée** ; le C++ ne porte que les *mécanismes* qu'elle compose
(dégâts de zone, jet de sauvegarde, condition appliquée, durée).

## Exigences couvertes

`EX-DND-*`.

## Critères d'acceptation

- Un emplacement consommé est indisponible jusqu'au repos.
- La concentration tombe au bon moment (dégâts avec échec de sauvegarde, second sort concentré),
  et **pas** aux mauvais.
- Un sort de zone touche exactement les cases du gabarit du `LOT-22`.
- Aucune règle de sort codée en dur dans le C++.
