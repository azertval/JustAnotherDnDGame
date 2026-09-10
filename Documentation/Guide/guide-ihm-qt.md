# IHM Qt — deux applications, deux technologies {#guide-ihm-qt}

> Statut : **refondu** (`LOT-86`). Le **jeu** est une application **Qt Quick** ; l'**éditeur de
> niveaux** reste en **Qt Widgets**, dans son propre binaire. Le rendu de scène passe par **QRhi**
> — Direct3D 11 par défaut sous Windows — des deux côtés. L'apparence des écrans du jeu et le mode
> d'emploi de la conception sont en @ref guide-conception-qds, que cette page laisse de côté.

## Pourquoi deux binaires

L'éditeur et le jeu vivaient dans une seule application, et c'est de là que venaient les **2 472
lignes** de `MainWindow.cpp` : une seule technologie d'IHM devait servir deux besoins opposés.

- L'**éditeur** est un outil d'auteur : docks détachables, arbres, disposition persistée
  (`EX-IHM-010`/`011`). Qt Widgets y est le bon outil, et QML n'y apporterait rien.
- Le **jeu** est l'inverse : une image agrandie d'un facteur entier, dont l'apparence doit pouvoir
  changer **sans compiler** (`EX-IHM-100`).

| Cible | Technologie | Point d'entrée |
|---|---|---|
| `JustAnotherDnDGame` | Qt Quick, `QGuiApplication` | `Source/App/Game/Main.cpp` |
| `LevelEditor` | Qt Widgets, `QApplication` | `Source/App/Editor/Main.cpp` |

Elles partagent `Core`, `HMI/Graphics`, `HMI/Game`, `HMI/Input`, `HMI/Audio` et l'amorçage
(`App/Common/Bootstrap`) — tout ce qui n'est pas de la présentation. Elles ne partagent **aucune**
technologie d'IHM, et le jeu **ne lie pas `Qt6::Widgets`** (`EX-IHM-102`). Ce n'est pas une
convention : un widget qui y réapparaîtrait ferait échouer l'édition de liens.

## Les trois couches du jeu

```
Core                     règles et état, SANS Qt
  ↑
HMI/Presentation         vues-modèles : ce que le jeu SAIT DIRE
  ↑                      Qt6::Qml seulement — jamais Quick ni Widgets (EX-IHM-101)
Source/Ui (QML)          ce que ça DONNE À VOIR
```

`HMI/Presentation` transforme l'état du jeu en propriétés et en modèles de liste, et **ne dessine
rien**. Un écran lui demande *ce que le jeu sait dire*, jamais *comment le montrer*. Un seul en-tête
d'IHM qui y entrerait signalerait que la logique de vue a commencé à redescendre dans la couche de
données — et c'est ainsi que `MainWindow.cpp` s'était épaissi.

`scripts/check_ui_layers.py` vérifie les six règles de cette séparation à chaque *Pull Request*.
Elles sont écrites en `EX-IHM-100` à `EX-IHM-105`. **Une règle qui n'est pas vérifiée n'est pas une
règle : c'est une intention** — le dépôt l'a appris deux fois, avec un défaut de taille d'écran
corrigé *trois* fois et une palette écrite *deux* fois.

## Le module QML

`qt_add_qml_module(JustAnotherDnDGame URI Jadg.Ui ...)` embarque les `.qml` dans la ressource, sous
`/qt/qml` — le préfixe est **explicite** : sans lui le module atterrit là où l'engine ne regarde pas,
et le chargement échoue sur un « type introuvable » que le `qmldir` dément.

Trois pièges de ce module, tous silencieux, tous consignés dans le CMake :

- un **singleton** doit être déclaré (`QT_QML_SINGLETON_TYPE`) : CMake ne déduit pas
  `pragma Singleton`. Non déclaré, le type se charge quand même — mais chaque `import` en construit
  une instance neuve, et le facteur d'agrandissement posé par la fenêtre n'est vu par aucun écran ;
- le fichier d'enregistrement des types est **engendré** et inclut les en-têtes par **nom de base**,
  dans un `__has_include` qui échoue sans bruit : le répertoire doit être dans les chemins
  d'inclusion ;
- `windeployqt` sans `--qmldir` n'embarque **aucun** module QML, et le jeu se lance alors sans
  interface, sans message.

### Éditer un écran sans rien reconstruire

La ressource imposerait une reconstruction à chaque retouche. Un **second `qmldir`** est donc
engendré sous `Source/Ui`, dont les chemins désignent les **sources** ; `JADG_QML_FROM_SOURCE=1` le
place en tête des chemins d'import.

Il est engendré depuis **la même liste** que la ressource : ajouter un écran ne crée pas un second
endroit à synchroniser — ce serait exactement la surcouche que ce lot supprime ailleurs. C'est aussi
lui qui rend `Source/Ui` importable tel quel, donc ouvrable par Qt Design Studio.

## La surface de rendu

`hmi::GameViewportItem` (`QQuickRhiItem`) est le jumeau Qt Quick de `hmi::GameViewport`
(`QRhiWidget`, côté éditeur). Les deux rendent dans une **texture d'appui** que leur hôte compose :
la cible technique ne change pas (`EX-ARCH-050`), seul l'hôte change. Un recouvrement redevient donc
un enfant ordinaire — plus aucun empilement de fenêtres natives.

**La différence qui compte** : `QRhiWidget` peint sur le fil graphique, `QQuickRhiItem` sur le **fil
de rendu**. Toute donnée que la simulation produit doit traverser `synchronize()`, appelée pendant
que le fil graphique est **bloqué** — le seul instant où les deux fils peuvent se parler sans verrou.

C'est pour cela que `hmi::ComposedScene` — liste de primitives **pure et sans GPU**
(`EX-NFR-004`/`005`) — est le bon objet de transfert : le fil graphique la remplit, `synchronize()`
la remet, le fil de rendu la soumet. La frontière que le projet s'était donnée pour tester le rendu
sans GPU sert ici une seconde fois.

`hmi::SceneResources` regroupe ce que les **deux** surfaces créent à l'identique — lot de sprites,
atlas, police bitmap, cache de textures, catalogue de skins. Le regroupement tient moins à
l'économie qu'à l'**ordre de libération** : ce qui tient une texture doit mourir avant elle, et la
texture avant le pipeline qui l'échantillonne. Le désordre ne produit pas une erreur nette mais un
plantage à la fermeture, intermittent selon le pilote.

> Le viewport du jeu **n'affiche encore aucune scène** : `Source/Elements/Levels/` est vide par
> construction depuis le `LOT-01`. La plomberie est établie et vérifiée — le journal nomme le
> backend au démarrage — et la session se branchera quand il y aura une carte à jouer.

## La navigation

`hmi::ScreenRouter` ne **décide rien**. Toute la règle vit dans `hmi::resolveTransition` — table
pure, sans Qt, couverte par ses tests — et le routeur ne fait que l'appeler et diffuser le résultat.
Une transition non déclarée est **refusée**, jamais silencieusement acceptée (`EX-GP-041`) : sans
cette discipline, un `openOptions()` appelé depuis un écran d'où les options ne s'ouvrent pas
produirait un état que la table ne décrit pas, et dont personne ne saurait revenir.

Il publie un **état**, jamais un chemin de fichier. La correspondance entre état et écran vit dans
`Source/Ui/Logic/ScreenStack.qml` — côté développeur, mais du bon côté de la frontière : la
conception peut réorganiser `Screens/` sans qu'une ligne de C++ ne s'en aperçoive.

`--screen=<Nom>` court-circuite le routeur et ouvre un écran directement. C'est un outil de
vérification, pas un chemin de jeu.

## Vérifier une interface sans la regarder

`--screenshot=<chemin>` capture la fenêtre **par Qt lui-même**. Les API de capture de Windows rendent
une image **noire** d'une fenêtre Qt Quick, dessinée par le GPU : seul Qt sait relire son propre
graphe de scène. La vérification visuelle des écrans devient ainsi reproductible, au lieu de dépendre
d'un œil devant l'écran au bon moment.

## Voir aussi

- @ref guide-conception-qds — le mode d'emploi de la **conception** : ce qu'on modifie sans code.
- @ref guide-design-ihm — les jetons et la répartition de l'information dans l'éditeur.
- @ref guide-ecrans — la navigation entre écrans.
- @ref guide-boucle — la boucle et le pas de temps fixe.
- @ref guide-rendu — le pipeline QRhi, partagé par les deux applications.
