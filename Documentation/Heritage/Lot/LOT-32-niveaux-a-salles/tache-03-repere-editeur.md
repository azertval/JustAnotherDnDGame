# TACHE-03 — Repère visuel de salles dans l'éditeur {#lot-32-tache-03-repere-editeur}

**Lot :** [LOT-32](epic.md) · **Emplacement :** `HMI/Interface` · **Statut :** fait (confirmation
visuelle à l'écran non automatisée — dépendance rendu D3D11, laissée à un essai manuel par un
humain lançant l'éditeur)

## Contexte
`EditorScreen::renderGrid` sait déjà dessiner un **quadrillage de repère** fin sur chaque bord de
case (`_showGridLines`, un quad étroit et teinté par bord — `SpriteBatch` n'a pas de primitive de
ligne). Cette tâche ajoute un second quadrillage, plus **épais**/plus **visible**, uniquement sur
les frontières de **salles** (`RoomGrid`, TACHE-01), pour qu'un level designer puisse aligner ses
couloirs sur les bords de salles sans avoir à compter les cases. La caméra de l'éditeur (pan/zoom
manuel, `LOT-15`/`LOT-16`) **ne change pas** : ce repère est un calque visuel superposé, pas un
changement de cadrage (décision de cadrage de l'épic — la caméra par salle, TACHE-02, est
spécifique au jeu).

## Travail à réaliser
- **Construire un `hmi::RoomGrid`** à partir de la taille courante du brouillon (`_draft.tileMap()`)
  — recalculé quand la taille change (redimensionnement, `Ctrl+R`/flèches, `LOT-16`), comme les
  autres états dérivés de la taille de grille.
- **Dessiner les frontières de salles** dans `EditorScreen::renderGrid`, après les tuiles peintes
  et le quadrillage de repère existant (même ordre de calque) : un quad étroit sur chaque colonne/
  ligne de `RoomGrid` (pas chaque case), teinte **distincte** du quadrillage fin existant (plus
  opaque et/ou plus épais) pour rester lisible même quand les deux quadrillages sont actifs
  ensemble.
- **Bascule d'affichage** : réutilise le raccourci existant (`F10`, `_showGridLines`) plutôt qu'un
  second raccourci dédié — un seul geste affiche les deux repères (case par case **et** salles)
  ensemble ; plus simple et plus découvrable (`EX-EDIT-015`) qu'une commande de plus à mémoriser,
  pour un repère qui sert le même besoin (aligner son geste de peinture).
- **Aucun changement de cadrage caméra** de l'éditeur : `_manualCamera`/`_cameraCenter`/
  `_cameraZoom` restent pilotés exactement comme avant (`LOT-15`/`16`), le quadrillage de salles
  est un calque de plus, pas une nouvelle règle de zoom.

## Fichiers impactés
- `Source/HMI/Interface/EditorScreen.h`/`.cpp` (`renderGrid`, état `RoomGrid`, éventuel bouton de
  bascule).

## Tests (obligatoires)
- Non testable automatiquement (dépendance rendu D3D11, même limite que le reste de `renderGrid`) :
  vérifié par relecture et essai manuel — ouvrir/créer un niveau plus grand qu'une salle dans
  l'éditeur et constater que le quadrillage de salles apparaît aux bonnes frontières (comparer aux
  bornes renvoyées par `RoomGrid::roomBounds`, déjà testées en TACHE-01) et reste correct après un
  redimensionnement.
- Non-régression : le quadrillage de repère existant (`_showGridLines`) et le pan/zoom manuel
  (`LOT-15`) restent inchangés.

## Points d'attention
- Un niveau tenant dans une seule salle produit un `RoomGrid` à une seule salle (TACHE-01) : le
  quadrillage de salles se réduit alors au **contour du niveau entier**, sans ligne intermédiaire —
  comportement correct, pas un cas particulier à exclure.
- Garder la distinction visuelle nette avec le quadrillage de repère existant (bords de case) :
  l'un aide à viser une case précise, l'autre à repérer les frontières de salles — les confondre
  viderait la fonctionnalité de son intérêt.

## Définition de fait (DoD)
- Quadrillage de salles affiché dans l'éditeur, correct après redimensionnement, sans régression
  sur le pan/zoom ni le quadrillage de repère existant ; build `/W4 /WX` sans avertissement.

## Exigences
`EX-EDIT-023` (nouvelle), `EX-EDIT-013` (cadrage/pan/zoom de l'éditeur, inchangé), `EX-EDIT-015`
(commandes découvrables).
