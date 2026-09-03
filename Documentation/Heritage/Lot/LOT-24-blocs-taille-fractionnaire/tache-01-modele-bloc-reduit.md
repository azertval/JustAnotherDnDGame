# TACHE-01 — Modèle de bloc réduit {#lot-24-tache-01-modele-bloc-reduit}

**Lot :** [LOT-24](epic.md) · **Emplacement :** `Core/Levels`, `Core/Gameplay` · **Statut :** fait

## Contexte
Pose le vocabulaire (types de tuiles, facteur de taille) avant la routine de collision propre
(TACHE-02) — sur le modèle de `LOT-21` (`Block`) et `LOT-22`/`LOT-23` (types + géométrie séparés
de la résolution physique).

## Travail à réaliser
- **`Core/Levels/TileType.h`** : deux nouvelles valeurs, `BlockHalf` et `BlockQuarter`. `isSolid`
  renvoie `true` pour les deux (comme `Block` : solides tant qu'ils n'ont pas été résolus par
  `BlockController`, voir `LOT-21`).
- **`Core/Gameplay/BlockController`** : reconnaît les trois types de bloc (`Block`, `BlockHalf`,
  `BlockQuarter`) à la construction, et associe à chaque position un **facteur de taille**
  (`1.0`/`0.5`/`0.25`) — probablement un `std::vector<float> _scales` parallèle à `_positions`
  (même index), sur le modèle de `_fallTimers`.
- **`Core/Levels/LevelLoader.cpp`/`LevelWriter.cpp`** : reconnaît/sérialise
  `"blockHalf"`/`"blockQuarter"`.

## Fichiers impactés
- `Source/Core/Levels/TileType.h`.
- `Source/Core/Gameplay/BlockController.h`/`.cpp`.
- `Source/Core/Levels/LevelLoader.cpp`, `LevelWriter.cpp`.
- Tests : `Source/Test/Unit/Core/Gameplay/test_block_controller.cpp` (nouveaux cas taille),
  `test_level_loader.cpp`, `test_level_writer.cpp`.

## Tests (obligatoires)
- Un niveau avec les trois types de bloc (`Block`/`BlockHalf`/`BlockQuarter`) charge chacun avec le
  facteur de taille attendu.
- Poussée et chute d'un bloc réduit suivent exactement la même logique **case par case** qu'un bloc
  plein (réutilisation directe des tests de `LOT-21`, appliqués aux nouveaux types).
- Round-trip JSON préserve le type (donc le facteur de taille, dérivé du type à la relecture).

## Points d'attention
- **Le facteur de taille ne change rien à la poussée/chute elle-même** (toujours case par case,
  voir décision de cadrage de l'épic) — seule la boîte de collision utilisée par TACHE-02 diffère.
  Ne pas introduire de position continue ici : ce serait une régression vers la complexité que
  `LOT-21` a justement évitée pour les mécanismes de ce moteur.

## Définition de fait (DoD)
- Types de bloc réduit chargeables/sérialisables, facteur de taille exposé par `BlockController` ;
  poussée/chute identiques à un bloc plein.

## Exigences
`EX-GP-005` (déjà déclarée dans `gameplay.md`, modèle de données).
