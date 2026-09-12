# Interface utilisateur (IHM) {#spec-interface-ihm}

> Statut : **livré** (`0.1.0`). Refonte de l'interface hors-jeu (programme `LOT-H-34` → `LOT-H-39`),
> étendue par le **système de design** (section 6, `LOT-H-56`) et l'**architecture de l'information de
> l'éditeur** (section 7, `LOT-H-57`) — toutes livrées.
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
- \anchor EX-IHM-003 **EX-IHM-003** — Le jeu doit afficher, **dans la scène rendue**, un
  **affichage tête haute** minimal indiquant l'état dont le joueur a besoin pour décider, et lui
  seul. L'affichage passe par le catalogue de traduction (`EX-REN-033`) et n'a aucun effet sur le
  gameplay (`EX-ARCH-012`).
  > **Refondue au `LOT-67`.** Elle **énumérait** ce contenu — budgets de sauts et de dashs, nom du
  > tableau en cours — c'est-à-dire l'état d'un jeu de plateforme. Ce que doit y lire un joueur de
  > RPG (points de vie, initiative, actions restantes) est le sujet de l'IHM de combat du
  > `LOT-24` ; l'exigence garde donc son critère — *ce dont le joueur a besoin pour décider* — et
  > cesse d'en fixer la liste, que chaque lot ajusterait sinon en la contredisant.
- \anchor EX-IHM-004 **EX-IHM-004** — Le jeu doit offrir un **écran de pause** suspendant
  réellement la simulation, sans consommer de pas de temps fixe, navigable au clavier, à la souris
  et à la manette comme le reste de l'interface, et passant par le catalogue de traduction
  (`EX-REN-033`). Détaille `EX-REN-031` du côté de l'interface.
  > **Refondue au `LOT-67`.** Elle exigeait aussi un **écran de fin de niveau**, qui n'a plus
  > d'objet : un bac à sable n'a pas de tableau à terminer. L'écran de pause, lui, reste — et
  > s'étoffera des entrées du RPG (fiche, inventaire, journal, carte) avec le châssis du `LOT-68`,
  > pas avant : une entrée de menu qui ne mène nulle part coûte plus de confiance qu'elle
  > n'apporte d'information (`EX-IHM-072`).

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

## 6. Système de design et habillage (LOT-H-56)
La refonte `LOT-H-34` → `LOT-H-39` a livré une interface Qt **fonctionnelle**, sans jamais traiter son
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
  **nettes à toute échelle d'affichage** (facteur de mise à l'échelle du système) : les **icônes**
  sont vectorielles et se retracent à la taille demandée (`LOT-H-56`), et une **vignette** d'asset
  est agrandie selon la nature de l'image qu'elle représente (`EX-ARCH-022`) — au plus proche
  voisin pour une tuile pixellisée, interpolée pour une illustration peinte.
  > **Refondue au `LOT-66`.** Elle imposait l'absence de lissage *pour toutes* les vignettes, au
  > motif qu'elles représentent du pixel art. Une vignette de panneau peint ([LOT-76](@ref lot-76))
  > agrandie au plus proche voisin serait crénelée, et la bibliothèque d'assets montrerait alors
  > une image que le jeu ne rend pas ainsi — la vignette cesserait d'être un aperçu.
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

## 7. Architecture de l'information de l'éditeur (LOT-H-57)
L'éditeur a gagné un panneau ou un onglet à presque chaque lot du programme d'habillage (`LOT-H-42`,
`LOT-H-43`, `LOT-H-45`, `LOT-H-50`, `LOT-H-51`), sans que la répartition d'ensemble soit jamais revue. Tous
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

## 8. Identité visuelle des écrans du jeu (LOT-H-68)
Le `LOT-H-56` a donné à l'interface un habillage cohérent, mais **générique** : la portée identité et
le châssis d'édition partagent la même police et la même échelle typographique, si bien que rien, à
l'écran, ne distingue le menu d'un jeu de plateforme en pixel art du panneau d'un outil de travail.
Les titres sont fixés à 32 pt et les entrées de menu à 16 pt quelle que soit la taille de la
fenêtre, ce qui donne une interface visiblement petite dès qu'on dépasse la définition d'un
ordinateur portable. Et le focus n'est signalé que par un changement de teinte — suffisant à la
souris, insuffisant à la manette, qui n'a pas de pointeur pour dire où elle en est.

- \anchor EX-IHM-070 **EX-IHM-070** — Les écrans du **jeu** doivent porter l'identité du **parchemin
  de Tanares** : fond de parchemin, encre sépia, filets et cabochons dorés, titrage à empattements
  dans une police **embarquée** avec l'application (repli sur une famille générique si elle est
  absente, `EX-IHM-052`). Les **teintes de cette palette sont relevées sur le corpus** — les feuilles
  de personnage et les livres Tanares — et jamais choisies à vue : une couleur inventée ressemble à
  la source sans en venir, et rien ne le dit jamais.
  Les grandeurs de l'habillage suivent un facteur d'agrandissement **entier** dérivé de la taille de
  la fenêtre. Le facteur reste entier pour une raison qui n'est plus celle du filtrage : les
  longueurs de la feuille de style sont des **entiers de pixels**, et à 1,5× le trait et le filet
  d'un encadrement s'arrondissent tous deux à la même épaisseur — la réserve de parchemin qui les
  sépare disparaît, et l'encadrement se lit comme une bordure épaisse, sans qu'aucune erreur ne
  soit levée.
  Cette identité est **bornée aux écrans du jeu** — le châssis d'édition conserve son apparence
  d'outil de travail et ses thèmes clair/sombre (`EX-IHM-054`), et aucun parchemin ne se répand dans
  ses tables et ses arbres denses.
  > **Refondue au `LOT-66`.** Elle imposait une identité **pixel art** — police bitmap, bordures
  > franches, aucun lissage — héritée du jeu de plateforme dont ce dépôt est issu. Elle entrait en
  > contradiction frontale avec les références du jeu visé : une police bitmap non lissée et une
  > illustration peinte à 300 ppp ne cohabitent pas. Le [LOT-01](@ref lot-01) avait délibérément
  > conservé l'atelier pixel art ; ce renversement est assumé, pas subi.
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
  exclusifs** (édition de niveau, atelier de dessin d'assets) : seuls les panneaux, la barre d'outils et les
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

- \anchor EX-IHM-075 **EX-IHM-075** — L'habillage ornemental des écrans du jeu — encadrements,
  cabochons, bandeaux de titre — doit être **tracé par le code**, jamais livré en image. C'est le
  prolongement d'`EX-IHM-070`, et la raison n'est pas la place que prendraient ces fichiers :
  - **une image ne s'étire pas honnêtement.** Un cabochon posé sur un panneau bas s'ovalise ou
    mange le tiers de sa hauteur ; un bandeau étiré déforme ses ailes. Un ornement tracé se
    *redessine* à la taille demandée, et ses ailes peuvent suivre la **hauteur** quand sa plaque
    suit la **largeur** — ce qu'aucun découpage en tranches ne sait faire ;
  - **une image fige ses couleurs hors des jetons** (`EX-IHM-051`) et devrait être réexportée à
    chaque retouche de palette. Une forme tracée porte un **rôle**, jamais une teinte ;
  - **une image ne suit pas le facteur d'agrandissement** (`EX-IHM-081`) : elle est nette à un seul
    facteur, un tracé l'est à tous.

  Ce que le corpus apporte n'est donc pas de la matière mais de la **mesure** : les proportions et
  les teintes de ces ornements sont **relevées** sur `Documentation/SourceBook/`, jamais choisies à
  vue — même règle qu'`EX-IHM-070` pour la palette, et même raison. Un ornement inventé ressemble à
  la source sans en venir, et rien ne le dit jamais.
  > **Ajoutée au `LOT-76`.** `EX-IHM-070` imposait déjà de relever les couleurs sur le corpus, mais
  > rien n'était écrit des **formes** : le `LOT-66` avait donc pu conclure, à juste titre pour son
  > périmètre mais sans que rien ne le garantisse au-delà, qu'aucun fichier d'image ne serait livré.
  > Cette exigence tranche l'autre moitié de la question, et écarte explicitement la voie du
  > découpage d'images — essayée, puis abandonnée au `LOT-76`.

- \anchor EX-IHM-076 **EX-IHM-076** — Une **illustration** d'interface — fond d'écran, carte,
  portrait — doit être **extraite du corpus** et livrée telle quelle, jamais redessinée. C'est le
  pendant d'`EX-IHM-075`, et la frontière entre les deux est nette : un **ornement** se trace parce
  qu'il doit se redimensionner et suivre les jetons ; une **illustration** ne le peut pas — une
  carte du monde peinte ne se trace pas, elle se prend ou elle n'existe pas.
  Trois obligations en découlent :
  - la région extraite est **déclarée** (document, page, rectangle) et l'extraction se rejoue à
    l'identique (`EX-CNT-020`), par **rendu de région** (`EX-CNT-022`) ;
  - ce qui est livré est décrit par un **manifeste** — dimensions, empreinte, provenance — que
    l'intégration continue recoupe avec les fichiers **et avec le code qui les nomme**. Le corpus
    étant absent du runner, rien d'autre ne peut dire d'où une image vient ;
  - une illustration **absente** est un cas attendu (`EX-NFR-040`) : l'écran retombe sur son décor
    tracé. Aucun écran ne doit dépendre d'un binaire pour s'afficher.
  Un **filigrane** ou un folio présent sur la page source se **recadre**, jamais ne s'efface :
  l'effacer demanderait de repeindre ce qu'il recouvre, c'est-à-dire d'inventer des pixels.

## Exigences retirées {#ihm-retirees}

> Retirées par le `LOT-67`, qui retire du programme la notion de **niveau discret**. Les ancres
> sont **conservées** — jamais renumérotées, jamais supprimées : les lots hérités s'y réfèrent, et
> réécrire un lot livré falsifierait son histoire (règle de [`lots.md`](@ref lots)). Le texte
> ci-dessous est celui d'origine ; il décrit ce qui **a été** livré, pas ce qui est attendu
> aujourd'hui.

- \anchor EX-IHM-005 **EX-IHM-005** *(retirée en `LOT-67`)* — Le menu principal doit distinguer
  **reprendre** une partie (`EX-LVL-014`), en **commencer une nouvelle** et **choisir un niveau**
  parmi ceux déjà atteints. Un niveau **hors séquence** (créé dans l'éditeur) doit être jouable
  sans passer par l'essai de l'éditeur, et sans modifier la progression de la séquence. Motif :
  les trois entrées reposaient sur une séquence et sur une progression au tableau, l'une et
  l'autre retirées. « Reprendre » reviendra avec la sauvegarde du `LOT-17` — quand il y aura
  quelque chose à reprendre.

## 9. Taille, réactivité et réglages effectifs (LOT-H-73)

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
  corollaire `IHM ⊇ CLI` du `LOT-ANNEXE-22` (hérité). Un réglage inerte est pire qu'un
  réglage absent : il se règle, il s'enregistre dans la configuration du run, et il ment.

## 10. Le châssis des écrans du RPG (LOT-68)

Huit écrans manquent au jeu — fiche de personnage, inventaire et équipement, journal de quêtes,
carte du monde, dialogue, marchand, tableau de la Guilde, affichage tête haute de combat — et
quatre lots à venir les remplissent chacun de leur côté (`LOT-38`, `LOT-42`, `LOT-45`,
`LOT-24`). Sans règle commune, ces quatre lots produiraient quatre écrans qui s'ouvrent
différemment, se ferment différemment et se naviguent différemment : le défaut ne se voit sur aucun
d'eux pris isolément, et sur les quatre ensemble il n'est plus rattrapable sans les refaire.

- \anchor EX-IHM-090 **EX-IHM-090** — Les écrans du **RPG** doivent partager un **châssis** unique :
  le même cadre (panneau, titre, zone de contenu, pied d'actions), la même ouverture et la même
  fermeture, le même passage d'un écran à l'autre **sans repasser par le menu**, et le même parcours
  de focus à la manette (`EX-IHM-071`). L'**ossature** de chaque écran — ses blocs, leurs genres,
  leurs libellés — doit être portée par une **description en données**, non par du code d'interface
  écrit écran par écran : ajouter un écran ne doit demander de toucher à **aucun** des autres, ni
  dans le code, ni dans la feuille de style. Une convention à réappliquer à chaque écran se reperd
  au premier ajout — c'est la leçon qu'`EX-IHM-080` a déjà tirée pour la taille des écrans.
  Corollaire : les libellés de cette description passent par le catalogue de traduction
  (`EX-REN-033`) comme tout autre texte, et rien ne les y rattachant qu'une table, leur présence
  dans **les deux** catalogues doit être vérifiée automatiquement.
- \anchor EX-IHM-091 **EX-IHM-091** — Chaque écran du RPG doit déclarer sa **règle de
  superposition** : suspend-il la simulation, ou se consulte-t-il en marchant ? Cette règle
  appartient à la **description** de l'écran, jamais au code qui l'ouvre : ouvert depuis la pause,
  depuis le jeu ou depuis une touche, un même écran doit se comporter de la même façon, et une règle
  décidée au point d'appel se contredit d'un appel à l'autre sans que rien ne le signale.

## 11. La conception séparée du code (LOT-86) {#ihm-conception}

> Statut : **en cours** (`LOT-86`). Cette section remplace, pour les écrans du **jeu**, ce que la
> section 10 confiait à une description en données. Le
> châssis d'édition (sections 6, 7 et 9) n'est pas concerné : il reste en Qt Widgets, dans son
> propre binaire.

### Ce qui a changé, et pourquoi

Les écrans du jeu étaient en Qt Widgets, décrits par une table C++ (`EX-IHM-090`). Une tentative
de les porter sur des fichiers Qt Designer a été menée puis **abandonnée** : elle demandait
1 268 lignes d'outillage — plugin de widgets promus, résolveur de feuille de style, générateur de
`.ui` — dont l'unique fonction était de rendre ces fichiers visualisables dans le designer. Les
deux formes partageaient le même défaut : **la mise en page vivait du côté du code**. Une retouche d'apparence demandait un développeur, une compilation, et
une relecture — pour déplacer un bloc de huit pixels.

Le prix s'est vu deux fois. Le défaut du plancher de taille des écrans s'est produit **trois fois**
avant qu'`EX-IHM-080` ne le déplace sur un chemin commun. Et la palette d'identité a été écrite
**deux fois** — en CSS dans les maquettes, en C++ dans les jetons — tenue par un contrôle dont le
commentaire disait qu'auparavant *« rien ne les reliait »*.

Les écrans du jeu passent donc à **Qt Quick**, et leur mise en page à des fichiers que Qt Design
Studio ouvre, modifie et réenregistre. Ce n'est pas un changement de bibliothèque : c'est un
déplacement de la frontière entre deux métiers.

### La frontière, telle qu'elle est tenue

- \anchor EX-IHM-100 **EX-IHM-100** — Une modification **purement visuelle** d'un écran du jeu —
  mise en page, couleurs, typographie, ornements, animations, textes — doit être réalisable **sans
  modifier ni recompiler une ligne de C++**, depuis Qt Design Studio ouvrant
  `Source/Ui/JadgUi.qmlproject`. Ce projet ne décrit que `Source/Ui`, les jumeaux de câblage et
  les assets : ni CMake, ni `Source/HMI`, ni code. La frontière n'est pas une consigne de
  relecture, c'est le périmètre d'un fichier. Depuis le `LOT-87`, `Source/Ui` est un module QML
  **sans C++** (`Jadg.Ui`) : c'est ce qui le rend résolvable par l'atelier, dont le marionnettiste
  ne charge aucun plugin du projet ; les types C++ forment un module à part (`Jadg.Runtime`) que
  seuls les jumeaux importent, et que l'atelier remplace par des doublures QML (`Source/Ui/Mocks`).
- \anchor EX-IHM-101 **EX-IHM-101** — La couche de **présentation** (`Source/HMI/Presentation`,
  et les vues-modèles de `Source/HMI/Runtime` exposées au QML) transforme l'état du jeu en données
  affichables et **ne dessine rien** : elle ne connaît ni Qt Quick, ni Qt Widgets. Seule exception,
  nommée : la surface de rendu (`GameViewportItem`), qui est un item de scène et non un modèle. Un écran lui demande *ce que le jeu sait dire*, jamais *comment le
  montrer*. Un seul en-tête d'IHM qui y entrerait signalerait que la logique de vue a commencé à
  redescendre dans la couche de données — et c'est ainsi que les 2 472 lignes de `MainWindow.cpp`
  se sont accumulées.
- \anchor EX-IHM-102 **EX-IHM-102** — L'exécutable du **jeu** ne lie pas `Qt6::Widgets`. Ce n'est
  pas une convention mais une impossibilité : un widget qui y réapparaîtrait ferait échouer
  l'édition de liens. Les widgets n'appartiennent qu'à l'éditeur de niveaux, binaire séparé.
- \anchor EX-IHM-103 **EX-IHM-103** — Tout écran et tout contrôle du jeu est un **formulaire
  `.ui.qml`** — le sous-ensemble **déclaratif** de QML — et ne contient aucun code impératif.
  Qt Design Studio relit et **réenregistre** ces fichiers : ce qu'il n'y comprend pas, il le perd,
  sans avertir. La logique vit dans un fichier jumeau `.qml`, côté développeur. La règle ne vise
  donc pas le style, mais ce que l'outil détruirait.
- \anchor EX-IHM-104 **EX-IHM-104** — Un formulaire n'importe que des modules connus **à la fois**
  de l'installation Qt et de Qt Design Studio. Le designer livre les siens (`QtQuick.Studio.*`),
  absents d'une installation ordinaire : un formulaire qui en importerait s'ouvrirait parfaitement
  chez la conception et casserait le jeu — le pire des deux mondes, découvert le plus tard
  possible.
- \anchor EX-IHM-105 **EX-IHM-105** — Aucune couleur, famille de police ni taille de texte n'est
  écrite en dur hors de `Source/Ui/Theme`. C'est ce qui donne son sens aux jetons : une valeur
  écrite dans un écran survit à un changement de palette, ne suit plus rien, et personne ne
  remarque qu'un seul écran a cessé de ressembler aux autres (`EX-IHM-051`).

### Les ornements restent tracés, et `EX-IHM-075` avec eux

Le passage à Qt Quick a d'abord semblé condamner `EX-IHM-075` — *« l'habillage ornemental se trace,
il ne se livre pas en image »* —, puisqu'une règle qui place le dessin dans du code place aussi
l'apparence hors de portée de la conception.

**C'est faux, et l'erreur méritait d'être corrigée plutôt que suivie.** Les trois raisons que cette
exigence invoque tiennent toujours : une image ne s'étire pas honnêtement (un cabochon posé sur un
panneau bas s'ovalise), elle fige ses couleurs hors des jetons (`EX-IHM-051`), et elle ne suit pas
le facteur d'agrandissement entier (`EX-IHM-081`) — nette à un seul facteur, floue aux deux autres.

Or **Qt Quick Shapes** honore les trois : une forme s'y redessine à la taille demandée, prend ses
couleurs de `Tokens.qml`, et reste nette à tous les facteurs. Et une `Shape` déclarative s'édite
dans Qt Design Studio comme le reste.

Ce qui change n'est donc pas « tracé par du code » mais « tracé par du **C++** ». La géométrie pure
relevée sur le corpus (`hmi::parchmentFrameStrokes`, `hmi::cabochonShapes`, `hmi::titleBannerShapes`)
reste la **source** de ce portage, et n'est pas jetée en attendant : la redessiner plus tard, sans
se souvenir de ce qui avait été relevé, coûterait bien plus que de la conserver.

### Ce qui rend ces exigences autre chose que des intentions

`scripts/check_ui_layers.py` les vérifie toutes à chaque *Pull Request*, et vérifie en outre
qu'`EX-ARCH-001` et `EX-NFR-010` restent vraies — `Core` sans un seul en-tête Qt. Il ne **crée** pas
cette dernière règle, il la **verrouille** : elle est tenue depuis le `LOT-01`, et un seul `QString`
suffirait à la rendre fausse sans que rien d'autre ne le signale.

Le contrôle s'auto-vérifie contre la vacuité : un relevé vide est un **échec**, jamais un succès.
C'est la panne du `LOT-78`, où un contrôle vert ne lisait rien.

### Ce que la conception ne peut pas faire seule

Aucune chaîne ne met la totalité d'une interface entre les mains d'un artiste, et le prétendre
serait un mensonge utile à personne. Exposer une **donnée** que le jeu ne calculait pas, ajouter une
**interaction** qui change l'état du jeu, écrire une **règle de navigation** ou faire exister un
**écran** demandent un développeur : ce sont des notions de jeu, pas d'apparence. La conception
dispose librement de tout ce que le jeu sait déjà dire. C'est la même frontière que dans les moteurs
du commerce, et c'est la bonne.

## Traçabilité
Tout ceci relève de `Source/HMI` — depuis le `LOT-H-38`, l'unique application Qt `JustAnotherDnDGame` (rendu
de jeu Direct3D 11 + widgets Qt répartis par domaine) ; les assets Qt déclaratifs vivent dans
`Source/Elements`. La logique testable (édition, validation, remappage) reste découplée de l'UI et
couverte par des tests (`EX-NFR-010`, `EX-NFR-020`). Détail du séquencement : lots
`LOT-H-34` à `LOT-H-39` pour la refonte initiale ;
`LOT-H-56` (section 6) et `LOT-H-57` (section 7) pour la révision de
l'apparence et de la répartition de l'information ; `LOT-H-73` (section 9) pour
l'invariant de taille, les portées de thème et les réglages effectifs ; `LOT-68` (section 10)
pour le châssis des écrans du RPG ; `LOT-86` (section 11) pour la séparation de la conception et
du code — les écrans du **jeu** passent à Qt Quick dans un binaire propre, l'**éditeur** reste en
Qt Widgets dans le sien.
