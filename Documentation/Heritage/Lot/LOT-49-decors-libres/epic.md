# LOT-49 — Décors libres : modèle, rendu et parallaxe {#lot-49}

> Statut : **fait**. Prérequis : [LOT-40](@ref lot-40) (calques), [LOT-41](@ref lot-41)
> (bascule), [LOT-44](@ref lot-44) (format de niveau versionné). Concrétise
> [`decors.md`](../../Specification/decors.md).

## Objectif
Livrer les **calques de décor** — arrière-plan, décor, premier plan — demandés dès l'origine et
absents du cadrage initial du programme, qui ne prévoyait qu'un fond et des tuiles habillées.

Deux manques concrets, aujourd'hui :

- Le `core::TileMap` est une grille dense d'**une** `TileType` par case. Il est impossible d'y poser
  un buisson, une torche ou une colonne qui ne soit pas une tuile de gameplay.
- Rien n'est jamais dessiné **au-dessus** du personnage. Or c'est précisément ce qui permet au joueur
  de comprendre d'un coup d'œil ce qui est physique : ce qui passe devant lui ne le porte pas et ne
  le bloque pas.

Ce système est déjà **spécifié** (`EX-DEC-001` à `EX-DEC-005`, `EX-DEC-010`) depuis longtemps, sans
avoir jamais été rattaché à un lot.

## Périmètre

### Inclus
- **`Core`** : *%core::Decor* — nom d'asset, position en unités monde (**flottante, hors grille**),
  échelle, rotation, couche, et l'indicateur statique/manipulable de `EX-DEC-005`. Vecteur annexe
  sur `Level`/`LevelDraft`, **exactement** le patron `Mechanism`/`DangerLink`
  (`Source/Core/Levels/Level.h`). Données pures, aucun handle de texture.
- **`%core::DecorLayer` `{ Background, Decor, Foreground }`** (`EX-DEC-002`), projeté sur les valeurs
  correspondantes de *%RenderLayer* côté `HMI`. `Core` ne connaît pas *%RenderLayer*.
- **JSON** : tableau racine optionnel `"decors": [...]`, dans le format versionné de LOT-44 —
  rétrocompatible. Undo/redo gratuit via `LevelDraft::State`.
- **Rendu** sur les trois calques, en mode Texture uniquement. Le calque *Foreground* est **au-dessus
  du personnage** (contrat posé en LOT-40).
- **Parallaxe** (`EX-DEC-006`) : facteur de défilement par couche, appliqué au rendu uniquement.
- **Traversabilité** : un décor ne participe **jamais** aux collisions. Aucune ligne de physique
  n'est touchée.
- Dossier `Assets/Decors/` + commande `POST_BUILD`.
- **Placement minimal** dans l'éditeur, juste assez pour voir le résultat : poser un décor sur une
  couche et le supprimer. La manipulation complète (déplacer, redimensionner, pivoter, réordonner)
  relève de LOT-50.

### Exclus (hors périmètre de ce lot)
- **Manipulation par le joueur en cours de partie** (`EX-DEC-020`/`EX-DEC-021`) : reste
  post-programme. L'indicateur `manipulable` est stocké et sérialisé dès maintenant, mais n'a aucun
  effet en jeu.
- **Pipeline photo → pixel art** (`EX-DEC-030` à `EX-DEC-032`, `EX-EDIT-041`) : hors programme.
- Outils d'édition avancés (LOT-50), visibilité par calque (LOT-51).
- Décors animés : gratuits si l'asset désigné porte un `nom-asset.anim.json` (LOT-46), aucune
  disposition particulière n'est nécessaire.

## Décisions de cadrage
- **Décor = donnée de niveau, pas entité de gameplay.** `EX-DEC-004` demande des entités ECS
  sérialisables ; le patron de vecteur annexe sur `Level` répond à la sérialisation et au
  déterminisme, et le monde ECS est peuplé à la construction de la scène comme pour les tuiles
  (`core::buildLevelScene`). Aucun nouveau mécanisme de persistance.
- **Position flottante, aucune contrainte de grille** (`EX-DEC-001`). `core::Transform::position`
  est déjà en unités monde flottantes : `SpriteRenderer` n'a **rien** à changer pour afficher un
  décor hors grille.
- **Parallaxe relative au centre de la salle courante** (arbitré). La caméra bascule d'une salle à
  l'autre sans transition (`EX-REN-015`, `LOT-32`), alors que la parallaxe suppose un défilement
  continu. Le décalage est donc calculé depuis la position **dans la salle**, pas dans le niveau :
  le décor se replace à chaque salle, et la parallaxe n'agit qu'à l'intérieur d'une salle.
  L'alternative — un décalage absolu en espace niveau — ferait sauter visiblement le décor à chaque
  bascule, un artefact ; le replacement par salle, lui, a lieu au moment exact où toute l'image
  change déjà, donc invisible. Le décalage doit également être pris en compte par le culling
  (LOT-40, TACHE-05) : une couche parallaxée n'occupe pas le même rectangle monde que le niveau.
- **Aucune collision, jamais.** Un décor qui pourrait bloquer serait une tuile ; la distinction
  décor/physique est le sujet même du lot.

## Exigences couvertes
- Nouvelle : `EX-DEC-006` (facteur de défilement par couche).
- Concrétisées : `EX-DEC-001` (objet libre avec transform), `EX-DEC-002` (trois couches),
  `EX-DEC-003` (rendu pixel art net), `EX-DEC-004` (entités de la simulation, sérialisables),
  `EX-DEC-005` (statique ou manipulable).
- Réutilisées : `EX-REN-014` (ordonnancement des couches), `EX-REN-043` (multi-textures),
  `EX-REN-046` (bascule), `EX-LVL-005` (format versionné), `EX-ARCH-012` (rendu sans effet sur la
  simulation), `EX-NFR-005` (culling).

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-modele-decor.md) | *%core::Decor* et `DecorLayer` sur `Level`/`LevelDraft` + JSON rétrocompatible + undo/redo | `Source/Core/Levels` | ✅ |
| [TACHE-02](tache-02-rendu-couches.md) | Rendu des trois couches, premier plan au-dessus du personnage | `Source/HMI/Graphics` | ✅ |
| [TACHE-03](tache-03-parallaxe.md) | Parallaxe : arbitrage avec la caméra par salle, application au rendu et au culling | `Source/HMI/Graphics` | ✅ |
| [TACHE-04](tache-04-placement-minimal.md) | Placement minimal dans l'éditeur + dossier `Assets/Decors/` + `POST_BUILD` | `Source/HMI/Editor`, `Source/HMI/CMakeLists.txt` | ✅ |

## Critères d'acceptation du lot
1. Un décor posé sur la couche premier plan est dessiné **au-dessus** du personnage ; sur les couches
   arrière, en dessous des tuiles — asserté via le *QuadRecorder*.
2. Un décor ne bloque jamais le personnage : les tests de franchissabilité des niveaux passent sans
   modification, décors ajoutés ou non.
3. Un décor peut être placé à une position non alignée sur la grille.
4. La parallaxe produit un défilement différencié entre couches à l'intérieur d'une salle, et le
   décalage se recalcule depuis le centre de la nouvelle salle à chaque bascule.
5. Un niveau existant (sans décors) se charge sans changement de comportement.
6. Round-trip JSON, undo/redo et projection couche → calque testés sans GPU ; build `/W4 /WX`,
   Doxygen, lint verts.

## Dépendances
Bâtit sur [LOT-40](@ref lot-40) (calques, culling), [LOT-41](@ref lot-41) (bascule) et
[LOT-44](@ref lot-44) (format versionné, patron de données annexes). Bénéficie de
[LOT-46](@ref lot-46) (décors animés) et de [LOT-43](@ref lot-43) (import d'assets). Prérequis de
[LOT-50](@ref lot-50) ; étend le périmètre de [LOT-51](@ref lot-51).

## Navigation des tâches
- @subpage lot-49-tache-01-modele-decor
- @subpage lot-49-tache-02-rendu-couches
- @subpage lot-49-tache-03-parallaxe
- @subpage lot-49-tache-04-placement-minimal
