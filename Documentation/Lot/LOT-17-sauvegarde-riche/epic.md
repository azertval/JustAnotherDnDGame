# LOT-17 — Sauvegarde riche {#lot-17}

> Statut : **à faire**.
> Prérequis : [LOT-09](@ref lot-09), [LOT-10](@ref lot-10), [LOT-13](@ref lot-13),
> [LOT-16](@ref lot-16).

## Objectif

Remplacer la progression héritée — qui ne retenait qu'un **tableau atteint** — par un état de
partie complet et versionné.

## Le problème

`hmi::Progression` stocke un identifiant de séquence, un nom de tableau courant et un ensemble de
tableaux terminés. C'était suffisant pour un jeu de niveaux discrets ; c'est sans rapport avec un
monde persistant où le joueur a une position, un inventaire, des quêtes en cours et un monde qui a
gardé la trace de ses actes.

## Périmètre

`Source/Core/Rpg/SaveGame.{h,cpp}` : JSON **versionné**, portant

- la carte courante et la position exacte du personnage ;
- la **liste** des personnages (décision de cadrage : un héros au départ, quatre à terme — c'est
  une liste dès maintenant, pas un champ unique qu'on pluraliserait plus tard) ;
- l'inventaire et l'équipement ;
- les drapeaux de monde et les entités consommées (coffres, ennemis vaincus) ;
- les quêtes et leur étape.

`hmi::Progression` disparaît ou devient un cas dégénéré.

## Règles de format

Mêmes règles que le format de carte, pour les mêmes raisons : **versionné**, **tolérant aux champs
inconnus**, migration ascendante. Une sauvegarde est la donnée que le joueur ne peut pas
reconstruire — un format qui casse lui fait perdre sa partie.

## Exigences couvertes

`EX-RPG-*` (sauvegarde, chargement, versionnement, tolérance).

## Critères d'acceptation

- Aller-retour sauvegarde → chargement **à l'identique**, sur un état riche (plusieurs quêtes en
  cours, inventaire garni, plusieurs cartes visitées).
- Une sauvegarde d'une version antérieure se charge avec des valeurs par défaut sensées.
- Un champ inconnu est ignoré **et préservé** à la réécriture.
- Testable headless.
