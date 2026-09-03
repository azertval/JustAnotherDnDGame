# TACHE-01 — `TileTextureOverride` : modèle, JSON, nettoyage et historique {#lot-45-tache-01-modele-override}

**Lot :** [LOT-45](epic.md) · **Emplacement :** `Source/Core/Levels` · **Statut :** fait

## Contexte
`core::TileMap` est une grille **dense** d'une `TileType` par case, sans métadonnée numérique par
case — limite actée en `LOT-19` et documentée dans `Level.h`. Toutes les données par position
(mécanismes, liens de danger, configurations de mouvement et de clignotement) vivent donc dans des
**vecteurs annexes** de `core::Level`, keyés par position.

Cette tâche ajoute le énième membre de cette famille. L'intérêt est qu'il n'y a **rien à inventer** :
le patron, son nettoyage et son historique existent et sont testés.

## Travail à réaliser
- ***core::TileTextureOverride*** : une position de grille et un nom d'asset. Vecteur annexe sur
  `Level` et `LevelDraft`, accesseur de lecture, mutateurs d'assignation et de retrait.
- **JSON** : champ optionnel **par tuile** (ex. `"texture": "door_red.png"`), exactement comme
  `switch.id` et `door.opensWith`. Rétrocompatible, dans le format versionné de LOT-44.
- **Nettoyage** : brancher l'override sur le funnel **existant** `LevelDraft::removeLinkedDataAt`,
  appelé par `paintTileInternal` — donc par `paintTile` **et** `paintRegion`. Repeindre un autre type
  sur une case retire son override.
- **Redimensionnement** : participer à la troncature silencieuse de `resize`, comme les autres
  données annexes — un override hors des nouvelles bornes disparaît.
- **Copier-coller** : `paintRegion` utilisé pour le collage n'importe **pas** les overrides de la
  source. Décision explicite : un override reste attaché à sa case d'origine.
- **Historique** : gratuit via `LevelDraft::State`, à condition que le champ y soit ajouté.

## Fichiers impactés
- `Source/Core/Levels/Level.h`, `LevelDraft.{h,cpp}`, `LevelLoader.cpp`, `LevelWriter.cpp`.
- `Source/Test/Unit/Core/Levels/test_tile_texture_override.cpp` (nouveau).

## Tests (obligatoires)
- Round-trip JSON : assignation présente, absente, plusieurs overrides, niveau existant sans le
  champ (rétrocompatibilité).
- Nettoyage : repeindre un autre type retire l'override ; repeindre le **même** type le conserve ;
  `paintRegion` nettoie aussi ; `resize` tronque.
- Copier-coller : les overrides de la région source ne sont pas copiés.
- Annulation/rétablissement d'une assignation et d'un retrait.
- Tout dans `Core`, sans GPU.

## Points d'attention
- **Un seul point de nettoyage.** Ne pas ajouter de chemin parallèle : `removeLinkedDataAt` est le
  funnel, tout le reste en découle. Un second mécanisme divergerait au premier cas particulier.
- Repeindre le **même** type sur une case ne doit pas retirer l'override — sinon un coup de pinceau
  involontaire effacerait un travail d'habillage. Vérifier le comportement exact de
  `paintTileInternal` sur ce point.
- Le nom d'asset est une **chaîne** : `Core` ne vérifie pas son existence (il n'a pas accès au
  dossier d'assets), et un override pointant un fichier absent reste un niveau **valide**.
- Aucune liste blanche de types éligibles : tout type non vide peut recevoir un override, car
  l'usage est purement visuel — contrairement aux liens de mécanismes qui, eux, ont une sémantique.

## Définition de fait (DoD)
- Le modèle existe, transite par le JSON de façon rétrocompatible, est nettoyé par le funnel unique,
  tronqué au redimensionnement, absent du copier-coller, et couvert par l'annulation ; tests `Core`
  verts ; `/W4 /WX` propre.

## Exigences
`EX-EDIT-043` (texture par instance) ; réutilise `EX-LVL-003` (format JSON), `EX-LVL-005` (version),
`EX-EDIT-005` (annuler/refaire), `EX-EDIT-010` (réutilisation du modèle `Core`), `EX-NFR-011`
(pas de dépendance `Core → HMI`).
