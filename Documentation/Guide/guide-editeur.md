# Éditeur de niveaux {#guide-editeur}

> **Binaire séparé depuis le `LOT-86`** (`LevelEditor`). Il n'héberge plus les écrans du jeu, qui
> sont passés en Qt Quick dans `JustAnotherDnDGame` : sa fenêtre s'ouvre directement sur son espace
> de travail, et son widget central est de nouveau le viewport. Ce qui suit décrit l'outil, dont le
> fonctionnement est inchangé.


Cette page explique comment le mode éditeur transforme le personnage jouable, la caméra et le rendu
déjà vus dans les pages précédentes en un **outil de création de contenu**, sans écrire un nouveau
moteur de rendu. Le **modèle d'édition** (mutabilité, validation, annuler/refaire, sérialisation)
vit dans `Source/Core/Levels/LevelDraft.*`/`LevelWriter.*` ; l'**interaction** (peinture souris,
outils, essai, garde-fous) vit dans le viewport Qt `Source/HMI/Game/GameViewport.*`. L'habillage de
l'IHM Qt lui-même — fenêtre, docks (Palette, Outils, Niveaux), arbre de palette, navigateur de
fichiers — est décrit dans @ref guide-ihm-qt et @ref guide-ecrans ; cette page se concentre sur ce
qui est **propre à l'édition**.

## Le problème : éditer un niveau sans (re)coder le moteur

@ref guide-niveaux a montré que `core::Level` est **immuable** une fois construit : ses champs sont
posés au constructeur, sans mutateur. C'est un choix délibéré — un niveau **en cours de jeu** ne
doit jamais changer de forme sous les pieds du joueur. Mais un **éditeur**, par nature, fait
exactement l'inverse : poser une tuile, la retirer, déplacer l'entrée, doivent être des opérations
courantes, répétées des dizaines de fois par minute. Réutiliser `Level` tel quel pour l'édition
obligerait soit à le rendre mutable (fragilisant l'invariant « un niveau chargé est valide », dont
dépend tout le reste du moteur), soit à dupliquer sa logique dans un second type — exactement ce
que `EX-EDIT-010` interdit (« aucune duplication de la logique de niveau »).

La solution retenue : un type **distinct**, `core::LevelDraft`, qui porte toute la mutabilité, et
qui ne redevient un `Level` **validé** qu'au moment décisif (l'enregistrement ou l'essai), en
repassant par le chemin de validation déjà existant plutôt que d'en écrire un second.

## \ref core::LevelDraft "core::LevelDraft" : un niveau qu'on peut défaire

`LevelDraft` reprend les mêmes données qu'un `Level` (nom, grille de tuiles, entrée, sortie,
mécanismes, budgets) mais expose des **mutateurs** : `paintTile`, `setEntry`, `setExit`,
`linkMechanism`, `unlinkMechanism`, `resize`. Deux invariants structurent tout le reste de la
page :

- **La grille de tuiles reste l'unique source de vérité.** Exactement comme pour `Level`
  (@ref guide-niveaux), une case `Entry`/`Exit`/`Switch`/`Door` dans `core::TileMap` **est** la
  donnée — `entry()`/`exit()` ne sont que des accès en cache, toujours resynchronisés par les
  mutateurs. Peindre autre chose par-dessus une entrée l'invalide automatiquement (et retire les
  liaisons de mécanismes qui la référençaient) : il ne peut jamais exister d'état où la grille dit
  une chose et le cache une autre.
- **`LevelDraft::toLevel()` ne réimplémente aucune règle de validation.** Plutôt que de vérifier
  « y a-t-il une entrée ? y a-t-il une sortie ? » une seconde fois, `toLevel()` **sérialise** le
  brouillon en JSON (via `core::LevelWriter`, ci-dessous) puis le fait passer par
  `core::LevelLoader::loadFromString` — le **même** chemin qu'un fichier chargé depuis le disque.
  Un brouillon incomplet produit donc exactement le même message d'erreur qu'un fichier de niveau
  mal formé (`EX-LVL-004`), sans qu'une seule règle de `LevelLoader` n'ait été dupliquée.

### Mécanismes : qui a le droit de se lier à qui

`linkMechanism(switchPosition, doorPosition)` exige (par assertion, @ref guide-journalisation) que
les deux cases portent déjà respectivement un `Switch` et une `Door` — lier ne **peint** rien, ça
n'associe que deux tuiles déjà posées. Relier une porte déjà liée **remplace** la liaison
précédente (une porte n'a qu'un seul interrupteur), alors qu'un même interrupteur peut ouvrir
plusieurs portes — cette asymétrie découle directement du format de fichier (@ref guide-niveaux) :
chaque tuile `door` porte un unique champ `opensWith`, mais plusieurs portes peuvent référencer le
même `switch.id`.

### Lier des mécanismes dans l'éditeur (`LOT-37`, `EX-IHM-030`/`EX-IHM-031`)

Avant le `LOT-37`, une paire déclencheur→cible n'était signalée que par une **teinte de case**
partagée (`LINK_TINTS`) : au-delà de quelques liens, illisible (impossible de savoir quel
interrupteur ouvre quelle porte sans comparer des couleurs). Deux ajouts corrigent ce point :

- **Rendu par flèches explicites** (`hmi::DraftRenderer::drawLinks`) : chaque `core::Mechanism`
  (interrupteur/plaque → porte) et `core::DangerLink` (déclencheur → danger commuté) est dessiné
  comme une flèche allant du centre de la case déclencheur au centre de la case cible, par-dessus
  la grille de repère. Quand un déclencheur a plusieurs cibles, les flèches partent d'une **base
  commune** (le centre du déclencheur) et s'écartent en éventail vers chaque cible — plus lisible
  qu'un empilement de traits parallèles décalés des deux côtés. La géométrie (centres de case,
  éventail anti-superposition, pointe de flèche) est calculée par `hmi::LinkGeometry`
  (`Source/HMI/Editor/LinkGeometry.h`), pure et testée sans GPU (`EX-NFR-010`) ; le tracé
  lui-même réutilise la primitive de segment orienté ajoutée à `hmi::SpriteBatch` (voir
  @ref guide-rendu).
- **Outil « Lien »** (`hmi::EditorTool::Link`, barre d'outils) : cliquer un déclencheur
  (interrupteur/plaque) passe en **attente de cible** (case signalée par un voile jaune, trait
  provisoire vers la souris) ; cliquer une cible (porte/danger commuté) **crée** la liaison
  (`LevelDraft::linkMechanism`) ; refaire la même paire la **supprime** (`unlinkMechanism`,
  bascule conservée). **`Échap`** annule une attente en cours (aucune liaison créée, brouillon
  inchangé). La machine à état du geste (`hmi::resolveLinkClick`,
  `Source/HMI/Editor/LinkGesture.h`) est pure et testée indépendamment de Qt/GPU.
- **Panneau « Liens »** (`hmi::LinkPanel`, dock) : liste toutes les liaisons du brouillon courant
  (type, position du déclencheur, position de la cible) ; sélectionner une ligne met en
  surbrillance la flèche correspondante dans le viewport ; le bouton « Supprimer » retire la
  liaison sélectionnée. C'est une **vue** du modèle (`hmi::buildLinkRows(draft)`), sans aucun état
  dupliqué — elle se resynchronise (`refresh`) à chaque mutation du brouillon
  (`GameViewport::draftChanged`), y compris quand peindre par-dessus un déclencheur ou une cible en
  retire la liaison.

Le modèle de liaison lui-même (`LevelDraft::linkMechanism`/`unlinkMechanism`, résolution par
position) est **inchangé** par ce lot : seule sa présentation/édition dans l'IHM Qt change
(`EX-EDIT-010`, pas de duplication de la logique de niveau).

## \ref core::LevelWriter "core::LevelWriter" : l'inverse du chargement, avec un piège

Écrire un niveau est presque l'inverse exact de `LevelLoader::loadFromString` (@ref guide-niveaux)
— parcourir la grille ligne par ligne, émettre un objet JSON par tuile non vide. Le piège tient
aux **identifiants d'interrupteurs** : le fichier source les porte (`"id": "s1"`), mais ni `Level`
ni `LevelDraft` ne les conservent après chargement — seules les **positions** sont retenues
(`core::Mechanism{switchPosition, doorPosition}`). `LevelWriter` doit donc **régénérer** des
identifiants à l'écriture, de façon déterministe (balayage de la grille, `"s0"`, `"s1"`, …). Cela
n'a aucune incidence sur le niveau rechargé : le nom exact d'un identifiant d'interrupteur n'est
qu'un détail du fichier texte, jamais une donnée de jeu — un interrupteur **non lié** à une porte
obtient tout de même un identifiant (le format l'exige), simplement absent de toute liaison.

Cette sérialisation sert `EX-EDIT-011` : sérialiser puis recharger un niveau produit un niveau
**équivalent**, jamais un niveau différent — la propriété qui rend `toLevel()` fiable.

## Peindre, c'est convertir un pixel en case

Le viewport d'édition (`hmi::GameViewport` en mode édition) réutilise **exactement** l'infrastructure
de rendu déjà vue en @ref guide-rendu (`SpriteBatch`, `TextureAtlas`, `Camera2D`) via
`hmi::DraftRenderer` — aucun nouveau pipeline graphique n'existe pour l'éditeur. La seule nouveauté
conceptuelle est l'**interaction** : convertir une position souris (`QMouseEvent`, en pixels
physiques) en case de grille, en composant deux briques déjà connues, `Camera2D::screenToWorld`
(@ref guide-rendu) puis `std::floor` (une position monde `4.7` désigne la case `4`, pas la case
`5`). C'est le rôle de `GameViewport::cellAt` ; `paintAt` applique ensuite le type actif à la case
survolée via `core::LevelDraft::paintTile`.

### La palette et les outils : des panneaux Qt séparés du canevas

Là où l'ancien éditeur « maison » dessinait palette et barre d'outils **par-dessus** le canevas — au
prix d'une logique de priorité de clic pour ne pas peindre une case cachée sous un panneau — l'IHM
Qt les héberge dans des **panneaux dockables distincts** (`QDockWidget`), physiquement hors du
viewport. Le problème d'occlusion disparaît par construction :

- la **Palette** (`hmi::PalettePanel`, un `QTreeView` alimenté par la taxonomie pure
  `hmi::tileTaxonomy`) émet le type sélectionné, que `MainWindow` relaie au viewport via
  `GameViewport::setActiveTile` ;
- l'**outil actif** est choisi depuis la barre d'outils à icônes (`hmi::EditorActions`, `LOT-56`) et
  relayé via `GameViewport::setTool`. L'ancien panneau de boutons radio a disparu. Le panneau
  **Décors**, qui portait les options de l'outil du même nom, a été remplacé au `LOT-69` par le
  panneau **Plans** (`hmi::PlanesPanel`) : l'habillage d'un niveau ne se pose plus, il se peint
  (voir « Mode création » ci-dessous).

Depuis le `LOT-68`, cette barre d'outils ne porte **que** la sélection d'outil et quatre commandes
à usage continu — enregistrer, annuler, refaire, essayer. Tout le reste vit dans la barre de menus,
organisée par nature d'action (Fichier, Édition, Niveau, Affichage, Atelier, Aide). Et l'éditeur se
présente en **deux espaces de travail exclusifs**, édition de niveau ou atelier pixel art, chacun
n'affichant que ses propres panneaux : le détail de ces arbitrages est en @ref guide-design-ihm.

Le viewport ne reçoit donc que des **clics de grille** ; il n'a plus à arbitrer entre « peindre » et
« cliquer un panneau ». Détail de ces widgets Qt : @ref guide-ihm-qt.

### L'outil « Parcours » : dessiner la route d'un élément mobile

`hmi::EditorTool::Path` (`LOT-67`) donne au *level designer* la maîtrise des trajectoires, qui
imposaient jusque-là d'ouvrir le JSON à la main. Le geste tient en deux temps :

1. **cliquer la tuile de départ** d'une plateforme mobile ou d'un danger mobile la sélectionne — la
   tuile elle-même sert de zone de sélection, il n'y a pas de poignée dessus ;
2. **glisser une poignée** modifie la route : une poignée de **point** déplace ce point (clic droit
   le retire), une poignée de **milieu de segment** en insère un nouveau et le déplace du même
   geste — le patron des éditeurs de courbe, qui évite un mode « ajouter » séparé.

Une route encore **vide** est le cas qu'on rencontre en premier, et il n'avait pas de réponse
jusqu'au `LOT-68` : sans point à déplacer ni segment à couper, une plateforme fraîchement peinte
n'offrait rien à saisir, et son parcours ne pouvait jamais être *commencé* — seulement modifié s'il
existait déjà. Elle expose désormais une poignée d'**amorce** sur sa tuile de départ, dont le
glisser crée le premier point de passage.

> L'amorce ne déplace pas le départ : on le déplace en **repeignant** la tuile. La règle « le point
> de départ n'a pas de poignée » reste donc entière — lui en donner une créerait deux façons
> contradictoires de faire la même chose.

Les poignées gardent une taille **écran** constante (`hmi::pathHandleLayout`) : leur taille en
unités monde suit l'échelle de la caméra, sans quoi elles deviendraient inutilisables aux extrêmes
de zoom. Un geste complet ne produit qu'**une** action, donc un seul pas d'annulation.

Les réglages qui ne se dessinent pas — vitesse, déphasage, mode de parcours, période d'un danger
temporisé, règles du tableau — vivent dans le panneau **Propriétés** (`hmi::PropertiesPanel`).

### Trois outils, une même grille : \ref hmi::EditorTool "EditorTool"

Au-delà du pinceau (peindre case par case), l'éditeur propose **Rectangle** (glisser définit un
rectangle, rempli du type sélectionné au relâchement) et **Sélection** (glisser mémorise une zone,
copier/coller la déplacent ailleurs). Les trois sont un simple `enum class hmi::EditorTool` ; le
viewport en tient l'état courant (`_tool`) et la mécanique de glisser (`_painting` pour le pinceau,
`_dragging` + `_dragStart`/`_dragCurrent` pour Rectangle/Sélection, `applyRectangle`,
`copySelection`/`pasteClipboard`). Changer d'outil **pendant** un glisser en cours l'annule plutôt
que de l'appliquer à moitié — un choix délibéré pour qu'un changement d'avis ne produise jamais de
mutation partielle et surprenante.

### Peindre par lot sans dupliquer la logique de peinture : \ref core::LevelDraft::paintRegion "LevelDraft::paintRegion"

Remplissage rectangulaire et collage partagent le même besoin : appliquer un **bloc** de types de
tuiles en une seule fois, plutôt qu'une case. Une implémentation naïve dupliquerait la sémantique
de `paintTile` (déplacement d'entrée/sortie, nettoyage des liaisons) pour chaque case du bloc.
`LevelDraft::paintRegion` évite cela en factorisant cette sémantique dans une méthode privée sans
`pushUndo()` (`paintTileInternal`), appelée une fois par case du bloc ; `paintTile` devient
elle-même un appel à `paintRegion` avec un bloc `1×1`. Résultat : remplir un rectangle de 50 cases
ou peindre une seule case suivent **exactement** le même chemin de code, et ne poussent
**qu'un seul** instantané sur la pile d'annulation pour toute l'opération — cohérent avec le
principe « un geste = une mutation undoable ».

Le **copier** n'a besoin d'aucun ajout à `Core` : le viewport lit directement
`LevelDraft::tileMap()` (déjà publique) pour construire un presse-papiers local
(`std::vector<std::vector<TileType>>`, `_clipboard`). Seul le **coller** repasse par `paintRegion`.

## Annuler/refaire : pourquoi des instantanés complets

Chaque mutateur de `LevelDraft` empile, **avant** de s'appliquer, une copie complète de l'état du
brouillon (`snapshot()`) sur une pile d'annulation ; `undo()` la restitue et bascule l'état courant
sur la pile de refaire, symétriquement pour `redo()` (`Ctrl+Z`/`Ctrl+Y` dans le viewport). Une
nouvelle mutation après un `undo()` vide la pile de refaire — l'historique reste **linéaire**,
jamais arborescent, comme dans la plupart des éditeurs grand public.

Le choix d'un **instantané complet** plutôt que d'un enregistrement différentiel (« quelle case a
changé ») est délibéré : un différentiel serait plus économe en mémoire, mais demande une logique
d'inversion propre à **chaque** type de mutation (annuler un redimensionnement n'est pas l'inverse
symétrique de le refaire, par exemple). Les niveaux de ce projet restent petits (une grille de
tuiles, quelques dizaines à quelques centaines de cases) : copier l'état entier à chaque étape est
largement assez rapide, et la garantie de correction (« l'état restitué est identique à l'octet
près ») est bien plus simple à établir qu'avec des deltas.

## Essai immédiat : jouer sans quitter l'éditeur

Appuyer sur `P` doit lancer une **vraie** partie sur le niveau en cours d'édition — avec le
personnage, la physique, les mécanismes (@ref guide-physique, @ref guide-niveaux) — puis, à
`Échap`, **revenir exactement où l'édition en était**. Le viewport fait vivre la session de jeu
**à l'intérieur même** de son mode édition (`GameViewport::startPlaytest`/`stopPlaytest`) : il
embarque une `hmi::GameSession` optionnelle ; `P`, sur un brouillon **valide**, l'instancie ; tant
qu'elle existe, le tick lui **délègue** entièrement la frame ; à `Échap` (ou niveau terminé), le
viewport la détruit et reprend la main — le `LevelDraft` et son historique n'ont, à aucun moment,
été touchés.

**Transmettre le niveau, pas un chemin de fichier.** `hmi::GameSession` accepte un niveau **déjà en
mémoire** (`loadLevel(core::Level)`) : la construction de scène (monde ECS, grille de collision des
mécanismes, personnage à l'entrée) est factorisée là, partagée par le chargement de fichier et
l'essai. `startPlaytest` passe directement `draft.toLevel()`, sans écrire ni lire aucun fichier
temporaire. Bénéfice secondaire : un niveau qui **échoue** pendant l'essai (danger, chute) se
relance via le `Level` déjà tenu en mémoire, sans relire le fichier source.

## Enregistrer : valider avant d'écrire, jamais l'inverse

`Ctrl+S` (`GameViewport::save`) appelle `draft.toLevel()` en premier. Si la validation échoue,
**aucun fichier n'est écrit** — le brouillon invalide reste en mémoire, et un message d'erreur
traduit en langage compréhensible par un non-développeur (« il manque une sortie », plutôt que le
texte technique du validateur) est émis via le signal `statusMessage` (barre de statut de la
fenêtre). Si elle réussit, `LevelWriter::saveToFile` écrit le JSON dans le dossier `Levels` de
l'application — le **même** dossier que `core::LevelLoader` lit au démarrage du jeu (@ref
guide-niveaux), garantissant qu'un niveau enregistré est immédiatement chargeable.

**Traduire l'erreur sans deviner le texte.** `LevelLoadResult` porte un `LevelValidationError`
(énuméré, `ParseError`/`InvalidEntryCount`/`UnresolvedMechanism`/…), rempli par `LevelLoader` en
plus du message technique ; l'IHM bascule sur ce **code**, jamais sur des sous-chaînes du message
technique (qu'une reformulation aurait cassées silencieusement).

## Garde-fous contre la perte de travail

**Ouvrir en écrasant un travail non enregistré.** Le viewport suit les modifications non
enregistrées (`_dirty`, mis à jour par toute mutation, remis à `false` après un enregistrement
réussi) : ouvrir un autre niveau ou revenir au menu alors que `_dirty` est vrai pose une
confirmation avant d'abandonner le brouillon courant.

**Redimensionner en perdant du contenu.** Détecter un redimensionnement destructeur ne duplique
aucune règle de `resize` : une requête pure, `LevelDraft::wouldResizeDropContent(largeur, hauteur)`
(exposée par le viewport via `wouldResizeDrop`), inspecte les positions actuelles de l'entrée, de la
sortie et des mécanismes contre les nouvelles bornes, **sans rien modifier** — le viewport
l'interroge avant d'appeler `resizeLevel`, et pose une confirmation si du contenu serait perdu.
Réimplémenter cette détection côté `HMI` aurait demandé de rederiver la même logique de troncature
que `resize` porte déjà ; l'exposer comme requête pure évite cette duplication (`EX-EDIT-010`).

## Cadrer un niveau plus grand que la fenêtre

Le cadrage automatique de la caméra de l'éditeur et celui du jeu (`hmi::GameSession`) partagent une
même correction, factorisée en fonction **pure** dans `Camera2D::fitZoom(largeurDisponible,
hauteurDisponible, largeurContenu, hauteurContenu, marge)` :

```cpp
const float rawZoom = std::min(fitX, fitY) * margin;
return rawZoom >= 1.0f ? std::floor(rawZoom) : rawZoom;
```

Zoom **entier** (netteté pixel art, `EX-ARCH-022`) tant que l'ajustement brut reste `≥ 1` —
comportement inchangé pour tout niveau livré à ce jour ; zoom **fractionnaire** (la valeur brute,
sans `floor`) uniquement lorsque c'est strictement nécessaire pour qu'un niveau plus grand tienne
malgré tout. `EX-ARCH-022` dit déjà « zoom **de préférence** en facteurs entiers » — cette
correction n'en change pas la politique par défaut, elle active l'exception que le mot « préférence »
anticipait. La placer dans `Camera2D` (@ref guide-rendu) plutôt que de la dupliquer dans l'éditeur
et le jeu la rend en prime testable sans GPU (`Camera2D` est déjà compilé dans `UnitTests`).

**Caméra manuelle et grille de repère.** En mode édition, molette (zoom) et glisser prennent le
relais du cadrage automatique (`updateEditCamera`) ; `F10` bascule un **quadrillage de repère** —
fines lignes à chaque bord de case, plus, aux **frontières de salles** (\ref hmi::RoomGrid
"hmi::RoomGrid", `LOT-32`), un quadrillage plus épais pour aligner les couloirs inter-salles. La
capture de `F10` (et de toutes les touches d'édition) passe désormais par `keyPressEvent` de Qt et la
table `hmi::qtKeyToHmiKey` (@ref guide-entrees) — l'ancienne fenêtre Win32 et ses messages
`WM_SYSKEYDOWN` n'existent plus.

## Le mode « définition des textures » : auditer les calques sans les confondre avec `F8` (`LOT-51`)

`F8` (@ref guide-rendu, `LOT-41`) **compose** le rendu final — surcharge par instance (`LOT-45`) >
skin de type (`LOT-42`) > damier de repli — exactement ce que le joueur voit. À partir de six
calques empilés (fond, plans d'arrière-plan, skin des tuiles, objets interactifs, personnage,
plans de premier plan), la seule bascule Physique/Texture ne dit plus **d'où** vient ce qui est
affiché à l'écran.

L'onglet **Calques** du panneau **Textures** répond à cette question précise : il **décompose**
plutôt que de composer. Une case à cocher par calque, dans l'**ordre de dessin** (`hmi::RenderLayer`,
`EX-REN-014`) — Fond, Plans d'arrière-plan, Ombres, Skin des tuiles, Objets interactifs, Personnage,
Plans de premier plan — chacune indépendamment activable/désactivable, plus une case « Physique
seul » (la même bascule que `F8`, vue sous cet angle) et un bouton « Tout afficher ». C'est un mode
d'**inspection de l'éditeur**, sans persistance entre deux sessions et **sans aucun effet sur le
jeu** — masquer un calque ici ne le masque jamais en jeu ni en essai (`hmi::GameSession` ne connaît
pas cet onglet).

**Ce qui distingue ce mode d'un simple masquage** : quand un seul calque de contenu est coché,
l'affichage **isole** — il ne retombe **jamais** sur un niveau de priorité inférieur. Cocher
uniquement « Objets interactifs » ne montre que les cases portant une surcharge par instance
assignée, sans repli sur le skin de leur type ni sur le damier ; cocher uniquement « Skin des
tuiles » ne montre que les types dont un skin est chargé dans le jeu courant, une case vide révélant
directement **quels types n'ont pas encore de skin** — le diagnostic le plus utile du programme
d'habillage (`LOT-40` → `LOT-55`). Combiner plusieurs cases répond à des questions d'audit
différentes (« le plan de premier plan cache-t-il quelque chose d'important au-dessus du
personnage ? » suppose deux calques visibles à la fois, pas un seul).

Les libellés « Aperçu » de cet onglet ne doivent jamais se confondre avec « Jeu » (`F8`) : les deux
réutilisent le même résolveur de priorité (`hmi::resolveTileAppearance`), avec deux règles
d'affichage différentes plutôt que deux résolveurs — c'est ce qui garantit que l'audit ne divergera
jamais silencieusement du rendu réel.

## Le mode création : peindre le décor du niveau (`LOT-69`)

Le `LOT-68` avait donné à l'éditeur deux **espaces de travail exclusifs** — édition de niveau,
atelier pixel art. Le mode création est le **troisième** (menu *Affichage > Espace de travail*), et
c'est cette structure, plutôt qu'un énième bouton, qui le rend praticable : y entrer masque les
panneaux qui n'ont rien à y faire et fait apparaître ceux du canevas.

### Le panneau « Plans »

Il occupe la place laissée par l'ancien panneau « Décors ». Il liste les plans du niveau **dans
l'ordre**, chacun avec son fichier, sa densité, sa profondeur, sa parallaxe par axe, son opacité, un
œil de visibilité et un bouton « isoler ».

L'ordre est significatif : il décide de la superposition à profondeur égale. D'où des boutons
**Monter/Descendre** plutôt qu'un tri de colonne, qui laisserait croire que l'ordre n'est qu'un
confort d'affichage.

Deux détails qui se paient à l'usage s'ils sont mal faits :

- Les champs numériques émettent sur `editingFinished`, **jamais** sur `valueChanged` : taper
  « 0.75 » dans un facteur de parallaxe produirait sinon quatre mutations, donc quatre pas
  d'annulation pour un seul geste (leçon du `LOT-67`).
- La **visibilité** d'un plan (masquage, isolement) n'est pas une propriété du niveau : elle vit
  dans `hmi::PlaneVisibility`, n'est pas enregistrée, et n'empile aucun pas d'annulation. C'est une
  aide de travail, pas une donnée.

La case « parallaxe active » du niveau est **grisée**, avec son explication, quand le cadrage est
*niveau entier* : la caméra n'y défile pas, un facteur ne produirait qu'un désalignement constant.

### Cycle de vie des fichiers

« Ajouter » crée un PNG **entièrement transparent** aux dimensions exactes qu'impose la densité,
nommé d'après le niveau et rendu unique par un suffixe numérique croissant — `foret.png`,
`foret-2.png`, … plutôt qu'un identifiant aléatoire, pour qu'un dossier de plans reste lisible à
l'œil. Changer la densité **rééchantillonne** l'image et la réécrit : le fichier et le format ne
peuvent jamais dire deux choses différentes. Le rééchantillonnage est un ratio **entier**, sans
interpolation — un filtrage introduirait des teintes que l'artiste n'a pas posées.

> **Supprimer un plan ne supprime jamais son fichier.** Retirer une entrée annule proprement côté
> brouillon (`Ctrl+Z` la restaure), mais un fichier effacé, lui, ne revient pas. Le choix retenu est
> donc de ne retirer que l'entrée : un PNG orphelin dans `Levels/Plans/` est moins grave qu'un
> dessin perdu. À nettoyer à la main si le dossier s'encombre.

### Peindre

Le bouton « Peindre » bascule dans l'espace Plans avec le plan sélectionné chargé. Le canevas est
celui de l'atelier pixel art (`hmi::PixelCanvas`) : mêmes outils, même palette, même historique
visuel, même copier/coller. Rien n'a été réécrit — c'est d'ailleurs vérifié, les tests de
`hmi::PixelOperations` et `hmi::PixelCanvasGeometry` passent **sans retouche**.

Deux règles à connaître :

- **`Ctrl+S` enregistre le PNG**, pas le niveau. Le canevas est « modifié » indépendamment du
  brouillon — deux notions distinctes depuis le `LOT-54` — et chacune a son garde-fou de perte de
  travail. L'enregistrement du niveau, lui, écrit toujours le JSON.
- **Un coup de pinceau dans un plan n'apparaît jamais dans l'historique d'édition du niveau**, ni
  réciproquement. Les deux piles sont distinctes par construction, et changer de sujet dans le
  canevas repart d'une pile vierge.

La barre d'état annonce, en plus de ce qu'elle affiche déjà pour un asset : la résolution du plan,
sa densité et son **poids mémoire** — le chiffre que le budget du dépôt plafonne (`EX-NFR-043`).

### La référence : un repère géométrique, pas un aperçu

Sous l'image éditée, l'éditeur dessine une **pelure d'oignon** des tuiles du niveau (une couleur
plate par type, reprise du mode Physique) et les plans qui passent **derrière** celui qu'on peint,
aplatis ; au-dessus, ceux qui passent **devant**. Une grille de **tuiles**, distincte de la grille
de pixels, permet de viser une case.

Ce que la référence n'est **pas**, et ne sera pas : ni raccords automatiques, ni skins, ni objets
animés. Elle dit *où* sont les choses, pas *à quoi elles ressembleront*. L'aperçu fidèle reste
l'essai (`P`). Le dire ici évite une attente déçue.

Le zoom descend **sous** le 1:1 — un plan fait la taille du niveau entier, le voir en entier est le
cas normal, pas l'exception.

## Gérer ses fichiers de niveaux

Le panneau **Niveaux** (`hmi::LevelBrowserPanel`) liste les fichiers `.json` du dossier `Levels` et
offre **créer / renommer / dupliquer / supprimer**. Le nommage passe par un `QInputDialog` validé
par `hmi::isValidLevelName` — refus d'un nom vide ou contenant un caractère interdit par le système
de fichiers Windows (liste **noire** minimale ; les accents restent autorisés). Les opérations
fichiers elles-mêmes sont une couche **pure et testée**, `hmi::LevelFileOperations` (créer/renommer/
dupliquer/supprimer, sans dépendance Qt) — la même séparation « logique pure / accès disque » que
`LevelLoader`/`Core` appliquent à la validation. Détail du panneau : @ref guide-ihm-qt.

## Voir aussi
- `core::LevelDraft` (dont `paintRegion`, `wouldResizeDropContent`), `core::LevelWriter`,
  `core::LevelLoader` (dont `LevelValidationError`), `core::Mechanism`.
- `hmi::GameViewport`, `hmi::EditorTool`, `hmi::PalettePanel`, `hmi::tileTaxonomy`,
  `hmi::PlanesPanel`, `hmi::LevelBrowserPanel`, `hmi::LevelFileOperations`, `hmi::isValidLevelName`.
- `hmi::Camera2D::fitZoom` — le cadrage partagé par l'éditeur et le jeu.
- `hmi::LayerVisibility`, `hmi::resolveTileAppearance` — le mode d'inspection « définition des
  textures » (`LOT-51`) et le résolveur unique qu'il réutilise avec `F8`.
- @ref guide-ihm-qt — l'IHM Qt : fenêtre, docks, arbre de palette, navigateur de fichiers, viewport.
- @ref guide-atelier-pixel-art — l'atelier pixel art : modifier les assets sans quitter l'éditeur,
  avec un historique **distinct** de celui du brouillon décrit ici.
- @ref guide-design-ihm — la barre d'état permanente, le regroupement des panneaux et l'unicité des
  commandes de l'éditeur (`LOT-56`, `LOT-57`).
- @ref guide-niveaux — le modèle de niveau immuable, la validation et le format JSON réutilisés sans
  duplication.
- @ref guide-rendu — `SpriteBatch`/`Camera2D`/`TextureAtlas`, réutilisés tels quels par
  `hmi::DraftRenderer` pour dessiner la grille.
- @ref guide-physique — la simulation rejouée telle quelle pendant l'essai immédiat.
