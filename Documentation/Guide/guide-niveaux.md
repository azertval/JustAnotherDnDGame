# Niveaux : modèle, chargement, mécanismes, budgets {#guide-niveaux}

Un niveau est une **grille de tuiles typées** plus des métadonnées (nom, entrée, sortie,
mécanismes, budgets de mouvements). Tout vit dans `Source/Core/Levels` (le modèle en mémoire et son
chargement) et `Source/Core/Gameplay` (le comportement des mécanismes).

## Le modèle en mémoire

### Deux systèmes de coordonnées à ne pas confondre

Le moteur manipule **deux** représentations de position différentes, et le confondre est une source
d'erreurs fréquente :

- `core::GridPosition` : une paire d'**entiers** `(column, row)` — désigne une **case** de la
  grille, sans unité de mesure continue. C'est ce que manipule tout ce qui parle du niveau comme
  d'un plateau discret (entrée, sortie, position d'un interrupteur).
- `core::Vector2` (@ref guide-maths) : une paire de **flottants** — une position **continue** dans
  le monde physique, utilisée par la simulation (position du personnage, résultat d'un balayage).

Comme une tuile = 1 unité monde, convertir l'un vers l'autre est une simple conversion de type
(`column` devient `x`, `row` devient `y`) — mais les deux ne sont **jamais** interchangeables dans
le code : une case n'a pas de position « à mi-chemin », un personnage si.

### \ref core::TileType "core::TileType" : le vocabulaire des cases

Chaque case de la grille a l'un de ces types :

| Type | Rôle |
|------|------|
| `Empty` | Case traversable, par défaut (aucun contenu). |
| `Solid` | Bloque le déplacement en toute circonstance — un mur ou le sol. |
| `Danger` | Traversable, mais son contact est mortel. Ce que devient la mort d'un personnage — agonie, jets de sauvegarde — est le sujet du `LOT-72`. |
| `Entry` | Position d'apparition du personnage au chargement du niveau. |
| `Exit` | Sortie de la carte. Ne termine plus rien depuis le `LOT-67` ; redeviendra une transition vers une autre carte au `LOT-09`. |
| `Switch` | Interrupteur : son activation **bascule** l'état d'une `Door` liée (voir §« Mécanismes »). |
| `PressurePlate` | Plaque de pression : ouvre une `Door` liée **tant qu'un poids y repose** (`EX-GP-025`) — activation **continue**, pas de bascule. |
| `Door` | Porte : solide **fermée**, franchissable **ouverte** — son état dépend du `Switch`/`PressurePlate` lié. |
| `Block` | Bloc poussable (`EX-GP-022`) : solide comme un mur tant qu'il n'a pas bougé, mais peut être **déplacé** par le personnage et **retombe** sous gravité (voir §« Blocs poussables »). |
| `SlopeUpRight` | Pente à 45° (`EX-GP-003`) montant de **gauche à droite** : `Empty` pour toute autre logique (jamais solide), mais sa surface **inclinée** est suivie par la physique (@ref guide-physique). |
| `SlopeUpLeft` | Symétrique de `SlopeUpRight` : pente montant de **droite à gauche**. |
| `RoundedUpRight` | Variante **courbe** (quart de cercle, `EX-GP-004`) de `SlopeUpRight` : même orientation, même suivi de surface, formule de hauteur différente. |
| `RoundedUpLeft` | Symétrique de `RoundedUpRight` : arrondi montant de **droite à gauche**. |

Notez que `Door` n'est **pas** statiquement solide au sens de `core::isSolid(TileType)` — sa
solidité dépend de son **état**, calculé par le `MechanismController` (voir plus bas), pas du type
de tuile seul. Les pentes et arrondis ne sont, eux, **jamais** solides (`core::isSolid` renvoie
toujours faux) — voir @ref guide-physique pour le mécanisme de suivi qui les rend praticables.

### \ref core::TileMap "core::TileMap" : la grille

`core::TileMap` est une grille dense `width × height` de `TileType`, origine **haut-gauche** (même
convention que tout le moteur, @ref guide-maths). Elle expose `tile(colonne, ligne)` (lecture) et
`isSolid(colonne, ligne)` (utilisée directement par le balayage de collision, @ref guide-physique
§1). C'est une donnée pure, sans dépendance à un fichier ou à un rendu — testable isolément.

### \ref core::Level "core::Level" : le niveau assemblé

`core::Level` regroupe : un nom, une `TileMap`, une position d'**entrée** et de **sortie**
(`GridPosition`), une liste de `core::Mechanism` (liaisons interrupteur↔porte, résolues en
positions), des **budgets** de mouvements (`jumpBudget`/`dashBudget`, décrits plus bas) et un
**cadrage de caméra** résolu (`core::CameraFramingConfig`, `LOT-64`, détaillé plus bas). C'est
l'objet que le chargeur produit et que le reste du moteur (rendu, gameplay) consomme en lecture
seule — la `TileMap` d'un `Level` ne change **jamais** après le chargement ; c'est le contrôleur de
mécanismes (voir plus loin) qui maintient sa **propre** copie mutable pour représenter les portes en
cours de partie.

## Chargement JSON

Un niveau est décrit dans un fichier texte au format [JSON](https://www.json.org/json-fr.html) ⧉,
parsé par la bibliothèque **nlohmann/json**, dont l'usage est confiné au fichier `.cpp` du
chargeur — le reste du moteur ne dépend jamais directement de cette bibliothèque. La classe
`core::LevelLoader` expose deux points d'entrée statiques : `loadFromFile` (depuis un chemin) et
`loadFromString` (depuis du texte déjà en mémoire, pratique pour les tests).

### Exemple concret

```json
{
  "name": "Demonstration 4 (puzzle)",
  "width": 14,
  "height": 8,
  "jumpBudget": 2,
  "dashBudget": 1,
  "tiles": [
    { "x": 0, "y": 0, "type": "solid" },
    { "x": 0, "y": 1, "type": "solid" },
    { "x": 1, "y": 6, "type": "entry" },
    { "x": 3, "y": 6, "type": "switch", "id": "s1" },
    { "x": 7, "y": 6, "type": "door", "opensWith": "s1" },
    { "x": 11, "y": 6, "type": "exit" }
  ]
}
```

(exemple illustratif, inspiré de `Source/Elements/Levels/demo-interrupteur.json` — voir ce fichier
pour la version réelle et complète). À lire ainsi :

- `name`, `width`, `height` décrivent le niveau et les dimensions de sa grille ;
- `jumpBudget`/`dashBudget` sont **optionnels** — absents, le budget correspondant est **illimité**
  (voir plus bas) ;
- `tiles` est une **liste éparse** : seules les cases **non vides** (différentes de `Empty`) sont
  listées, chacune par ses coordonnées `x`/`y` (colonne/ligne) et son `type`. Une case absente de la
  liste est implicitement `Empty` ;
- les tuiles `switch` et `door` portent en plus un identifiant textuel : `id` **nomme**
  l'interrupteur, `opensWith` d'une porte **référence** cet identifiant. C'est ainsi qu'un
  interrupteur et sa porte sont **liés** dans le fichier — par un nom partagé, pas par leur position.
  Le chargeur résout ensuite cette liaison en un `core::Mechanism` reliant les deux
  `GridPosition` (l'identifiant textuel `"s1"` n'existe que dans le fichier ; en mémoire, le lien
  est purement géométrique).

### Validation

Le chargement **valide** le contenu (`EX-LVL-004`) avant de produire un `Level` utilisable :
dimensions cohérentes avec les tuiles listées, tuiles toutes dans les bornes de la grille, aucune
case dupliquée dans la liste, **exactement une** tuile `Entry` et **exactement une** tuile `Exit`
(un niveau sans sortie, ou avec deux entrées, est une erreur de contenu, pas une situation ambiguë à
tolérer), et chaque liaison de mécanisme **résolue** (un `opensWith` qui référence un `id`
inexistant est une erreur).

En cas d'échec — JSON malformé, champ manquant, type de tuile inconnu, échec d'une des validations
ci-dessus — le chargeur ne lève **jamais d'exception** vers l'appelant (`EX-NFR-040`) : il renvoie
un `core::LevelLoadResult`, une paire `{ optional<Level> level, std::string error }`. `ok()` indique
le succès ; en cas d'échec, `level` est vide et `error` décrit le problème de façon exploitable
(affichable au joueur ou au développeur). Ce choix — résultat récupérable plutôt qu'exception —
garde la gestion d'erreur explicite à chaque site d'appel, cohérent avec le reste du moteur qui ne
s'appuie pas sur les exceptions pour son flux de contrôle normal.

### Cadrage de caméra (`cameraFraming`, `LOT-64`)

Un champ racine optionnel, `"cameraFraming"`, porte le mode de cadrage choisi par le level designer
(`EX-LVL-006`) — la mécanique de ce cadrage (zone morte, anticipation, lissage du mode suivi) est
détaillée dans @ref guide-rendu, cette section ne couvre que sa place dans le **format** :

```json
"cameraFraming": { "mode": "perRoom", "roomWidthTiles": 20, "roomHeightTiles": 12 }
```

`mode` vaut `"wholeLevel"`, `"perRoom"` ou `"follow"` ; `roomWidthTiles`/`roomHeightTiles` sont lus
en mode `"perRoom"` **et** `"follow"` (`EX-REN-017` : la taille de vue du suivi réutilise les mêmes
champs que la taille de salle) et restent optionnels dans les deux cas (taille par défaut si
absents). **Champ absent** — tous les niveaux antérieurs à ce lot — le chargeur applique la **règle
de repli** (`core::resolveCameraFraming`) : elle reproduit exactement le comportement historique
(`"wholeLevel"` si le niveau tient dans une salle de taille par défaut, `"perRoom"` sinon), pour que
la rétrocompatibilité des niveaux existants reste garantie (`EX-LVL-005`). C'est pour ce champ que le
numéro de version de format est passé de `1` à `2` (`core::kLevelFormatVersion`) — le mécanisme de
repli lui-même n'exigeait pas ce bump (un champ absent se lit déjà sans erreur), mais la convention
du projet est de tracer chaque champ significatif ajouté au format.

`core::LevelWriter` n'émet le champ que si le cadrage **diverge** de ce que la règle de repli
recalculerait pour les dimensions du niveau : un niveau jamais retouché sur ce point reste sans le
champ après un aller-retour éditeur, exactement comme avant ce lot.

En mode `"perRoom"`, un tableau optionnel `"zones"` (`EX-LVL-007`) porte des rectangles de caméra
dessinés à la main par le level designer, chacun `{ "x", "y", "width", "height" }` en tuiles :

```json
"cameraFraming": {
  "mode": "perRoom",
  "zones": [
    { "x": 0, "y": 0, "width": 20, "height": 12 },
    { "x": 20, "y": 0, "width": 10, "height": 12 }
  ]
}
```

Ce tableau permet de **mélanger plusieurs tailles de caméra** dans un même niveau, là où
`roomWidthTiles`/`roomHeightTiles` seuls n'en autorisent qu'une, uniforme sur tout le niveau. La
résolution de la zone active (la **première** de la liste couvrant la position du personnage) est
un mécanisme de `HMI`, détaillé dans @ref guide-rendu — ce champ n'est ici qu'une **donnée**,
absente par défaut : un niveau sans `"zones"` (ou avec un tableau vide) garde exactement le
comportement de grille automatique `roomWidthTiles`/`roomHeightTiles` décrit ci-dessus.

## De la grille aux entités : \ref core::buildLevelScene "buildLevelScene"

Le `TileMap` d'un niveau n'est, en lui-même, qu'un tableau de types — il ne peut pas être
**affiché**. `core::buildLevelScene(world, level, régionParType)` fait le pont vers le rendu, en
projetant chaque tuile **non vide** en une **entité ECS** (@ref guide-ecs) portant un `Transform`
(position = colonne, ligne, converties en unités monde) et un `Sprite` (l'image à afficher, choisie
par la correspondance `régionParType` **injectée** en paramètre plutôt que codée en dur — ce qui
permet de tester la projection sans dépendre du rendu réel ou du GPU).

Point important : cette projection est **à sens unique** et n'a lieu **qu'une fois**, au chargement
du niveau. Le `TileMap` reste la **source de vérité** pour tout ce qui touche à la **collision** —
la physique (@ref guide-physique) interroge directement `isSolid(colonne, ligne)`, jamais les
entités-sprites générées. Modifier une entité-sprite ne changerait donc rien à la collision ; c'est
le mécanisme de portes ci-dessous qui montre comment un changement d'état du niveau doit réellement
se propager.

## Mécanismes déclencheur ↔ porte

`core::MechanismController` (logique **pure**, dans `Core/Gameplay`, sans dépendance rendu) donne
un **comportement** aux liaisons déclencheur↔porte que le modèle de niveau ne fait que
**représenter** (`EX-GP-020`, `EX-GP-021`, `EX-GP-025`). Deux types de déclencheur, deux
comportements — la nature de chacun est figée **une fois pour toutes** à la construction, d'après
la tuile d'origine (`TileType::Switch` ou `TileType::PressurePlate` à sa position) :

- à sa construction, le contrôleur **copie** la `TileMap` du niveau dans une grille de collision
  **mutable** qui lui est propre (`_collision`) — le `Level` d'origine, lui, ne change jamais.
  Chaque porte y est posée **fermée**, c'est-à-dire remplacée par `TileType::Solid`, au départ ;
- `update(playerBox, playerMass)` est appelé chaque pas fixe avec la boîte englobante du
  personnage et sa masse (`core::Player::mass`, `EX-GP-019`) :
  - **interrupteur** (`Switch`) : si la boîte **recouvre** la case, et que ce recouvrement vient de
    **commencer** ce pas-ci (un **front** — pas un contact déjà en cours, sinon rester debout sur
    l'interrupteur ferait osciller la porte à chaque pas), l'état de la porte **liée** **bascule**
    (ouverte ↔ fermée) — inchangé depuis LOT-12 ;
  - **plaque de pression** (`PressurePlate`) : la porte liée est ouverte **si et seulement si** la
    boîte recouvre la plaque **et** que la masse du personnage atteint le seuil `MIN_TRIGGER_MASS`
    (constante interne, calée par défaut sur la masse par défaut du personnage) — réévalué à
    **chaque** pas, sans notion de front : rester dessus la garde ouverte, en partir la referme
    **immédiatement**.
- à chaque changement d'état, la grille `_collision` est mise à jour en conséquence : porte
  ouverte → `TileType::Door` (franchissable), porte fermée → `TileType::Solid` (bloquante) ;
- la **physique** ne consomme jamais directement la `TileMap` du `Level` : elle reçoit
  `collisionMap()`, la grille du contrôleur. C'est ce fil qui fait qu'une porte fermée **bloque**
  réellement le personnage, sans jamais muter la carte d'origine du niveau — le `Level` reste la
  donnée statique de référence, le `MechanismController` porte l'état **dynamique** de la partie en
  cours.

Concrètement, avec l'exemple JSON ci-dessus : la porte en `(7, 6)` est solide tant que
l'interrupteur `s1` en `(3, 6)` n'a pas été touché ; dès que le personnage marche dessus, elle
devient franchissable — et se refermerait si l'interrupteur était retouché après être sorti du
contact (nouveau front). Une **plaque de pression** liée à la même porte, elle, la garderait
ouverte pas à pas tant que le personnage y resterait, sans qu'un second passage soit nécessaire
pour la refermer.

Le seuil de poids ne **discrimine** encore rien tant qu'un seul acteur (le personnage) pèse sur une
plaque — toute plaque s'active « prête à l'emploi ». Les blocs poussables (ci-dessous) existent
désormais, mais `MechanismController` ne les interroge pas encore : un bloc posé sur une plaque de
pression ne l'active pas — l'infrastructure de comparaison de poids est prête à l'accueillir, mais
le câblage bloc → plaque reste une évolution à venir.

### Clé et porte verrouillée (`EX-GP-023`, `LOT-63`)

`TileType::Key`/`LockedDoor` sont une **troisième** paire déclencheur↔cible, résolue par la
**même** liaison `core::Mechanism` que interrupteur/plaque↔porte ci-dessus — ajoutée au **même**
vecteur (`Level::mechanisms()`), aucune notion de liaison dupliquée. `MechanismController` fige la
nature de chaque déclencheur à la construction (comme `_continuous` pour interrupteur/plaque) :
une clé se distingue par son `TileType::Key` d'origine.

Deux différences de comportement, câblées directement dans `MechanismController::update` :

- **ramassage** : contrairement à l'interrupteur (contact seul), une clé exige le contact **et**
  le front de l'action « Interagir » (`core::PlayerInput::interactPressed`, `EX-CTRL-022`) —
  c'est le premier usage réel de cette action ;
- **irréversibilité** : une fois ramassée, la porte liée s'ouvre et **le reste** — jamais de
  re-fermeture, contrairement à l'interrupteur qui bascule. `_switchOn[index]` ne repasse donc
  jamais à `false` une fois vrai pour un mécanisme de type clé.

La grille de collision est mise à jour exactement comme pour une porte classique (`Solid` fermée,
type d'origine — `Door` ou `LockedDoor` — une fois ouverte), via le même champ `_openType` capturé
à la construction (avant que le constructeur ne fige toutes les portes en `Solid`).

## Blocs poussables

`core::BlockController` (logique **pure**, dans `Core/Gameplay`, sans dépendance rendu) fait vivre
les tuiles `TileType::Block` (`EX-GP-022`) : contrairement aux mécanismes ci-dessus, dont seul
**l'état** change, un bloc change de **position**. Chaque bloc occupe exactement **une case**,
jamais à mi-chemin — pousser ou tomber le déplace d'une case entière, comme une porte bascule d'un
état à l'autre sans étape intermédiaire.

- **Poussée** : `update(playerBox, moveIntentX, baseCollision)` teste si la boîte du personnage
  touche un bloc du côté vers lequel `moveIntentX` (@ref guide-entrees) l'entraîne ; si la case
  suivante dans cette direction est libre (ni solide, ni un autre bloc), le bloc avance d'une case.
  Appelé **avant** la physique du personnage (`hmi::GameSession::update`), avec la boîte **laissée
  par le pas précédent** : un bloc qui vient de se dégager ne bloque donc jamais le personnage sur
  ce même pas — la case est déjà libre quand le balayage de collision (@ref guide-physique) s'exécute.
- **Chute** : un bloc dont la case du dessous est libre tombe d'une case, au rythme d'une case
  toutes les `BlockController::FALL_INTERVAL_STEPS` pas fixes — une chute **discrète** (case par
  case), pas une intégration continue comme celle du personnage (@ref guide-physique §2) :
  cohérent avec le reste des mécanismes de ce moteur, tous résolus case par case plutôt qu'en
  unités monde continues.

`collisionMap(base)` complète une grille déjà résolue par `core::MechanismController` (portes) avec
la position **courante** des blocs **pleins** : chaque case marquée `Block`/`BlockHalf`/
`BlockQuarter` dans `base` — qui ne porte que la position de **départ**, jamais mise à jour — est
d'abord effacée, puis chaque position **courante** d'un bloc **plein** seulement est reposée comme
solide. Sans cet effacement, une case quittée par un bloc resterait perçue comme un mur,
indéfiniment (un bug réellement rencontré en écrivant ce contrôleur, corrigé avant livraison).

### Blocs à taille réduite (`×0.5`/`×0.25`)

`EX-GP-005` demande des blocs poussables plus **petits** qu'une case pleine (`TileType::BlockHalf`/
`BlockQuarter`), pour des défis de précision (sauts millimétrés) en préparation de futurs blocs
poussables **plus grands** qu'une case. `BlockController` les reconnaît au même titre que `Block`
(même poussée, même chute, toujours **case par case** — `EX-GP-005` ne change jamais le
déplacement, seulement la boîte de collision), avec un facteur de taille associé
(`scales()`, même index que `positions()`) : `1` pour `Block`, `0.5`/`0.25` pour les tailles
réduites. La boîte **réelle** d'un bloc (`boxAt(index)`) est **centrée** dans sa case : une marge
`(1 - facteur) / 2` de chaque côté, laissant un espace vide symétrique tout autour.

**Pourquoi une routine de collision séparée.** `core::sweepAabb` (le balayage sur grille, @ref
guide-physique) raisonne en cases **entières** : solide ou vide, jamais « partiellement occupée ».
Marquer la case d'un bloc réduit comme solide dans `collisionMap` bloquerait donc à tort l'espace
vide qui l'entoure — exactement le défaut que `EX-GP-005` cherche à éviter (un bloc `×0.5` qui
occupe toute sa case au sens de la collision ne serait, à l'usage, pas différent d'un bloc plein).
`collisionMap` laisse donc les cases des blocs réduits **franches** dans la grille classique ; leur
collision réelle est résolue par une seconde routine, `core::sweepAabbVsAabb`
(`Core/Physics/AabbVsAabb.h`, quart de cercle mis à part — une simple boîte fixe, pas une surface),
composée par `hmi::GameSession::update` **après** le balayage sur grille : le déplacement réellement
obtenu par la grille est retesté contre la boîte réelle de chaque bloc réduit, la restriction la
plus stricte des deux l'emportant toujours (cette seconde passe ne peut que réduire davantage le
déplacement, jamais l'étendre — la grille reste la référence pour les murs/blocs pleins).

Cohérence stricte entre le rendu et la collision : `hmi::GameSession::refreshBlockVisuals` calcule
la **même** marge (`(1 - facteur) / 2`) pour positionner et mettre à l'échelle le sprite d'un bloc
réduit — le sprite affiché correspond donc exactement, par construction, à la boîte réellement
testée, sans risque de divergence entre deux calculs indépendants.

## Plateformes mobiles (`EX-GP-026`, `LOT-63`)

`core::PlatformController` (logique **pure**, dans `Core/Gameplay`) fait vivre les tuiles
`TileType::MovingPlatform` : contrairement aux blocs poussables ci-dessus (position **case par
case**), la position d'une plateforme est **continue**, fonction **déterministe** du nombre de pas
fixes écoulés depuis le chargement (`EX-NFR-002`) — jamais d'accumulation flottante
(`position += vitesse * dt`, qui dériverait sur une session longue).

Depuis le `LOT-67` (`EX-GP-054`), le trajet est une **route à N points** : `startPosition` est le
point de départ, `MovingPlatformConfig::waypoints` liste les points suivants, et `mode` choisit
entre l'**aller-retour** (la route puis son inverse, comportement historique généralisé) et le
**circuit fermé** (`Loop` : le dernier point rejoint le premier en ligne droite, ce segment de
fermeture faisant partie du cycle et se parcourant à la même vitesse). La vitesse (`speed`, cases
par seconde) est constante sur toute la route, et le déphasage optionnel (`phase`, en pas fixes —
même principe que `DangerBlinkConfig::phase`) désynchronise plusieurs plateformes d'un même niveau.

La géométrie vit dans `core::PlatformPath` (`Core/Gameplay/PlatformPath.h`), **partagé** avec
l'overlay d'édition : le trajet dessiné dans l'éditeur est littéralement celui que le gameplay
parcourt, jamais une réimplémentation parallèle. Les longueurs cumulées sont précalculées au
chargement (`boxAtStep` est appelée plusieurs fois par pas et par consommateur) et la distance
parcourue est cumulée en **double précision** : convertie en `float`, elle perdrait le bit de poids
faible au-delà d'environ 16,7 millions de pas (~77 h), ce qui décalerait visiblement la plateforme
en fin de longue session. Un segment de longueur nulle (point dupliqué) est traversé sans incident,
et une route vide décrit une plateforme immobile plutôt qu'un niveau invalide (`EX-NFR-040`).

Un fichier écrit avant le multi-points (couple `endX`/`endY`) reste **lu** tel quel et converti en
route à un point : son comportement est inchangé (`EX-LVL-008`). L'éditeur, lui, réécrit toujours en
`waypoints`.

**L'ordre de résolution dans le pas est la décision structurante**, documentée ici parce qu'un
autre ordre produit des défauts subtils et intermittents (`hmi::GameSession::update`,
`Source/Test/Systeme/test_parcours_complet.cpp`, même composition) :

1. `PlatformController::update()` avance **en premier** — toutes les plateformes du niveau
   atteignent leur position de ce pas avant que quoi que ce soit d'autre ne s'exécute.
2. Les blocs poussables reposant sur une plateforme sont **portés** avec elle
   (`BlockController::update(..., platforms)`) : un déplacement infra-case est accumulé par bloc
   et converti en poussée d'une case entière dès qu'il atteint `1.0`, pour rester cohérent avec le
   reste du contrôleur (jamais de position infra-case pour un bloc).
3. Le personnage est **porté** s'il reposait sur une plateforme au pas précédent (translation
   directe de sa position, avant que sa propre physique ne s'applique) — vérifié purement par
   géométrie (`core::restsOnTopOfPlatform` contre la position de la plateforme **au pas
   précédent**), sans état à mémoriser d'un pas à l'autre.
4. La physique du personnage s'applique normalement sur la grille (murs, sols), **puis** sa
   collision continue contre chaque plateforme est résolue (`core::sweepAabbVsAabb`, même patron
   que les blocs réduits ci-dessus) — c'est cette seconde passe qui pose le personnage sur le
   dessus d'une plateforme la première fois (avant que le portage n'ait quoi que ce soit à faire),
   et qui empêche toute traversée quelle que soit la vitesse de la plateforme.

**Écrasement** : une plateforme montante contre un plafond, avec le personnage entre les deux, est
**mortelle** (`core::Player::squished`, décision de cadrage retenue) — plutôt que de mettre la
plateforme en pause, ce qui casserait sa position purement fonction du numéro de pas.

> ⚠️ **Défaut connu, non corrigé (`LOT-65`, consigné dans `CHANGELOG.md`)** : la seule
> **présence** d'une configuration `MovingPlatformConfig` dans un niveau — même **immobile**
> (`speed = 0`) et géométriquement **loin** du personnage — casse la résolution de collision
> pendant qu'un autre personnage suit une **pente** ailleurs dans ce même niveau (constaté :
> une chute erronée en pleine ascension d'une pente, alors qu'aucune plateforme n'est en jeu à
> cet endroit). Isolé par bissection (retirer uniquement la tuile `movingPlatform` d'une copie de
> niveau suffit à faire disparaître l'échec) ; racine non creusée plus loin, décision de cadrage du
> `LOT-65` (consigner, pas corriger en cours de refonte de contenu). En pratique : éviter de
> combiner une plateforme mobile et une pente dans un même fichier de niveau tant que ce défaut
> n'est pas corrigé.

## Budget de mouvements et capacités du tableau

Deux notions **distinctes**, à ne jamais confondre — c'est la confusion la plus facile à faire ici,
et le panneau « Propriétés » de l'éditeur les sépare en deux groupes explicitement libellés pour
cette raison :

| | Champs JSON | Sémantique |
|---|---|---|
| **Budget** (`EX-GP-024`) | `jumpBudget`, `dashBudget` | Total consommable sur **tout le tableau**, jamais rechargé ; `-1` = illimité. Réinitialisé au (re)chargement du niveau. |
| **Capacité** (`EX-GP-055`, `LOT-67`) | `airJumps`, `dashCharges` | Nombre de sauts aériens / de dashs **rechargés à chaque contact avec le sol** ; absent = réglage du moteur (`core::PhysicsConfig`). |

Les capacités sont appliquées par `hmi::GameSession::loadLevel`, qui construit une
`core::PhysicsConfig` dérivée du niveau et la pose sur le système de physique
(`CharacterPhysicsSystem::setConfig`) **avant** de faire apparaître le personnage — sa recharge
initiale en dépend. Le dash porte désormais un compteur de charges
(`core::Player::dashChargesRemaining`) et non plus un booléen : un tableau peut en accorder
plusieurs par saut. Valeur par défaut `1`, soit le comportement historique à l'identique.

Un tableau **puzzle** peut vouloir limiter délibérément le nombre de sauts et/ou de dashs
disponibles, pour forcer le joueur à les utiliser avec parcimonie plutôt que librement
(`EX-GP-024`). Les compteurs eux-mêmes vivent dans le composant `core::Player`
(`jumpsRemaining`/`dashesRemaining`, @ref guide-ecs) — pas dans le `Level` : le niveau ne fait que
fournir la **valeur initiale** du budget (`jumpBudget`/`dashBudget`), copiée dans le composant au
moment où le personnage apparaît (spawn). Ensuite :

- chaque saut ou dash effectué **décrémente** le compteur correspondant (voir @ref guide-physique
  §3 et §4) ;
- une action dont le compteur est tombé à **zéro** est **refusée** par la physique, exactement comme
  si les autres conditions (au sol, dash disponible, …) n'étaient pas remplies ;
- la valeur spéciale **`-1`** signifie « illimité » — aucun décompte n'a lieu, et c'est la valeur
  par défaut d'un niveau qui ne précise pas de budget (voir l'exemple JSON : un niveau **sans**
  `jumpBudget`/`dashBudget` a des sauts et dashs illimités) ;
- les compteurs sont **réinitialisés** à chaque (re)chargement du niveau — un échec (retour au menu
  ou nouvelle tentative) rend donc le budget complet à nouveau, jamais définitivement épuisé d'une
  tentative à l'autre.

## Dangers avancés (`LOT-31`)

Quatre variantes étendent le danger classique (`TileType::Danger`, case pleine et statique) sans
toucher à la règle de contact mortel elle-même — seules la **géométrie** ou
l'**activation** varient. Chacune vit dans une couche différente, selon que sa mortalité est
**géométrique** (résolue directement par `Core/Levels`, sans état) ou **temporelle** (résolue par
un contrôleur de `Core/Gameplay`, qui possède un état à faire vivre chaque pas fixe) :

- **Directionnel** (`DangerUp`/`DangerDown`/`DangerLeft`/`DangerRight`, `EX-GP-050`) : purement
  géométrique, aucun contrôleur. `core::dangerHitbox(type, col, row)` (`Core/Levels/
  DangerGeometry.h`) renvoie une bande étroite (`kDangerEdgeThickness`, un quart de case) alignée
  sur le bord désigné par le suffixe, au lieu de la case pleine — seule source de vérité, partagée
  par `core::evaluateOutcome` (ci-dessous) et le rendu (`core::buildLevelScene`,
  `hmi::DraftRenderer`), même garantie de non-divergence que `core::tileVisualScale` pour les blocs
  réduits.
- **Mobile** (`TileType::DangerMover`, `EX-GP-051`) : `core::DangerController` (nouveau, `Core/
  Gameplay`, aux côtés de `MechanismController`/`BlockController`) fait progresser un compteur de
  pas fixes et en dérive un **aller-retour triangulaire déterministe** (0 → portée → 0) sur l'axe
  de sa configuration (`core::DangerMoverConfig::axis`/`range`), à une vitesse de conception fixe
  (`MOVER_SPEED_CELLS_PER_SECOND = 2`, `DangerController.cpp`) — pas d'accumulation flottante,
  seulement une fonction du nombre de pas écoulés, comme `core::BlockController::FALL_INTERVAL_STEPS`
  (déterminisme, `EX-NFR-002`). `moverBox(index)` renvoie sa boîte **courante**, en unités de
  grille — sa position **de départ** dans le fichier (`DangerMoverConfig::startPosition`) ne bouge
  jamais, seule cette boîte calculée compte pour la collision.
- **Commuté** (`TileType::DangerSwitched`, `EX-GP-052`) : résolu par `core::MechanismController`
  lui-même, **étendu** plutôt que dupliqué dans `DangerController` — un danger commuté a besoin de
  la **même** détection front/continu (interrupteur vs plaque de pression) qu'une porte, déjà
  écrite dans `MechanismController::update`. Le contrôleur porte donc aussi `Level::dangerLinks()`
  (`core::DangerLink`, miroir de `core::Mechanism` — struct **dupliquée**, pas une généralisation :
  `Mechanism::doorPosition` est consommé par nom dans une trentaine de sites tous spécifiques à une
  porte, le renommer en un champ générique n'aurait profité qu'à ce second cas) et expose
  `isDangerActive(GridPosition)` — actif quand le déclencheur lié l'est, **inverse** d'une porte
  (qui devient franchissable quand active).
- **Temporisé** (`TileType::DangerBlink`, `EX-GP-053`) : `core::DangerController::isBlinkActive
  (position)`, formule déterministe `(pasFixe - phase) mod period < activeDuration` — un déphasage
  différent par tuile permet des motifs désynchronisés dans un même niveau, sans dépendance à un
  interrupteur.

`TileMap` ne portant qu'un `TileType` par case (aucune métadonnée numérique, limite déjà actée en
`LOT-19` pour le poids d'une plaque de pression), les paramètres du mobile et du temporisé
(`DangerMoverConfig`/`DangerBlinkConfig`) vivent dans des vecteurs annexes de `Level`/`LevelDraft`,
keyés par position — même patron que `Mechanism`/`DangerLink`, pas une extension de `TileMap`.

## Issue et enchaînement

`core::evaluateOutcome(playerBox, level, extraDangerBoxes = {})` est une fonction **pure
d'observation** (elle ne modifie rien, ne déclenche aucune transition) qui **classe** l'état
courant du niveau à partir de la position du personnage, en trois issues possibles
(`core::LevelOutcome`) :

- `Won` : la boîte du personnage recouvre la case `Exit` ;
- `Lost` : contact avec un danger **statique** (`Danger` classique ou l'une des quatre variantes
  directionnelles, résolues via `core::dangerHitbox` directement depuis la grille), **ou**
  recouvrement d'une des boîtes de `extraDangerBoxes`, **ou** le personnage est tombé sous la
  limite basse de la grille (une chute hors du niveau, typique d'un plateformer, sans mur invisible
  artificiel à poser en bas de chaque niveau) ;
- `Playing` : aucun des cas précédents — la partie continue.

`extraDangerBoxes` couvre les dangers **à état** (mobile/commuté/temporisé, `LOT-31`) :
`Core/Levels` n'a et ne doit pas avoir de dépendance vers `Core/Gameplay` (c'est l'inverse qui est
vrai — `MechanismController`/`DangerController` incluent déjà `Level.h`), donc `evaluateOutcome` ne
peut pas interroger ces contrôleurs lui-même. L'**appelant**, qui les possède déjà, assemble leurs
boîtes actuellement mortelles et les passe en paramètre —
`hmi::GameSession::collectActiveDangerBoxes` en jeu (mover : `DangerController::moverBox` pour
chaque configuration ; commuté : `DangerController`/`MechanismController::isDangerActive` par
position, converti en boîte via `dangerHitbox` ; temporisé : `DangerController::isBlinkActive` par
position, même conversion). Un appelant qui ignore ces variantes (ou un niveau qui n'en a aucune)
laisse simplement le paramètre à sa valeur par défaut, `{}` — comportement inchangé.

L'ordre de classement est **déterministe et volontaire** : si, au même pas, le personnage se trouve
à la fois sur la sortie et au contact d'un danger (situation limite mais possible géométriquement),
l'**échec l'emporte sur le succès** — une règle simple et prévisible plutôt que dépendante de
l'ordre de test interne.

Cette fonction ne fait que **classer** l'état ; c'est côté présentation qu'il a un effet. Et depuis
le `LOT-67`, cet effet est volontairement **minimal** : une issue `Won` en partie ne termine plus
rien — il n'y a ni tableau suivant, ni écran de fin — elle ramène au menu, faute de destination.

C'est un provisoire, et il est assumé plutôt que déguisé : la sortie reste dessinée dans les cartes
et redeviendra une **transition** vers la carte que le graphe du `LOT-09` désignera. Y laisser un
écran de fin de séquence aurait été plus spectaculaire et plus faux.

## Ce que le `LOT-67` a retiré

Ce guide décrivait ici deux mécanismes de plus : la **séquence de niveaux** comme donnée de contenu
et le **garde-fou de couverture des mécaniques**, qui vérifiait que chaque type de tuile
apparaissait dans un tableau franchi par le test système. Les deux supposaient une séquence
ordonnée, retirée avec la notion de niveau discret (`EX-LVL-010` → `EX-LVL-015`, consignées
« retirées » dans [`niveaux.md`](@ref spec-niveaux) plutôt que supprimées).

Ce qui les remplace n'est pas encore écrit : le **graphe de cartes** du `LOT-09` dira comment on
passe d'une carte à l'autre, et le **contrôle de cohérence du contenu** du `LOT-49` reprendra la
question de la couverture — sur un catalogue de cartes, pas sur une file.

## Voir aussi
- `core::Level`, `core::TileMap`, `core::TileType`, `core::LevelLoader`, `core::LevelLoadResult`.
- `core::buildLevelScene`, `core::MechanismController`, `core::BlockController`, `core::DangerController`, `core::PlatformController`, `core::dangerHitbox`, `core::evaluateOutcome`, `hmi::GameSession`.
- @ref guide-ecrans — la machine à états des écrans, et ce que le `LOT-67` en a retiré.
- @ref guide-physique — comment le balayage consomme `isSolid`/`collisionMap()`.
- @ref guide-ecs — le composant `core::Player` qui porte les compteurs de budget.
