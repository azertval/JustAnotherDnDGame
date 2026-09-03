# TACHE-03 — Éditeur et rendu {#lot-24-tache-03-editeur-rendu}

**Lot :** [LOT-24](epic.md) · **Emplacement :** `HMI/Editor`, `HMI/Graphics` · **Statut :** fait

## Contexte
Rendre les blocs réduits utilisables : plaçables dans l'éditeur, visuellement plus petits qu'un
bloc plein (`LOT-21`) pour que leur taille de collision réduite (TACHE-02) soit lisible à l'écran.

## Travail à réaliser
- **`HMI/Editor/EditorLayout.h`** : `PALETTE_TYPE_COUNT` augmente de 2.
- **`HMI/Editor/TilePalette.cpp`** : `PALETTE_TYPES` gagne `BlockHalf`/`BlockQuarter` ; libellés
  courts (ex. `"Bloc 1/2"`/`"Bloc 1/4"`).
- **`HMI/Graphics/TileVisuals.cpp`** : couleurs d'atlas dédiées, ou variante teintée du bloc plein
  (`Block`) pour signaler visuellement la famille sans multiplier les couleurs disponibles.
- **Rendu à l'échelle** : le sprite du bloc doit visuellement occuper `facteur × 1` unité, **centré**
  dans sa case (cohérent avec la boîte de collision de TACHE-02) — sur le modèle de
  `refreshBlockVisuals` (`GameScreen.cpp`, `LOT-21`), qui devra lire le facteur de taille en plus
  de la position pour ajuster `Transform.scale` et recentrer `Transform.position`.

## Fichiers impactés
- `Source/HMI/Editor/EditorLayout.h`, `TilePalette.h`/`.cpp`.
- `Source/HMI/Graphics/TileVisuals.cpp`.
- `Source/HMI/Interface/GameScreen.cpp` (`refreshBlockVisuals`, mise à l'échelle et recentrage).

## Tests (obligatoires)
- `ctest` existant (`test_tile_palette.cpp`) reste vert (13→15 entrées, `static_assert` inchangé).
- **Cohérence visuel/collision vérifiée par lecture croisée du code**, pas par une vérification
  visuelle manuelle dans l'application (hors périmètre de ce qu'un agent doit faire de façon
  autonome) : `GameScreen::refreshBlockVisuals` calcule `margin = (1 - scale) * 0.5` et pose
  `Transform.position = case + margin`, `Transform.scale = {scale, scale}` — **exactement** la même
  formule que `BlockController::boxAt` (`Core/Gameplay/BlockController.cpp`, fonction interne
  `blockBox`) utilisée par la collision réelle (TACHE-02). `SpriteRenderer` interprète
  `Transform.position` comme le coin haut-gauche du sprite et multiplie la région de base par
  `Transform.scale` : la même paire position/échelle pilote donc le rendu et la collision, par
  construction, sans code dupliqué qui pourrait diverger.

## Points d'attention
- **Cohérence visuel/collision stricte** : un décalage entre le sprite affiché et la boîte
  réellement testée (TACHE-02) serait perçu comme un bug de collision par le joueur, même si le
  code est correct — garanti ici en réutilisant la **même formule** (`margin`/`scale`) des deux
  côtés plutôt qu'en dupliquant le calcul, ce qui rend une divergence structurellement impossible
  (à la différence d'une simple vérification visuelle ponctuelle, qui ne couvrirait qu'un cas
  précis).

## Définition de fait (DoD)
- Blocs réduits plaçables, visuellement centrés et à l'échelle correcte (même formule que la
  collision réelle) ; aucune régression sur les autres entrées de la palette (`test_tile_palette.cpp`
  vert).

## Exigences
Aucune exigence propre — intégration de `EX-GP-005` (TACHE-01/TACHE-02) dans `HMI`.
