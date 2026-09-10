# Écrans, navigation et boucle de jeu {#guide-ecrans}

> **Refondu au `LOT-86`.** Les écrans du jeu ne sont plus des pages empilées de widgets Qt mais des
> fichiers QML, et l'éditeur de niveaux est un binaire séparé qui n'en héberge plus aucun. La
> **table de transitions** décrite ci-dessous, elle, n'a pas bougé d'une ligne : elle est pure,
> testée, et `hmi::ScreenRouter` se contente de l'appeler pour le compte du QML. Ce qui suit reste
> vrai du *quoi* et du *pourquoi* ; le *comment* de l'affichage est en @ref guide-ihm-qt.


Cette page explique comment l'application passe du menu au jeu, à l'éditeur ou aux options, ainsi
que la boucle de jeu complète ajoutée en `LOT-59` — pause, fin de niveau, sélection de niveau et
progression persistée. Elle relie les écrans décrits dans les autres pages (@ref guide-entrees pour
le menu et les options, @ref guide-niveaux et @ref guide-editeur pour ce que le jeu et l'éditeur
affichent une fois actifs). Depuis le `LOT-38`, l'application est une IHM **Qt** (@ref guide-ihm-qt) :
la navigation n'est pas une machine à états rejouée à chaque pas de simulation, mais un mécanisme
**événementiel** classique de Qt (widgets empilés + signaux/slots) piloté par une table pure.

## La machine à états : `hmi::ScreenFlow`

`hmi::ScreenFlow.h` (`Source/HMI/Interface/ScreenFlow.h`) porte la navigation comme une **table
pure**, sans dépendance Qt — testable hors instance d'application (`EX-NFR-010`), même patron que
`hmi::PanelFocus`/`hmi::ActionCatalog`. Deux fonctions :

- `hmi::resolveTransition(current, event)` : résout un `hmi::ScreenEvent` depuis l'état courant
  (`hmi::ScreenState`) vers le nouvel écran, ou `std::nullopt` si la transition est **interdite**
  depuis cet écran — jamais de bascule silencieuse (`EX-GP-041`).
- `hmi::dressingFor(screen)` : l'habillage attendu (docks visibles, barre de menus, barres
  d'outils, commandes d'édition, navigation manette) — ce que chaque ancien `showXxx()` répétait à
  la main.

`hmi::ScreenId` compte six écrans : `Menu`, `Editor`, `Game`, `Options`, `Pause`, `Credits`.
`MainWindow::transitionScreen(event)` est l'**unique** point d'entrée : il résout la
transition, puis applique l'habillage (`applyScreenDressing`) — aucun code ne bascule d'écran
autrement.

### Pages empilées et recouvrements : deux patrons distincts

`hmi::MainWindow` (un `QMainWindow`) héberge les écrans dans un **`QStackedWidget`** central, mais
tous n'y vivent pas de la même façon :

- **Pages normales** (une par écran hors jeu) : `Menu` (`hmi::MainMenu`), `Options`
  (`hmi::OptionsPage`), `Credits` (`hmi::CreditsScreen`), et le **conteneur du viewport** partagé
  par `Editor`/`Game` (`hmi::GameViewport`). Ajoutées via `QStackedWidget::addWidget`, Qt gère leur
  taille.
- **Recouvrement** (`Pause`) : `hmi::PauseScreen` n'est **jamais** une page de la pile —
  `applyScreenDressing` laisse le conteneur du viewport affiché derrière lui, ce qui permet à la
  **scène de rester visible** (figée) derrière l'écran de pause. Ce sont des **fenêtres de haut niveau** propres (`Qt::Dialog |
  Qt::FramelessWindowHint`, fond translucide, possédées par `MainWindow`), positionnées en
  coordonnées **écran** sur le rectangle de `_editorContainer` (`MainWindow::syncOverlayGeometry`,
  resynchronisé à chaque déplacement **et** redimensionnement de la fenêtre principale — un
  recouvrement en fenêtre de haut niveau ne suit sinon aucun des deux automatiquement), et
  activées explicitement (`activateWindow()`, focus posé un tour de boucle d'événements plus tard
  via `QTimer::singleShot(0, …)` -- `activateWindow()` ne fait que **poster** la demande, Qt ne
  marque la fenêtre réellement active qu'en traitant le `WM_ACTIVATE` en retour) puisqu'une fenêtre
  de haut niveau distincte ne partage pas l'activation de `MainWindow`. **Pas `Qt::Tool`** (essayé
  d'abord) : sur Windows, Qt affiche une fenêtre `Qt::Tool` avec `SW_SHOWNOACTIVATE` par conception
  (pensé pour les palettes flottantes qui ne doivent jamais voler le focus), ce qui empêchait
  `activateWindow()` de fonctionner du tout. Et même fenêtre bien activée, un bouton qui a le focus
  ne répond à **Entrée** que si `QPushButton::autoDefault` vaut vrai — vrai par défaut seulement
  si un ancêtre **C++** du bouton est un vrai `QDialog` (`qobject_cast`, indépendant de l'indicateur
  de fenêtre) : posé explicitement (`setAutoDefault(true)`) sur chaque bouton de ces écrans **et**
  de `hmi::MainMenu`, qui partage le même défaut.

  **Ce n'était pas la première tentative.** La première version faisait de ces écrans de simples
  widgets Qt **frères** du conteneur du viewport (même parent, `_stack`), sur la foi d'un patron cru
  documenté par Qt : la fenêtre **native** qu'embarque `QWidget::createWindowContainer` (le viewport
  Direct3D 11) peinturait par-dessus ses **descendants** Qt, donc un widget **frère** — simple widget
  Qt aux yeux du compositeur — se dessinerait normalement par-dessus. Faux en pratique, découvert à
  l'essai manuel (`LOT-59` TACHE-07) : la simulation se figeait bien, mais **aucun écran
  n'apparaissait** — personnage figé, rien à l'écran. La documentation Qt confirme, une fois le bon
  terme cherché : une fenêtre native embarquée peint **toujours** par-dessus **tous** ses frères Qt
  de la même fenêtre de haut niveau, quel que soit leur `raise()` — ce n'est pas une affaire de
  descendant vs frère, c'est une limitation de superposition **au sein d'une même fenêtre native**.
  Seule une fenêtre de haut niveau **distincte** s'en affranchit, d'où la conception actuelle.

`Credits`, à l'inverse, est une page normale : atteint depuis le **menu**, jamais en cours de
partie, il n'a aucune scène à laisser visible derrière lui.

## Qui déclenche les transitions : les signaux

Aucun écran ne bascule lui-même vers un autre : chaque écran **émet un signal** d'intention, et
`MainWindow` le **connecte** à la méthode qui appelle `transitionScreen` puis agit. Un écran est
ainsi **testable/éditable en isolation** (il émet ses signaux sans rien connaître des autres), et
ajouter un écran revient à ajouter une page/un recouvrement à la pile et un `connect` — sans
toucher aux écrans existants.

Le menu principal (`hmi::MainMenu`) expose cinq signaux — un par entrée, et rien de plus. Le
`LOT-67` en a retiré deux : « Continuer » et « Choisir un niveau » n'avaient plus d'écran où mener.

```cpp
connect(_menu, &MainMenu::newGameRequested, this, &MainWindow::newGame);
connect(_menu, &MainMenu::editorRequested,  this, &MainWindow::showEditor);
connect(_menu, &MainMenu::optionsRequested, this, &MainWindow::showOptions);
connect(_menu, &MainMenu::creditsRequested, this, &MainWindow::openCredits);
connect(_menu, &MainMenu::quitRequested,    this, &QWidget::close);
```

## Le viewport partagé : éditeur **et** jeu

Le même `hmi::GameViewport` sert de canevas à l'éditeur et de surface de jeu (une seule intégration
Direct3D 11 à maintenir, @ref guide-ihm-qt) :

- **`startGame(levels, startIndex)`** joue une **séquence** de niveaux via `hmi::GameSession` (@ref
  guide-niveaux), à partir de `startIndex` (0 = depuis le début ; « Continuer »/sélection de niveau
  reprennent plus loin, ci-dessous). `Échap`/bouton manette **B** ouvre désormais la **pause**
  plutôt que de quitter directement (`GameViewport::pauseRequested`, ancien comportement décrit par
  `EX-REN-031` avant `LOT-59`).
- en **mode édition**, le viewport peint sur `core::LevelDraft` et propose l'**essai immédiat** :
  jouer le brouillon courant puis y revenir exactement où l'édition en était — la même `GameSession`
  est réutilisée, sans duplication (@ref guide-editeur). L'essai (`stopPlaytest`) garde son propre
  chemin de retour, entièrement séparé de la pause/fin de niveau ci-dessous : `Échap` en essai
  revient directement à l'édition, jamais à un écran de pause.

Le viewport signale aussi ses messages d'état via **`statusMessage`** (barre de statut de la
fenêtre) — par exemple l'échec d'un enregistrement de brouillon invalide.

## Pause : suspendre sans perdre un pas

`Échap` (ou bouton **B** manette) en **partie réelle** ouvre l'écran de pause
(`hmi::PauseScreen`) : *Reprendre*, *Recommencer le niveau*, *Options*, *Quitter vers le menu*
(confirmation requise, la progression du tableau en cours serait perdue). L'exigence
(`EX-IHM-004`) est que la simulation soit **réellement suspendue**, pas ralentie — et c'est là
qu'un piège classique de l'accumulateur à pas fixe (@ref guide-boucle) apparaît.

**Suspendre, ce n'est pas multiplier `dt` par zéro.** Si `GameViewport::tick()` continuait
d'appeler `core::FixedTimestep::advance(0)` pendant la pause, la boucle consommerait quand même des
pas (zéro à chaque fois, mais des pas) — ou pire, laisserait l'illusion que le temps ne compte pas
alors que tout ce qui se mesure en **pas** (animations, dangers temporisés, budgets) resterait
figé sans que ce soit délibéré. La bonne suspension, c'est **ne pas appeler `advance()` du tout**
tant que `_paused` est vrai : l'accumulateur ne reçoit aucun temps réel à convertir, donc `steps`
vaut zéro par construction, sans même avoir à le vérifier.

**Le piège du réarmement d'horloge.** `_previousFrame` (l'instant de référence dont
`GameViewport::tick()` soustrait `now()` pour obtenir le temps écoulé) continue, lui, de s'écouler
pendant la pause — c'est une horloge murale, pas un compteur de pas. Si rien ne le corrige,
`GameViewport::resumeSimulation()` verrait au **premier** tick suivant un `elapsedSeconds` égal à
la **durée entière de la pause** (potentiellement plusieurs minutes) ; `FixedTimestep::advance`
rendrait alors des **dizaines de pas d'un coup** — le personnage traverserait le niveau. C'est
exactement la « spirale de la mort » que le plafond `maximumStepsPerCall` de `FixedTimestep`
atténue (@ref guide-boucle) sans jamais s'y substituer : un plafond limite les dégâts d'un
rattrapage, il ne l'empêche pas de commencer. La correction est de **réarmer** `_previousFrame` sur
l'instant courant au moment de la reprise :

```cpp
void GameViewport::resumeSimulation() {
    _paused = false;
    _previousFrame = Clock::now();  // sans ceci : rattrapage massif au premier tick suivant
}
```

**Le rendu continue** pendant la pause : la scène reste dessinée derrière l'écran (l'interpolation
d'affichage se fige naturellement, la position simulée n'avançant plus) — c'est ce que permet le
patron de recouvrement décrit plus haut.

**Piège annexe, la manette tenue.** La manette est *pollée*, pas événementielle : un bouton **B**
maintenu au moment précis où la pause s'ouvre laisserait, sans précaution, un front « bouton
pressé » périmé qui ferait immédiatement ressortir de la pause à la reprise (front fantôme). La
correction : `GameViewport::tick()` continue d'appeler `hmi::InputState::beginFrame()` à **chaque**
image même en pause (juste hors de la boucle de pas), pour que la ligne de base des fronts reste à
jour et qu'un bouton simplement *tenu* ne soit jamais relu comme *pressé* à la reprise.

## Ce que le `LOT-67` a retiré

Ce guide décrivait, jusqu'au `LOT-67`, trois mécanismes de plus : un **écran de fin de niveau**,
une **sélection de niveau** et une **progression persistée** au tableau. Les trois supposaient une
séquence ordonnée de tableaux, que le jeu visé n'a pas — et ils ont été retirés avec elle, code,
écrans et exigences (`EX-LVL-010` → `EX-LVL-015`, `EX-IHM-005`, `EX-GP-030` → `EX-GP-032`, toutes
consignées « retirées » dans leur spécification plutôt que supprimées ; `EX-GP-040`, `EX-IHM-003`
et `EX-IHM-004`, elles, sont **refondues** — elles avaient un objet au-delà du niveau discret).

Ce qui les remplace n'est pas écrit ici, parce que ce n'est pas encore écrit du tout : le **graphe
de cartes** du `LOT-09` dira comment on passe d'une carte à l'autre, et la **sauvegarde riche** du
`LOT-17` ce qu'on retrouve en revenant. « Continuer » reviendra au menu avec elle — pas avant : une
entrée de menu qui ne mène nulle part coûte plus de confiance qu'elle n'apporte d'information
(`EX-IHM-072`).

## Où ça s'insère dans la boucle

L'**event loop Qt** (`QApplication::exec`) possède la navigation. La **boucle de jeu à pas fixe**
(déterminisme `EX-NFR-002`, @ref guide-boucle) ne tourne que **dans** le viewport, tant qu'il est en
mode jeu ou édition **et non en pause** : elle est cadencée par `QEvent::UpdateRequest`
(`QWindow::requestUpdate`) et rejoue exactement la discipline historique — sonder la manette,
convertir le temps réel en pas fixes, mettre à jour puis `hmi::InputState::beginFrame` **par pas**,
rendre **une fois** par frame avec interpolation (`EX-ARCH-031`). Le détail est dans @ref
guide-ihm-qt.

## Voir aussi
- `hmi::MainWindow`, `hmi::MainMenu`, `hmi::OptionsPage`, `hmi::GameViewport`, `hmi::GameSession`.
- `hmi::ScreenFlow`, `hmi::ScreenId`, `hmi::ScreenEvent`, `hmi::ScreenState`, `hmi::ScreenDressing`.
- `hmi::PauseScreen`, `hmi::CreditsScreen`.
- @ref guide-ihm-qt — le socle Qt : viewport Direct3D 11 embarqué, boucle et entrées Qt.
- @ref guide-entrees — `hmi::MainMenu` et `hmi::OptionsPage`, deux écrans concrets et le remappage.
- @ref guide-niveaux, @ref guide-editeur — ce que le jeu et l'éditeur font une fois actifs, le
  format des cartes, et comment l'éditeur réutilise `hmi::GameSession` pour l'essai immédiat.
- @ref guide-boucle — l'accumulateur à pas fixe suspendu pendant la pause.
