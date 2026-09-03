# TACHE-01 — Machine à états d'écran {#lot-59-tache-01-machine-etats-ecrans}

**Lot :** [LOT-59](epic.md) · **Emplacement :** `Source/HMI/Interface` · **Statut :** fait

## Contexte
`hmi::ScreenId` connaît quatre écrans — `Menu`, `Game`, `Editor`, `Options` — et la navigation entre
eux est écrite à même `hmi::MainWindow` : chaque méthode `showXxx()` répète la même litanie
(basculer le `QStackedWidget`, montrer ou cacher les docks, la barre de menus, les deux barres
d'outils, activer ou non la navigation manette, réarmer la barre d'état). Ajouter deux écrans en
copiant ce bloc deux fois de plus rendrait la dixième incohérence inévitable.

`EX-GP-041` demande des transitions **explicites et unidirectionnelles**. Aujourd'hui elles sont
implicites : n'importe quelle méthode peut appeler n'importe quelle autre, et rien ne dit qu'on ne
peut pas atteindre `Pause` depuis l'éditeur.

## Travail à réaliser
- **Ajouter `Pause` et `NiveauTermine`** à `hmi::ScreenId`, et un `FinDeSequence` si la
  `TACHE-03` conclut qu'il ne se confond pas avec le précédent.
- **Extraire la navigation** de `MainWindow` vers une unité dédiée : à partir de (état courant,
  événement), quel écran devient courant et quel est l'habillage de fenêtre associé (docks, menus,
  barres d'outils, navigation manette). Table de transitions **pure**, testable sans Qt — même
  patron que `hmi::PanelFocus` et `hmi::ActionCatalog`, qui séparent déjà la décision du widget.
- **Transitions autorisées uniquement** : toute transition non déclarée est refusée et journalisée,
  plutôt que de basculer silencieusement. On ne peut pas atteindre `Pause` autrement que depuis
  `Game`, ni `NiveauTermine` autrement que par une réussite.
- **Ne pas déplacer autre chose.** Les méthodes `showXxx()` deviennent des consommatrices de la
  table ; leur contenu propre (construction des panneaux, connexions) reste où il est.

## Fichiers impactés
- `Source/HMI/Interface/ScreenFlow.{h,cpp}` (nouveau) — table de transitions pure.
- `Source/HMI/Interface/MainWindow.{h,cpp}` — `ScreenId` étendu, `showXxx()` réécrites en termes de
  la table.
- `Source/Test/Unit/HMI/Interface/test_screen_flow.cpp` (nouveau).
- `Source/Test/CMakeLists.txt` (compiler `ScreenFlow.cpp` dans `UnitTests`, comme `PanelFocus.cpp`).

## Tests (obligatoires)
- Chaque transition **autorisée** mène à l'écran attendu, pour chacun des six écrans.
- Chaque transition **interdite** est refusée et laisse l'écran courant inchangé (notamment :
  `Editor` → `Pause`, `Menu` → `NiveauTermine`).
- L'habillage de fenêtre associé à chaque écran est celui attendu (docks visibles ou non, barres,
  navigation manette) — c'est ce qui est aujourd'hui répété à la main et diverge.
- Tests purs, sans Qt ni GPU.

## Points d'attention
- **Ne pas transformer ceci en refonte de `MainWindow`.** Le lot ajoute deux écrans ; la table est
  extraite parce qu'on la modifie, pas parce que le fichier est long.
- La bascule `Game` → `Options` existe déjà depuis le menu de pause **et** depuis le menu principal :
  le retour doit revenir d'où l'on vient, ce que la table doit porter explicitement plutôt que par
  une variable « écran précédent » posée ailleurs.
- `hmi::EditorActions::setEditingCommandsEnabled` et la visibilité des docks sont déjà pilotées par
  écran : les faire dériver de la table, pas les recopier à côté.

## Définition de fait (DoD)
- `Pause` et `NiveauTermine` existent, toute transition d'écran passe par une table pure et testée,
  les transitions interdites sont refusées, aucun comportement d'écran existant n'a changé ;
  `/W4 /WX` propre.

## État
`hmi::ScreenFlow` (table pure) et le câblage de `MainWindow` sont en place, avec les six écrans et
leurs transitions testées (`test_screen_flow.cpp`, 5 tests). `Pause` et `NiveauTermine` existent
dans l'énumération et la table, mais ne sont **pas encore atteignables depuis l'IHM réelle** :
aucun bouton ni raccourci ne déclenche encore `OpenPause`/`LevelSucceeded` — c'est le travail de
`TACHE-02`/`TACHE-03`, qui ajouteront les écrans eux-mêmes (`PauseScreen`, `LevelCompleteScreen`)
et les événements qui les déclenchent.

## Exigences
`EX-IHM-004` (écrans de pause et de fin de niveau) ; réutilise `EX-GP-040` (états de jeu),
`EX-GP-041` (machine à états), `EX-IHM-001` (interface hors-jeu en Qt), `EX-NFR-010` (logique
testable sans fenêtre).
