# TACHE-01 — Modèle de tuile et physique de suivi {#lot-26-tache-01-modele-physique-plafond}

**Lot :** [LOT-26](epic.md) · **Emplacement :** `Core/Levels`, `Core/Physics`, `Core/Ecs/Systems` · **Statut :** fait

## Contexte
Ajoute les quatre nouveaux types de tuile et leur physique de blocage, en réutilisant au maximum
l'infrastructure de suivi de surface posée par `LOT-22` (`core::slopeSurfaceHeight`,
`core::resolveSlopeFollow`) plutôt qu'une nouvelle famille de règles.

## Travail à réaliser
- **`Core/Levels/TileType.h`** : quatre nouveaux énumérateurs, `SlopeDownRight`/`SlopeDownLeft`/
  `RoundedDownRight`/`RoundedDownLeft` ; `core::isSolid` **ne** les inclut **pas** (comme leurs
  équivalents de sol — leur collision est résolue par suivi, pas par la grille classique).
- **`Core/Physics/SlopeGeometry.h`/`.cpp`** :
  - `core::isCeilingSlope(TileType)` — reconnaît les quatre types de plafond.
  - `core::ceilingSlopeHeight(TileType, localX)` — miroir vertical exact de
    `slopeSurfaceHeight` : mappe chaque type de plafond vers son type de sol miroir
    (`SlopeDownRight` → `SlopeUpRight`, etc.) et renvoie `1 - slopeSurfaceHeight(miroir, localX)`.
    Aucune formule physique dupliquée.
  - `core::resolveCeilingSlopeFollow(previousTopY, newBox, velocityY, tiles)` — miroir exact de
    `resolveSlopeFollow` : bord **haut** plutôt que bas, déclenché si `velocityY < 0` (monte,
    plutôt que `>= 0`, tombe), parcourt les lignes traversées par le bord haut pendant le pas
    (comme le sol, pour ne jamais « traverser » un plafond incliné à grande vitesse d'ascension),
    bloque dès que le bord haut atteint la silhouette — gère le franchissement **par en dessous**.
  - **`slopeSurfaceHeight` étendue** pour reconnaître aussi les quatre types de plafond, avec une
    hauteur **constante `0.0f`** (leur face du haut est toujours plate, au sommet de la case, quel
    que soit `localX`) : gère le cas **par au-dessus** (tomber sur le dessus d'un plafond incliné)
    en réutilisant `resolveSlopeFollow` tel quel, sans nouveau code de résolution — un gap découvert
    après la première passe de revue (voir la décision de cadrage de l'épic).
- **`Core/Ecs/Systems/CharacterPhysicsSystem.cpp`** : nouvelle étape (6ter), après le suivi de sol
  existant (6bis) — si non calé sur une pente de sol, appelle `resolveCeilingSlopeFollow` ; si
  bloqué, cale `transform.position.y` sur la silhouette et annule `velocity.value.y` (comme un
  choc contre un plafond classique).
- **`Core/Levels/LevelLoader.cpp`/`LevelWriter.cpp`** : mapping JSON ↔ enum (`slopeDownRight`,
  `slopeDownLeft`, `roundedDownRight`, `roundedDownLeft`).

## Fichiers impactés
- `Source/Core/Levels/TileType.h`.
- `Source/Core/Physics/SlopeGeometry.h`/`.cpp`.
- `Source/Core/Ecs/Systems/CharacterPhysicsSystem.cpp`.
- `Source/Core/Levels/LevelLoader.cpp`/`LevelWriter.cpp`.
- Tests : `Source/Test/Unit/Core/Physics/test_slope_geometry.cpp`,
  `Source/Test/Unit/Core/Levels/test_level_loader.cpp`/`test_level_writer.cpp`,
  `Source/Test/Integration/test_physique_personnage.cpp`.

## Tests (obligatoires)
- `isSolid`/`isFollowableSurface` renvoient `false` pour les quatre types de plafond ;
  `isCeilingSlope` les reconnaît exactement, aucun autre type.
- `ceilingSlopeHeight` est le miroir vertical exact de `slopeSurfaceHeight` pour chacune des quatre
  orientations (bords + centre, valeurs calculées à la main comme pour les variantes de sol).
- **Un saut qui franchit une pente/arrondi de plafond est bloqué selon sa silhouette réelle, pas
  comme un carré plein** : sous le bord fin (silhouette quasi vide), le personnage monte nettement
  plus haut que sous le bord épais (silhouette quasi pleine) — `PlafondInclineBloqueSelonSaSilhouette`.
- **Un personnage qui tombe sur le dessus d'une pente/arrondi de plafond s'y pose normalement**
  (face du haut plate), sans tomber au travers jusqu'à un sol lointain en dessous —
  `PlafondInclineSupportePersonnageParLeDessus`.
- Aller-retour JSON (chargement puis écriture) pour les quatre nouveaux types.
- **Suite de régression complète** : tous les tests physique existants (sol, pentes/arrondis de
  sol, murs, sauts, dash, wall jump…) restent verts, sans modification.

## Points d'attention
- **Écart au plan initial de l'épic** : la première implémentation rendait ces tuiles solides
  (`isSolid == true`), avec une silhouette purement visuelle. Revu en cours de tâche suite à la
  demande explicite d'une physique fidèle à la silhouette — voir la décision de cadrage de l'épic.
- **`resolveCeilingSlopeFollow` ne nécessite pas d'ajustement de tolérance avant `floor()`**,
  contrairement à `resolveSlopeFollow` : le parcours des lignes s'y fait en ordre **décroissant**
  (on monte), ce qui inclut déjà naturellement la ligne d'entrée sans risque de l'omettre par
  arrondi — l'ajustement `- kFollowTolerance` du sol répond à un problème spécifique à l'ordre
  **croissant** de son propre parcours, qui ne se pose pas ici. Détaillé en commentaire dans
  `SlopeGeometry.cpp`.
- **Deuxième écart, découvert après une première revue de la tâche** : `resolveCeilingSlopeFollow`
  ne couvre que le franchissement par en dessous (saut) ; rien ne gérait un personnage tombant sur
  le **dessus** d'une pente/arrondi de plafond, qui tombait au travers faute de toute résolution
  (ni `isSolid`, ni aucune passe existante). Corrigé en étendant le `switch` de `slopeSurfaceHeight`
  (physique de **sol**, pas plafond) pour reconnaître aussi ces quatre types avec une hauteur
  **constante `0.0f`** — leur face du haut est toujours plate, au sommet de la case, donc
  `resolveSlopeFollow` (déjà appelé chaque pas, sans changement) les traite correctement comme un
  sol normal, sans aucun code de résolution supplémentaire.

## Définition de fait (DoD)
- Physique de blocage fonctionnelle et testée (précision de silhouette démontrée par un test
  dédié) ; zéro régression sur la suite de tests physique existante ; build `/W4 /WX` sans
  avertissement.

## Exigences
`EX-GP-006` (modèle et physique).
