# LOT-29 — Groupe de quatre personnages {#lot-29}

> Statut : **à faire**.
> Prérequis : [LOT-27](@ref lot-27). Vient **après** le *vertical slice*, délibérément.

## Objectif

Passer d'un héros seul à un groupe de quatre — recrutement, compagnons suiveurs en exploration,
combat tactique à quatre alliés.

## Pourquoi ce lot est un lot d'**ajout**, pas une refonte

C'est la décision de cadrage n° 4, et tout le programme la prépare : **rien ne doit supposer
l'unicité du personnage**. Concrètement, au moment d'aborder ce lot, ces précautions doivent déjà
être en place :

- `CharacterSheet` est un objet **autonome** (`LOT-13`), jamais un singleton joueur ;
- `TurnOrder` est **multi-alliés** dès le `LOT-20`, avec un test qui monte quatre alliés ;
- `SaveGame` stocke une **liste** de personnages dès le `LOT-17`, pas un champ unique ;
- les écrans de fiche et d'inventaire sont conçus pour un **sélecteur de personnage**, même quand
  ils n'en affichent qu'un.

Si l'un de ces points a dérivé en chemin, ce lot redevient une refonte — c'est le signal
d'alarme à surveiller pendant les phases C et D.

## Périmètre

- **Recrutement** de compagnons (dialogue, quête).
- **Personnages suiveurs** en exploration : ordre de marche, suivi du héros, pathing simple.
- **Sélecteur de personnage** activé dans les écrans de fiche, d'inventaire et d'équipement.
- Quatre alliés dans l'ordre d'initiative ; **ciblage allié** (soins, sorts de soutien).
- Répartition de l'expérience et du butin.

## Le point délicat

Le combat à quatre alliés multiplie les tours et allonge la boucle : l'IHM du `LOT-24` doit rester
lisible avec huit combattants au bandeau d'initiative. C'est le seul endroit où ce lot peut exiger
un vrai travail d'interface plutôt qu'un simple ajout.

## Exigences couvertes

`EX-RPG-*`, `EX-CBT-*`, `EX-IHM-*`.

## Critères d'acceptation

- Un combat à **4 alliés contre 4 ennemis** se déroule en headless jusqu'à une condition de fin.
- **Aucune régression** du jeu à un personnage : le contenu du `LOT-27` reste jouable tel quel.
- Les suiveurs ne restent pas coincés dans le décor ni ne bloquent le héros dans un passage étroit.
- Le bandeau d'initiative reste lisible à huit combattants.
