# TACHE-01 — Partition en salles (`RoomGrid`, logique pure) {#lot-32-tache-01-partition-salles}

**Lot :** [LOT-32](epic.md) · **Emplacement :** `HMI/Graphics` · **Statut :** fait

## Contexte
Le cadrage caméra par salle (TACHE-02) et le repère visuel de l'éditeur (TACHE-03) ont tous les
deux besoin de la **même** réponse à deux questions : « quelle salle contient telle position ? » et
« quel est le rectangle (en cases) de la salle `n` ? ». Cette logique est purement géométrique
(aucune dépendance rendu), donc factorisée une fois, comme `Camera2D::fitZoom` l'a été pour le
calcul de zoom d'ajustement (`LOT-16`).

## Travail à réaliser
- **Nouveau type `hmi::RoomGrid`** (`Source/HMI/Graphics/RoomGrid.h`/`.cpp`) :
  - Construit à partir de la taille du niveau en cases (`levelWidth`, `levelHeight`) et de la
    **taille de salle** en cases (constante de code partagée, ex. `ROOM_WIDTH_TILES`/
    `ROOM_HEIGHT_TILES` — choisir une valeur confortable à l'échelle actuelle du jeu, zoom pixel
    art natif dans la fenêtre par défaut 1280×720 ; documenter la valeur retenue et pourquoi).
  - `roomIndexAt(core::GridPosition tile) const -> core::GridPosition` — indice **colonne/ligne**
    de la salle (pas la position en cases) contenant la case donnée (division entière par la taille
    de salle), bornée à `[0, colonnes)` / `[0, lignes)`.
  - `roomBounds(core::GridPosition roomIndex) const` — rectangle **en cases** de la salle
    (origine, largeur, hauteur) ; la dernière colonne/ligne est **rognée** à la taille réelle du
    niveau si celle-ci n'est pas un multiple exact de la taille de salle (jamais de salle qui
    déborderait du niveau).
  - `columns() const` / `rows() const` — nombre de salles sur chaque axe (`ceil(taille niveau /
    taille salle)`).
- **Aucune dépendance à `Camera2D` ni à D3D11** dans `RoomGrid` lui-même — reste **pur**, testable
  sans GPU (`EX-NFR-010`). `Camera2D::fitZoom` continue d'être appelé **par les appelants**
  (TACHE-02/03) avec le rectangle renvoyé par `roomBounds`, pas dupliqué ici.
- Un niveau qui tient dans une seule salle doit produire un `RoomGrid` à **une seule** entrée
  (`columns() == rows() == 1`), dont `roomBounds({0, 0})` couvre exactement le niveau entier — la
  condition de non-régression exploitée par TACHE-02.

## Fichiers impactés
- `Source/HMI/Graphics/RoomGrid.h`/`.cpp` (nouveaux).
- `Source/Test/Unit/HMI/Graphics/test_room_grid.cpp` (nouveau).
- `CMakeLists.txt` (ou fichier de liste de sources concerné) : enregistrement des nouveaux fichiers.

## Tests (obligatoires)
- Unitaires, sans GPU : niveau plus petit qu'une salle (une seule salle, `roomBounds` = niveau
  entier), niveau exactement multiple de la taille de salle (grille pleine, pas de rognage), niveau
  **non multiple** (dernière colonne/ligne rognée, vérifier la largeur/hauteur exacte), `roomIndexAt`
  aux quatre coins et au centre de plusieurs salles, position hors bornes (comportement défini —
  bornée, pas de comportement indéfini).

## Points d'attention
- **Origine haut-gauche**, comme `TileMap`/`GridPosition` (`EX-ARCH-020`) — pas d'inversion d'axe.
- La taille de salle est une **constante de code unique pour tout le jeu** (décision de cadrage de
  l'épic) : ne pas la rendre configurable par niveau/JSON à ce stade.
- `RoomGrid` ne connaît **aucune notion de personnage ni de position monde continue** (`Vector2`) —
  seulement des cases (`GridPosition`, entier). La conversion case ↔ position monde reste la
  responsabilité de l'appelant (TACHE-02), comme pour le reste du rendu (`EX-ARCH-021`).

## Définition de fait (DoD)
- `RoomGrid` livré et testé (`ctest` vert), aucune dépendance rendu ; build `/W4 /WX` sans
  avertissement.

## Exigences
`EX-NFR-010` (logique pure, testable sans GPU) ; prépare `EX-REN-015` et `EX-EDIT-023` (TACHE-02/03).
