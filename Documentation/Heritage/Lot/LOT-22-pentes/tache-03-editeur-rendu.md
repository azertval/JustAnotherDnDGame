# TACHE-03 — Éditeur et rendu {#lot-22-tache-03-editeur-rendu}

**Lot :** [LOT-22](epic.md) · **Emplacement :** `HMI/Editor`, `HMI/Graphics` · **Statut :** fait

## Contexte
Rendre les pentes **utilisables** : plaçables dans l'éditeur, visuellement distinguables d'une
tuile pleine (un triangle, pas un carré) une fois la logique posée par TACHE-01/TACHE-02.

## Travail à réaliser
- **`HMI/Editor/EditorLayout.h`** : `PALETTE_TYPE_COUNT` augmente de 2 (les deux orientations de
  pente).
- **`HMI/Editor/TilePalette.cpp`** : `PALETTE_TYPES` gagne `SlopeUpRight`/`SlopeUpLeft` ; `labelFor`
  renvoie des libellés courts (ex. `"Pente D"`/`"Pente G"`).
- **`HMI/Graphics/TileVisuals.cpp`** : `regionForTile` associe les deux orientations à des couleurs
  d'atlas non utilisées.
- **Rendu en triangle** : contrairement aux tuiles existantes (toutes des carrés pleins),
  une pente doit se distinguer visuellement de `Solid` à l'écran. Deux approches possibles, à
  trancher en implémentant : (a) une région d'atlas générée en code représentant déjà un triangle
  (zones transparentes sur la moitié non couverte, sur le modèle de la tuile transparente déjà
  générée par `TextureAtlas`) ; (b) un sprite carré simple (couleur pleine) en première itération,
  quitte à affiner le rendu triangulaire dans un lot ultérieur si le temps manque. **Préférer (a)**
  si le budget du lot le permet — une pente qui a l'air d'un carré plein serait trompeuse pour qui
  conçoit un niveau.

## Fichiers impactés
- `Source/HMI/Editor/EditorLayout.h`, `TilePalette.h`/`.cpp`.
- `Source/HMI/Graphics/TileVisuals.cpp` (et `TextureAtlas.cpp` si un nouveau motif triangulaire est
  généré en code).

## Tests (obligatoires)
- `ctest` existant (`test_tile_palette.cpp`) reste vert (compte non figé, `EXPECT_GE`).
- **Vérification visuelle obligatoire** dans l'application compilée : peindre une pente montante
  et une pente descendante dans l'éditeur, tester (`P`) — le personnage doit suivre la pente
  visuellement de la même façon que la collision le prévoit (TACHE-02).

## Points d'attention
- Si le rendu triangulaire (option a) s'avère disproportionné en effort face au reste du lot,
  documenter explicitement le report à un lot ultérieur plutôt que de le laisser non dit — ne pas
  faire passer un carré plein pour une pente sans le signaler.

## Définition de fait (DoD)
- Pentes plaçables et visuellement identifiables dans l'éditeur et en jeu ; aucune régression sur
  les autres entrées de la palette.

## Exigences
Aucune exigence propre — intégration de `EX-GP-003` (TACHE-01/TACHE-02) dans `HMI`.
