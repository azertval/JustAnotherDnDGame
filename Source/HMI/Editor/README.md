# HMI/Editor/

Périmètre **éditeur de niveau** de l'application Qt : les **panneaux** dockables et la **logique pure**
(testable hors Qt/GPU) qui les alimente. Le canevas d'édition lui-même est le viewport partagé
`hmi::GameViewport` (dossier `Game/`, mode édition : peinture, outils, grille `F10`) ; il opère sur
`core::LevelDraft` (modèle mutable/sérialisable de `Core`) et rend via `hmi::DraftRenderer`
(dossier `Graphics/`).

Panneaux Qt :

- **Palette** (`PalettePanel`) — `QTreeView` catégories → sous-groupes → tuiles, alimenté par la
  taxonomie pure `tileTaxonomy` (`TileTaxonomy.{h,cpp}`, tous les `core::TileType` couverts).

- **Niveaux** (`LevelBrowserPanel`) — liste/recherche du dossier `Levels`, création / renommage /
  duplication / suppression, déléguant aux opérations fichiers pures.
  Un onglet « Graphe » (`WorldGraphView`, `LOT-11`) montre les cartes du dossier et leurs portails.
- **Liens** (`LinkPanel`) — liaisons déclencheur → cible (interrupteur/plaque → porte, danger
  commuté) du niveau courant, en regard des flèches dessinées dans le viewport (`LOT-37`).
- **Textures** (`TexturePanel`) — panneau d'habillage unique, organisé en sections (Skins pour
  l'instant, `LOT-42`) : jeu de skins courant, association type de tuile → asset/mode, choix de
  l'asset par vignettes (double-clic, `AssetPickerDialog` interne, `LOT-43`).

**`AssetThumbnailView`** — widget de vignettes partagé (grille, recherche, import / renommage /
duplication / suppression avec avertissement de références), réutilisable tel quel par toutes les
sections du panneau « Textures » à venir (Fond, Objets, Animations, Décors — `LOT-43`). Ignore
délibérément la sémantique d'une section : la détection des références lui est fournie par un
`ReferenceChecker` externe.

Logique pure (aucune dépendance Qt/GPU, couverte par `Source/Test/Unit`) :

- `tileTaxonomy` — arbre catégories/tuiles de la palette.
- `LevelFileOperations` — créer / renommer / dupliquer / supprimer un fichier de niveau.
- `LevelNameValidation` — validation d'un nom de niveau saisi.
- `EditorTool` — énumération de l'outil actif.
- `LinkGesture` — machine à états du geste de liaison (`resolveLinkClick`), indépendante de Qt.
- `LinkGeometry` — géométrie des flèches de liaison (segment, pointe, écartement anti-superposition).
- `WorldGraphLayout` — disposition du graphe du monde (cercle, fantômes, flèches regroupées, désignation).
- `SkinAssignments` — lignes du panneau « Textures », balayage des skins, effet d'une assignation.
- `AssetLibrary` — balayage/filtrage d'un dossier d'assets, partagé par `AssetThumbnailView`.
- `AssetFileOperations` — import / renommer / dupliquer / supprimer un fichier d'asset (`LOT-43`).
- `AssetReferences` — détection des entrées de `skins.json` citant un asset (`LOT-43`).

## À venir

Le programme d'habillage `LOT-40` → `LOT-55` ajoute ici : les sections Fond, Objets, Animations et
Décors du panneau « Textures » (`LOT-44` et suivants), un outil d'**assignation de texture** par
case (`LOT-45`), un outil de **placement de décors** (`LOT-50`), le contrôle de **visibilité par
calque** (`LOT-51`) et l'**atelier pixel art** (`LOT-54`), qui se branchera sur
`AssetThumbnailView` comme point d'entrée.

Réf. specs : [`editeur-niveaux.md`](../../../Documentation/Specification/editeur-niveaux.md),
[`interface-ihm.md`](../../../Documentation/Specification/interface-ihm.md) (`EX-EDIT-*`, `EX-IHM-*`).
