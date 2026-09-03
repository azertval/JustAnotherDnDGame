# LOT-35 — Refonte IHM (Qt) : éditeur (docking, palette, outils, peinture) {#lot-35}

> Statut : **implémenté** (build/tests verts ; vérification IHM manuelle en cours). Prérequis : [LOT-34](@ref lot-34) (socle Qt + viewport).

## Objectif
Sur le socle Qt du [LOT-34](@ref lot-34), reconstruire le **cœur de l'éditeur de niveau** en widgets
Qt : une fenêtre à **panneaux dockables** (`QDockWidget`) autour du viewport D3D11 central, avec la
palette de tuiles, la barre d'outils, la peinture à la souris et l'annuler/refaire. L'objectif est la
**maintenabilité** (séparer interaction et rendu, plus de dessin manuel de widgets) et des
**fenêtres réglables hors code** (disposition sauvegardée/restaurée). La logique d'édition
**ne change pas** : les widgets pilotent `core::LevelDraft`, source de vérité déjà testée.

## Périmètre

### Inclus
- **`QMainWindow` à docks** : viewport D3D11 (LOT-34) en zone centrale ; `QDockWidget` dockables,
  redimensionnables, détachables et empilables pour Palette, Outils, Statut.
- **Palette de tuiles** en `QTreeView` (catégories → sous-groupes → tuiles) remplaçant l'accordéon
  maison `TilePalette` ; **réutilise la taxonomie existante** (catégories `Tuile/Interactif/Piège/
  Jalon`, sous-groupes `Pente/Arrondi/Concave/Bloc/Directionnel`) et l'énumération `core::TileType`.
- **Barre d'outils** `QToolBar` / `QActionGroup` : `Paint`, `Rectangle`, `Selection` (réutilise
  `hmi::EditorTool`), avec raccourcis (Tab pour cycler, conservé).
- **Peinture sur le viewport** : les événements souris Qt sont convertis en case de grille (réutilise
  la logique `Camera2D::screenToWorld`) et appliqués via `core::LevelDraft::paintTile` /
  `paintRegion` ; le survol/curseur est prévisualisé dans le viewport.
- **Annuler/Refaire** via `QAction` (Ctrl+Z / Ctrl+Y) → `LevelDraft::undo` / `redo`.
- **Copier/Coller** d'une région (outil Sélection) → réutilise le presse-papiers local existant.
- **Caméra** pan/zoom (molette, glisser) portée dans le viewport (réutilise `Camera2D`).
- **Redimensionnement de niveau** exposé (dialogue Qt, remplace la saisie `Ctrl+R`) via
  `LevelDraft::resize` + confirmation si destructeur (`wouldResizeDropContent`).
- **Disposition persistée hors code** : `QMainWindow::saveState` / `restoreState` dans `QSettings`
  (positions, tailles, flottement des docks restaurés au lancement).
- Enregistrement du niveau (`LevelWriter::saveToFile`) via une action Qt ; essai immédiat (playtest)
  déclenché dans le viewport (réutilise le chemin `GameScreen`/`LevelScene`).
- Documentation (guide éditeur mis à jour) et tests de la logique nouvelle découplée de Qt.

### Exclus (hors périmètre de ce lot)
- **Liste/gestion des fichiers de niveaux** (recherche, duplication, suppression) — [LOT-36](@ref lot-36).
- **Liens de mécanismes visuels** (traits/flèches, panneau Liens) — [LOT-37](@ref lot-37) ; le geste
  de liaison Maj+clic existant peut rester temporairement tel quel ou être désactivé jusqu'au LOT-37.
- **Menus principaux / Options / remappage** en Qt — [LOT-38](@ref lot-38).
- **Textures depuis fichiers** — [LOT-39](@ref lot-39) ; rendu des tuiles via `TextureAtlas`
  procédural inchangé.
- **Nouvelles mécaniques de niveau ou nouveau format** — aucun ; `Core` intact.

## Décisions de cadrage
- **Widgets Qt natifs, pas de re-dessin maison** : la palette (`QTreeView`), les outils (`QToolBar`)
  et les dialogues sont des contrôles Qt standard — c'est tout l'intérêt de la migration (lisibilité,
  accessibilité clavier, maintenance). Seul le **viewport** reste du dessin D3D11.
- **`core::LevelDraft` reste la source de vérité** : les widgets sont de **fins contrôleurs** qui
  appellent son API (aucune règle d'édition dupliquée dans Qt), exactement comme `EditorScreen`
  aujourd'hui. La logique reste testable sans Qt ni GPU.
- **Disposition sauvegardée via `QSettings`**, pas un format maison : « réglable hors code » demandé
  par le demandeur = l'utilisateur arrange ses docks à la souris, la disposition persiste seule.
- **Réutiliser la taxonomie de palette existante** plutôt que la redéfinir : les catégories/
  sous-groupes du `LOT-27` sont conservés, seul leur **rendu** passe de l'accordéon maison à un arbre
  Qt.
- **Le viewport reste l'unique surface D3D11** : la peinture et le survol sont dessinés dedans (quads),
  pas en overlay Qt, pour rester au pixel près sur la grille de jeu.

## Exigences couvertes
- Nouvelles : `EX-IHM-010` (éditeur en fenêtre à docks Qt réglables), `EX-IHM-011` (disposition des
  panneaux persistée hors code).
- Portées/reconduites depuis l'éditeur historique (comportement inchangé, présentation Qt) :
  `EX-EDIT-002` (peinture), `EX-EDIT-005` (undo/redo, redimensionnement), `EX-EDIT-014` (outils
  Paint/Rectangle/Sélection), `EX-EDIT-018` (palette par catégories).
- Réutilisées : `EX-EDIT-010`/`EX-EDIT-030`/`EX-EDIT-031` (pas de duplication de la logique de
  niveau), `EX-NFR-010` (logique testable sans GPU).

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé. Les tâches seront détaillées à l'ouverture du lot.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-fenetre-docks-persistance.md) | `QMainWindow` à docks + persistance `QSettings` de la disposition | `Source/Editor` | ✅ |
| [TACHE-02](tache-02-palette-arbre.md) | Palette `QTreeView` (taxonomie `LOT-27`) reliée à la sélection de tuile | `Source/Editor` | ✅ |
| [TACHE-03](tache-03-outils-peinture-viewport.md) | Barre d'outils + peinture viewport → `LevelDraft` (undo/redo, copier/coller) | `Source/Editor` | ✅ |
| [TACHE-04](tache-04-redim-enregistrement-essai.md) | Redimensionnement, enregistrement, essai immédiat ; documentation & vérification | `Source/Editor`, `Documentation` | ✅ |

## Critères d'acceptation du lot
1. L'éditeur Qt ouvre une fenêtre à panneaux dockables autour du viewport ; l'utilisateur peut
   **déplacer, redimensionner, détacher** les panneaux, et la disposition est **restaurée au
   relancement**.
2. La palette (arbre par catégories) sélectionne un type de tuile ; peindre au pinceau, remplir au
   rectangle et sélectionner une région fonctionnent sur le viewport et modifient le niveau.
3. Annuler/Refaire, copier/coller une région, et redimensionner le niveau (avec confirmation si
   destructeur) se comportent comme dans l'éditeur historique.
4. Enregistrer produit un fichier de niveau **identique** (même sérialisation `LevelWriter`) à celui
   de l'éditeur historique pour un même contenu ; l'essai immédiat lance le niveau en cours.
5. Aucune règle d'édition n'est dupliquée dans Qt (tout passe par `core::LevelDraft`) ; la logique
   nouvelle découplée de Qt est **couverte par des tests**.
6. Build `/W4 /WX` sans avertissement, Doxygen et lint des exigences verts ; l'IHM Qt est **vérifiée
   visuellement**.

## Dépendances
- Bâtit sur [LOT-34](@ref lot-34) (viewport, boucle, entrées). Réutilise `core::LevelDraft`
  (`LOT-14`), la taxonomie de palette (`LOT-27`), `hmi::EditorTool`/`Camera2D` et le chemin
  `LevelWriter`/`GameScreen`. Ne modifie pas `Core`.

## Navigation des tâches
- @subpage lot-35-tache-01-fenetre-docks-persistance
- @subpage lot-35-tache-02-palette-arbre
- @subpage lot-35-tache-03-outils-peinture-viewport
- @subpage lot-35-tache-04-redim-enregistrement-essai
