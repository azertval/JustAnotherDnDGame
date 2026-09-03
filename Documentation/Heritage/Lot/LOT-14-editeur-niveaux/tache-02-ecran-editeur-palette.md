# TACHE-02 — Écran éditeur : grille cliquable + palette de tuiles {#lot-14-tache-02-ecran-editeur-palette}

**Lot :** [LOT-14](epic.md) · **Emplacement :** `HMI/Editor` · **Statut :** à faire

## Contexte
`EditorScreen` (LOT-06) n'est aujourd'hui qu'un texte « à venir ». Cette tâche pose le **socle
interactif** du mode éditeur : afficher la grille d'un `core::LevelDraft` (TACHE-01), convertir la
position souris en case de grille, et peindre des tuiles depuis une **palette** de types — le
cœur du WYSIWYG (`EX-EDIT-002`).

## Travail à réaliser
- **Rendu de la grille** : réutiliser `hmi::Camera2D`/`hmi::SpriteRenderer`/`hmi::TextureAtlas`
  (LOT-05) pour afficher le `TileMap` du `LevelDraft` courant, comme le fait `GameScreen` pour un
  niveau joué — même correspondance type de tuile → région d'atlas.
- **Curseur monde** : convertir la position souris (pixels, `hmi::InputState`) en position monde
  (`Camera2D::screenToWorld`) puis en `core::GridPosition` ; surligner la case survolée.
- **Palette de tuiles** : une bande d'interface (texte/icônes via `BitmapFont`, espace écran) listant
  les types éditables (`Empty`, `Solid`, `Danger`, `Entry`, `Exit`, `Switch`, `Door` — **pas** de
  bloc poussable ni de clé, hors périmètre `Core` actuel) ; clic pour sélectionner le type actif.
- **Peinture** : clic (et glisser-clic pour peindre en continu) sur la grille applique le type actif
  via `LevelDraft::paintTile` — sauf pour `Entry`/`Exit`/`Switch`/`Door`, gérés en TACHE-03 (poser
  une tuile simple ici, liaison/unicité dans la tâche suivante).
- **Nouveau `hmi::EditorScreen`** : possède un `core::LevelDraft` (créé vierge par défaut à ce
  stade ; le choix nouveau/existant arrive en TACHE-06), le rendu de la grille, la palette et
  l'interaction souris ; **Échap** revient au menu (comme le placeholder actuel).

## Fichiers impactés
- `Source/HMI/Interface/EditorScreen.h`/`.cpp` (remplace le placeholder).
- `Source/HMI/Editor/` : nouveaux fichiers pour la palette et la conversion souris → grille (ex.
  `TilePalette.h`/`.cpp`, `EditorCursor.h`/`.cpp`) — logique d'interaction, pas de dépendance directe
  au rendu quand c'est possible, pour rester testable.
- Tests unitaires (`Source/Test/Unit/HMI/Editor/`, nouveau dossier).

## Tests (obligatoires)
- Conversion position souris (pixels) → `GridPosition`, y compris hors bornes de la grille (aucune
  case retournée / case rejetée proprement).
- Sélection de la palette : cliquer une entrée change le type actif ; hors zone palette, sans effet.
- Peindre une case via le type actif modifie bien le `LevelDraft` sous-jacent (test d'intégration
  simulant une séquence clic/glisser).

## Points d'attention
- Séparer **interaction** (testable sans GPU) et **rendu** (dépend de Direct3D), comme le reste de
  `HMI` — la logique de sélection de case/palette ne doit pas nécessiter de fenêtre pour être testée
  (`EX-NFR-010`).
- Réutiliser les ressources de rendu du `RenderContext` existant (atlas, police, lot de sprites) —
  aucune ressource dupliquée entre `GameScreen` et `EditorScreen`.
- Le rendu reste en **lecture seule** vis-à-vis du modèle : c'est l'interaction souris qui mute le
  `LevelDraft`, jamais le rendu lui-même (`EX-ARCH-012`, par analogie).

## Définition de fait (DoD)
- Grille affichée et peignable à la souris depuis une palette, testé (`ctest` vert pour la logique
  d'interaction), vérifié visuellement dans l'application ; build `/W4 /WX` ; Doxygen à jour.

## Exigences
`EX-EDIT-001`, `EX-EDIT-002`, `EX-EDIT-030`, `EX-EDIT-031`, `EX-ARCH-020`, `EX-ARCH-021`,
`EX-NFR-010`.
