# TACHE-02 — Éditeur et rendu {#lot-23-tache-02-editeur-rendu}

**Lot :** [LOT-23](epic.md) · **Emplacement :** `HMI/Editor`, `HMI/Graphics` · **Statut :** fait

## Contexte
Rendre les tuiles arrondies utilisables : plaçables dans l'éditeur, visuellement distinctes des
pentes linéaires de `LOT-22` malgré la même orientation générale.

## Travail à réaliser
- **`HMI/Editor/EditorLayout.h`** : `PALETTE_TYPE_COUNT` augmente de 2.
- **`HMI/Editor/TilePalette.cpp`** : `PALETTE_TYPES` gagne `RoundedUpRight`/`RoundedUpLeft` ;
  libellés courts (ex. `"Arrondi D"`/`"Arrondi G"`).
- **`HMI/Graphics/TileVisuals.cpp`** : couleurs d'atlas dédiées, distinctes de celles des pentes.
- **Rendu courbe** : approximation d'un quart de cercle en pixel art généré en code (comparable en
  esprit à la silhouette du personnage ou aux drapeaux, déjà générés par blocs de pixels dans
  `TextureAtlas`/`FlagIcons`) — même arbitrage qu'en `LOT-22-TACHE-03` entre une région d'atlas
  fidèle à la forme et une couleur pleine en repli si l'effort dépasse le budget du lot.

## Fichiers impactés
- `Source/HMI/Editor/EditorLayout.h`, `TilePalette.h`/`.cpp`.
- `Source/HMI/Graphics/TileVisuals.cpp` (et `TextureAtlas.cpp` si un motif courbe est généré).

## Tests (obligatoires)
- `ctest` existant (`test_tile_palette.cpp`) reste vert.
- **Vérification visuelle obligatoire** : peindre les deux orientations arrondies, tester (`P`) —
  le suivi de courbe doit être visuellement cohérent avec la formule de `TACHE-01`.

## Points d'attention
- Vérifier que les couleurs choisies restent **distinctes** de celles des pentes de `LOT-22` dans
  la palette — un niveau qui mélange les deux doit rester lisible pour qui le conçoit.
- **Décision retenue** : couleur plate (option repli), comme pour les pentes de `LOT-22-TACHE-03` —
  bleu violet pour `RoundedUpRight` (`atlas.tile(3, 2)`), magenta pour `RoundedUpLeft`
  (`atlas.tile(0, 1)`), distinctes entre elles et des pentes (vert sarcelle/vieux rose). Un rendu
  fidèle à la courbe est un raffinement possible d'un lot ultérieur, non fait ici (même arbitrage
  que `LOT-22`, documenté explicitement plutôt que laissé non dit).

## Définition de fait (DoD)
- Tuiles arrondies plaçables et visuellement identifiables ; aucune régression sur les entrées
  existantes de la palette (pentes incluses). Vérifié visuellement dans l'application compilée
  (palette à 13 entrées, couleurs distinctes, aucun chevauchement ni troncature).

## Exigences
Aucune exigence propre — intégration de `EX-GP-004` (TACHE-01) dans `HMI`.
