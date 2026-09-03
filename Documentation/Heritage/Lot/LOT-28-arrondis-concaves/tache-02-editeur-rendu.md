# TACHE-02 — Éditeur et rendu {#lot-28-tache-02-editeur-rendu}

**Lot :** [LOT-28](epic.md) · **Emplacement :** `HMI/Editor`, `HMI/Graphics` · **Statut :** ✅

## Contexte
Rend les quatre nouvelles tuiles plaçables dans l'éditeur (nouveau sous-groupe « Concave », frère
de « Arrondi » sous la catégorie « Tuile », `LOT-27`) et affichées avec leur silhouette réelle
(courbe concave), cohérente avec la physique posée en TACHE-01 — sur le modèle de l'arrondi convexe
(`LOT-23`/`LOT-26`).

## Travail à réaliser
- **`HMI/Editor/TilePalette.h`** : nouveau `Subgroup::Concave` (`SUBGROUP_COUNT` `3 → 4`).
- **`HMI/Editor/TilePalette.cpp`** : nouveau sous-groupe déplié sous « Tuile », juste après
  « Arrondi » — quatre feuilles, mêmes libellés que « Arrondi » (`"Sol D"`, `"Sol G"`, `"Plaf D"`,
  `"Plaf G"`), l'en-tête du sous-groupe portant l'icône `ConcaveUpRight`.
- **`HMI/Graphics/TileVisuals.h`/`.cpp`** : `slopeTileGridPosition` leur assigne une position dans
  la grille de tuiles procédurale — quatre cases déjà libres depuis l'agrandissement `LOT-26`
  (`(4,1)`, `(4,2)`, `(4,3)`, `(2,4)` — pas de nouvel agrandissement, voir décision de cadrage de
  l'épic) ; `regionForTile` les ajoute à la liste des types à position partagée (masque de forme
  généré par `TextureAtlas`, comme les huit types existants).
  > Écart constaté en cours de lot (voir TACHE-03) : `(4,4)` avait été initialement retenue pour
  > `ConcaveDownLeft`, mais cette case est **réservée** au damier de transparence
  > (`TextureAtlas::transparentTileIndex`, vérifiée avant tout masque de forme) — elle affichait
  > donc le damier au lieu de la silhouette. L'épic comptait « sept cases libres » après
  > l'agrandissement `LOT-26`, mais l'une d'elles était en réalité cette case réservée : il n'y en
  > avait donc réellement que **six**. Corrigé en réassignant `ConcaveDownLeft` à `(2,4)`,
  > authentiquement libre.
- **`HMI/Graphics/TextureAtlas.cpp`** : `kSlopeTileTypes` gagne les quatre nouveaux types —
  `TILES_PER_SIDE` reste `5`, `tileColor` n'est **pas** retouché (les quatre cases visées sont déjà
  remplies de noir par convention, comme documenté par le commentaire existant de `tileColor`).
  > Écart constaté en cours de lot (voir TACHE-03) : `slopeShapePixel` échantillonnait au **centre**
  > de chaque pixel (`(localX+0.5)/TILE_SIZE`), qui n'atteint jamais exactement `0`/`1` (le dernier
  > pixel plafonne à `≈0,969`). Sans conséquence pour les pentes/arrondis existants (tangente raide
  > du côté **creux**, jamais critique pour un raccord entre cases), mais pour un arrondi
  > **concave** — tangente raide du côté **plein**, justement le bord qui doit rester quasi solide —
  > cela manquait la vraie valeur de bord d'une marge visible (~0,25 de hauteur de case), rendu
  > comme une encoche à l'endroit précis où la silhouette doit au contraire être la plus pleine.
  > Corrigé en échantillonnant au bord des pixels de coin (`localX/(TILE_SIZE-1)`), qui atteint
  > exactement `0` et `1` — améliore aussi, sans le changer visiblement, le rendu des pentes/
  > arrondis convexes existants (différence négligeable là où leur tangente est déjà peu raide).

## Fichiers impactés
- `Source/HMI/Editor/TilePalette.h`/`.cpp`.
- `Source/HMI/Graphics/TileVisuals.h`/`.cpp`, `TextureAtlas.cpp`.
- Tests : `Source/Test/Unit/HMI/Editor/test_tile_palette.cpp`.

## Tests (obligatoires)
- `TilePaletteTest` : le sous-groupe « Concave » apparaît sous « Tuile » une fois dépliée, à côté
  de « Pente »/« Arrondi » ; déplié, il expose ses quatre orientations (`"Sol D"`, `"Sol G"`,
  `"Plaf D"`, `"Plaf G"`) ; cliquer une feuille sélectionne le bon `core::TileType`. Le total de
  lignes entièrement déplié (`expandEverything`/`FenetreReduiteLimiteLesEntreesVisibles`,
  actuellement 25) passe à **30** (1 en-tête + 4 feuilles de plus) — mettre à jour la fonction
  d'aide `expandEverything` (déplier aussi « Concave ») et l'assertion `totalRowCount()`.
- Build `/W4 /WX` sans avertissement (pas de test HMI dédié à la génération d'atlas — cohérent avec
  l'absence de tests directs pour les variantes existantes, `TextureAtlas` dépendant de Direct3D).
- Revue de code : confirmer que les quatre cases visées par `slopeTileGridPosition` pour les
  nouveaux types n'entrent en collision avec **aucune** case déjà utilisée par un autre type de
  tuile (Solid, Block/BlockHalf/BlockQuarter, les huit types de pente/arrondi existants, la case de
  damier de transparence).

## Points d'attention
- **Ne pas agrandir la grille de l'atlas** : contrairement à `LOT-26`, qui avait dû passer
  `TILES_PER_SIDE` de `4` à `5` (une seule case restant libre), l'agrandissement précédent a laissé
  sept cases inoccupées — largement de quoi loger ces quatre nouvelles silhouettes sans retoucher
  `tileColor` ni risquer de décaler la couleur d'une tuile existante (le risque principal identifié
  lors de `LOT-26-TACHE-02`).

## Définition de fait (DoD)
- Quatre nouvelles tuiles plaçables depuis un sous-groupe « Concave » dédié de la palette, rendues
  avec leur silhouette réelle en gris, cohérente avec la physique (TACHE-01) ; aucune couleur de
  tuile existante décalée ; aucune case d'atlas partagée par erreur entre deux types.

## Exigences
`EX-GP-007` (éditeur et rendu).
