# LOT-07 — Rendu top-down et tri par Y {#lot-07}

> Statut : **à faire**.
> Prérequis : [LOT-04](@ref lot-04) (couches), [LOT-06](@ref lot-06) (un personnage qui bouge).

## Objectif

Rendre un monde en vue de dessus **crédible** : le personnage passe **devant** ce qui est au-dessus
de lui à l'écran, **derrière** ce qui est en dessous.

## Ce qui existe déjà

`hmi::ComposedScene` trie **de façon stable** par `(calque, texture, sortOrder)`, et `sortOrder`
est un `std::int32_t` **par quad**. Le tri par Y n'est donc pas une refonte du rendu : c'est
**alimenter un champ qui existe déjà** avec le Y monde quantifié.

La stabilité du tri compte autant que le tri lui-même : à Y égal, deux sprites doivent garder un
ordre constant d'une image à l'autre, sinon ils scintillent.

## Périmètre

- Alimenter `sortOrder` depuis le Y monde dans les calques `Object` et `Player` — en pratique le
  **pied** du sprite, pas son coin haut : c'est le point de contact avec le sol qui décide de la
  profondeur.
- Sprites **4 directions** et animations de marche, en réutilisant `AnimationCatalog` et
  `SkinCatalog` (données `.anim.json`, aucun code d'animation nouveau).
- Caméra : suivi sans biais vertical (le platformer décalait la vue vers le haut pour anticiper les
  sauts — sans objet ici).
- Repli procédural du personnage 4 directions dans `ProceduralAtlas`, pour que le jeu reste
  lançable sans aucun asset.

## Jalon

C'est ici que le projet redevient **montrable** : une carte, un personnage qui s'y déplace et s'y
insère visuellement. À viser tôt — c'est ce qui fait apparaître les frictions réelles bien avant
qu'on ait investi dans les règles et le combat.

## Exigences couvertes

`EX-REN-*` : tri par Y, décor devant/derrière selon le pied du sprite, sprites directionnels.

## Critères d'acceptation

- Test `QuadRecorder` (sans GPU, `EX-NFR-004`) : trois sprites à Y croissants sortent dans l'ordre
  attendu.
- Aucun scintillement d'ordre entre deux images à Y égal — le tri est déjà stable, un test le fige.
- Le personnage passe visiblement derrière un arbre situé plus bas, devant un arbre situé plus haut.
