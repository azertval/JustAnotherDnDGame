# TACHE-08 — Espace « Plans », panneau et cycle de vie des fichiers {#lot-69-tache-08-espace-plans}

**Lot :** [LOT-69](epic.md) · **Emplacement :** `Source/HMI/{Editor,Interface}`,
`Source/Elements/UI` · **Statut :** livré

## Contexte
Le `LOT-68` a introduit des **espaces de travail exclusifs** (`hmi::EditorWorkspace`) : édition de
niveau, atelier pixel art. Le mode création est le troisième — c'est cette structure, et non un
énième bouton, qui le rend propre.

Le panneau des plans prend la place laissée par celui des décors. `TexturePanel` serait défendable,
un plan relevant de l'apparence, mais le `LOT-57` l'a recentré et le `LOT-67` a explicitement refusé
d'y remettre autre chose ; une liste réordonnable avec propriétés par élément est exactement la forme
qui justifiait un dock au `LOT-67`.

## Travail à réaliser
- `EditorWorkspace` : ajouter `Plans`, porter le compte d'espaces de 2 à 3, adapter l'habillage.
  **`workspaceForPanel` doit renvoyer un masque** et non une valeur unique : il est aujourd'hui
  déclaré *total* — « chaque panneau appartient à exactement un espace » — alors que le canevas,
  l'historique et la palette doivent apparaître dans **deux** espaces. Dupliquer les docks serait
  pire : deux canevas, deux historiques. La garde de complétude devient « masque non vide ».
- `PanelId::Decors` → `PanelId::Planes` (le nombre de panneaux ne change pas).
- `PlanesPanel.{h,cpp}` + **`PlanesPanel.ui`** : liste ordonnée (nom, densité, profondeur, parallaxe
  X/Y, opacité, œil de visibilité, bouton « isoler »), boutons Ajouter / Supprimer / Monter /
  Descendre / Premier plan / Arrière-plan, case « parallaxe active » du niveau, et un bouton
  « Peindre » qui bascule dans l'espace Plans avec le plan sélectionné chargé.
- **Cycle de vie du fichier** : « Ajouter » crée un PNG **entièrement transparent** aux dimensions
  exactes `width × ppu` par `height × ppu`, nommé d'après le niveau, dans le dossier des plans, avec
  unicité par suffixe numérique. Changer la densité **rééchantillonne** (`resamplePlane`) et
  réécrit. Redimensionner le niveau recadre tous les plans.
- **Enregistrement** : le canevas est *dirty* indépendamment du brouillon — deux notions déjà
  distinctes depuis le `LOT-54`. Dans l'espace Plans, `Ctrl+S` enregistre **le PNG** ; l'enregistrement
  de niveau écrit le JSON. Garde-fou de perte de travail sur les deux.
- **Griser la case « parallaxe » en mode de cadrage `WholeLevel`** et l'expliquer (voir `TACHE-06`).
- Barre d'état : nom du plan, résolution, densité, coordonnée pixel sous le curseur, couleur
  courante et **poids mémoire** (`EX-NFR-043`).

## Fichiers impactés
`Source/HMI/Interface/EditorWorkspace.{h,cpp}`, `MainWindow.{h,cpp}`,
`Source/HMI/Editor/{PlanesPanel,PanelFocus,EditorStatus}.{h,cpp}`,
`Source/Elements/UI/{PlanesPanel.ui,MainWindow.ui}`, `Source/Elements/Localization/*`.

## Tests (obligatoires)
- `test_editor_workspace.cpp` : complétude **par masque** ; les trois espaces ; jamais deux barres
  d'outils simultanées.
- `test_panel_focus.cpp` : adapté au renommage du panneau.
- `test_plane_file_naming.cpp` (nouveau) : unicité du nom, dimensions attendues pour une densité
  donnée, caractères acceptés (patron de `LevelNameValidation`).

Le panneau lui-même relève de la **vérification IHM manuelle**, comme le panneau de propriétés du
`LOT-67`.

## Points d'attention
La mise en page vit **intégralement dans le `.ui`** (Qt Designer) ; le C++ ne branche que le
fonctionnel. Contrainte de projet, pas préférence de style.

Les champs numériques émettent sur **`editingFinished`**, jamais `valueChanged` : sinon taper
« 0.75 » empile quatre pas d'annulation. Leçon déjà tirée au `LOT-67`.

**La suppression d'un plan n'est pas annulable côté fichier.** Le brouillon annule l'entrée JSON,
pas la disparition du PNG. Choix retenu : **ne jamais supprimer le fichier**, seulement l'entrée. À
écrire dans la documentation utilisateur — un fichier orphelin est moins grave qu'un travail perdu.

## Definition de fait (DoD)
Trois espaces de travail cohérents, plans éditables et réordonnables, PNG créés aux bonnes
dimensions, aucun mélange d'historique. `ctest` à 100 %.

## Exigences
`EX-EDIT-047`, `EX-EDIT-046`, `EX-DEC-041`, `EX-DEC-043`, `EX-NFR-043`, `EX-IHM-073`.
