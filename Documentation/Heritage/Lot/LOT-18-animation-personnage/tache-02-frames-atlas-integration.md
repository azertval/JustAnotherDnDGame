# TACHE-02 — Images dans l'atlas et intégration au rendu {#lot-18-tache-02-frames-atlas-integration}

**Lot :** [LOT-18](epic.md) · **Emplacement :** `HMI/Graphics`, `HMI/Interface` · **Statut :** fait

## Contexte
`TextureAtlas` expose aujourd'hui une seule région carrée pour le personnage (`playerRegion()`,
LOT-17). Ce lot la remplace par une **grille de 7 images** (`Idle`×2, `Run`×4, `Jump`×1) et branche
le composant `core::Animation` (TACHE-01) sur le rendu, pour que le sprite change de région à
chaque frame plutôt qu'une seule fois au spawn.

## Travail à réaliser
- **`hmi::TextureAtlas`** (`Source/HMI/Graphics/TextureAtlas.h`/`.cpp`) :
  - Remplace `playerRegion()`/`PLAYER_REGION_WIDTH`/`PLAYER_REGION_HEIGHT` par une **grille de
    frames** : `PLAYER_FRAME_SIZE = TILE_SIZE` (16, carrée — ne pas régresser sur la correction de
    LOT-17), `PLAYER_FRAME_COLUMNS = 4` (largeur de la grille existante, réutilisée telle quelle).
    Le total d'images (`core::IDLE_FRAME_COUNT + core::RUN_FRAME_COUNT + core::JUMP_FRAME_COUNT`
    = 7) détermine le nombre de lignes nécessaires (`ceil(7 / 4)` = 2), donc une bande de 32 pixels
    sous la grille de tuiles (au lieu de 16 en LOT-17).
  - Ordre des images dans la grille (index plat 0-6, ligne = index/4, colonne = index%4) : les 2
    images `Idle`, puis les 4 `Run`, puis la `Jump` — un seul ordre, pas de table de correspondance
    séparée à maintenir en cohérence avec `Core`.
  - `playerPixel(x, y, pose)` (namespace anonyme) : généralise le dessin de silhouette de LOT-17
    avec un paramètre de pose (variation des jambes/bras selon l'image — foulée alternée pour
    `Run`, bras relâchés vs légèrement resserrés pour `Idle`, jambes resserrées et bras levés pour
    `Jump`), toujours par blocs rectangulaires, dans le même canevas 16×16.
  - Nouvel accesseur `core::AtlasRegion playerFrameRegion(core::AnimationClip clip, int
    frameIndex) const`.
- **`hmi::GameScreen`** (`Source/HMI/Interface/GameScreen.cpp`) :
  - `spawnPlayer` : ajoute `core::Animation{}` à l'entité joueur (au lieu de fixer `sprite.region`
    une fois — la première image est posée par le premier appel du système, cf. ci-dessous).
  - `update` : après `_physics.update(...)`, exécute `core::AnimationSystem` sur `_world` (ordre
    important, cf. TACHE-01).
  - `render` (ou juste avant) : lit `core::Animation` de l'entité joueur et met à jour
    `sprite.region = _atlas.playerFrameRegion(animation.clip, animation.frameIndex)` — chaque
    frame de rendu, pas seulement au spawn.

## Fichiers impactés
- `Source/HMI/Graphics/TextureAtlas.h`/`.cpp`.
- `Source/HMI/Interface/GameScreen.h`/`.cpp`.

## Tests (obligatoires)
- Aucun test unitaire nouveau côté `HMI` (génération de texture non testable hors GPU, comme
  LOT-17) : la logique de clip/image reste entièrement couverte côté `Core` (TACHE-01).
- `ctest --preset ninja` reste vert (aucune régression sur les tests existants).
- **Vérification visuelle obligatoire** dans l'application compilée (comme LOT-17) : capture
  d'écran de chaque état (immobile, en course, en l'air) confirmant une silhouette lisible, sans
  déformation, bornée à la boîte de collision.

## Points d'attention
- **Ne pas répéter le bug de LOT-17** : chaque région de frame doit rester carrée (16×16) — c'est
  `Transform::scale` qui donne la proportion finale à l'écran, jamais la région elle-même.
- **Une seule texture par passe de rendu** (`SpriteRenderer::render`) : toutes les images restent
  dans la même texture `TextureAtlas`, comme la silhouette statique de LOT-17.
- L'entité joueur doit porter `core::Animation` **avant** le premier appel à
  `AnimationSystem::update` dans la boucle (ajouté dans `spawnPlayer`, avant que `update()` ne soit
  appelé) — sans quoi la vue `Player + Velocity + Animation` ne verrait pas l'entité et le sprite
  garderait une région par défaut (`AtlasRegion{}`, vide/invisible).
- Vérifier que le personnage **rechargé** (échec de niveau, `loadLevel`) reçoit bien un nouveau
  composant `Animation` à l'état initial (`spawnPlayer` est déjà appelé à chaque rechargement,
  cf. LOT-09) — pas d'état d'animation qui « fuit » d'un essai à l'autre.

## Définition de fait (DoD)
- Personnage animé (repos/course/saut) visible en jeu ; build `/W4 /WX` sans avertissement ;
  `ctest` vert ; Doxygen local sans sortie ; vérifié visuellement (captures des trois états).

## Exigences
`EX-REN-012` (rendu des images, partie `HMI`).
