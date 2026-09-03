# LOT-08 — Vocabulaire de tuiles RPG {#lot-08}

> Statut : **à faire**.
> Prérequis : [LOT-04](@ref lot-04) (les couches donnent leur sens aux types).

## Objectif

Donner à la palette les types de terrain d'un RPG en vue de dessus, là où le `LOT-01` a laissé le
strict minimum hérité (11 types).

## Périmètre

Types à ajouter dans `core::TileType`, à peu près : `Grass`, `Dirt`, `Sand`, `Water`, `DeepWater`,
`Wall`, `Cliff`, `Bridge`, `Stairs`. La liste exacte se décide au contact du level design, pas
d'avance.

Pour **chaque** type ajouté, la chaîne complète — c'est la leçon la plus chère de l'héritage, où
ajouter un type touchait « exactement la même chaîne de huit fichiers » :

1. `TileType.h` (avant le dernier énumérateur, `TILE_TYPE_COUNT` suit tout seul) ;
2. `TileTypeName.cpp` (le `switch` est exhaustif et sans `default` : le compilateur désigne
   lui-même ce qu'il reste à faire) ;
3. `Editor/TileTaxonomy.cpp` (catégorie de palette) et `TaxonomyLabels.cpp` ;
4. libellés `fr.lang` **et** `en.lang` ;
5. `ProceduralAtlas` : **rendu de repli déterministe**, dans le même lot, jamais « plus tard » —
   c'est ce qui garde le jeu lançable sans aucun fichier d'image ;
6. `TileAutotile` si le type a des raccords ;
7. `TileSilhouette` si sa matière n'occupe pas toute la case (falaises, bords d'eau, ponts) — le
   `LOT-01` a **conservé ce mécanisme vidé** précisément pour ce lot ;
8. test d'aller-retour chargeur/écrivain.

## Note de conception

Le `LOT-01` a supprimé les 25 types de plateforme mais gardé les 11 génériques (`Empty`, `Solid`,
`Danger`, `Entry`, `Exit`, `Switch`, `Door`, `PressurePlate`, `Block`, `Key`, `LockedDoor`) : le
vocabulaire de puzzle sert tel quel au RPG. Ce lot **ajoute**, il ne remplace pas.

## Exigences couvertes

`EX-EXP-*`, `EX-EDIT-*`, `EX-REN-*`.

## Critères d'acceptation

- Chaque nouveau type a : un libellé fr/en, une classe de palette, un rendu procédural de repli, et
  un test d'aller-retour.
- Le jeu se lance et affiche une carte **sans aucun fichier d'image** présent.
- `TILE_TYPE_COUNT` reste dérivé du dernier énumérateur — aucune borne recopiée à la main.
