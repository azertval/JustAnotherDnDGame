# TACHE-02 — Éditeur et rendu {#lot-26-tache-02-editeur-rendu}

**Lot :** [LOT-26](epic.md) · **Emplacement :** `HMI/Editor`, `HMI/Graphics` · **Statut :** fait

## Contexte
Rend les quatre nouvelles tuiles plaçables dans l'éditeur et affichées avec leur silhouette réelle
(triangle/courbe), cohérente avec la hitbox posée en TACHE-01 — sur le modèle des pentes/arrondis
de sol (`LOT-22`/`LOT-23`).

## Travail à réaliser
- **`HMI/Editor/EditorLayout.h`** : `PALETTE_TYPE_COUNT` `15 → 19`.
- **`HMI/Editor/TilePalette.cpp`** : `PALETTE_TYPES` gagne les quatre types ; libellés
  (`"Pente D Plaf"`, `"Pente G Plaf"`, `"Arrondi D Plaf"`, `"Arrondi G Plaf"`).
- **`HMI/Graphics/TileVisuals.h`/`.cpp`** : `slopeTileGridPosition` leur assigne une position dans
  la grille de tuiles procédurale (quatre nouvelles cases) ; `regionForTile` les traite comme les
  variantes de sol (position partagée, masque de forme généré par `TextureAtlas`).
- **`HMI/Graphics/TextureAtlas.h`/`.cpp`** :
  - `TILES_PER_SIDE` `4 → 5` (16 → 25 cases) pour loger les quatre nouvelles silhouettes — une
    seule case restait libre dans la grille `4×4`.
  - `tileColor` (jeu de couleurs procédural) **recalé explicitement** : la grille passant de 4 à 5
    colonnes, l'index linéaire d'une case (`ligne × largeur + colonne`) change pour toute ligne
    au-delà de la première si la largeur n'est pas prise en compte — le tableau de couleurs a été
    réécrit ligne par ligne pour que les quatre premières colonnes de chaque ligne conservent
    **exactement** leurs couleurs historiques, les nouvelles cases (5ᵉ colonne, 5ᵉ ligne) étant soit
    réservées (damier de transparence), soit remplacées par le masque de forme (couleur de base
    sans importance, noir par convention).
  - `slopeShapePixel` : masque de forme pour les huit types (sol + plafond) — plein **sous** la
    silhouette pour le sol (`core::slopeSurfaceHeight`), **au-dessus** pour le plafond
    (`core::ceilingSlopeHeight`), rempli en **gris** (même couleur que `Solid`) dans les deux cas,
    pas une teinte distincte par variante (décision reprise de la correction apportée aux pentes/
    arrondis de sol dans la même conversation).

## Fichiers impactés
- `Source/HMI/Editor/EditorLayout.h`, `TilePalette.cpp`.
- `Source/HMI/Graphics/TileVisuals.h`/`.cpp`, `TextureAtlas.h`/`.cpp`.

## Tests (obligatoires)
- Build `/W4 /WX` sans avertissement (pas de test HMI dédié à la génération d'atlas — cohérent
  avec l'absence de tests directs pour les variantes de sol existantes, `TextureAtlas` dépendant de
  Direct3D).
- Revue de code : vérifier que le jeu de couleurs recalé ne décale aucune couleur existante
  (comparaison ligne par ligne avec l'ancien tableau `4×4`).

## Points d'attention
- **Le recalage du jeu de couleurs est la partie la plus risquée de cette tâche** : un
  agrandissement de grille naïf (juste changer `TILES_PER_SIDE`) aurait silencieusement redécalé
  la couleur de **toutes** les tuiles existantes à partir de la deuxième ligne, puisque
  `tileColor` indexe une palette à plat par `ligne × largeur + colonne` — un bug qui ne se serait
  révélé qu'à l'exécution (aucun test ne compare les couleurs pixel par pixel), pas à la
  compilation.

## Définition de fait (DoD)
- Quatre nouvelles tuiles plaçables depuis la palette, rendues avec leur silhouette réelle en gris,
  cohérente avec la hitbox (TACHE-01) ; aucune couleur de tuile existante décalée.

## Exigences
`EX-GP-006` (éditeur et rendu).
