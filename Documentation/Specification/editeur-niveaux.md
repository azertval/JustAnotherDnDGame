# Éditeur de cartes {#spec-editeur}

> Statut : **livré**. `LevelEditor` peint les trois couches d'une carte, pose et renseigne ses
> entités, montre le graphe du monde, avertit d'un terrain tactique invalide, et joue la carte en
> cours avec le moteur du jeu. Dépend de [`niveaux.md`](niveaux.md).

> **Refonte décidée le 18 septembre 2026.** L'éditeur devient un module à part, refait lot par lot
> selon la [feuille de route de l'éditeur](@ref roadmap-editeur). Cette page reste sa
> spécification ; chaque `LOT-EDITOR` révise les exigences qu'il touche. Le
> [LOT-EDITOR-01](@ref lot-editor-01) a déplacé le code dans `Source/Editor`, fait de l'éditeur un
> outil interne (style Fusion, textes anglais, sans charte) et ajouté la section 8.

## Objectif
Permettre la **création et la modification des cartes sans écrire de code ni de JSON**, afin que
des membres de l'équipe **non-développeurs** (game design, level design) contribuent directement
au contenu du jeu.

## 1. Exigences fonctionnelles
- \anchor EX-EDIT-001 **EX-EDIT-001** — L'éditeur doit permettre de créer et modifier une carte
  **sans compétence en programmation** ni ligne de commande.
- \anchor EX-EDIT-002 **EX-EDIT-002** — L'édition doit être **directe** : une grille visuelle où
  l'on peint les tuiles à la souris, depuis une **palette** de types, chaque type dessiné de la
  couleur que le canevas lui donne.
- \anchor EX-EDIT-004 **EX-EDIT-004** — L'éditeur doit permettre de poser l'**entrée** de la carte,
  unique : la poser ailleurs la déplace.
- \anchor EX-EDIT-005 **EX-EDIT-005** — L'éditeur doit permettre de **redimensionner** la grille et
  de gérer **annuler/refaire**.
- \anchor EX-EDIT-006 **EX-EDIT-006** — L'éditeur doit **enregistrer et charger** au format JSON
  défini par `EX-LVL-003`, en produisant des fichiers **valides**.
- \anchor EX-EDIT-007 **EX-EDIT-007** — L'éditeur doit **valider** la carte avant enregistrement
  (entrée présente et unique, dimensions cohérentes — `EX-LVL-004`) et signaler les erreurs de façon
  compréhensible par un non-codeur.
- \anchor EX-EDIT-008 **EX-EDIT-008** — L'éditeur doit permettre d'**essayer la carte**
  immédiatement, sans l'enregistrer, pour un cycle création → essai rapide (`EX-EDIT-055`).
- \anchor EX-EDIT-009 **EX-EDIT-009** — L'éditeur doit permettre de **nommer** une carte à sa
  création et de la **renommer**, et **avertir avant d'écraser** un fichier existant différent de la
  carte en cours d'édition.

## 2. Réutilisation & cohérence
- \anchor EX-EDIT-010 **EX-EDIT-010** — L'éditeur doit **réutiliser le modèle de carte et la
  validation de `Core`** — aucune duplication de la logique de carte entre le jeu et l'éditeur.
- \anchor EX-EDIT-011 **EX-EDIT-011** — Une carte enregistrée par l'éditeur doit être **directement
  jouable** par le jeu sans conversion, et réciproquement. Ce que l'éditeur ne sait pas modifier, il
  doit le **transporter** : les pièces assignées par case, les propriétés libres des couches et des
  entités (`EX-LVL-018`) traversent un cycle ouvrir/enregistrer sans perte — un éditeur qui efface
  en silence ce qu'il n'affiche pas est pire qu'un éditeur incomplet.
- \anchor EX-EDIT-043 **EX-EDIT-043** — La **pièce** assignée à une case (`"texture"`, `niveaux.md`)
  doit être conservée par l'éditeur, et retirée si l'on repeint la case d'un autre type ; la poser
  relève des ateliers qui produisent les cartes à partir des planches de lieux (`LOT-92`).

## 3. Distribution & collaboration
- \anchor EX-EDIT-020 **EX-EDIT-020** — L'éditeur doit être fourni comme un **outil exécutable** que
  les non-codeurs lancent sans étape de build.
- \anchor EX-EDIT-021 **EX-EDIT-021** — Les cartes sont des **fichiers** rangés dans
  `Source/Elements/Levels` et versionnés ; l'éditeur enregistre directement à cet emplacement.
- \anchor EX-EDIT-022 **EX-EDIT-022** — Le partage des cartes passe par **Git via une interface
  graphique** (type GitHub Desktop) : les cartes sont versionnées dans le dépôt au même titre que le
  reste du projet, publiées et récupérées **sans ligne de commande**. Un court guide est fourni dans
  le manuel.

## 4. Approche d'implémentation (décidée)
**Un outil d'auteur à part, et un mode intégré au jeu qui n'en est pas un.** L'édition de contenu
vit dans `LevelEditor`, un exécutable Qt Widgets distinct du jeu ; l'édition **dans la scène**,
depuis le jeu, est l'**arène** du `LOT-50`.

- \anchor EX-EDIT-030 **EX-EDIT-030** — L'**outil d'auteur** est un exécutable distinct du jeu
  (`LevelEditor`), qui partage le code du jeu mais pas sa technologie d'interface : le jeu ne lie
  pas Qt Widgets (`EX-IHM-102`). Le **mode intégré** au jeu est l'arène, un bac à sable de
  débogage où l'on pose des combattants et rejoue à graine fixée — pas un outil qui produit du
  contenu versionné. Refondue au `LOT-11` (décision de l'auteur, 16 septembre 2026) : retarger
  `LevelEditor` plutôt que reconstruire l'édition dans la scène en Qt Quick.
- \anchor EX-EDIT-031 **EX-EDIT-031** — L'éditeur réutilise le **rendu du jeu** (QRhi,
  `hmi::SceneResources`, composition de `HMI`), le **modèle et la validation de carte** de `Core`,
  et, pour l'essai, la **mise en scène du jeu** elle-même (`hmi::WorldPlay`,
  `hmi::WorldSceneRenderer`) — sans duplication.

## 5. Non-objectifs
- Édition collaborative en temps réel.
- Édition des assets graphiques et sonores : l'éditeur agence des cartes, il ne dessine pas les
  planches. Les planches de lieux et les figurines viennent de leurs ateliers (`LOT-92`, `LOT-91`).
- Sélection multiple non contiguë et historique annuler/refaire par delta (l'historique par
  instantanés complets reste adapté à la taille des cartes du projet).

## 6. Robustesse et confort d'édition
- \anchor EX-EDIT-012 **EX-EDIT-012** — L'éditeur doit **demander confirmation** avant toute action
  destructrice : un redimensionnement qui supprimerait l'entrée, une entité ou une pièce assignée,
  et l'ouverture d'une autre carte alors que des modifications ne sont **pas enregistrées**.
- \anchor EX-EDIT-013 **EX-EDIT-013** — L'éditeur doit permettre de **déplacer (pan)** et de
  **zoomer** la vue indépendamment du cadrage automatique, pour éditer confortablement des cartes de
  toute taille.
- \anchor EX-EDIT-014 **EX-EDIT-014** — Au-delà de la peinture case par case, l'éditeur doit fournir
  un **outil de remplissage rectangulaire** et un **outil de sélection** avec **copier/coller** d'une
  zone de tuiles.
- \anchor EX-EDIT-015 **EX-EDIT-015** — L'éditeur doit exposer ses commandes de façon
  **découvrable** à l'écran : une barre d'outils pour changer d'outil, un aperçu des raccourcis
  clavier, et des libellés sur les entrées de la palette.
- \anchor EX-EDIT-017 **EX-EDIT-017** — L'éditeur doit permettre de **saisir directement** une
  largeur et une hauteur cibles, sous un **plafond généreux** qui reste configurable au niveau du
  code, pas une limite arbitraire de `Core`.
- \anchor EX-EDIT-018 **EX-EDIT-018** — La palette doit regrouper les types de tuiles en
  **catégories** (Tuile, Jalon, Sol, Obstacle, Passage) plutôt qu'en liste plate, et rester
  entièrement accessible par **défilement** quand tout est déplié.
- \anchor EX-EDIT-023 **EX-EDIT-023** — L'éditeur doit afficher, en superposition de la grille de
  tuiles, un **quadrillage de repère** case par case (bascule `F10`), sans effet sur le cadrage.

## 7. Couches, entités et monde (`LOT-11`)
- \anchor EX-EDIT-048 **EX-EDIT-048** — L'éditeur doit peindre les **trois couches** d'une carte :
  un **sélecteur de couche active** désigne la grille que visent le pinceau, le rectangle, la copie
  et le collage — la **collision** (grille racine, qui porte seule l'entrée) ou une couche
  **visuelle** (sol, décor). Une couche visuelle **refuse** l'entrée, qui porte une règle, en le
  disant. Couches visuelles ajoutées, retirées, renommées, changées de rôle et réordonnées, chaque
  geste **annulable** (`EX-EDIT-005`). Ajouter la première couche visuelle à une carte à grille
  unique y recopie l'image de la grille. Concrétisé au `LOT-11`.
- \anchor EX-EDIT-049 **EX-EDIT-049** — L'éditeur doit régler la **visibilité** et l'**opacité** de
  chaque couche, pour voir le sol sous le décor. Ce réglage est une aide d'édition : ni annulable,
  ni enregistré. Sur une carte à couches, la collision se montre **par-dessus** en masque coloré par
  catégorie (obstacle, entrée), pas en image. Concrétisé au `LOT-11`.
- \anchor EX-EDIT-050 **EX-EDIT-050** — L'éditeur doit **poser, sélectionner, déplacer et retirer
  des entités** par un outil dédié, et en éditer les **propriétés** dans un panneau dont le
  formulaire est **dérivé** de la table des familles (`core::knownEntityKinds`, `niveaux.md`) — une
  famille ou une propriété ajoutée à la table y apparaît sans toucher au panneau. Une entité d'un
  type inconnu, et toute propriété que la table ne déclare pas, sont **montrées et transportées**
  (`EX-EDIT-011`). Chaque geste est **annulable**. Une entité sans illustration se dessine par le
  **marqueur généré** de sa famille (`EX-CNT-041`). Concrétisé au `LOT-11`.
- \anchor EX-EDIT-051 **EX-EDIT-051** — Une propriété qui **référence** une donnée hors de la carte
  doit se choisir dans ce qui existe : dialogues **acceptés** au chargement, rencontres, cartes,
  points d'arrivée de la carte cible. Une référence cassée est **signalée** dans le panneau, à sa
  case, sans empêcher d'enregistrer : une carte s'écrit dans le désordre, le portail vers la forêt
  avant la forêt. Concrétisé au `LOT-11`.
- \anchor EX-EDIT-052 **EX-EDIT-052** — L'éditeur doit poser des **portails** — une carte cible et
  un **point d'arrivée nommé**, jamais des coordonnées — et des **points d'arrivée** dont le nom est
  unique dans la carte (`niveaux.md`). Concrétisé au `LOT-11`.
- \anchor EX-EDIT-053 **EX-EDIT-053** — Le navigateur de cartes doit offrir une **vue du graphe du
  monde** : les cartes, les portails qui les relient, et, visiblement distincts, les portails
  cassés. Ouvrir une carte depuis le graphe suit le même garde-fou que depuis la liste
  (`EX-EDIT-012`). Concrétisé au `LOT-11`.
- \anchor EX-EDIT-054 **EX-EDIT-054** — Le combat se jouant sur la carte d'exploration, l'éditeur
  doit **avertir** quand une rencontre posée n'est pas un **terrain tactique valide** : un
  combattant de sa formation hors de la carte, sur un obstacle ou sur un autre — selon la règle même
  du montage d'une rencontre (`core::BattleGrid::place`) —, ou une zone atteignable en un
  déplacement depuis le déclencheur trop petite pour la rencontre et un groupe de quatre
  (`core::analyzeEncounterTerrain`, seuils nommés). Avec l'outil « Entité », la zone et la formation
  de la rencontre sélectionnée se voient sur la carte. Concrétisé au `LOT-11`.
- \anchor EX-EDIT-055 **EX-EDIT-055** — L'**essai immédiat** (`EX-EDIT-008`) doit jouer la carte en
  cours avec la **mise en scène du jeu** (`hmi::WorldPlay`) : même lieu, mêmes figurines, mêmes
  portails ; un portail qui ramène à la carte éditée retrouve le brouillon, pas le fichier. Ce que
  l'éditeur n'ouvre pas — un dialogue, un combat — est **dit** dans la barre d'état plutôt que tu.
  Refondue au `LOT-88`.

## 8. Le socle du module (`LOT-EDITOR-01`)
Un outil d'atelier dure si l'on n'y perd jamais de travail. Ces trois exigences viennent du socle
du module ([LOT-EDITOR-01](@ref lot-editor-01)).

- \anchor EX-EDIT-056 **EX-EDIT-056** — L'éditeur doit **sauvegarder automatiquement** un brouillon
  modifié, hors du dépôt, peu après chaque geste, et proposer de le **reprendre** au démarrage
  suivant quand la session précédente ne s'est pas terminée normalement. Un brouillon que l'auteur
  refuse de reprendre est mis de côté, pas effacé. Fermer l'éditeur avec des modifications demande
  s'il faut les enregistrer.
- \anchor EX-EDIT-057 **EX-EDIT-057** — Une carte ouverte **changée sur disque** (par un script, un
  autre outil, un changement de branche) ne doit jamais être **écrasée en silence**. Brouillon
  intact : la carte est relue. Brouillon modifié : l'auteur choisit de relire le disque ou de garder
  son brouillon, et la version écartée est mise de côté avant tout. La comparaison porte sur le
  **contenu** du fichier, pas sur sa date.
- \anchor EX-EDIT-058 **EX-EDIT-058** — L'historique d'annulation est **plafonné** (le pas le plus
  ancien est oublié au-delà), et l'état « modifié » suit le **contenu** : défaire jusqu'à l'état
  enregistré rend une carte non modifiée, et un geste sans effet (repeindre une case du même type)
  ne la modifie pas.

## 9. Le canevas qui montre le lieu (`LOT-EDITOR-02`)
On édite sur le lieu tel qu'on le jouera. Ces trois exigences viennent du canevas du module
([LOT-EDITOR-02](@ref lot-editor-02)).

- \anchor EX-EDIT-059 **EX-EDIT-059** — Le canevas doit montrer la carte **en isométrie, comme le
  jeu** : la liste de primitives est celle que compose le jeu (`hmi::composeWorldScene`), dans le
  même ordre, et son image rendue hors écran égale celle du jeu à une tolérance près. Une bascule
  montre la carte **à plat**, une case par unité et les types en couleurs, pour lire types et
  collision. Le canevas ne peint que la partie visible.
- \anchor EX-EDIT-060 **EX-EDIT-060** — Le **pointage** désigne la case dont le **losange** est sous
  le pointeur, jamais l'image qui la couvre : sous un relief haut, on pointe la case de derrière. Il
  prend la hauteur en paramètre (réserve de la décision D11), et reste juste aux quatre coins de la
  carte. La case survolée, ses coordonnées et ses pièces se lisent à l'écran.
- \anchor EX-EDIT-061 **EX-EDIT-061** — Une couche peut être **masquée**, **grisée** ou
  **verrouillée** (visible, mais aucun geste ne la peint) ; les reliefs peuvent passer **en
  transparence** ; une **mini-carte** montre toute la carte et le cadre de la vue, et ramène la vue
  d'un clic. Ce sont des aides d'édition : rien n'est enregistré dans la carte.

## Exigences retirées {#edit-retirees}

> Ancres conservées, jamais renumérotées : les lots livrés s'y réfèrent. Chacune servait un
> habillage ou une mécanique que le jeu ne lit pas.

- \anchor EX-EDIT-003 **EX-EDIT-003** *(retirée au `LOT-88`)* — liaison visuelle des mécanismes.
- \anchor EX-EDIT-016 **EX-EDIT-016** *(retirée au `LOT-88`)* — distinction des liaisons de
  mécanismes.
- \anchor EX-EDIT-019 **EX-EDIT-019** *(retirée au `LOT-88`)* — liaison d'un danger commuté.
- \anchor EX-EDIT-024 **EX-EDIT-024** *(retirée au `LOT-88`)* — jeux de skins nommés.
- \anchor EX-EDIT-025 **EX-EDIT-025** *(retirée au `LOT-88`)* — raccords automatiques des tuiles
  solides.
- \anchor EX-EDIT-026 **EX-EDIT-026** *(retirée au `LOT-88`)* — gestion des fichiers d'assets.
- \anchor EX-EDIT-027 **EX-EDIT-027** *(retirée au `LOT-88`)* — palette montrant les skins.
- \anchor EX-EDIT-028 **EX-EDIT-028** *(retirée au `LOT-88`)* — choix du cadrage de caméra.
- \anchor EX-EDIT-029 **EX-EDIT-029** *(retirée au `LOT-88`)* — zones de caméra.
- \anchor EX-EDIT-032 **EX-EDIT-032** *(retirée au `LOT-88`)* — trajectoires des éléments mobiles.
- \anchor EX-EDIT-033 **EX-EDIT-033** *(retirée au `LOT-88`)* — temporisation des éléments mobiles.
- \anchor EX-EDIT-040 **EX-EDIT-040** *(retirée au `LOT-88`)* — placement de décors.
- \anchor EX-EDIT-041 **EX-EDIT-041** *(retirée au `LOT-88`)* — conversion d'une photo en asset.
- \anchor EX-EDIT-042 **EX-EDIT-042** *(retirée au `LOT-88`)* — texture par type de tuile.
- \anchor EX-EDIT-044 **EX-EDIT-044** *(retirée au `LOT-88`)* — inspection par calque de rendu.
- \anchor EX-EDIT-045 **EX-EDIT-045** *(retirée au `LOT-88`)* — atelier de dessin d'assets.
- \anchor EX-EDIT-046 **EX-EDIT-046** *(retirée au `LOT-88`)* — mode création des plans.
- \anchor EX-EDIT-047 **EX-EDIT-047** *(retirée au `LOT-88`)* — panneau des plans.

## Traçabilité
Le code vit dans `Source/Editor` : `Logic/` (bibliothèque `EditorLogic`, testée sous
`Source/Test/Unit/Editor`) et `Ui/` (l'exécutable `LevelEditor`). L'éditeur s'appuie sur `Core`
(modèle et validation de carte, `niveaux.md`) et sur le rendu de `HMI` (`rendu-technique.md`) ; ce
qui reste de sa présentation est dans [`interface-ihm.md`](interface-ihm.md).
