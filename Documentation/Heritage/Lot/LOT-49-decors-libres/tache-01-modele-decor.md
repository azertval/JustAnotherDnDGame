# TACHE-01 — *%core::Decor* : modèle, couches et sérialisation {#lot-49-tache-01-modele-decor}

**Lot :** [LOT-49](epic.md) · **Emplacement :** `Source/Core/Levels` · **Statut :** fait

## Contexte
Tout ce que contient un niveau est aujourd'hui **calé sur la grille** : `core::TileMap` est une
grille dense d'une `TileType` par case, et les données annexes (`Mechanism`, `DangerLink`,
`DangerMoverConfig`, `TileTextureOverride`) sont keyées par une position **entière**.

Un décor est le premier objet du projet à ne pas l'être : `EX-DEC-001` demande un objet **libre**,
doté d'un transform en unités monde. C'est la seule vraie nouveauté de modèle du programme, et elle
reste modeste — le patron de vecteur annexe s'applique tel quel, seule la clé change de nature.

## Travail à réaliser
- **`%core::Decor`** : nom d'asset (chaîne), position en unités monde (**flottante**), échelle,
  rotation, couche, et l'indicateur statique/manipulable de `EX-DEC-005`. `struct` de données pures.
- **`%core::DecorLayer` `{ Background, Decor, Foreground }`** (`EX-DEC-002`). `Core` ne connaît pas
  *%RenderLayer* : la projection vers les calques de rendu est le travail de `HMI`.
- **Vecteur annexe** sur `Level` (lecture) et `LevelDraft` (mutation), sur le patron
  `Mechanism`/`DangerLink` de `Source/Core/Levels/Level.h`.
- **JSON** : tableau racine optionnel `"decors": [...]`, dans le format versionné de LOT-44 —
  rétrocompatible, omis quand vide.
- **Ordre significatif** : l'ordre dans le vecteur détermine la superposition **à l'intérieur** d'une
  couche. Ce n'est pas un détail d'implémentation, c'est une donnée — le sérialiser et le préserver.
- **Historique** : champ ajouté à `LevelDraft::State`, annulation/rétablissement gratuits.
- **Redimensionnement** : décider explicitement du sort d'un décor hors des nouvelles bornes après
  `resize`. Contrairement aux données annexes keyées par case, un décor libre peut légitimement
  déborder du niveau (une branche qui dépasse) — le tronquer serait une perte de travail. Conserver,
  et documenter ce choix comme une différence assumée avec les autres données annexes.

## Fichiers impactés
- `Source/Core/Levels/Decor.h` (nouveau), `Level.h`, `LevelDraft.{h,cpp}`, `LevelLoader.cpp`,
  `LevelWriter.cpp`.
- `Source/Test/Unit/Core/Levels/test_decor.cpp` (nouveau).

## Tests (obligatoires)
- Round-trip JSON : aucun décor, un décor, plusieurs décors sur des couches différentes, positions
  fractionnaires, rotation et échelle non par défaut.
- **Ordre préservé** à l'écriture et à la relecture.
- Rétrocompatibilité : un niveau existant sans `"decors"` se charge à l'identique.
- Annulation/rétablissement d'un ajout, d'une suppression, d'une modification.
- Redimensionnement : les décors hors bornes sont conservés, conformément au choix documenté.
- Tout dans `Core`, sans GPU.

## Points d'attention
- **Position flottante** : attention aux comparaisons d'égalité dans les tests et à la précision de
  sérialisation JSON. Fixer une précision d'écriture pour que le round-trip soit exact.
- Le nom d'asset est une **chaîne** : `Core` ne vérifie pas son existence, et un décor pointant un
  fichier absent reste un niveau **valide** (`EX-LVL-004` ne le rejette pas).
- `EX-DEC-004` demande des « entités ECS de la simulation ». Le vecteur annexe assure la
  sérialisation et le déterminisme ; les entités ECS sont créées à la construction de la scène,
  comme pour les tuiles (`core::buildLevelScene`). Ne pas introduire de second mécanisme de
  persistance.
- L'indicateur `manipulable` est stocké et sérialisé, mais **sans effet** : `EX-DEC-020`/`EX-DEC-021`
  (manipulation en jeu) restent hors programme.

## Définition de fait (DoD)
- *%core::Decor* existe, transite par le JSON de façon rétrocompatible en préservant son ordre, est
  couvert par l'annulation, et le comportement au redimensionnement est documenté ; tests `Core`
  verts ; `/W4 /WX` propre.

## Exigences
`EX-DEC-001` (objet libre avec transform), `EX-DEC-002` (couches), `EX-DEC-004` (entité
sérialisable), `EX-DEC-005` (statique ou manipulable) ; réutilise `EX-LVL-003` (format JSON),
`EX-LVL-005` (version), `EX-EDIT-005` (annuler/refaire), `EX-NFR-011` (frontière `Core`/`HMI`).
