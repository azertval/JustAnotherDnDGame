# HMI/Editor/

Périmètre **éditeur de cartes** (cible `LevelEditor`) : le canevas, les **panneaux** dockables et la
**logique pure** (testable hors Qt/GPU) qui les alimente. Le canevas opère sur `core::LevelDraft`
(modèle mutable/sérialisable de `Core`).

Canevas :

- `EditorViewport` — deux états, jamais mêlés. En **édition**, le brouillon est dessiné à plat par
  `hmi::DraftRenderer` (dossier `Graphics/`) — une couleur par type de tuile, les entités par leur
  marqueur — sous les aides d'édition (grille `F10`). En **essai** (`P`), la carte est jouée par
  `hmi::WorldPlay` (dossier `Game/`) et dessinée par `hmi::WorldSceneRenderer`, avec les touches du
  jeu : la mise en scène du jeu, à l'identique (`EX-EDIT-055`).

Panneaux Qt :

- **Palette** (`PalettePanel`) — `QTreeView` catégories → sous-groupes → tuiles, alimenté par la
  taxonomie pure `tileTaxonomy` (`TileTaxonomy.{h,cpp}`, tous les `core::TileType` couverts).
- **Niveaux** (`LevelBrowserPanel`) — liste/recherche du dossier `Levels`, création / renommage /
  duplication / suppression, déléguant aux opérations fichiers pures.
  Un onglet « Graphe » (`WorldGraphView`, `LOT-11`) montre les cartes du dossier et leurs portails.
- **Couches** (`LayersPanel`) — couche active, visibilité, opacité, ajout, retrait, ordre et nom.
- **Entités** (`EntityPanel`) — famille à poser, liste des entités, propriétés de l'entité
  sélectionnée.

Logique pure (aucune dépendance Qt/GPU, couverte par `Source/Test/Unit`) :

- `tileTaxonomy` — arbre catégories/tuiles de la palette ; `TaxonomyLabels` — ses clés de
  traduction.
- `LevelFileOperations` — créer / renommer / dupliquer / supprimer un fichier de niveau.
- `LevelNameValidation` — validation d'un nom de niveau saisi.
- `EditorTool` — énumération de l'outil actif ; `PanelFocus` — panneau à mettre en avant selon
  l'outil.
- `EntityGesture` — geste de l'outil « Entité » : sélectionner, poser, déplacer (`LOT-11`).
- `EntityReferences` — catalogues que les entités référencent (dialogues, rencontres, cartes,
  points d'arrivée) ; `EditorDiagnostics` — avertissements sur les entités d'une carte.
- `LayerView` — couches d'une carte telles que l'éditeur les montre.
- `EditorStatus` — contenu de la barre d'état ; `EditContextTarget` — cible des commandes
  Annuler/Refaire/Copier/Coller.
- `WorldGraphLayout` — disposition du graphe du monde (cercle, fantômes, flèches regroupées, désignation).
- `ThumbnailGeometry` — dimensionnement des vignettes à l'échelle d'affichage réelle.

Réf. specs : [`editeur-niveaux.md`](../../../Documentation/Specification/editeur-niveaux.md),
[`interface-ihm.md`](../../../Documentation/Specification/interface-ihm.md) (`EX-EDIT-*`, `EX-IHM-*`).
