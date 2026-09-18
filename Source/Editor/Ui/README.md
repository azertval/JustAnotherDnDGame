# Source/Editor/Ui/

Les **widgets** de l'éditeur (`LevelEditor`, Qt Widgets), tous construits en code : style Fusion,
textes anglais, aucun formulaire `.ui` (`LOT-EDITOR-01`).

- `MainWindow` — la fenêtre : le canevas au centre, quatre docks (palette, cartes, couches,
  entités) dont la disposition est persistée (`EX-IHM-011`), les menus et la barre d'état. Elle
  tient aussi le filet de sécurité : sauvegarde automatique et reprise, garde du fichier modifié sur
  disque, question à la fermeture.
- `EditorActions` — les commandes comme `QAction` uniques, partagées par la barre d'outils, les
  menus et les raccourcis remappables.
- `EditorViewport` — le canevas (`QRhiWidget`). En **édition**, le brouillon est dessiné à plat par
  `DraftRenderer` ; en **essai** (`P`), la carte est jouée par `hmi::WorldPlay` et dessinée par
  `hmi::WorldSceneRenderer`, comme dans le jeu (`EX-EDIT-055`). Réécrit au `LOT-EDITOR-02`.
- `DraftRenderer` — le brouillon à plat : une couleur par type de tuile, les entités par leur
  marqueur.
- `PalettePanel` — l'arbre des tuiles (`hmi::tileTaxonomy`).
- `LevelBrowserPanel`, `WorldGraphView` — la liste des cartes et le graphe du monde.
- `LayersPanel` — couche active, visibilité, opacité, ajout, retrait, ordre et nom.
- `EntityPanel` — famille à poser, liste des entités, propriétés et avertissements.

Chaque panneau garde ses widgets dans une `struct Widgets` privée, construite dans son `.cpp`.
