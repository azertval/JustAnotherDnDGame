# TACHE-04 — Bloc fragile : destruction au ground pound {#lot-74-tache-04-bloc-fragile}

**Lot :** [LOT-74](epic.md) · **Emplacement :**
`Source/Core/Gameplay/VolatileBlockController.{h,cpp}` · **Statut :** fait

## Contexte
`EX-GP-028` : le bloc fragile est solide comme un `Solid`, et **détruit** par un ground pound
(`EX-GP-058`) qui l'atteint par le dessus — par lui seul.

C'est la cible que le ground pound n'a jamais eue. Livré au `LOT-72`, il n'a aujourd'hui d'autre
effet propre qu'une secousse caméra héritée de tout impact lourd, faute de matière fragile dans le
moteur — exclusion écrite deux fois dans le dossier du `LOT-72`, et que ce lot ferme.

Le bloc fragile ne bouge pas : il ne fait que **quitter** la grille de collision. Le patron est donc
celui de `core::MechanismController`, qui maintient déjà « une **copie mutable** du `TileMap` que la
physique consomme » pour ouvrir et fermer ses portes. Le contrôleur est **partagé** avec le bloc
éphémère (TACHE-05) : deux règles de retrait, un seul overlay.

## Travail à réaliser
- `core::VolatileBlockController`, logique **pure** : construction depuis le `Level` (repérage des
  tuiles `FragileBlock` et `VanishingBlock`), copie mutable du `TileMap`, `update(...)` par pas
  fixe, accesseur de grille de collision composable avec celle des mécanismes.
- **Règle de destruction** : un bloc fragile est retiré quand, au pas courant, le personnage est en
  ground pound (`core::Player::groundPounding`) **et** que le contact vient du **dessus** — sa boîte
  descend sur la face haute du bloc. Le test de face reprend le patron de portage
  (`core::restsOnTopOfPlatform`), pas une simple intersection : un ground pound qui frôle le bloc
  par le côté ne le brise pas.
- **Retrait définitif** jusqu'au rechargement : le bloc quitte l'overlay de collision et cesse d'être
  dessiné. `hmi::GameSession::reload()` reconstruit le contrôleur, donc remet le tableau à neuf sans
  code de remise à zéro dédié — le vérifier plutôt que de le supposer.
- Exposer la liste des blocs détruits **au pas courant**, pour que l'IHM puisse en tirer un effet
  (particules d'éclats, son) sans que le contrôleur ne connaisse ni l'un ni l'autre — même patron
  que les fronts déjà consommés par `hmi::GameSession`.

## Fichiers impactés
- `Source/Core/Gameplay/VolatileBlockController.{h,cpp}` (nouveaux),
  `Source/Core/Gameplay/README.md`, `Source/Core/CMakeLists.txt`.
- Câblage dans `hmi::GameSession` : voir TACHE-06.

## Tests (obligatoires)
- **Le ground pound brise** : personnage en ground pound arrivant par le dessus → le bloc quitte la
  grille de collision au pas suivant.
- **Rien d'autre ne brise** : marche dessus, saut dessus, atterrissage normal même rapide, dash
  horizontal, dash vertical **avec** charge disponible, poussée d'un bloc contre lui, ground pound
  qui passe à côté — le bloc survit dans tous ces cas. C'est le test le plus important de la tâche :
  il verrouille la décision de cadrage.
- **Solide tant qu'il existe** : le personnage se tient dessus et s'y cogne exactement comme sur un
  `Solid`.
- **Le pound se poursuit** : après destruction, la chute accélérée continue au travers sans arrêt
  d'un pas — voir « Points d'attention ».
- **Destruction durable** : le bloc ne revient pas ; il revient après `reload()`.
- **Déterminisme** (`EX-NFR-002`).

## Points d'attention
- **L'ordre dans le pas est la difficulté de cette tâche.** Le contrôleur doit tourner **avant**
  `_physics.update(...)`, à l'étape 4 du pas (avec `core::BlockController`), sur la boîte du
  personnage **d'avant** le pas — exactement l'argument que `BlockController::update` documente pour
  sa poussée (« la poussée doit libérer la case avant que la physique ne résolve le déplacement du
  personnage sur ce même pas »). Résolu après la physique, le ground pound s'arrêterait un pas sur
  un bloc qu'il est précisément censé traverser en le brisant, et le joueur verrait un à-coup.
- Ne pas confondre « en ground pound » et « vient d'atterrir » : la destruction se décide sur l'état
  `groundPounding` **pendant** la chute, pas sur le front d'atterrissage, qui arrive trop tard.
- Ne pas toucher à `CharacterPhysicsSystem` : le ground pound n'a besoin d'aucune connaissance des
  blocs fragiles ; c'est le bloc qui disparaît, pas le pound qui gagne un cas particulier.
- La destruction ne doit **jamais** modifier le `TileMap` du `Level` lui-même, qui est immuable et
  partagé — uniquement l'overlay du contrôleur, comme les portes.

## Définition de fait (DoD)
- Destruction fonctionnelle et **testée**, non-destruction par tous les autres gestes testée
  explicitement, continuité du ground pound vérifiée ; build `/W4 /WX`.

## Exigences
`EX-GP-028`, `EX-GP-058`, `EX-NFR-002`, `EX-ARCH-011`.
