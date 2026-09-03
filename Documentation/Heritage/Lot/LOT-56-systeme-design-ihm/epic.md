# LOT-56 — Système de design de l'IHM Qt {#lot-56}

> Statut : **fait**. Prérequis : [LOT-38](@ref lot-38) (unification sur Qt, retrait de l'UI
> « maison »).

## Objectif
Donner à l'interface hors-jeu une **apparence tenue et cohérente**, en prenant enfin la main sur la
couche de style de Qt — que le projet n'a jamais configurée.

La refonte `LOT-34` → `LOT-39` a livré une interface **fonctionnelle** : fenêtre à panneaux dockables,
menus, options, remappage, éditeur. Son apparence, en revanche, n'a jamais fait l'objet d'un lot. Elle
est aujourd'hui celle que Qt produit par défaut, et le seul thème existant (`Source/Elements/Themes/
theme.qss`, une soixantaine de lignes) ne couvre que le menu principal et la page Options.

La cause n'est pas que ce thème serait trop court : c'est que **l'application ne choisit jamais son
style Qt**. Faute d'appel à `QApplication::setStyle`, elle s'exécute sur le style **natif** de la
plate-forme, qui dessine la majorité des contrôles via l'API du système et **ignore délibérément une
large part de toute feuille de style posée par-dessus**. C'est précisément pourquoi le thème a dû être
*restreint* par `objectName` au menu et aux options plutôt qu'étendu : l'étendre ne donnait pas un
résultat homogène. Écrire davantage de feuille de style sans traiter cette cause ne changerait rien.

Le constat, vérifié sur `Source/HMI` : aucun appel à `QApplication::setStyle`, aucune `QPalette`,
aucune `QToolBar`, aucun `QActionGroup`, aucun `QProxyStyle`, aucun `QFontDatabase`, aucun recours à
`QStyleHints`. `QIcon` n'apparaît que comme conteneur de vignettes — il n'existe aucun jeu d'icônes.
Trois conséquences directes :

- **Deux apparences dans la même fenêtre** : écrans thématisés d'un côté, panneaux de l'éditeur au
  rendu natif de l'autre. À ne pas confondre avec la distinction **délibérée** que ce lot introduit
  entre l'identité du jeu et le châssis d'édition (cf. décisions de cadrage) : l'une est un défaut
  subi, l'autre un choix assumé et cohérent de part et d'autre.
- **La typographie a deux sources de vérité** : tailles de police et marges figées dans les fichiers
  `.ui`, famille et taille redéfinies dans la feuille de style — qui impose par ailleurs une police
  disponible **sur Windows uniquement**, alors que `Source/Elements/Assets/Fonts/` existe et ne
  contient qu'un fichier d'explication.
- **Les vignettes sont floues dès que l'affichage est mis à l'échelle** : `devicePixelRatio` n'est pris
  en compte que par la surface Direct3D 11 (*GameViewport*), jamais par les trois chemins de vignettes
  Qt (*PalettePanel*, *AssetThumbnailView*, *TexturePanel*). Sur un écran à 125 % ou 150 %, toute la
  palette et toutes les grilles d'assets sont agrandies par interpolation — dans un projet dont le
  contenu est du pixel art rendu en filtrage *nearest* (`EX-ARCH-022`), c'est une contradiction.

Un point de départ favorable, en revanche : il n'existe **qu'un seul** appel à `setStyleSheet` dans
tout le dépôt, et aucun style en dur dispersé dans les widgets. Il n'y a rien à démêler — seulement une
couche à construire.

## Périmètre

### Inclus
- **Style maîtrisé** : choix explicite d'un style Qt non natif et **palette applicative** complète,
  toutes deux dérivées d'un jeu unique de **jetons de design** (couleurs nommées, échelle
  d'espacement, échelle typographique, tailles d'icônes et de vignettes, largeurs de contrôles).
- **Thème couvrant toute l'application** : fenêtre, panneaux dockables, barres de menus et d'état,
  onglets, arbres et tables, champs, barres de défilement, infobulles, boîtes de dialogue standard.
  La feuille de style est **produite à partir des jetons**, et cesse donc d'être une seconde source de
  vérité.
- **États de focus visibles**, condition de la navigation à la manette dans les menus (`EX-IHM-040`).
- **Police embarquée** avec repli sur une famille générique, et typographie unique.
- **Jeu d'icônes dessinées par code** et **barre d'outils** remplaçant les boutons radio empilés du
  panneau Outils ; les commandes deviennent des **actions** porteuses de leur libellé, icône,
  raccourci et état.
- **Netteté à toute échelle d'affichage** pour les vignettes et les icônes.
- **Thème clair et sombre de l'éditeur uniquement**, suivant le système par défaut, réglable et
  persisté. Le menu principal, l'écran Options et le jeu conservent l'identité sombre en toute
  circonstance.

### Exclus (hors périmètre de ce lot)
- **Réorganisation des panneaux de l'éditeur** : quels panneaux sont visibles, quelle information est
  affichée, quelle commande vit où — c'est l'objet de [LOT-57](@ref lot-57), qui s'appuie sur les
  actions livrées ici. Ce lot change l'**apparence** des panneaux, pas leur **répartition**.
- **Qt Quick / QML** : la refonte `LOT-34` → `LOT-39` a acté Qt Widgets, et tout le patrimoine `.ui`,
  les widgets et leurs tests reposent dessus. Le besoin est un défaut d'habillage, pas de framework.
- **Bibliothèque de style tierce** : cohérent avec la politique du projet de préférer Qt à une
  dépendance (cf. décision de cadrage de [LOT-54](@ref lot-54)).
- **Animations et transitions** entre écrans : sans valeur tant que le socle n'est pas posé,
  envisageable dans un lot ultérieur.
- **Découpage de *GameViewport* et de *TexturePanel*** : ces deux fichiers sont volumineux, mais leur
  refactoring est un sujet distinct de l'habillage. Ce lot ne déplace du code que là où l'apparence
  l'exige.
- **Habillage du jeu rendu** (HUD, écrans de pause) : le HUD relève du pipeline Direct3D 11
  ([LOT-52](@ref lot-52)), pas de la couche de widgets. Seule la couleur d'effacement du viewport est
  concernée ici, parce qu'elle est visuellement contiguë aux widgets.

## Décisions de cadrage
- **Prendre la main sur le style est la condition de tout le reste.** Tant que l'application s'exécute
  sur le style natif de la plate-forme, une partie de la feuille de style est ignorée et le résultat
  reste hétérogène quoi qu'on écrive. Ce lot commence donc par le style et la palette, et seulement
  ensuite par la feuille de style — l'ordre inverse a déjà été tenté au `LOT-38` et a produit le thème
  restreint par `objectName` qu'on cherche à dépasser.
- **Une source unique de grandeurs, dont dérive la feuille de style.** Le réflexe naturel serait
  d'écrire les couleurs dans la feuille de style et de les recopier en C++ là où le code en a besoin.
  C'est exactement le défaut actuel de la typographie. Les jetons sont donc la source, et la feuille de
  style est produite à partir d'eux au chargement.
- **Icônes dessinées par code, pas produites comme fichiers.** Le projet a déjà deux précédents :
  `hmi::buildProceduralAtlasImage` (`LOT-39`) et `hmi::ProceduralFont` (`LOT-52`) génèrent leur contenu
  graphique par code, avec repli déterministe. Des icônes peintes depuis la palette de jetons suivent
  ce précédent : aucun fichier à produire, netteté à toute taille et à toute échelle d'affichage,
  recoloration automatique avec le thème. Cela transforme ce qui aurait été un chantier graphique en
  une tâche de code — et lève l'objection qui avait initialement écarté les icônes du périmètre.
- **La barre d'outils est aussi une préparation de `LOT-57`.** Une action portant son libellé, son
  icône, son raccourci et son état peut être placée simultanément dans une barre d'outils, un menu et
  un menu contextuel **sans duplication**. C'est ce qui rendra la déduplication du `LOT-57`
  structurelle plutôt que cosmétique.
- **L'identité du jeu n'est pas un thème.** Le fond sombre et l'accent ambre du menu principal font
  partie de l'apparence du jeu, au même titre que ses tuiles : ils ne suivent pas les préférences
  d'affichage du poste. L'éditeur, lui, est un **outil de travail** — utilisé de jour, sur de longues
  sessions, souvent à côté d'autres applications — et c'est à ce titre qu'il suit le réglage
  clair/sombre du système. L'habillage se scinde donc en deux portées : une part **invariante**
  (menu principal, Options, jeu) et une part **variable** (châssis d'édition : panneaux, barre
  d'outils, barre d'état, barre de menus de l'éditeur, boîtes de dialogue ouvertes depuis lui).
  Ce n'est pas une exception ajoutée à la fin : la frontière est portée par les jetons dès la
  TACHE-01 et par la feuille de style dès la TACHE-02, faute de quoi la TACHE-06 ne pourrait pas
  l'introduire après coup.
- **Six tâches, au-dessus du grain habituel.** Les tâches 1 et 2 sont indissociables (le style et la
  palette conditionnent la feuille de style), et les tâches 3 à 5 corrigent chacune un défaut
  autonome. La tâche 6 (thème clair de l'éditeur) est explicitement la première à retirer si le lot
  doit être resserré : elle emporte alors `EX-IHM-054` avec elle, sans rien invalider d'autre — la
  séparation des deux portées, elle, reste acquise depuis la TACHE-01.
- **Amende le statut « brouillon » de [`interface-ihm.md`](../../Specification/interface-ihm.md)** en y
  ajoutant une section 6 : la spécification cadrait la refonte `LOT-34` → `LOT-39` sans jamais exiger
  quoi que ce soit sur l'apparence.

## Exigences couvertes
- Nouvelles : `EX-IHM-050` (style maîtrisé et thème couvrant toute l'IHM, en deux portées),
  `EX-IHM-051` (source unique des grandeurs d'habillage), `EX-IHM-052` (typographie unique et police
  embarquée), `EX-IHM-053` (netteté à toute échelle d'affichage), `EX-IHM-054` (thème clair/sombre de
  l'éditeur), `EX-IHM-055` (commandes exposées comme actions dans une barre d'outils).
- Concrétise enfin la partie « barre d'outils » d'`EX-EDIT-015` (découvrabilité), restée à l'état de
  boutons radio empilés depuis le `LOT-15`.
- Réutilisées : `EX-IHM-001` (interface hors-jeu en Qt), `EX-IHM-040` (menus, options, remappage —
  navigation à la manette par le focus), `EX-REN-033` (traduction), `EX-ARCH-022` (pixel art net),
  `EX-NFR-010` (`Core` indépendant de la présentation).

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-socle-style.md) | Socle de style : choix du style Qt, palette applicative et jetons de design | `Source/HMI/Interface` | ✅ |
| [TACHE-02](tache-02-theme-global.md) | Thème couvrant toute l'application, produit à partir des jetons, états de focus visibles | `Source/Elements/Themes`, `Source/HMI/Interface` | ✅ |
| [TACHE-03](tache-03-typographie-police.md) | Police embarquée avec repli et typographie à source unique | `Source/Elements`, `Source/HMI/Interface` | ✅ |
| [TACHE-04](tache-04-icones-barre-outils.md) | Icônes dessinées par code et barre d'outils à actions | `Source/HMI/Interface`, `Source/HMI/Editor` | ✅ |
| [TACHE-05](tache-05-nettete-dpi.md) | Netteté des vignettes à toute échelle d'affichage | `Source/HMI/Editor` | ✅ |
| [TACHE-06](tache-06-theme-clair-sombre.md) | Thème clair/sombre **de l'éditeur**, suivant le système, réglable et persisté | `Source/HMI/Interface`, `Source/Elements/Themes` | ✅ |

## Critères d'acceptation du lot
1. L'ensemble de l'application partage une seule apparence : aucun panneau, aucune boîte de dialogue
   standard, aucune barre ne se distingue par un rendu natif résiduel.
2. Modifier une couleur ou un espacement en un seul endroit se répercute sur les widgets **et** sur la
   couleur d'effacement du viewport ; aucune constante de style ne subsiste dans le code des widgets.
3. L'interface s'affiche correctement sans aucune police installée sur le système hôte.
4. Le focus est visible sur tous les contrôles atteignables, et la navigation à la manette dans les
   menus reste utilisable de bout en bout.
5. Le changement d'outil se fait depuis une barre d'outils à icônes ; chaque commande affiche son
   raccourci sans qu'il soit saisi deux fois.
6. Les vignettes de la palette et des grilles d'assets restent nettes à 100 %, 125 % et 150 % d'échelle
   d'affichage, sans lissage du pixel art.
7. Le thème **de l'éditeur** suit le réglage clair/sombre du système et peut être forcé depuis le menu
   Affichage de l'éditeur, sans redémarrage.
8. Le menu principal, l'écran Options et le jeu ont **exactement la même apparence** quel que soit le
   thème d'éditeur actif, y compris après une bascule à chaud.
9. Tous les libellés ajoutés passent par le catalogue de traduction et existent dans les deux langues ;
   build `/W4 /WX`, Doxygen, lint verts.

## Dépendances
Bâtit sur [LOT-38](@ref lot-38) (unification sur Qt : c'est le lot qui a produit le thème restreint
qu'on généralise ici). Aucune dépendance sur les lots du programme d'habillage `LOT-40` → `LOT-55` :
ce lot peut être exécuté avant eux.

Deux lots dépendent en revanche de celui-ci et s'exécutent derrière : [LOT-57](@ref lot-57), et
[LOT-54](@ref lot-54) — l'atelier pixel art, recadré pour consommer les jetons, les actions, les
icônes et le dimensionnement à l'échelle d'affichage plutôt que d'arriver avec son propre habillage.
Deux tâches d'ici sont donc à écrire en prévoyant un second consommateur : la TACHE-04 (les actions
ne sont pas réservées à l'éditeur de niveaux) et la TACHE-05 (le calcul de dimensionnement doit être
réutilisable par un autre widget que les trois chemins de vignettes).

## Navigation des tâches
- @subpage lot-56-tache-01-socle-style
- @subpage lot-56-tache-02-theme-global
- @subpage lot-56-tache-03-typographie-police
- @subpage lot-56-tache-04-icones-barre-outils
- @subpage lot-56-tache-05-nettete-dpi
- @subpage lot-56-tache-06-theme-clair-sombre
