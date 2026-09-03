# LOT-04 — Format de carte `version: 3`, multi-couches {#lot-04}

> Statut : **à faire**.
> Prérequis : [LOT-03](@ref lot-03) (`LevelData` accueille les nouveaux champs sans repasser à 17
> paramètres positionnels).

## Objectif

Passer le format de carte d'une **grille plate unique** à **N couches typées**, plus une liste
d'entités — la seule évolution de format structurante du programme.

## Pourquoi maintenant, et pas plus tard

Un RPG en vue de dessus a besoin d'au moins trois couches là où un platformer se contentait d'une :

- **sol** (herbe, dalle, eau) — ce qu'on voit sous les pieds ;
- **décor** (arbre, tonneau, tapis) — dessiné au-dessus du sol, et **devant ou derrière** le
  personnage selon sa position (cf. `LOT-07`) ;
- **collision** — masque indépendant du visuel : un tapis se traverse, un tonneau non, et les deux
  peuvent partager la même image de sol.

Plus une couche **`objects`** qui n'est pas une grille mais une **liste** : PNJ, coffres,
panneaux, portails, déclencheurs de rencontre.

Ce lot doit précéder toute production de carte. Une carte dessinée sur le format plat serait à
refaire — et c'est le genre de dette qu'on ne repaie jamais.

## Périmètre

- `struct TileLayer { std::string name; LayerKind kind; TileMap tiles; int renderLayer; }` ;
  `Level` porte un `std::vector<TileLayer>`.
- **Migration ascendante** : le chargeur promeut une grille `version: 2` en couche unique de
  `kind = Legacy`. Le mécanisme existe déjà — `LevelLoader.cpp` traite l'absence de champ `version`
  comme la version initiale, et refuse proprement une version qu'il ne connaît pas.
- `TileAutotile` (raccords 16 voisinages) s'applique **par couche**, sans modification.
- `LevelScene::buildLevelScene` boucle sur les couches.
- La grille de **collision** devient la source de vérité du balayage AABB — et, au `LOT-19`, de la
  grille de combat tactique.

## Le point à ne pas rater

**Prévoir dès maintenant un dictionnaire de propriétés libres** par couche et par entité, et la
**tolérance aux champs inconnus**. Sans cela, les besoins du combat (terrain difficile, couverture,
hauteur — découverts en phase D) imposeraient un `version: 4` en plein milieu du programme, avec
migration de tout le contenu déjà produit. C'est le risque numéro un de ce lot.

## Exigences couvertes

`EX-LVL-*` (multi-couches, migration ascendante, tolérance aux champs inconnus), extension
`EX-EDIT-*`.

## Critères d'acceptation

- Aller-retour chargement → écriture → rechargement **à l'identique** sur une carte à trois couches
  et une liste d'entités.
- Une carte `version: 2` se charge sans erreur, promue en couche unique.
- Une `version: 4` est refusée avec un message explicite (`EX-NFR-040`).
- Un champ inconnu dans une couche ou une entité est **ignoré sans erreur**, et **préservé** à la
  réécriture.
