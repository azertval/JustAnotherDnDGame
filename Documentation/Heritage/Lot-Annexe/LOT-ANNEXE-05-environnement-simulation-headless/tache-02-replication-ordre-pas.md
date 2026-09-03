# TACHE-02 — step : réplication exacte de l'ordre de composition par pas {#lot-annexe-05-tache-02-replication-ordre-pas}

**Lot :** [LOT-ANNEXE-05](epic.md) · **Emplacement :** `Source/AiSolver/Env` · **Statut :** à faire

## Contexte
`playLevel()` (`Source/Test/Systeme/test_parcours_complet.cpp`) est la référence normative de
l'ordre de composition d'un pas de simulation, elle-même alignée sur `hmi::GameSession::update`
(`Source/HMI/Game/GameSession.cpp`, étapes 1bis à 5). Cette tâche implémente
`HeadlessLevelEnvironment::step` en reproduisant cet ordre **exactement**, étape par étape, avec
l'ajout du
`core::DangerController` (présent chez `GameSession`, absent de `playLevel()` — voir la décision de
cadrage de l'épic).

## Travail à réaliser
- **`aisolver::HeadlessLevelEnvironment::step`** (`Source/AiSolver/Env/HeadlessLevelEnvironment.cpp`),
  dans cet ordre strict, à chaque appel :
  1. Capture `previousBox` : `core::Aabb::fromTopLeftSize(transform.position, collider.size)` **avant**
     tout déplacement de ce pas (comme `playLevel()`/`GameSession::update`, étape « avant »).
  2. `const core::TileMap mechanismMap = _mechanisms.collisionMap();` puis `_blocks.update(
     previousBox, input.moveX, mechanismMap);` (poussée/chute des blocs, avec la boîte du personnage
     **avant** son propre déplacement de ce pas).
  3. `const core::TileMap collision = _blocks.collisionMap(mechanismMap);` puis `_physics.update(
     _world, collision, input, kFixedDelta);` (`kFixedDelta = 1.0f / 60.0f`, la même constante que
     `STEP` dans `test_parcours_complet.cpp`).
  4. Sweep boîte-boîte des blocs à taille réduite (`EX-GP-005`) : reproduit tel quel la boucle sur
     `_blocks.scales()`/`_blocks.boxAt(index)` et `core::sweepAabbVsAabb` de `playLevel()`, y compris
     la mise à zéro de la vitesse et `grounded = true` sur normale verticale négative.
  5. Recalcule `box` (boîte finale du personnage après déplacement + sweep), puis `_mechanisms.
     update(box, world.getComponent<core::Player>(player).mass);` (masse transmise, `EX-GP-019`/
     `EX-GP-025`).
  6. `_dangers.update();` (avance le compteur de pas fixes des dangers mobile/temporisé, `EX-GP-051`/
     `EX-GP-053`).
  7. Assemble les boîtes de danger actif — même logique que `hmi::GameSession::
     collectActiveDangerBoxes()` (dangers mobiles via `_dangers.moverBox(index)`, dangers temporisés
     actifs via `core::dangerHitbox(TileType::DangerBlink, …)` filtré par `_dangers.isBlinkActive`,
     dangers commutés actifs via `core::dangerHitbox(TileType::DangerSwitched, …)` filtré par
     `_mechanisms.isDangerActive`) — puis `core::evaluateOutcome(box, level(), extraDangerBoxes)`.
  8. Incrémente `_stepIndex`, construit et renvoie `StepObservation{outcome, box, playerState,
     playerVelocity, _stepIndex}`.

## Fichiers impactés
- `Source/AiSolver/Env/HeadlessLevelEnvironment.cpp` (corps de `step`).
- `Source/AiSolver/Env/HeadlessLevelEnvironment.h` (méthode privée `collectActiveDangerBoxes`,
  miroir de celle de `GameSession`, si extraite pour la lisibilité).
- Tests : `Source/Test/Unit/AiSolver/Env/test_headless_level_environment.cpp` (complété).

## Tests (obligatoires)
- **Un pas de marche simple** (`demo-deplacement.json`, `moveX = 1.0f`) : la position `x` du
  personnage augmente d'un pas à l'autre, l'issue reste `Playing` tant que la sortie n'est pas
  atteinte.
- **Interrupteur/porte** (`demo-interrupteur.json`) : la porte s'ouvre (vérifiable via
  `MechanismController::isDoorOpen`, exposé en lecture par `HeadlessLevelEnvironment` ou vérifié
  indirectement par la franchissabilité) au pas où la boîte du personnage recouvre l'interrupteur.
- **Plaque de pression** (`demo-plaque-pression.json`) : la porte se referme dès que le personnage
  quitte la plaque (contrairement à l'interrupteur), reproduisant `EX-GP-025`.
- **Bloc à taille réduite** (`demo-bloc-reduit.json`) : le sweep boîte-boîte produit le même
  ressaut/atterrissage que la simulation en jeu (position finale identique à un rejeu de référence).
- **Danger avancé** (`demo-dangers-avances.json`, script s'aventurant volontairement vers une alcôve
  à danger mobile) : l'issue devient `Lost` au contact — preuve que `DangerController` est bien
  actif, à la différence de `playLevel()`.
- **Ordre exact** : un test dédié vérifie qu'inverser deux étapes (ex. physique avant blocs) change
  le résultat sur un niveau où l'ordre compte (bloc poussé puis marché dessus le même pas) — documente
  pourquoi l'ordre n'est pas arbitraire.

## Points d'attention
- **`previousBox` doit être capturée avant l'étape 2**, jamais recalculée après : la poussée d'un
  bloc doit voir la position du personnage **telle que laissée par le pas précédent**, pas celle en
  cours de résolution (piège déjà documenté dans `playLevel()`/`GameSession::update`).
- **La grille passée à la physique est `blocks.collisionMap(mechanisms.collisionMap())`**, jamais
  `mechanisms.collisionMap()` seule : oublier la composition des blocs ferait traverser un bloc plein
  au personnage.
- **`mechanisms.update` reçoit la boîte finale du pas** (après physique et sweep), pas `previousBox` :
  un mécanisme doit réagir à la position **actuelle**, pas à celle d'avant le déplacement.
- **`DangerController` est un ajout par rapport à `playLevel()`** (voir décision de cadrage de
  l'épic) : ne pas le lire comme une divergence involontaire lors de la revue de TACHE-05.

## Définition de fait (DoD)
- `step` implémente l'ordre complet ci-dessus, testé (`ctest` vert) sur au moins un niveau par
  mécanique (mécanisme, plaque, bloc plein, bloc réduit, danger avancé) ; build `/W4 /WX` sans
  avertissement ; Doxygen à jour.

## Notions abordées
@ref guide-annexe-apprentissage-renforcement — agent, environnement, boucle `reset`/`step`, épisode,
propriété de Markov.

## Exigences
`EX-IA-005` (nouvelle, portée principalement par cette tâche).
