# TACHE-01 — Modèle et contrôleur (Core) {#lot-21-tache-01-controleur-blocs}

**Lot :** [LOT-21](epic.md) · **Emplacement :** `Core/Levels`, `Core/Gameplay` · **Statut :** fait

## Contexte
`EX-GP-022` demande un bloc poussable. `TileType` ne porte que des tuiles statiques ou à état ;
cette tâche ajoute la tuile et la logique qui la fait bouger, sans dépendance rendu ni fenêtre.

## Travail à réaliser
- **`Core/Levels/TileType.h`** : nouvelle valeur `Block` ; `isSolid(TileType)` renvoie `true` pour
  `Block` (un bloc non encore déplacé bloque comme un mur).
- **`Core/Levels/LevelLoader.cpp`** : `parseTileType` reconnaît `"block"`.
- **`Core/Levels/LevelWriter.cpp`** : `tileTypeName` sérialise `TileType::Block` en `"block"`.
- **`Core/Gameplay/BlockController.h`/`.cpp`** (nouveau), sur le modèle de `MechanismController` :
  - Construit à partir d'un `core::Level` : repère chaque tuile `Block` comme un bloc mobile
    (`std::vector<GridPosition> _positions`) ; un minuteur de chute par bloc
    (`std::vector<int> _fallTimers`).
  - `update(playerBox, moveIntentX, baseCollision)` : tente une poussée (case suivante dans la
    direction de `moveIntentX`, si libre), puis fait avancer les minuteurs de chute (une case
    entière au bout de `FALL_INTERVAL_STEPS` pas si non soutenu).
  - `collisionMap(base)` : copie de `base` où chaque case `Block` d'origine est d'abord **effacée**
    (`TileType::Empty`), puis chaque position **courante** est reposée comme `TileType::Solid`.
  - `positions()` : accesseur, pour que `HMI` puisse repositionner les entités-tuiles visuelles.
- **`Source/Core/CMakeLists.txt`** : ajoute `Gameplay/BlockController.cpp` à la bibliothèque `Core`.

## Fichiers impactés
- `Source/Core/Levels/TileType.h`.
- `Source/Core/Levels/LevelLoader.cpp`, `LevelWriter.cpp`.
- `Source/Core/Gameplay/BlockController.h`/`.cpp` (nouveau).
- `Source/Core/CMakeLists.txt`.
- Tests : `Source/Test/Unit/Core/Gameplay/test_block_controller.cpp` (nouveau),
  `Source/Test/Unit/Core/Levels/test_level_loader.cpp`,
  `Source/Test/Unit/Core/Levels/test_level_writer.cpp`.

## Tests (obligatoires)
- Position initiale d'un bloc conforme au niveau chargé ; case correspondante solide.
- Poussée acceptée si la case suivante est libre ; refusée contre un mur ou un autre bloc.
- Sans intention de déplacement horizontal, un bloc touché ne bouge pas.
- Un bloc soutenu (sol juste en dessous) ne tombe jamais ; un bloc non soutenu tombe d'une case au
  bout de `FALL_INTERVAL_STEPS` mises à jour, puis s'arrête une fois posé.
- `collisionMap` n'expose plus l'ancienne position d'un bloc déplacé comme solide (pas de mur
  fantôme).
- Chargement/écriture/rechargement (round-trip) d'un niveau contenant un bloc : type et absence de
  liaison de mécanisme préservés.

## Points d'attention
- **`isFree` ignore la tuile `Block` de `base`.** `base` (la grille passée en paramètre) porte la
  position **d'origine** de chaque bloc, jamais mise à jour — seule `_positions` (courante) fait
  autorité sur l'occupation par un bloc. Une implémentation qui testerait `isSolid(base.tile(...))`
  sans cette exception verrait indéfiniment la case de départ comme solide, même après que le bloc
  s'en soit éloigné.
- **`FALL_INTERVAL_STEPS`** est un choix délibéré de chute discrète, pas une approximation à
  corriger plus tard (voir décision de cadrage de l'épic) : ne pas le remplacer par une intégration
  continue sans en rediscuter l'architecture (interaction avec `sweepAabb`, rendu).

## Définition de fait (DoD)
- `BlockController` compilé, testé (`ctest` vert), sans avertissement (`/W4 /WX`) ; `TileType::Block`
  chargeable/sérialisable ; aucune régression sur les mécanismes existants.

## Exigences
`EX-GP-022` (déjà déclarée dans `gameplay.md`, implémentation).
