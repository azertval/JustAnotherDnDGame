# Source/Editor/

Le **module éditeur de cartes** (`LOT-EDITOR-01`) : un outil interne, fait pour l'auteur seul, qui
sert à fabriquer les cartes du jeu. Son programme est la
[feuille de route de l'éditeur](../../Documentation/Editeur/feuille-de-route.md) ; sa spécification,
[`editeur-niveaux.md`](../../Documentation/Specification/editeur-niveaux.md).

Le module dépend de `Core` (modèle et validation de carte, manifeste des pièces), de
`SceneComposition` (la composition d'un lieu, sans GPU, partagée avec le jeu) et de `HmiLib` (carte
jouée par l'essai). **Rien ne dépend de lui** : ni le jeu, ni `HmiLib`. Depuis le `LOT-EDITOR-02`,
l'éditeur ne parle plus au GPU : il peint la composition du jeu par `QPainter`.

| Dossier | Contenu | Cible |
|---|---|---|
| [`Logic/`](Logic/) | La logique pure : sans Qt, testée sous `Source/Test/Unit/Editor` | `EditorLogic` (bibliothèque statique, liée par `UnitTests`) |
| [`Ui/`](Ui/README.md) | Les widgets, construits en code : fenêtre, canevas, panneaux | `LevelEditor` (point d'entrée : `Source/App/Editor/Main.cpp`) |

## Règles du module

- **Outil interne.** Style Fusion de Qt, icônes standard ou libellés texte, **textes anglais écrits
  dans le code** : ni charte, ni thème, ni catalogue de traduction, ni formulaire `.ui`.
- **Le brouillon est la seule source.** `core::LevelDraft` porte toute la carte ; le canevas en est
  le seul propriétaire, les panneaux demandent et il applique.
- **Tout ce qui a une règle est pur et testé** dans `Logic/` ; l'IHM ne fait que l'afficher.
- **Aucun travail perdu.** Sauvegarde automatique et reprise (`EX-EDIT-056`), garde du fichier
  modifié sur disque (`EX-EDIT-057`), historique plafonné et « modifié » qui suit le contenu
  (`EX-EDIT-058`).
- **On édite ce qu'on jouera.** Le canevas peint la liste de primitives que compose le jeu, et son
  image égale celle du GPU à une tolérance près (`EX-EDIT-059`).
- **On pose des pièces, la collision suit.** La palette est la planche du lieu ; une pièce écrit sa
  couche, sa pièce et sa collision en un geste, et seule la main force une case (`EX-EDIT-063` à
  `EX-EDIT-065`).
- **Un geste, un pas.** Du clic au relâchement, tout outil — trait, ligne, seau, rectangle, reflet
  du miroir — se défait d'un seul `Ctrl+Z` (`core::GestureScope`, `EX-EDIT-066`).

## Logique pure (`Logic/`)

- `TileTaxonomy` — l'arbre catégories/tuiles de la palette des types, libellés compris.
- `PieceCatalog` — le catalogue des pièces d'un lieu (groupes par classe, recherche, pièces
  absentes de la planche), la couche où va une pièce et le type qu'elle écrit (`EX-EDIT-063`).
- `BrushGesture` — un coup de pinceau sur un rectangle : un type, une pièce ou la gomme, refus
  compris ; ce que la souris appelle, et ce qu'appellera l'éditeur sans fenêtre (`EX-EDIT-064`,
  `EX-EDIT-065`).
- `PaintTools` — les outils du peintre : trait, rectangle, ligne, seau, pipette, miroir et mesure,
  chacun en un geste du brouillon (`EX-EDIT-066`, `EX-EDIT-067`, `EX-EDIT-069`).
- `EditorSidecar` — l'annexe d'une carte, `<carte>.editor.json`, et ses notes d'auteur
  (`EX-EDIT-068`).
- `MapFormat` — `--migrate` et `--check`, la garde du format v4 (`EX-EDIT-062`).
- `LevelFileOperations`, `LevelNameValidation` — créer, renommer, dupliquer, supprimer une carte.
- `EditorTool`, `PanelFocus` — l'outil actif et le panneau qu'il met en avant.
- `EntityGesture` — le geste de l'outil « Entité » : sélectionner, poser, déplacer.
- `EntityReferences`, `EditorDiagnostics` — les catalogues que les entités citent, et les
  avertissements rendus en anglais.
- `LayerView` — les couches telles que l'éditeur les montre : visibles, opacité, grisées,
  verrouillées (`EX-EDIT-061`).
- `CanvasPicking` — le pointage du canevas, iso et à plat : la case sous un point, par son losange,
  hauteur en paramètre ; les cases visibles d'un cadrage (`EX-EDIT-060`).
- `CanvasScene` — l'instantané que le canevas compose (celui du jeu, PNJ compris, sans héros) et
  l'opacité de chaque bande de la scène selon les couches (`EX-EDIT-059`, `EX-EDIT-061`).
- `EditorStatus`, `EditContextTarget` — la barre d'état, la cible des commandes d'édition.
- `WorldGraphLayout` — la disposition du graphe du monde.
- `ThumbnailGeometry` — les vignettes à l'échelle d'affichage réelle.
- `EditorKeyBindings` — les raccourcis remappables, persistés en JSON.
- `Autosave` — les brouillons de reprise (`%LOCALAPPDATA%\JustAnotherRpgGame\Editor\autosave`) et
  les versions mises de côté (`conflicts\`).
- `DiskGuard` — l'empreinte d'un fichier et la réaction à son changement.

## Fichiers du poste

| Chemin | Contenu |
|---|---|
| `%LOCALAPPDATA%\JustAnotherRpgGame\Editor\autosave\*.autosave.json` | Brouillon en attente de reprise, un par carte ; retiré à l'enregistrement et à la fermeture voulue. |
| `%LOCALAPPDATA%\JustAnotherRpgGame\Editor\autosave\conflicts\` | Versions écartées par un choix (reprise refusée, disque relu ou gardé de côté). Jamais nettoyé automatiquement. |

`LevelEditor --crash-test` plante juste après la première sauvegarde automatique : c'est la façon
d'éprouver la reprise.

## Fichiers à côté des cartes

| Chemin | Contenu |
|---|---|
| `Levels/<carte>.editor.json` | L'annexe de la carte : ses notes d'auteur, une par case. Le jeu ne la lit jamais, aucune liste de cartes ne la prend pour une carte ; elle suit la carte qu'on renomme, duplique ou supprime. |

## Touches des outils

| Touche | Outil |
|---|---|
| `B` · `R` · `L` · `G` | pinceau, rectangle, ligne, seau |
| `E` · `I` | gomme, pipette (`Alt` + clic : pipette depuis tout outil) |
| `S` · `O` | sélection (`Suppr` la gomme), entité |
| `D` · `N` | mesure (5 pieds la case), note d'auteur |
| `M` | miroir, par la case survolée |
| `P` · `Shift+P` | essai depuis l'entrée, depuis la case survolée |
