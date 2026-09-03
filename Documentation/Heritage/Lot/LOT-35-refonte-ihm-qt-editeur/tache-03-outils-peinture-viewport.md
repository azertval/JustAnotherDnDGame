# TACHE-03 — Barre d'outils + peinture viewport → `LevelDraft` (undo/redo, copier/coller) {#lot-35-tache-03-outils-peinture-viewport}

**Lot :** [LOT-35](epic.md) · **Emplacement :** `Source/Editor` · **Statut :** non commencé

## Contexte
Cœur interactif de l'éditeur : transformer les clics souris Qt sur le viewport en **mutations de
`core::LevelDraft`** (source de vérité déjà testée), avec les trois outils existants (`Paint`,
`Rectangle`, `Selection`), l'annuler/refaire et le copier/coller. Aucune règle d'édition n'est
réécrite : cette tâche **relie** les entrées Qt à l'API de `LevelDraft` et dessine le retour visuel
dans le viewport (au pixel de la grille).

## Travail à réaliser
- **Barre d'outils** (`QToolBar` + `QActionGroup` exclusif) : `Paint`, `Rectangle`, `Selection`
  (réutilise `hmi::EditorTool`), avec `Tab` pour cycler (parité). Outil courant = source de vérité de
  l'éditeur (comme aujourd'hui `_toolBar.selected()`).
- **Conversion souris → case** : réutiliser `hmi::Camera2D::screenToWorld` pour convertir la position
  Qt (pixels physiques) en `core::GridPosition` (bornée ou non selon l'outil, cf. `hoveredCell` /
  `clampedCell` de `EditorScreen`).
- **Outil Paint** : clic/glisser gauche → `LevelDraft::paintTile(col, row, typeActif)` (type issu de
  la palette, TACHE-02). Prévisualisation de la case survolée dessinée dans le viewport.
- **Outil Rectangle** : glisser → au relâchement, remplir le rectangle (`paintRegion` ou boucle
  `paintTile`). Aperçu du rectangle pendant le glisser.
- **Outil Selection** : glisser → zone (bornes min/max) ; `Ctrl+C` copie vers un presse-papiers local
  (types `[ligne][colonne]`), `Ctrl+V` colle via `paintRegion`. Réutiliser la sémantique existante.
- **Undo/Redo** : `QAction` Ctrl+Z / Ctrl+Y → `LevelDraft::undo()` / `redo()` ; états d'activation des
  actions reflétant la disponibilité de l'historique.
- **Caméra** : molette = zoom, glisser bouton droit = pan, touche de recadrage auto (parité
  `EditorScreen` `_manualCamera`), portés sur le viewport.

## Fichiers impactés
- `Source/Editor/ToolPanel.{h,cpp}` (ou intégration barre d'outils dans `MainWindow`).
- `Source/Editor/GameViewport.{h,cpp}` (mode édition : survol, peinture, aperçu rectangle/sélection ;
  rendu du retour visuel via `SpriteBatch`).
- Éventuel `Source/Editor/EditorController.{h,cpp}` (glue souris/outil → `LevelDraft`, **testable**).

## Tests (obligatoires)
- **Contrôleur d'édition testable** (sans Qt/GPU) : pour un outil donné et une suite d'événements
  (case de départ/arrivée), vérifier les appels résultants à `LevelDraft` (tuiles peintes, rectangle
  rempli, région copiée/collée) — en injectant un `LevelDraft` réel et en comparant l'état obtenu.
- **Non-régression** : les tests existants de `LevelDraft` (`Core`) et de sélection/presse-papiers
  restent verts ; la logique n'est pas dupliquée.
- **Vérification manuelle** : peindre, remplir un rectangle, sélectionner/copier/coller, annuler/
  refaire, zoom/pan — comportement identique à l'éditeur historique.

## Points d'attention
- **`LevelDraft` reste la seule autorité** : pas de mutation de grille hors de son API (undo/redo en
  dépend). Le contrôleur Qt ne fait que traduire et appeler.
- **Pixels physiques vs logiques** (DPI) dans `screenToWorld` — cohérent avec le viewport (`LOT-34`).
- **Retour visuel dans le viewport** (quads `SpriteBatch`), pas en overlay Qt, pour rester aligné au
  zoom/pan de la caméra.
- **Exclusivité des glissers** : peinture vs rectangle/sélection mutuellement exclusifs (parité
  `_paintingDrag`/`_areaDragActive`).

## Définition de fait (DoD)
- Les trois outils, l'undo/redo et le copier/coller fonctionnent via `LevelDraft` ; retour visuel
  correct ; contrôleur d'édition **couvert par des tests** ; `/W4 /WX` propre ; vérification manuelle
  OK.

## Exigences
`EX-EDIT-002` (peinture), `EX-EDIT-005` (undo/redo), `EX-EDIT-014` (outils) — présentation Qt,
comportement conservé ; `EX-EDIT-010` (pas de duplication de logique), `EX-NFR-010`.
