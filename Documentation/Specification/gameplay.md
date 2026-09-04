# Gameplay {#spec-gameplay}

> Statut : **livré** (`0.1.0`). Mécaniques du MVP (déplacement, saut, mécanismes de puzzle,
> conditions de fin) et mécaniques aériennes avancées (double saut, wall jump, dash) toutes
> implémentées et couvertes par la séquence de démonstration (`LOT-H-65`). Reste ouvert : le
> **réglage fin du ressenti** (Sec. 2, valeurs marquées ⚠️ dans `Source/Core/Physics/PhysicsConfig.h`)
> n'est pas figé — reporté au-delà de `0.1.0`, le jeu restant jouable avec les valeurs actuelles.
> Dépend de [`vision.md`](vision.md).

## 1. Monde en tuiles
Le niveau est une **grille de tuiles** de taille fixe : **16 × 16 px** par tuile. Chaque cellule porte un type.

| Type de tuile | Comportement |
|---------------|--------------|
| Vide | Traversable. |
| Solide | Bloque le déplacement ; supporte le personnage. |
| Danger | Provoque l'échec au contact (pics, vide mortel). |
| Entrée | Position d'apparition du personnage. |
| Sortie | Déclenche la victoire au contact. |
| Pente | **Non solide** pour la grille classique (`isSolid` renvoie faux), mais sa surface **inclinée** est suivie : la position verticale du personnage se cale sur le profil de la pente à sa position horizontale plutôt que d'être simplement bloqué ou arrêté à son pied. |
| Arrondi | **Non solide** pour la grille classique (comme une pente), mais sa surface **courbe** (quart de cercle) est suivie — même principe de suivi que la pente, formule différente. |
| Pente/arrondi de plafond | **Non solide** pour la grille classique, comme les variantes de sol : miroir vertical de la même silhouette (matière pleine en haut de la case). Une passe de suivi dédiée bloque précisément un saut qui la franchit **par en dessous** (bonk contre le profil incliné/courbe réel), sans jamais faire « marcher » le personnage dessus ; sa **face du haut**, toujours plate, supporte normalement un personnage qui tombe dessus **par au-dessus**. |
| Arrondi concave | Même principe de suivi que l'arrondi (quart de cercle, sol **et** plafond), mais courbure **inversée** : centre du cercle du côté **plein** plutôt que du côté creux — un raccord en **creux** entre deux surfaces perpendiculaires, plutôt qu'un coin saillant arrondi. |
| Danger directionnel/mobile/commuté/temporisé | Variantes du danger (mortel au contact) : bande étroite sur un bord, position mouvante, activation liée à un interrupteur, ou clignotement périodique — cf. sous-section dédiée ci-dessous. |

- \anchor EX-GP-001 **EX-GP-001** — Le niveau doit être représenté par une grille de tuiles typées.
- \anchor EX-GP-002 **EX-GP-002** — Une tuile solide doit empêcher le personnage de la traverser.
- \anchor EX-GP-003 **EX-GP-003** — Une tuile de **pente** doit laisser le personnage **suivre** sa surface inclinée en marchant (la position verticale du personnage se cale sur la hauteur de la pente à sa position horizontale), pas seulement le bloquer ou l'arrêter à son pied. Elle n'est **jamais solide** pour la grille classique (`core::isSolid`) — seul ce suivi de surface (`core::resolveSlopeFollow`, @ref guide-physique) la rend praticable, sans quoi son bord haut agirait comme un mur invisible.
- \anchor EX-GP-004 **EX-GP-004** — Une tuile **arrondie** (quart de cercle) doit offrir le même suivi de surface que la pente (`EX-GP-003`), avec un profil **courbe** plutôt que linéaire. Comme la pente, elle n'est **jamais solide** pour la grille classique — seule la fonction de hauteur (`core::slopeSurfaceHeight`) change, la passe de résolution (`core::resolveSlopeFollow`) est réutilisée sans modification.
- \anchor EX-GP-005 **EX-GP-005** — Un **bloc poussable** (`EX-GP-022`) doit pouvoir avoir une taille **réduite** par rapport à une case pleine (facteurs `×0.5`/`×0.25`), pour permettre des défis de précision (sauts millimétrés) — en prévision de blocs poussables **plus grands** qu'une case (multi-cases), non couverts par cette exigence. Sa boîte de collision réelle est **centrée** dans sa case et résolue par une routine dédiée (`core::sweepAabbVsAabb`, @ref guide-niveaux), distincte du balayage sur grille : la poussée/chute restent case par case, inchangées par rapport à un bloc plein.
- \anchor EX-GP-006 **EX-GP-006** — Une tuile de **pente/arrondi de plafond** (`SlopeDownRight`/`SlopeDownLeft`/`RoundedDownRight`/`RoundedDownLeft`) doit exister comme variante **miroir vertical** des pentes/arrondis de sol (`EX-GP-003`/`EX-GP-004`) : matière pleine en **haut** de la case plutôt qu'en bas. Comme ses équivalents de sol, elle n'est **jamais solide** pour la grille classique (`core::isSolid`) — sa collision est résolue par deux passes symétriques : `core::resolveCeilingSlopeFollow` (miroir de `resolveSlopeFollow`, déclenchée en **montant**, `velocityY < 0`) bloque un saut qui franchirait sa silhouette **par en dessous** (bonk précis contre le profil incliné/courbe réel, pas une case pleine uniforme) ; sa **face du haut**, toujours **plate** au sommet de la case (`core::slopeSurfaceHeight` y renvoie `0`, quel que soit `localX`), supporte normalement un personnage qui tombe dessus **par au-dessus**, via `resolveSlopeFollow` réutilisé sans modification (sans quoi il tomberait au travers). Le personnage ne **marche** en revanche jamais **latéralement** le long de sa silhouette inclinée (`core::isFollowableSurface` reste `false`).
- \anchor EX-GP-007 **EX-GP-007** — Une tuile d'**arrondi concave** (`ConcaveUpRight`/`ConcaveUpLeft`, sol ; `ConcaveDownRight`/`ConcaveDownLeft`, plafond) doit offrir le même suivi de surface/silhouette que l'arrondi (`EX-GP-004`/`EX-GP-006`), avec une courbure **inversée** : centre du cercle du côté **plein** plutôt que du côté creux (tangente **horizontale** du côté creux, **verticale** du côté plein — l'exact inverse de l'arrondi convexe). Même rayon (une case), mêmes valeurs aux bords que l'arrondi convexe de même orientation ; seule la fonction de hauteur (`core::slopeSurfaceHeight`, `core::ceilingSlopeHeight`) change, les passes de résolution (`core::resolveSlopeFollow`/`core::resolveCeilingSlopeFollow`) sont réutilisées sans modification.

### Dangers avancés (`LOT-H-31`)
> La tuile `Danger` reste le danger **classique** : case pleine, statique, mortelle sur toute sa
> surface. Les quatre variantes ci-dessous étendent ce vocabulaire sans changer la règle de fin de
> niveau elle-même (`EX-GP-031` : contact = échec) — seule la **géométrie** ou l'**activation** du
> danger varie.
- \anchor EX-GP-050 **EX-GP-050** — Un **danger directionnel** (pics) doit être mortel uniquement
  depuis l'un des quatre bords de sa case (haut/bas/gauche/droite), le reste de la case restant
  traversable sans risque — une bande étroite le long du bord concerné, pas la case entière.
- \anchor EX-GP-051 **EX-GP-051** — Un **danger mobile** doit se déplacer de façon **autonome**
  (aller-retour le long d'un axe, sans intervention du personnage, à la différence du bloc
  poussable `EX-GP-022`) ; le contact avec sa position **courante** (pas sa position de départ dans
  le fichier) provoque l'échec.
- \anchor EX-GP-052 **EX-GP-052** — Un **danger commuté** doit être mortel uniquement quand
  l'interrupteur ou la plaque de pression qui lui est lié est **actif** — inverse de la porte
  (`EX-GP-021`, qui devient franchissable quand active), même infrastructure de liaison
  déclencheur↔cible.
- \anchor EX-GP-053 **EX-GP-053** — Un **danger temporisé** doit alterner mortel/inoffensif selon
  une **période fixe**, indépendamment de toute action du personnage ou d'un interrupteur — un
  déphasage par tuile permet des motifs (plusieurs dangers temporisés désynchronisés dans un même
  niveau).
- \anchor EX-GP-054 **EX-GP-054** — Une **plateforme mobile** doit pouvoir suivre une **route à N
  points** (la position de sa tuile, puis une suite de points de passage), parcourue soit en
  **aller-retour** (la route puis son inverse), soit en **circuit fermé** (le dernier point rejoint
  le premier en ligne droite, ce segment de fermeture faisant partie du cycle). La vitesse est
  constante sur toute la route, segment de fermeture compris, et la position reste fonction du seul
  **numéro de pas** (`EX-NFR-002`) — jamais d'accumulation. Une route vide décrit une plateforme
  immobile, pas un niveau invalide (`EX-NFR-040`). Concrétisé en `LOT-H-67`.
- \anchor EX-GP-055 **EX-GP-055** — Un **tableau** doit pouvoir redéfinir les **capacités** de
  mobilité du personnage rechargées à chaque contact avec le sol : nombre de **sauts aériens**
  (`EX-GP-015`) et nombre de **charges de dash** (`EX-GP-017`). À distinguer strictement du
  **budget** de `EX-GP-024`, qui se consomme une fois pour toutes sur l'ensemble du tableau et n'est
  jamais rechargé. Un tableau qui n'en déclare aucune conserve les réglages du moteur, à
  l'identique. Concrétisé en `LOT-H-67`.
- \anchor EX-GP-056 **EX-GP-056** — Le dash (`EX-GP-017`) doit pouvoir être **chargé** : maintenir
  **le bouton de dash** et la direction opposée à celle du prochain dash pendant un seuil
  configurable, puis déclencher le dash, doit produire un **dash boosté** (vitesse et/ou durée
  majorées par rapport au dash normal). Le bouton de dash maintenu (pas seulement la direction) est
  une garde **délibérée** : un simple changement de direction en cours de déplacement normal, sans
  intention de dasher, ne doit **jamais** amorcer de charge. Relâcher le bouton, changer de
  direction, ou atteindre le seuil sans les deux maintenus ensemble annule/empêche la charge en
  cours. Ne crée **aucune** charge de dash supplémentaire (`dashChargesRemaining`) ni budget
  (`EX-GP-024`) : seul le dash normalement consommé est modifié. Concrétisé en `LOT-H-72`.
- \anchor EX-GP-057 **EX-GP-057** — Un **bloc poussable** (`EX-GP-022`) percuté pendant un dash
  **boosté** (`EX-GP-056` — un dash normal pousse comme avant ce lot, sans régression) doit être
  repoussé de **plusieurs cases en un seul pas fixe** (au lieu d'une case par contact comme la
  poussée en marche normale), jusqu'au premier obstacle ou une distance maximale configurable, sans
  jamais traverser d'obstacle. Concrétisé en `LOT-H-72`.
- \anchor EX-GP-058 **EX-GP-058** — Le personnage doit pouvoir déclencher, **en l'air uniquement et
  sans charge de dash disponible** (`dashChargesRemaining <= 0` — tant qu'une charge existe, la même
  combinaison reste un dash vertical normal, `EX-GP-017`, sans régression sur un usage existant), un
  **ground pound** : une chute accélérée verticale à vitesse imposée, qui se termine au contact du
  sol. N'altère pas l'interaction déjà existante avec les mécanismes sensibles au poids
  (`EX-GP-025`) ; l'atterrissage déclenche la secousse caméra déjà existante pour tout impact lourd,
  sans code dédié. Concrétisé en `LOT-H-72`.
- \anchor EX-GP-060 **EX-GP-060** — Pendant un dash, la trajectoire doit **suivre les pentes et
  plafonds inclinés** (`EX-GP-003`/`EX-GP-004`/`EX-GP-006`/`EX-GP-007`) comme le déplacement normal,
  sans jamais clipper à travers leur surface. Vérifié à l'implémentation : la passe de suivi de
  pente (`core::resolveSlopeFollow`/`resolveCeilingSlopeFollow`) s'applique déjà à **chaque** pas de
  simulation, dash ou non (`CharacterPhysicsSystem::resolveCollisionAndState`, appelée
  inconditionnellement) — cette exigence documente et **teste explicitement** un comportement déjà
  correct par construction, plutôt que d'en ajouter un nouveau. Concrétisé en `LOT-H-72`.
- \anchor EX-GP-061 **EX-GP-061** — Le personnage doit disposer d'un **combo dash + saut** :
  sauter **pendant** un dash **boosté** (`EX-GP-056` — jamais un dash normal, qui continue de
  bufferiser le saut jusqu'à l'expiration du dash comme avant ce lot) doit y mettre fin
  immédiatement et **conserver** sa vitesse horizontale (jump-cancel) ; si ce jump-cancel a lieu au
  contact d'un mur, c'est un **wall jump** (`EX-GP-016`) qui doit se déclencher, pas un saut simple ;
  un saut déclenché dans une courte
  fenêtre après une poussée renforcée (`EX-GP-057`) doit hériter d'une fraction configurable de la
  vitesse horizontale du bloc ; des jump-cancels rapprochés doivent cumuler un bonus de vitesse
  **plafonné**, remis à zéro au contact du sol. Le nombre d'enchaînements reste borné par les
  charges/budget de dash déjà existants (`EX-GP-055`, `EX-GP-024`) — aucune charge supplémentaire
  n'est créée. Concrétisé en `LOT-H-72`.

## 2. Personnage & déplacement
- \anchor EX-GP-010 **EX-GP-010** — Le personnage doit se déplacer horizontalement à vitesse constante (~3 tuiles/s en jeu, `Source/Core/Physics/PhysicsConfig.h` `moveSpeed` — ⚠️ réglage fin non figé, reporté au-delà de `0.1.0`).
- \anchor EX-GP-011 **EX-GP-011** — Le personnage doit sauter : impulsion verticale puis retombée sous gravité constante.
- \anchor EX-GP-012 **EX-GP-012** — La gravité doit s'appliquer en continu tant que le personnage n'est pas au sol.
- \anchor EX-GP-013 **EX-GP-013** — Le personnage ne doit pouvoir sauter que lorsqu'il est **au sol** (pas de double saut au MVP).
- \anchor EX-GP-014 **EX-GP-014** — Les collisions personnage ↔ tuiles solides doivent être résolues sur les deux axes (pas de traversée à vitesse élevée — collision par balayage ou pas fixes).

### Mécaniques aériennes avancées (au-delà du MVP)
> Ces exigences **étendent** le MVP et **assouplissent** `EX-GP-013` (qui interdit le double saut) : non requises au MVP, elles visent un platformer aux mécaniques riches.
- \anchor EX-GP-015 **EX-GP-015** — Le personnage doit pouvoir effectuer un nombre **paramétrable** de sauts **aériens** supplémentaires (double/multi saut), **rechargés au contact du sol**.
- \anchor EX-GP-016 **EX-GP-016** — Au contact d'un mur en l'air, le personnage doit **glisser** le long de celui-ci (wall slide) ; un saut le propulse alors **en diagonale opposée** au mur (wall jump).
- \anchor EX-GP-017 **EX-GP-017** — Le personnage doit pouvoir **dasher** : une ruée directionnelle (**8 directions**) à vitesse élevée sur une **courte durée**, disponible une fois puis **rechargée au contact du sol**.
- \anchor EX-GP-018 **EX-GP-018** — Le ressenti vertical doit être affiné : **gravité de chute renforcée** (chute plus rapide que la montée), **flottement à l'apex** (gravité réduite quand la vitesse verticale est faible) et **fast-fall** (chute accélérée en maintenant « bas »). La retombée reste sous gravité **constante** (à multiplicateur près), conformément à `EX-GP-011`.
- \anchor EX-GP-019 **EX-GP-019** — Le personnage doit avoir une **masse** ; la vitesse de chute doit résulter de l'équilibre entre le **poids** (masse × gravité effective) et une **traînée** proportionnelle à la vitesse, faisant émerger une **vitesse terminale** progressive plutôt qu'un plafond arbitraire. La montée du saut n'est pas concernée (gravité simple, `EX-GP-011`).

### Ressenti (game feel) — ⚠️ réglage fin reporté au-delà de `0.1.0`
Cible visée, non encore atteinte : hauteur de saut ~2,5 tuiles, apex en ~0,35 s. Valeurs en jeu
(`Source/Core/Physics/PhysicsConfig.h`) : ~2,25 tuiles, apex ~0,3 s — jouable et couvert par la
séquence de démonstration (`LOT-H-65`), mais le fichier de constantes marque encore chaque valeur
« à affiner ». *Coyote time* (~80 ms) et *jump buffering* (~120 ms) sont en revanche **au
paramètre visé** depuis `LOT-H-09`.

## 3. Mécanismes de puzzle
Concrétise l'objectif produit `EX-VIS-003` (`vision.md`).
- \anchor EX-GP-020 **EX-GP-020** — Un **interrupteur** doit changer d'état quand le personnage l'active (contact ou action dédiée).
- \anchor EX-GP-021 **EX-GP-021** — Une **porte** liée à un interrupteur doit s'ouvrir/se fermer selon l'état de celui-ci. Une porte qui se **referme sur le personnage** provoque l'**échec** du niveau, exactement comme un écrasement sous une plateforme mobile (`EX-GP-026`) — jamais un personnage encastré dans un mur, ce qui serait une situation sans issue (`niveaux.md`, Sec. 3). Complété en `LOT-H-65`.
- \anchor EX-GP-022 **EX-GP-022** — Un **bloc poussable** doit pouvoir être déplacé horizontalement par le personnage et retomber sous gravité. Une case de pente/arrondi (`EX-GP-003`/`EX-GP-004`/`EX-GP-006`/`EX-GP-007`) est traitée comme un **obstacle simple** (comme une case solide) pour la poussée et la chute — le contrôleur de blocs n'a aucune notion de suivi de surface, contrairement au personnage.
- \anchor EX-GP-023 **EX-GP-023** — Une **clé** collectée doit ouvrir une **porte verrouillée** correspondante. Le ramassage exige le contact **et** l'action « Interagir » (`EX-CTRL-022`) — le contact seul, suffisant pour un interrupteur (`EX-GP-020`), ne suffit pas ici. Une fois ouverte, la porte le reste **définitivement** (contrairement à la porte liée à un interrupteur, qui peut se refermer).
- \anchor EX-GP-024 **EX-GP-024** — Un **tableau** peut **limiter** le nombre de **sauts** et/ou de **dashs** disponibles (budget de mouvements, défini par le niveau) ; à budget épuisé, l'action est **refusée**. Le budget est **réinitialisé** au (re)chargement du niveau. Contrainte de **puzzle**.
- \anchor EX-GP-025 **EX-GP-025** — Une **plaque de pression** doit maintenir la porte liée **ouverte** tant qu'un poids suffisant y repose, et la **refermer** dès qu'il en repart — activation **continue**, à la différence de l'interrupteur à bascule (`EX-GP-020`), dont le comportement n'est pas affecté. Ce poids peut être celui du **personnage** ou celui d'un **bloc poussable** (`EX-GP-022`) : c'est ce qui rend possible de poser un poids et de **repartir**, la porte restant ouverte. Un bloc de taille réduite (`EX-GP-005`) est trop léger pour l'enfoncer, sa masse valant son facteur de taille — la distinction est **visible** dans le tableau, jamais une propriété cachée. Complété en `LOT-H-65`.
- \anchor EX-GP-026 **EX-GP-026** — Une **plateforme mobile** doit parcourir un trajet à vitesse
  constante (une **route** au sens de `EX-GP-054` depuis le `LOT-H-67` ; deux points jusque-là), en
  **portant** le personnage et les blocs poussables (`EX-GP-022`)
  qui reposent dessus, sans traversée (`EX-GP-014`), sans glissement cumulé ni décollement. Sa
  position est fonction du **numéro de pas** de simulation — jamais d'une accumulation ni du temps
  réel — de sorte que le déterminisme (`EX-NFR-002`) soit préservé. L'ordre de résolution dans le pas
  (déplacer les plateformes, porter les entités posées, appliquer la physique du personnage) est
  **documenté et testé** ; le cas d'écrasement contre un plafond est **mortel** (décision de cadrage
  retenue, `LOT-H-63`), plutôt que de mettre la plateforme en pause.
- \anchor EX-GP-027 **EX-GP-027** — Un **bloc descendant** doit être **armé** par un contact
  quelconque du personnage (par le dessus, par le côté ou par le dessous — jamais un test de portage,
  qui obligerait à définir un seuil de « vraiment posé dessus »), puis **descendre** à vitesse
  constante en **portant** le personnage et les blocs poussables (`EX-GP-022`) qui reposent dessus,
  aux mêmes conditions qu'une plateforme mobile (`EX-GP-026` : sans traversée, sans glissement cumulé
  ni décollement). L'armement est un **aller simple** : il ne s'annule pas si le personnage s'éloigne,
  et le bloc ne repart jamais en arrière. Le bloc **ne traverse jamais la matière** — il s'arrête
  contre une case pleine et n'y repart pas si elle se libère —, et il est **retiré** du niveau s'il
  franchit le bord bas du tableau. Un personnage écrasé entre un bloc descendant et le sol provoque
  l'**échec** (`EX-GP-031`), même décision de cadrage que l'écrasement sous une plateforme mobile
  (`LOT-H-63`), plutôt que de mettre le bloc en pause. Sa position est **continue** (jamais alignée sur
  la grille) et fonction du seul **numéro de pas** écoulé depuis l'armement — jamais d'une
  accumulation (`EX-NFR-002`) ; il n'est donc **jamais solide** pour la grille classique
  (`core::isSolid`), sa collision étant résolue boîte-contre-boîte comme celle d'une plateforme
  mobile. Sa vitesse est une **constante du moteur**, comme le délai du bloc éphémère
  (`EX-GP-029`) et non une donnée de niveau : le constructeur de `core::Level` atteint déjà
  19 paramètres et sa surface est actée comme maximale, si bien qu'ouvrir ce réglage par tuile
  coûterait plus que ce qu'il apporte aujourd'hui. Le rendre réglable plus tard n'invalidera aucun
  fichier existant, un champ optionnel absent valant le défaut. Concrétisé en `LOT-H-74`.
- \anchor EX-GP-028 **EX-GP-028** — Un **bloc fragile** doit être **solide** comme une case pleine,
  et **détruit** par un **ground pound** (`EX-GP-058`) qui l'atteint **par le dessus** — par ce geste
  et par lui seul. Aucun autre contact ne le brise : ni la marche, ni le saut, ni un atterrissage
  rapide, ni un dash (horizontal, vertical ou boosté, `EX-GP-056`), ni la poussée d'un bloc
  (`EX-GP-022`/`EX-GP-057`). Cette restriction est **délibérée** : tant qu'une charge de dash existe,
  « dash + bas » reste un dash vertical normal (`EX-GP-058`), donc en faire aussi un geste de
  destruction rendrait le comportement du bloc dépendant du **budget de dash restant**, illisible
  pour le joueur. La destruction doit être résolue **avant** la physique du personnage sur le même
  pas, de sorte que le ground pound se poursuive au travers sans s'arrêter d'un pas sur un bloc qu'il
  vient de briser. Elle est **définitive** jusqu'au rechargement du tableau, et ne modifie jamais la
  carte du niveau elle-même, seulement la grille de **collision** résolue (même infrastructure que
  l'ouverture d'une porte, `EX-GP-021`). Concrétisé en `LOT-H-74`.
- \anchor EX-GP-029 **EX-GP-029** — Un **bloc éphémère** doit être **solide** tant que le personnage
  repose dessus, quelle que soit la durée, puis **disparaître** après un délai fixe une fois qu'il
  l'a **quitté** — un **front** de départ (le personnage reposait dessus au pas précédent et n'y
  repose plus), jamais un simple contact : passer dessous ou le toucher par le côté ne l'arme pas.
  Le délai est une constante du **moteur**, exprimée en pas fixes (`EX-NFR-002`), et non une donnée
  de niveau. Revenir dessus pendant le compte à rebours ne l'annule pas. La disparition est
  **définitive** jusqu'au rechargement du tableau, et se résout comme celle du bloc fragile
  (`EX-GP-028`), sur la grille de collision et non sur la carte du niveau. Le risque de rendre un
  tableau **insoluble** est assumé : il relève du level design, et le garde-fou système qui relève la
  trajectoire réelle (`EX-NFR-021`) le détecte. Concrétisé en `LOT-H-74`.

Chaque mécanisme est déterministe : à état d'entrée identique, comportement identique (facilite tests et rejouabilité).

## 4. Conditions de fin de niveau
Concrétise les objectifs `EX-VIS-002` (succès) et `EX-VIS-004` (échec/redémarrage), `vision.md`.
- \anchor EX-GP-030 **EX-GP-030** — Atteindre la tuile de **sortie** termine le niveau en **succès**.
- \anchor EX-GP-031 **EX-GP-031** — Le contact avec un **danger** ou la sortie des limites basses du niveau provoque l'**échec**.
- \anchor EX-GP-032 **EX-GP-032** — En cas d'échec, le niveau doit **redémarrer** à son état initial sans quitter le jeu.

## 5. États de jeu
- \anchor EX-GP-040 **EX-GP-040** — Le jeu doit gérer des états distincts : `Menu`, `EnJeu`,
  `Pause`, `NiveauTermine`. Portés par `hmi::ScreenId` (`Menu`, `Editor`, `Game`, `Options`,
  `Pause`, `NiveauTermine`, `LevelSelect` — les trois derniers ajoutés par ce lot ; `LevelSelect`
  pour la sélection de niveau, `EX-IHM-005`), avec des transitions explicites et unidirectionnelles
  (`EX-GP-041`). Détaillé côté interface par `EX-REN-031`. Concrétisé en `LOT-H-59`.
- \anchor EX-GP-041 **EX-GP-041** — Les transitions entre états doivent être explicites et unidirectionnelles à chaque événement (machine à états).

## Traçabilité
Contrôles associés : [`controles.md`](controles.md). Format des niveaux : [`niveaux.md`](niveaux.md). Ces exigences seront couvertes par des tests unitaires (`Core`) et système.
