# Source/Editor/

Le **module éditeur de cartes** (`LOT-EDITOR-01`) : un outil interne, fait pour l'auteur seul, qui
sert à fabriquer les cartes du jeu. Son programme est la
[feuille de route de l'éditeur](../../Documentation/Editeur/feuille-de-route.md) ; sa spécification,
[`editeur-niveaux.md`](../../Documentation/Specification/editeur-niveaux.md).

Le module dépend de `Core` (modèle et validation de carte) et de `HMI` (composition, rendu, carte
jouée par l'essai). **Rien ne dépend de lui** : ni le jeu, ni `HmiLib`.

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

## Logique pure (`Logic/`)

- `TileTaxonomy` — l'arbre catégories/tuiles de la palette, libellés compris.
- `LevelFileOperations`, `LevelNameValidation` — créer, renommer, dupliquer, supprimer une carte.
- `EditorTool`, `PanelFocus` — l'outil actif et le panneau qu'il met en avant.
- `EntityGesture` — le geste de l'outil « Entité » : sélectionner, poser, déplacer.
- `EntityReferences`, `EditorDiagnostics` — les catalogues que les entités citent, et les
  avertissements rendus en anglais.
- `LayerView` — les couches telles que l'éditeur les montre.
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
