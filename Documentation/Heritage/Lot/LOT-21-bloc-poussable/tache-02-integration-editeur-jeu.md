# TACHE-02 — Intégration éditeur et jeu (HMI) {#lot-21-tache-02-integration-editeur-jeu}

**Lot :** [LOT-21](epic.md) · **Emplacement :** `HMI/Editor`, `HMI/Graphics`, `HMI/Interface` · **Statut :** fait

## Contexte
`BlockController` (TACHE-01) est une logique pure ; cette tâche la rend **utilisable** : plaçable
dans l'éditeur, visible en jeu, et câblée dans la boucle de `GameScreen` au bon moment (avant la
physique du personnage, cf. décision de cadrage de l'épic).

## Travail à réaliser
- **`HMI/Editor/EditorLayout.h`** : `PALETTE_TYPE_COUNT` passe de 8 à 9 (dimensionnement du panneau
  latéral).
- **`HMI/Editor/TilePalette.cpp`** : `PALETTE_TYPES` gagne `core::TileType::Block` ; `labelFor`
  renvoie `"Bloc"`.
- **`HMI/Graphics/TileVisuals.cpp`** : `regionForTile` associe `TileType::Block` à une couleur
  d'atlas non utilisée (`atlas.tile(3, 1)`, violet).
- **`HMI/Interface/GameScreen.h`/`.cpp`** :
  - Nouveau membre `std::optional<core::BlockController> _blocks` + `std::vector<core::Entity>
    _blockEntities` (une entité-tuile par bloc, même ordre que `_blocks->positions()`).
  - `loadLevel` : construit `_blocks` et repère l'entité-tuile de chaque bloc (même principe que
    `_doorEntities`).
  - `update` : **avant** `_physics.update(...)`, appelle `_blocks->update(previousBox, intent.moveX,
    _mechanisms->collisionMap())` avec la boîte du personnage **du pas précédent**, puis
    `refreshBlockVisuals()` (repositionne les sprites), puis construit la grille de collision
    combinée (`_blocks->collisionMap(_mechanisms->collisionMap())`) passée à la physique.

## Fichiers impactés
- `Source/HMI/Editor/EditorLayout.h`, `TilePalette.h`/`.cpp`.
- `Source/HMI/Graphics/TileVisuals.cpp`.
- `Source/HMI/Interface/GameScreen.h`/`.cpp`.

## Tests (obligatoires)
- `ctest` existant (`test_tile_palette.cpp`) reste vert sans modification (utilise déjà
  `EXPECT_GE(entries().size(), 7u)`, pas un compte figé).
- **Vérification visuelle obligatoire** dans l'application compilée : peindre un bloc dans
  l'éditeur, l'enregistrer, le recharger, tester (`P`) — pousser le personnage contre le bloc le
  déplace ; le retirer de son support le fait tomber.

## Points d'attention
- **Ordre d'appel critique** : `_blocks->update(...)` doit s'exécuter avant
  `_physics.update(...)`, avec la boîte du personnage **avant** ce pas (pas après) — c'est ce qui
  garantit qu'un bloc qui vient de se dégager ne bloque jamais le personnage sur ce même pas
  (voir décision de cadrage de l'épic).
- **`_blocks->collisionMap(...)` recopie toute la grille à chaque pas fixe** (balayage complet,
  comme `MechanismController` le fait déjà pour son propre état) : acceptable pour les tailles de
  niveau de ce projet (`EX-NFR-010`), pas optimisé au-delà — cohérent avec le reste du moteur, qui
  privilégie la simplicité à la micro-optimisation prématurée.

## Définition de fait (DoD)
- Bloc plaçable dans l'éditeur, visible et fonctionnel en jeu (poussée + chute), sans régression
  sur les écrans existants ; build `/W4 /WX` sans avertissement.

## Exigences
Aucune exigence propre — intégration de `EX-GP-022` (TACHE-01) dans `HMI`.
