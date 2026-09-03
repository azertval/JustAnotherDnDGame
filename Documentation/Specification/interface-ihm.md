# Interface utilisateur (IHM) {#spec-interface-ihm}

> Statut : **livré** (`0.1.0`). Refonte de l'interface hors-jeu (programme `LOT-34` → `LOT-39`),
> étendue par le **système de design** (section 6, `LOT-56`) et l'**architecture de l'information de
> l'éditeur** (section 7, `LOT-57`) — toutes livrées.
> Dépend de [`rendu-technique.md`](rendu-technique.md) et [`editeur-niveaux.md`](editeur-niveaux.md).

L'interface **hors-jeu** (menus, options, remappage, éditeur de niveaux) est distincte du **rendu
in-game**. Ce dernier reste en Direct3D 11 (`EX-REN-002`) ; l'interface, elle, repose sur un
**framework d'UI dédié (Qt)**, pour une application maintenable, à fenêtres réglables, remplaçant
l'UI « maison » dessinée quad par quad. `Core` demeure indépendant de la présentation
(`EX-NFR-010`).

## 1. Socle applicatif
- \anchor EX-IHM-001 **EX-IHM-001** — Toute l'**interface hors-jeu** (menus, options, remappage,
  éditeur) doit reposer sur le framework **Qt** ; seul le **rendu in-game** reste en Direct3D 11.
- \anchor EX-IHM-002 **EX-IHM-002** — Le **rendu Direct3D 11 du jeu** doit être **embarqué dans un
  viewport Qt** (surface native), sans processus séparé ni duplication du pipeline de rendu ; le
  déterminisme de la simulation (`EX-NFR-002`) et la latence d'entrée (`EX-CTRL-020`/`EX-CTRL-021`)
  sont préservés.
- \anchor EX-IHM-003 **EX-IHM-003** — Le jeu doit afficher, **dans la scène rendue**, un **affichage
  tête haute** minimal indiquant l'état dont le joueur a besoin pour décider : les **budgets de
  sauts et de dashs** restants (`EX-GP-024`) et le nom du tableau en cours. Ces informations existent
  dans la simulation depuis `LOT-12` sans avoir jamais été rendues visibles. L'affichage passe par le
  catalogue de traduction (`EX-REN-033`) et n'a aucun effet sur le gameplay (`EX-ARCH-012`).
  Concrétisé en `LOT-52`.
- \anchor EX-IHM-004 **EX-IHM-004** — Le jeu doit offrir un **écran de pause** (suspendant réellement
  la simulation, sans consommer de pas de temps fixe) et un **écran de fin de niveau**, navigables
  au clavier, à la souris et à la manette comme le reste de l'interface, et passant par le catalogue
  de traduction (`EX-REN-033`). Détaille `EX-REN-031` du côté de l'interface. Concrétisé en `LOT-59`.
- \anchor EX-IHM-005 **EX-IHM-005** — Le menu principal doit distinguer **reprendre** une partie
  (`EX-LVL-014`), en **commencer une nouvelle** et **choisir un niveau** parmi ceux déjà atteints.
  Un niveau **hors séquence** (créé dans l'éditeur) doit être jouable sans passer par l'essai de
  l'éditeur, et sans modifier la progression de la séquence. Concrétisé en `LOT-59`.

## 2. Éditeur
- \anchor EX-IHM-010 **EX-IHM-010** — L'éditeur de niveaux doit se présenter en **fenêtre à panneaux
  dockables** (palette, outils, niveaux, liens, viewport central) : panneaux **déplaçables,
  redimensionnables, détachables**.
- \anchor EX-IHM-011 **EX-IHM-011** — La **disposition** des panneaux doit être **persistée hors
  code** (sauvegardée et restaurée entre deux sessions), et réinitialisable à une disposition par
  défaut.

## 3. Gestion des niveaux
- \anchor EX-IHM-020 **EX-IHM-020** — L'éditeur doit offrir un **panneau de gestion des niveaux**
  listant les fichiers du dossier des niveaux, avec **recherche/filtre**, restant lisible quel que
  soit le nombre de niveaux.
- \anchor EX-IHM-021 **EX-IHM-021** — Ce panneau doit permettre de **créer, renommer, dupliquer et
  supprimer** un niveau, avec **validation de nom** (`EX-EDIT-006`) et **confirmation** des actions
  destructrices ; aucune modification non enregistrée ne doit être perdue silencieusement.

## 4. Liens de mécanismes
- \anchor EX-IHM-030 **EX-IHM-030** — Les **liaisons** déclencheur → cible (interrupteur/plaque →
  porte, déclencheur → danger commuté, `EX-EDIT-003`) doivent être rendues par des **traits/flèches
  explicites** dans le viewport, en remplacement de l'indication par teinte de case.
- \anchor EX-IHM-031 **EX-IHM-031** — Un **panneau « Liens »** doit **lister** les liaisons du niveau,
  mettre en **surbrillance** la liaison sélectionnée et permettre d'en **supprimer**.

## 5. Menus, options, unification
- \anchor EX-IHM-040 **EX-IHM-040** — Le **menu principal**, l'écran **Options** (V-Sync `EX-REN-022`,
  langue `EX-REN-033`) et les écrans de **remappage** (jeu, éditeur `EX-CTRL-012`, manette) doivent
  être fournis en Qt, fonctionnellement équivalents aux écrans historiques.
- \anchor EX-IHM-041 **EX-IHM-041** — L'interface hors-jeu doit reposer sur **une seule technologie
  d'UI** : la pile d'UI « maison » (écrans dessinés au `SpriteBatch`, gestion d'écrans dédiée, fenêtre
  Win32 propre) est **retirée** une fois la parité atteinte.

## 6. Système de design et habillage (LOT-56)
La refonte `LOT-34` → `LOT-39` a livré une interface Qt **fonctionnelle**, sans jamais traiter son
apparence pour elle-même. L'application n'a jamais choisi de style Qt : elle s'exécute donc sur le
style **natif** de la plate-forme, qui dessine la plupart des contrôles hors du contrôle de
l'application et ignore une large part de toute feuille de style posée par-dessus. C'est la raison
pour laquelle le thème existant a dû être **restreint** au menu principal et à la page Options
(portée par `objectName`) au lieu d'être étendu : l'étendre ne produisait pas un résultat homogène.
Il en résulte deux apparences dans la même fenêtre — écrans thématisés d'un côté, panneaux de
l'éditeur au rendu natif de l'autre — et des grandeurs d'habillage (couleurs, marges, tailles de
vignettes, largeurs) éparpillées entre la feuille de style, les fichiers de description d'interface
et des constantes locales à chaque widget.

- \anchor EX-IHM-050 **EX-IHM-050** — L'interface hors-jeu doit reposer sur un **style d'interface
  maîtrisé par l'application** (et non sur le style natif de la plate-forme), assorti d'un **thème
  unique externalisé** couvrant **l'ensemble** des widgets — fenêtre, panneaux dockables, barres de
  menus et d'état, onglets, arbres et tables, champs de saisie, barres de défilement, infobulles,
  boîtes de dialogue standard — et non un sous-ensemble d'écrans. Ce thème distingue deux portées :
  une part **invariante**, qui porte l'identité visuelle du jeu (menu principal, Options, jeu), et une
  part **variable**, le châssis d'édition, seule concernée par `EX-IHM-054`. L'état de **focus** doit rester
  visible en toute circonstance : la navigation à la manette dans les menus repose sur le parcours de
  focus (`EX-IHM-040`), qu'un focus invisible rend inutilisable.
- \anchor EX-IHM-051 **EX-IHM-051** — Les grandeurs d'habillage (couleurs, espacements, tailles
  d'icônes et de vignettes, largeurs de contrôles) doivent provenir d'une **source unique**, dont
  dérivent à la fois la palette de l'application, la feuille de style et la **couleur d'effacement du
  viewport** — cette dernière étant aujourd'hui définie indépendamment, d'où une couture visible entre
  le canevas Direct3D 11 et les widgets qui l'entourent. Aucune constante de style ne doit subsister
  en dur dans le code des widgets.
- \anchor EX-IHM-052 **EX-IHM-052** — La **typographie** doit avoir une source de vérité unique (et
  non partagée entre feuille de style et fichiers de description d'interface), et reposer sur une
  **police embarquée avec l'application**, avec **repli** sur une famille générique si elle est
  absente — l'interface ne doit dépendre d'aucune police installée sur le système hôte.
- \anchor EX-IHM-053 **EX-IHM-053** — Les **icônes et vignettes** de l'interface doivent rester
  **nettes à toute échelle d'affichage** (facteur de mise à l'échelle du système), sans lissage : les
  vignettes représentent du pixel art rendu en filtrage *nearest* (`EX-ARCH-022`), et un
  agrandissement interpolé en trahit le contenu.
- \anchor EX-IHM-054 **EX-IHM-054** — L'**éditeur** doit proposer un thème **clair et sombre**, suivant
  par défaut le réglage du système, modifiable par l'utilisateur et **persisté** entre deux sessions ;
  le changement s'applique sans redémarrage. Ce réglage est **strictement limité au châssis
  d'édition** : le **menu principal, l'écran Options et le jeu conservent en toute circonstance
  l'identité visuelle sombre du jeu**, qui n'est pas un thème mais une composante de son apparence.
  L'éditeur est un **outil de travail**, utilisé de jour et pendant de longues sessions, et c'est à ce
  titre — et à ce titre seul — qu'il suit les préférences d'affichage de son utilisateur.
- \anchor EX-IHM-055 **EX-IHM-055** — Les **commandes de l'éditeur** doivent être exposées comme des
  **actions réutilisables** — porteuses de leur libellé, de leur icône, de leur raccourci et de leur
  état — présentées dans une **barre d'outils à icônes**, de sorte qu'une même commande placée à
  plusieurs endroits reste une seule définition (condition d'`EX-IHM-062`).

## 7. Architecture de l'information de l'éditeur (LOT-57)
L'éditeur a gagné un panneau ou un onglet à presque chaque lot du programme d'habillage (`LOT-42`,
`LOT-43`, `LOT-45`, `LOT-50`, `LOT-51`), sans que la répartition d'ensemble soit jamais revue. Tous
ses panneaux restent affichés simultanément quel que soit l'outil actif, plusieurs états sont pilotés
depuis deux endroits distincts, et l'aide de la barre d'état est une ligne unique de raccourcis
concaténés qu'un simple message transitoire efface définitivement. À l'inverse, l'état dont l'auteur
d'un niveau a besoin en permanence — quel niveau est ouvert, s'il comporte des modifications non
enregistrées, quel outil est actif, quelle case est survolée — n'est affiché nulle part, alors que
l'application le connaît.

- \anchor EX-IHM-060 **EX-IHM-060** — L'éditeur doit afficher **en permanence** son état de travail :
  niveau ouvert, présence de **modifications non enregistrées**, outil actif, case survolée et niveau
  de zoom. L'aide affichée doit être **contextuelle à l'outil actif**, et un message transitoire ne
  doit jamais la faire disparaître définitivement.
- \anchor EX-IHM-061 **EX-IHM-061** — Les panneaux de l'éditeur doivent être **groupés** plutôt que
  tous déployés simultanément, le panneau pertinent étant mis en avant selon l'outil actif. Cette mise
  en avant est une **suggestion** : un panneau que l'utilisateur a ouvert explicitement n'est jamais
  masqué automatiquement, et la disposition reste réglable et persistée (`EX-IHM-010`,
  `EX-IHM-011`).
- \anchor EX-IHM-062 **EX-IHM-062** — Un même **état** ou une même **commande** ne doit être exposé
  qu'à **un seul endroit** de l'interface, **raccourci clavier compris** : deux contrôles pilotant la
  même valeur peuvent diverger et obligent l'utilisateur à deviner lequel fait autorité. Un raccourci
  clavier reste un second chemin **légitime** vers une commande, à condition d'être affiché par la
  commande elle-même plutôt que dupliqué en contrôle distinct.

## 8. Identité visuelle des écrans du jeu (LOT-68)
Le `LOT-56` a donné à l'interface un habillage cohérent, mais **générique** : la portée identité et
le châssis d'édition partagent la même police et la même échelle typographique, si bien que rien, à
l'écran, ne distingue le menu d'un jeu de plateforme en pixel art du panneau d'un outil de travail.
Les titres sont fixés à 32 pt et les entrées de menu à 16 pt quelle que soit la taille de la
fenêtre, ce qui donne une interface visiblement petite dès qu'on dépasse la définition d'un
ordinateur portable. Et le focus n'est signalé que par un changement de teinte — suffisant à la
souris, insuffisant à la manette, qui n'a pas de pointeur pour dire où elle en est.

- \anchor EX-IHM-070 **EX-IHM-070** — Les écrans du **jeu** doivent porter une identité **pixel
  art** assumée : police bitmap **embarquée** avec l'application (repli sur une famille générique si
  elle est absente, `EX-IHM-052`) et cadres à **bordure franche**, rendus **sans lissage** à un
  facteur d'agrandissement **entier** dérivé de la taille de la fenêtre (`EX-IHM-053`). Un facteur
  fractionnaire rend une bordure d'un pixel tantôt sur un pixel, tantôt sur deux : la contrainte est
  technique, pas esthétique. Cette identité est **bornée aux écrans du jeu** — le châssis d'édition
  conserve son apparence d'outil de travail et ses thèmes clair/sombre (`EX-IHM-054`), et aucune
  police bitmap ne se répand dans ses tables et ses arbres denses.
- \anchor EX-IHM-071 **EX-IHM-071** — L'élément **focalisé** d'un écran du jeu doit être signalé par
  une **marque explicite** (curseur), et non par la seule teinte : la navigation à la manette
  (`EX-IHM-040`) repose entièrement sur le parcours de focus, qu'une simple nuance de couleur rend
  difficile à suivre — et impossible pour un joueur qui distingue mal les couleurs. Une feuille de
  style ne sachant pas ajouter de contenu, cette marque est nécessairement peinte par le contrôle.
- \anchor EX-IHM-072 **EX-IHM-072** — Aucun écran ne doit exposer de réglage **inopérant**. Un
  contrôle grisé et non branché coûte plus de confiance qu'il n'apporte d'information : il se retire,
  ou il se branche. Symétriquement, une capacité qui existe déjà (comptage de cadence, bascule de
  rendu) s'expose comme réglage plutôt que de rester derrière une touche non documentée — sans jamais
  en faire un **second** état (`EX-IHM-062`).

- \anchor EX-IHM-073 **EX-IHM-073** — L'éditeur doit se présenter en **espaces de travail
  exclusifs** (édition de niveau, atelier pixel art) : seuls les panneaux, la barre d'outils et les
  menus de l'espace **actif** sont affichés, la disposition de chaque espace est persistée
  séparément (`EX-IHM-011`), et sélectionner un outil bascule sur l'espace auquel il appartient.
  Afficher les deux ensemble laissait en permanence à l'écran une trentaine de contrôles dont les
  deux tiers étaient hors contexte, les deux activités ne se pratiquant jamais en même temps. Ce
  masquage est un **changement d'espace**, décidé par l'utilisateur — à ne pas confondre avec la
  mise en avant automatique d'`EX-IHM-061`, qui reste une suggestion et ne masque jamais rien.

- \anchor EX-IHM-074 **EX-IHM-074** — Les surfaces de commande de l'éditeur doivent être
  **hiérarchisées** : la barre d'outils ne porte que la sélection d'outil et les commandes à usage
  **continu** ; toute autre commande reste atteignable par la barre de menus et son raccourci.
  `EX-IHM-055` garantissait déjà qu'une commande placée à plusieurs endroits reste une seule
  définition ; il restait à arbitrer **lesquelles** méritent une place permanente à l'écran — une
  barre d'outils portant onze commandes, dont neuf figuraient déjà au menu, est illisible sans
  qu'aucune ne soit pour autant dupliquée. Cette répartition doit être portée par la **description**
  de chaque commande, et non décidée par le code qui peuple les barres : une répartition implicite
  ne se relit pas et dérive au premier ajout.

## 9. Taille, réactivité et réglages effectifs (LOT-73)

Les sections précédentes ont donné à l'interface son châssis, son habillage et sa répartition de
l'information. Aucune n'a jamais dit **qui décide de la taille de la fenêtre**. La réponse, de fait,
était : le plus dense des écrans. `QStackedWidget::minimumSizeHint` valant le maximum sur *toutes*
ses pages — y compris celles qu'on ne regarde pas — un seul écran chargé fixait le plancher de la
fenêtre entière ; et ce plancher était multiplié par le facteur d'agrandissement d'`EX-IHM-070`,
lui-même dérivé de la hauteur de la fenêtre. La boucle se refermait sur elle-même, sans que rien ne
la borne : la fenêtre grandissait, le facteur montait, le plancher montait, la fenêtre grandissait.
Windows finissait par refuser la géométrie, et l'interface débordait sous la barre des tâches en
rognant son contenu, sans rien dire. Le défaut s'est produit trois fois, et a été corrigé deux fois
écran par écran.

Symétriquement, deux préoccupations vivaient à une portée plus large que la leur. Le facteur
d'agrandissement, qui ne concerne que les écrans du jeu, était substitué dans la feuille de style de
l'**application** : en changer repolissait ses 862 widgets, cinq secondes durant en configuration
Debug. Et l'onglet Entraînement du Mode IA lisait neuf réglages que rien ne transmettait au moteur,
pendant que le `config.json` du run affirmait le contraire.

- \anchor EX-IHM-080 **EX-IHM-080** — Aucun écran ne doit **contraindre la taille de la fenêtre** :
  il s'y adapte, au besoin en **défilant**, et ne rogne jamais son contenu sans recours. Cette
  garantie doit être portée par le **chemin d'ajout commun** des écrans, et non par une convention à
  réappliquer dans chaque fichier de description d'interface : une règle qu'il faut se rappeler
  d'appliquer se reperd au premier écran ajouté — c'est déjà arrivé deux fois.
- \anchor EX-IHM-081 **EX-IHM-081** — Le **facteur d'agrandissement** (`EX-IHM-070`) doit être borné
  par la **zone d'affichage disponible**, et non par la seule hauteur de la fenêtre ; la géométrie
  **restaurée** d'une session précédente doit être ramenée dans cette même zone, position et taille.
  Un facteur dérivé d'une hauteur que lui-même fait croître n'a pas de point fixe : la borne doit
  venir d'une grandeur dont l'application ne décide pas.
- \anchor EX-IHM-082 **EX-IHM-082** — Une préoccupation limitée à une **portée** ne doit jamais
  provoquer un rejeu de style d'une portée **plus large**. Les deux portées d'`EX-IHM-050` (identité
  du jeu, châssis d'édition) sont donc deux feuilles **disjointes**, appliquées chacune là où elle
  porte : l'identité sur la pile d'écrans, le châssis sur l'application. Le coût d'un changement
  d'habillage doit rester proportionnel à ce qui change réellement.
- \anchor EX-IHM-083 **EX-IHM-083** — Tout **réglage exposé** par un écran doit **atteindre** le
  moteur, ou ne pas être exposé ; et l'écran doit **ouvrir sur les valeurs par défaut du moteur**,
  jamais sur une seconde liste de valeurs inscrite dans sa description. Généralise à toute l'IHM le
  corollaire `IHM ⊇ CLI` du [`LOT-ANNEXE-22`](@ref lot-annexe-22). Un réglage inerte est pire qu'un
  réglage absent : il se règle, il s'enregistre dans la configuration du run, et il ment.

## Traçabilité
Tout ceci relève de `Source/HMI` — depuis le `LOT-38`, l'unique application Qt `JustAnotherDnDGame` (rendu
de jeu Direct3D 11 + widgets Qt répartis par domaine) ; les assets Qt déclaratifs vivent dans
`Source/Elements`. La logique testable (édition, validation, remappage) reste découplée de l'UI et
couverte par des tests (`EX-NFR-010`, `EX-NFR-020`). Détail du séquencement : lots
[`LOT-34`](@ref lot-34) à [`LOT-39`](@ref lot-39) pour la refonte initiale ;
[`LOT-56`](@ref lot-56) (section 6) et [`LOT-57`](@ref lot-57) (section 7) pour la révision de
l'apparence et de la répartition de l'information ; [`LOT-73`](@ref lot-73) (section 9) pour
l'invariant de taille, les portées de thème et les réglages effectifs.
