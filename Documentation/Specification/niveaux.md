# Niveaux & contenu {#spec-niveaux}

> Statut : **livré** (`0.1.0`). Format JSON, chargement, validation, chaînage de niveaux, progression
> persistée (`EX-LVL-014`, `LOT-H-59`) et couverture de mécaniques garde-fou (`EX-LVL-015`, `LOT-H-65`)
> tous en place. Dépend de [`gameplay.md`](gameplay.md).

## 1. Représentation des niveaux
- \anchor EX-LVL-001 **EX-LVL-001** — Un niveau doit être décrit par un **fichier de données** externe (pas en dur dans le code), placé dans `Source/Elements`.
- \anchor EX-LVL-002 **EX-LVL-002** — Le format doit décrire au minimum : dimensions de la grille, type de chaque tuile, position d'entrée et de sortie, et les mécanismes (interrupteurs, portes, blocs) avec leurs liaisons.
- \anchor EX-LVL-003 **EX-LVL-003** — Le format retenu est un **JSON structuré orienté objets** : un niveau est un objet JSON portant ses **métadonnées** (nom, dimensions) et une **liste de tuiles**, chaque tuile étant un **objet** `{x, y, type, …}` (les cases vides sont omises) pouvant porter des **champs spécifiques** à son type (ex. liaison interrupteur↔porte par identifiant). Choisi pour un moteur **extensible et réutilisable** (données riches par tuile, sérialisation et *round-trip* d'éditeur directs), au prix d'une lisibilité « à l'œil » moindre qu'une grille ASCII — l'édition passe par l'**éditeur**, pas par le texte brut.
- \anchor EX-LVL-005 **EX-LVL-005** — Le fichier de niveau doit porter un **numéro de version de
  format**, afin que l'ajout de nouveaux champs (fond, jeu de skins, texture par case, décors) reste
  traçable et qu'une évolution non rétrocompatible future soit **détectée** plutôt que subie. Un
  fichier **sans** numéro de version est lu comme la version initiale, sans erreur ni avertissement :
  la rétrocompatibilité des niveaux existants est un invariant. Concrétisé en `LOT-H-44`.
- \anchor EX-LVL-004 **EX-LVL-004** — Le chargement d'un niveau doit **valider** les données (positions des tuiles **dans les bornes** `width × height`, présence d'une entrée et d'une sortie, liaisons de mécanismes valides) et signaler une erreur exploitable en cas de fichier invalide (cf. politique d'erreurs des conventions).
- \anchor EX-LVL-006 **EX-LVL-006** — Un niveau doit porter son **mode de cadrage** de caméra
  (`EX-REN-016`) comme une **donnée**, au même titre que sa géométrie : le cadrage est une décision
  de **conception** — un tableau de puzzle se voit en entier, un tableau d'adresse suit le
  personnage — et non une règle déduite des dimensions. Un fichier **sans** mode déclaré conserve
  **exactement** le comportement historique (niveau entier s'il tient dans une salle, cadrage par
  salle sinon) : la rétrocompatibilité des niveaux existants reste un invariant (`EX-LVL-005`).
  Concrétisé en `LOT-H-64`.
- \anchor EX-LVL-007 **EX-LVL-007** — En **mode par salle** (`EX-REN-015`), un niveau doit pouvoir
  porter une **liste de zones de caméra** dessinées à la main (rectangles en tuiles) plutôt que de
  subir une grille uniforme unique : la caméra retient la **première** zone de la liste couvrant la
  position du personnage, avec repli sur le **niveau entier** si aucune zone ne le couvre — ce qui
  permet de mélanger plusieurs tailles de caméra dans un même niveau, sans transitions ni
  déclencheurs, la liste étant vide par défaut (comportement de grille automatique inchangé,
  `EX-LVL-006`). Concrétisé en `LOT-H-64`.

- \anchor EX-LVL-008 **EX-LVL-008** — Le format de niveau doit porter la **route** des plateformes
  mobiles (points de passage et mode de parcours, `EX-GP-054`) et les **capacités** du tableau
  (`EX-GP-055`), avec **repli compatible** : un fichier écrit avant le multi-points (couple
  `endX`/`endY`, aucune capacité déclarée) se charge et se joue **à l'identique**, la
  rétrocompatibilité des niveaux existants restant un invariant (`EX-LVL-005`). Concrétisé en
  `LOT-H-67`.
- \anchor EX-LVL-016 **EX-LVL-016** — Une carte doit porter **N couches de tuiles typées**
  plutôt qu'une grille unique : un RPG en vue de dessus superpose un **sol** (herbe, dalle, eau),
  un **décor** (arbre, tonneau, tapis) et une **collision** — masque indépendant du visuel, un
  tapis se traverse et un tonneau non, les deux pouvant reposer sur la même image de sol. La grille
  de **collision** d'une carte est son tableau racine `tiles`, celui qui porte déjà l'entrée, la
  sortie et les cases de mécanismes : le tableau `layers` ne décrit que les couches **visibles**,
  et une couche de rôle `collision` qui y serait déclarée est **refusée** — deux grilles à tenir
  d'accord se désynchronisent, et c'est celle qu'on ne voit pas qui gagne. Au chargement, la grille
  racine est **promue** en couche de tête, pour que tout consommateur boucle sur les couches sans
  cas particulier. Un fichier **sans** tableau `layers` — tout niveau antérieur à ce champ — se
  charge à l'identique, sa grille promue en couche unique dite *legacy*, à la fois décor et
  collision comme dans le format d'origine : la rétrocompatibilité reste un invariant
  (`EX-LVL-005`). Concrétisé en `LOT-04`.
- \anchor EX-LVL-017 **EX-LVL-017** — Une carte doit porter une **liste d'entités** — PNJ,
  coffres, panneaux, portails, déclencheurs de rencontre — distincte de ses grilles : une entité
  est un **objet** à type libre, placé sur une case et porteur de ses propres données, là où une
  grille ne retient qu'un type par case. Le type n'est **pas** interprété au chargement — c'est le
  gameplay qui lui donne un sens — mais la position est validée comme celle d'une tuile
  (`EX-LVL-004`). Concrétisé en `LOT-04`.
- \anchor EX-LVL-018 **EX-LVL-018** — Une couche et une entité doivent pouvoir porter un
  **dictionnaire de propriétés libres**, et tout champ **inconnu** du chargeur doit y être rangé :
  ignoré sans erreur à la lecture, et **réémis** à l'écriture. Sans quoi le moindre besoin
  découvert plus tard — terrain difficile, couverture, hauteur, dialogue d'un PNJ — imposerait une
  nouvelle version de format et la migration de tout le contenu déjà produit ; et un fichier écrit
  par une version ultérieure de l'éditeur perdrait ses champs au premier enregistrement par une
  version antérieure. Concrétisé en `LOT-04`.

- \anchor EX-LVL-009 **EX-LVL-009** — Le format de niveau doit porter la **liste ordonnée des
  plans** (`EX-DEC-040`) — nom de fichier, densité, facteurs de parallaxe, opacité et profondeur —
  ainsi qu'un drapeau de niveau décidant si la **parallaxe** s'applique (`EX-DEC-043`). Les champs à
  leur valeur par défaut ne sont **pas écrits**. Le champ `decors` du format précédent devient
  **obsolète** : un fichier qui le porte encore reste **valide** et se charge, le champ étant
  **ignoré avec un avertissement journalisé** nommant le fichier — jamais un rejet. Rejeter
  casserait tout niveau personnel existant, et convertir automatiquement un assemblage de sprites
  en surface peinte est impossible sans rastérisation, donc mentirait sur le résultat
  (`EX-LVL-004`, `EX-LVL-005`, `EX-NFR-040`). Concrétisé en `LOT-H-69`.

### Format retenu (JSON, liste de tuiles-objets)
Types de tuiles : `entry` (entrée), `exit` (sortie), `solid` (solide), `danger`, `switch`
(interrupteur), `pressurePlate` (plaque de pression, activation continue tant qu'un poids y
repose), `door` (porte), `block` (bloc poussable, `EX-GP-022` — déplaçable par le personnage,
retombe sous gravité), `slopeUpRight` et `slopeUpLeft` (pentes à 45°, `EX-GP-003` — surface suivie,
jamais solide pour la grille classique ; « Up » désigne le côté qui monte, `Right`/`Left`),
`roundedUpRight` et `roundedUpLeft` (variante **courbe** — quart de cercle — des pentes, `EX-GP-004`,
même orientation et même principe de suivi, formule de hauteur différente), `slopeDownRight`,
`slopeDownLeft`, `roundedDownRight` et `roundedDownLeft` (variantes de **plafond** des quatre
tuiles précédentes, `EX-GP-006` — miroir vertical de la même silhouette ; comme leurs équivalents
de sol, jamais solides pour la grille classique, mais une passe de suivi dédiée bloque précisément
un saut qui franchit leur profil incliné/courbe par en dessous, sans jamais y faire « marcher » le
personnage — leur face du haut, toujours plate, supporte normalement un personnage qui tombe
dessus par au-dessus),
`concaveUpRight`, `concaveUpLeft`, `concaveDownRight` et `concaveDownLeft` (variante **concave**
des arrondis, `EX-GP-007` — même principe de suivi/silhouette, sol et plafond, mais courbure
**inversée** : centre du cercle du côté plein plutôt que du côté creux),
`blockHalf` et `blockQuarter` (blocs poussables à taille **réduite** — `×0.5`/`×0.25` —
`EX-GP-005`, mêmes règles de poussée/chute que `block`, boîte de collision centrée et plus
petite), `dangerUp`/`dangerDown`/`dangerLeft`/`dangerRight` (danger **directionnel**, `EX-GP-050` —
mortel uniquement sur une bande étroite du bord désigné par le suffixe, pas la case entière),
`dangerMover` (danger **mobile**, `EX-GP-051` — aller-retour linéaire déterministe autour de sa
position de départ ; champs optionnels `axis`, `"horizontal"` ou `"vertical"`, et `range`, en
cases, défauts respectifs `"horizontal"` et `2`), `dangerSwitched` (danger **commuté**, `EX-GP-052`
— mortel uniquement quand l'interrupteur/la plaque de pression qui lui est lié est actif ; champ
`opensWith` comme `door`, même résolution par identifiant), `dangerBlink` (danger **temporisé**,
`EX-GP-053` — alterne mortel/inoffensif selon une période fixe ; champs optionnels `period`,
`phase` et `activeDuration`, en pas fixes, défauts respectifs `120`, `0` et `60`),
`sinkingBlock` (bloc **descendant**, `EX-GP-027` — armé par un contact quelconque du personnage,
puis descend à vitesse constante en portant ce qui repose dessus, jusqu'à buter sur la matière
pleine ou à sortir par le bas du tableau ; position **continue** comme `movingPlatform`, donc
jamais solide pour la grille classique, et **aucun champ** : sa vitesse est une constante du
moteur), `fragileBlock` (bloc **fragile**, `EX-GP-028` — solide, détruit par un ground pound
`EX-GP-058` qui l'atteint par le dessus et par ce geste seul ; aucun champ) et `vanishingBlock`
(bloc **éphémère**, `EX-GP-029` — solide, disparaît un délai fixe après que le personnage a cessé
d'y **reposer** ; aucun champ, le délai étant lui aussi une constante du moteur). Pour ces deux
derniers, la disparition est **définitive** jusqu'au rechargement du tableau et ne touche que la
grille de collision résolue, jamais la carte du niveau. Une case
**vide** n'est pas listée (absence = vide).
```json
{
  "name": "Tutoriel 1",
  "width": 12,
  "height": 8,
  "tiles": [
    { "x": 1, "y": 1, "type": "entry" },
    { "x": 9, "y": 6, "type": "exit" },
    { "x": 5, "y": 5, "type": "danger" },
    { "x": 8, "y": 3, "type": "switch", "id": "s1" },
    { "x": 10, "y": 5, "type": "door", "opensWith": "s1" }
  ]
}
```
Une carte `version: 3` ajoute deux tableaux racine **optionnels** : `layers`, les couches visibles
superposées au-dessus de la grille racine, et `entities`, les objets posés sur la carte
(`EX-LVL-016`, `EX-LVL-017`). Toute clé non reconnue y est conservée telle quelle et réécrite
(`EX-LVL-018`) — ci-dessous `difficultTerrain` et `dialogue`.
```json
{
  "version": 3,
  "name": "Village",
  "width": 12,
  "height": 8,
  "tiles": [
    { "x": 1, "y": 1, "type": "entry" },
    { "x": 9, "y": 6, "type": "exit" },
    { "x": 4, "y": 4, "type": "solid" }
  ],
  "layers": [
    { "name": "sol", "kind": "ground", "tiles": [{ "x": 4, "y": 4, "type": "solid" }] },
    { "name": "decor", "kind": "decor", "tiles": [{ "x": 5, "y": 4, "type": "danger" }],
      "difficultTerrain": true }
  ],
  "entities": [
    { "type": "npc", "x": 6, "y": 3, "dialogue": "bonjour" },
    { "type": "chest", "x": 2, "y": 7 }
  ]
}
```
Rôles de couche reconnus : `ground`, `decor`, et `legacy` (rôle de la grille racine promue, jamais
écrit) ; un rôle inconnu retombe sur `ground` plutôt que de faire échouer la carte (`EX-NFR-040`),
et `collision` déclaré est refusé (`EX-LVL-016`).

Coordonnées `x` = colonne, `y` = ligne, origine **haut-gauche** ; toute tuile hors des bornes
`width × height` est invalide. Les **liaisons** interrupteur↔porte se font par **identifiant**
(un `switch` porte un `id`, une `door` le référence via `opensWith`), schéma extensible à
d'autres mécanismes. L'exemple omet les tuiles `solid` des bords pour rester lisible.

## 2. Conception (lignes directrices)
- Introduire une mécanique à la fois ; le premier niveau sert de tutoriel implicite (sans texte).
- Aucune situation sans issue (le joueur ne doit jamais être bloqué définitivement sans échec possible).
- Chaque carte doit être **franchissable** : aucune zone jouable ne doit être inatteignable.

### Doctrine de profondeur (`LOT-H-65` TACHE-05)

Les trois lignes ci-dessus disent ce qu'un tableau ne doit pas être ; elles ne disent pas ce qu'il
doit **exiger**. Un tableau peut les respecter toutes et n'enseigner rien : il suffit que sa
mécanique soit posée à côté d'un chemin que l'on parcourt sans elle. C'est ce qui s'est produit avec
la première séquence du `LOT-H-65`, dont dix tableaux sur vingt-deux se franchissaient en maintenant
« droite ». Les quatre règles suivantes complètent donc les précédentes et sont **vérifiées
automatiquement** (`EX-LVL-015`, `Source/Test/Systeme`).

1. **Chemin critique.** La mécanique d'un tableau est la *seule* façon d'en atteindre la sortie.
   Un tableau franchissable sans employer son sujet ne le démontre pas — il le décore.
2. **Répétition.** Une mécanique se pose au moins **trois** fois dans le tableau qui l'introduit :
   une première fois sans risque pour la montrer, une deuxième pour la pratiquer, une troisième pour
   la varier. Une occurrence unique prouve qu'elle se charge, pas qu'elle se joue.
3. **Contrainte de capacité.** Le double saut, le wall jump et le dash sont acquis définitivement dès
   les premiers tableaux et permettent de passer par-dessus la plupart des énigmes. Un tableau borne
   donc explicitement ce qu'il autorise (`jumpBudget`/`dashBudget`, `EX-GP-024`) plutôt que de
   compter sur la bonne volonté du joueur.
4. **Introduire avant d'employer.** Aucune mécanique mortelle ou bloquante n'apparaît sur le chemin
   critique avant le tableau qui l'a présentée sans risque. Le jeu n'ayant pas de texte d'indice, la
   première rencontre avec un danger doit être évitable et lisible.

Corollaire pour les mécaniques de **plafond** et de **danger** : une tuile posée hors de portée du
personnage (un saut simple monte d'environ 2,4 tuiles) ne démontre rien, quand bien même elle
apparaît dans le fichier. La proximité au trajet réellement parcouru fait partie du contrôle.

## Exigences retirées {#lvl-retirees}

> Retirées par le `LOT-67`, qui retire du programme la notion de **séquence ordonnée de tableaux**.
> Les ancres sont **conservées** — jamais renumérotées, jamais supprimées : les lots hérités s'y
> réfèrent, et réécrire un lot livré falsifierait son histoire (règle de [`lots.md`](@ref lots)).
> Le texte ci-dessous est celui d'origine ; il décrit ce qui **a été** livré, pas ce qui est attendu
> aujourd'hui.
>
> Ces six-là décrivaient une **campagne** : un ordre, un enchaînement, un compte de tableaux, une
> progression qui avance de l'un à l'autre. Le jeu visé est un bac à sable — dix régions, une
> centaine de lieux, aucun ordre imposé. Ce qui les remplace n'est pas une autre liste : c'est le
> **graphe de cartes** du `LOT-09`, où l'on va où l'on veut, et la **sauvegarde riche** du `LOT-17`,
> qui retient un état de monde plutôt qu'un rang dans une file.

- \anchor EX-LVL-010 **EX-LVL-010** *(retirée en `LOT-67`)* — Le jeu doit charger les niveaux dans un **ordre défini** (liste ordonnée).
- \anchor EX-LVL-011 **EX-LVL-011** *(retirée en `LOT-67`)* — À la réussite d'un niveau, le jeu doit charger automatiquement le suivant ; après le dernier, revenir au menu (ou écran de fin).
- \anchor EX-LVL-012 **EX-LVL-012** *(retirée en `LOT-67`)* — Le jeu doit fournir des niveaux de démonstration à
  **difficulté croissante**, une mécanique introduite à la fois puis combinée dans des tableaux de
  synthèse (`EX-LVL-015`). Le « 3 niveaux » du MVP (déplacement/saut, danger, puzzle
  interrupteur↔porte) ne décrit plus le contenu livré depuis longtemps ; la séquence courante,
  vingt-deux tableaux couvrant l'intégralité des mécaniques du moteur, est décrite par
  `Source/Elements/Levels/README.md`. Concrétisé en `LOT-H-25`, étendu en `LOT-H-65`.
- \anchor EX-LVL-013 **EX-LVL-013** *(retirée en `LOT-67`)* — La **séquence** de niveaux jouée doit être une **donnée de
  contenu** (fichier de `Source/Elements/Levels`), jamais un littéral du code : réordonner, ajouter
  ou retirer un tableau ne doit demander aucune recompilation. Même exigence de validation et de
  version de format que les niveaux eux-mêmes (`EX-LVL-004`, `EX-LVL-005`) ; un niveau référencé mais
  absent est une **erreur récupérable** (`EX-NFR-040`). Concrétisé en `LOT-H-59`.
- \anchor EX-LVL-014 **EX-LVL-014** *(retirée en `LOT-67`, remplacée par la sauvegarde du `LOT-17`)* — La **progression** du joueur (tableau atteint, tableaux
  terminés) doit être **conservée entre deux lancements**, à la granularité du **tableau** et non de
  l'instant. Elle est stockée par **nom** de niveau — de sorte qu'un réordonnancement de la séquence
  (`EX-LVL-013`) ne la rende pas fausse — et se dégrade proprement : fichier absent, vide ou
  corrompu donne une partie neuve, sans erreur bloquante. Concrétisé en `LOT-H-59`.
- \anchor EX-LVL-015 **EX-LVL-015** *(retirée en `LOT-67`, reprise par le `LOT-49`)* — Le contenu livré doit **couvrir toutes les mécaniques** du
  moteur : chaque type de tuile et chaque mode de cadrage (`EX-LVL-006`) doit apparaître dans au
  moins un tableau de la séquence franchi par le test système (`EX-NFR-021`). La vérification est
  **automatique** et **dérivée des énumérations du code**, de sorte qu'ajouter une mécanique sans
  tableau qui l'emploie échoue sans qu'un inventaire ait à être tenu à la main ; les exclusions
  légitimes sont **nommées et justifiées**. Une mécanique absente de tout niveau n'est vérifiée
  qu'en isolation, jamais dans une partie réelle. Concrétisé en `LOT-H-65`.

## Traçabilité
Le chargement et la validation relèvent de `Source/Core` ; les fichiers de niveaux et l'atlas sont dans `Source/Elements`. Types de tuiles : [`gameplay.md`](gameplay.md).

Le découpage d'un niveau en **salles** pour la caméra (`EX-REN-015`, `LOT-H-32`) est un comportement
de **cadrage**, entièrement porté par `Source/HMI` : le format de fichier (`EX-LVL-003`) et la
validation (`EX-LVL-004`, dont l'invariant « une entrée, une sortie ») n'en portent aucune trace —
un niveau à salles reste, du point de vue de `Core`, une grille de tuiles ordinaire.
