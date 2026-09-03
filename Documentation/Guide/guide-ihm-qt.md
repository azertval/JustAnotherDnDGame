# IHM Qt (refonte) — socle applicatif & viewport Direct3D 11 {#guide-ihm-qt}

> Statut : **socle en place** (`LOT-34` → `LOT-38`), **complété** par le système de design et la
> redistribution de l'information de l'éditeur (`LOT-56`, `LOT-57` — voir
> @ref guide-design-ihm, qui traite l'apparence et la répartition des commandes que cette page
> laisse de côté). Le rendu **du jeu** reste Direct3D 11 ; seule l'**interface hors-jeu** passe à
> Qt (voir [spécification IHM](@ref spec-interface-ihm)).

## Pourquoi Qt

L'IHM historique est dessinée quad par quad (police bitmap, aucune primitive de ligne, aucune
bibliothèque d'UI) : illisible pour les liaisons d'interrupteurs, fragile pour la liste des niveaux,
coûteuse à maintenir. La refonte (`LOT-34` → `LOT-39`) porte **toute l'UI hors-jeu** (éditeur, menus,
options) sur **Qt** — fenêtres dockables réglables hors code — tandis que le **rendu in-game reste
Direct3D 11**, embarqué dans un viewport Qt. `Core` n'est pas touché.

## Une seule cible : l'application Qt

Depuis le `LOT-38`, l'IHM « maison » (écrans, widgets, fenêtre Win32) et l'exécutable historique ont
été retirés. `Source/HMI` porte désormais **l'unique application** `JustAnotherDnDGame` (`QApplication` +
`QMainWindow`, widget central = viewport Direct3D 11), avec le code réparti par domaine, au plus près
de l'architecture d'origine :

- `Platform/` — provisionnement bas niveau (répertoire exécutable) ;
- `Input/` — entrées (`hmi::InputState`, `hmi::GamepadPoller`, `*KeyBindings`, pont Qt→`Key`
  `hmi::qtKeyToHmiKey`) ;
- `Graphics/` — rendu sur QRhi (`hmi::SpriteBatch`, `hmi::SpriteRenderer`,
  `hmi::TextureAtlas`, `hmi::Camera2D`, `hmi::DraftRenderer`…) ;
- `Game/` — **session de jeu** (`hmi::GameSession`) et viewport Qt jeu/édition (`hmi::GameViewport`) ;
- `Localization/` — catalogue de traduction ;
- `Interface/` — fenêtre principale, menu, options, remappage (widgets Qt) ;
- `Editor/` — périmètre éditeur de niveau (palette, outils, navigateur, logique pure).

Les **assets Qt déclaratifs** (mises en page `.ui`, ressource `.qrc`) et le **thème** (`.qss`, une
feuille par portée depuis le `LOT-73`) vivent dans `Source/Elements` (`UI/`, `Themes/`) — éditables hors code. La cible reste **optionnelle** côté
build : sans Qt, la configuration CMake n'échoue pas (les tests unitaires, qui compilent les sources
pures directement, se construisent quand même). `windeployqt` copie les DLL Qt à côté de
l'exécutable — **aucune bibliothèque à installer** côté utilisateur. Voir `External/README.md` pour
le provisionnement de Qt (local, CI, release).

## Le viewport : Direct3D 11 dans une fenêtre Qt

`hmi::GameViewport` dérive de `QWindow` et est inséré dans la hiérarchie de widgets par
`QWidget::createWindowContainer`. Le principe :

1. Qt fournit un **`HWND` natif** (`winId()`), passé à `hmi::GraphicsDevice` qui y crée sa swap
   chain et y présente — Direct3D 11 dessine **directement** sur cette surface (aucun `QBackingStore`
   Qt). Le device est construit **paresseusement** au premier `exposeEvent` (handle natif valide).
2. Le redimensionnement (`resizeEvent`) répercute la nouvelle taille **en pixels physiques** (DPI :
   taille logique × `devicePixelRatio`) sur la swap chain.

## La boucle : Qt pilote, le pas fixe est préservé

Qt possède la boucle d'événements (`QApplication::exec`). Le rendu est cadencé par
`QWindow::requestUpdate` : chaque `QEvent::UpdateRequest` déclenche un tick qui **rejoue exactement**
la discipline de la boucle historique (voir @ref guide-boucle) :

1. sonde la manette (`hmi::GamepadPoller`) ;
2. convertit le temps réel écoulé en un nombre entier de **pas fixes** (`core::FixedTimestep`,
   déterminisme `EX-NFR-002`) ;
3. pour **chaque pas** : met à jour la simulation puis appelle `hmi::InputState::beginFrame`
   (fronts avancés par pas consommé, jamais par frame de rendu — `LOT-33`, `EX-CTRL-020`/`021`) ;
4. rend **une fois** par frame réelle, avec le facteur d'**interpolation**
   (`FixedTimestep::interpolationAlpha`, `EX-ARCH-031`) ;
5. reprogramme le tick suivant.

## Les entrées : événements Qt vers l'état partagé

Le viewport traduit les événements Qt vers le **même** `hmi::InputState` que la fenêtre Win32 :

- **Clavier** : `keyPressEvent`/`keyReleaseEvent` → `hmi::Key`. Les lettres/chiffres/espace partagent
  déjà leur valeur entre Qt et Win32 (`Qt::Key_A` == code virtuel `VK 'A'` == `0x41`) ; seules les
  touches spéciales (flèches, Échap, Tab, Maj, Ctrl, F1/F2/F10) passent par une petite table. Les
  répétitions automatiques (`isAutoRepeat`) sont ignorées.
- **Souris/molette** : positions en **pixels physiques**, molette en unités Win32 (`WHEEL_DELTA`).
- **Perte de focus** (`QEvent::FocusOut`) : `hmi::InputState::releaseAll` — aucune touche « collée »
  au retour d'un `Alt+Tab`.
- **Manette** : XInput sondé par `hmi::GamepadPoller`, extrait de la fenêtre Win32 pour être partagé.

## Jouer un niveau : `hmi::GameSession` réutilisée

Toute la logique de jeu d'**un** niveau (scène ECS, physique, mécanismes, blocs, dangers, caméra par
salle, animation, interpolation) est isolée dans `hmi::GameSession` — **sans dépendance d'écran**.
`update()` avance d'un pas fixe et renvoie l'issue (`core::LevelOutcome`, rechargement interne sur
échec) ; `render()` dessine via le `SpriteRenderer`. La session est pilotée par le viewport Qt
(`hmi::GameViewport`) en mode jeu comme en essai depuis l'éditeur : une seule logique, comportement
identique (voir @ref guide-niveaux, @ref guide-physique).

## Éditeur : docks, palette, peinture (LOT-35)

`hmi::MainWindow` est un **poste de travail à panneaux dockables** (`QDockWidget` — Palette,
Outils, Niveaux) autour du viewport central. La **disposition** est persistée hors code
(`QSettings` : restaurée au lancement, sauvée à la fermeture, réinitialisable ; `EX-IHM-011`).

- **Palette** (`hmi::PalettePanel`) : un `QTreeView` alimenté par la taxonomie **pure**
  `hmi::tileTaxonomy` (catégories → sous-groupes → tuiles, tous les `core::TileType` couverts,
  testé sans GPU). Sélectionner une tuile définit le type peint.
- **Édition dans le viewport** : le viewport affiche le brouillon (`core::LevelDraft`) via
  `hmi::DraftRenderer` (scène ECS reconstruite à la demande, rendue par le `SpriteRenderer`), caméra
  cadrant le niveau entier. Le **clic/glisser gauche peint** le type actif à la case survolée
  (`Camera2D::screenToWorld` → `LevelDraft::paintTile`) ; `Ctrl+Z`/`Ctrl+Y` annulent/refont.
- **Enregistrer** (`Ctrl+S`) : `LevelDraft::toLevel` (validation) → `core::LevelWriter` ; un
  brouillon invalide n'écrit rien et l'erreur s'affiche en barre d'état.
- **Essai immédiat** (`P`) : rejoue le brouillon validé via `hmi::GameSession` dans le viewport ;
  `Échap` restitue l'éditeur, brouillon intact — **la même session que l'écran de jeu**, sans
  duplication.

### Gestion des niveaux (LOT-36)

Le panneau **Niveaux** (`hmi::LevelBrowserPanel`) liste les fichiers du dossier `Levels` avec
**recherche** incrémentale, et permet de **créer / renommer / dupliquer / supprimer** un niveau.
Ces opérations délèguent à `hmi::LevelFileOperations` — une couche **pure et testée** (aucune
dépendance Qt/GPU) qui réutilise `hmi::isValidLevelName`, `core::LevelLoader`/`LevelWriter` et
`core::LevelDraft` (aucune règle dupliquée) et renvoie un résultat récupérable (jamais d'exception).
Un double-clic **ouvre** le niveau dans le viewport, précédé d'un **garde-fou** si le brouillon
courant a des modifications non enregistrées (`EX-IHM-020`/`EX-IHM-021`).

### Liens de mécanismes (LOT-37) et unification des menus (LOT-38)

Deux étapes complètent le socle et sont décrites ailleurs pour ne pas être racontées deux fois :

- **Liaison des mécanismes par traits et flèches** (`LOT-37`, `EX-IHM-030`/`EX-IHM-031`) — le geste
  en deux temps (`hmi::resolveLinkClick`) et la géométrie des traits sont traités dans
  @ref guide-editeur.
- **Menu principal, options, jeu et remappage en Qt** (`LOT-38`) — la navigation entre pages
  empilées et leurs signaux sont traités dans @ref guide-ecrans. C'est à cette étape que l'IHM
  « maison » et l'exécutable historique ont été retirés, et que l'internationalisation, la
  journalisation et la manette ont été unifiées sur l'unique application Qt.

## La taille : aucun écran ne contraint la fenêtre (LOT-73)

`QStackedWidget::minimumSizeHint` vaut le **maximum sur toutes ses pages**, y compris celles qui ne
sont pas affichées. Un écran dense fixe donc, à lui seul, le plancher de la fenêtre entière — la
page Options posait le sien pendant qu'on regardait le menu principal. Combiné au facteur
d'agrandissement (@ref guide-design-ihm), ce plancher dépassait la hauteur de l'écran, Windows
refusait la géométrie, et l'interface débordait sous la barre des tâches en rognant son contenu,
sans rien dire.

Le défaut s'est produit **trois fois**. Les deux premières ont été corrigées écran par écran, en
posant une `QScrollArea` dans le fautif ; la troisième a montré ce que cette correction avait de
faux : une règle qu'il faut se rappeler d'appliquer se reperd au premier écran ajouté.

La garantie vit désormais sur le **chemin d'ajout commun** (`EX-IHM-080`).
\ref hmi::ScreenPageHost "ScreenPageHost" enveloppe **toute** page passée à
`MainWindow::addScreenPage`, et combine deux mécanismes :

- une `QScrollArea` redimensionnable, qui fait **défiler** ce qui ne tient pas au lieu de le rogner ;
- une politique de taille `Ignored`, qui fait contribuer **zéro** au minimum de la fenêtre. La
  `QScrollArea` seule ne suffirait pas : elle propage encore un plancher, et n'empêcherait pas un
  appelant de poser une taille minimale sur la page.

Le **viewport** en est exclu : surface de rendu QRhi, il remplit sa page sans jamais défiler, et son
minimum (320×240) tient sur tout écran.

Deux garde-fous complètent l'invariant : la géométrie restaurée d'une session précédente est ramenée
dans la zone utile (taille d'abord, position ensuite — déplacer une fenêtre trop grande ne la ferait
pas tenir), et `MainWindow` **compare** au démarrage la taille minimale imposée par les écrans à la
zone disponible, en journalisant un avertissement si elle la dépasse.

> `UnitTests` ne lie que `Qt6::Gui`, **pas `Qt6::Widgets`** : aucun test ne peut instancier un widget
> ni lire un `minimumSizeHint`. L'invariant se teste par ses fonctions **pures** (le facteur borné) et
> se constate à l'IHM.

## Ce que cette page ne couvre pas

L'**apparence** de tout ce qui précède (style, palette, thème clair/sombre, typographie, icônes,
netteté à l'échelle d'affichage) et la **répartition** des commandes et de l'état dans l'éditeur
(barre d'état permanente, regroupement des panneaux, unicité des commandes) sont l'objet d'un lot
à part entière : @ref guide-design-ihm. L'atelier pixel art, qui vit dans ce même châssis, a sa
page : @ref guide-atelier-pixel-art.

## Voir aussi
- @ref guide-design-ihm — jetons de design, thème, actions uniques, barre d'état structurée.
- @ref guide-atelier-pixel-art — l'atelier pixel art intégré à l'éditeur.
- [Spécification IHM](@ref spec-interface-ihm) — le *quoi/pourquoi* de la refonte (`EX-IHM-*`).
- @ref guide-boucle — la boucle et le pas de temps fixe (repris à l'identique côté Qt).
- @ref guide-entrees — la traduction des entrées en actions logiques (partagée).
- @ref guide-rendu — le rendu Direct3D 11 embarqué (pipeline inchangé).
