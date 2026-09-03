# TACHE-01 — HeadlessLevelEnvironment : squelette, reset, entité joueur {#lot-annexe-05-tache-01-headless-level-environment}

**Lot :** [LOT-ANNEXE-05](epic.md) · **Emplacement :** `Source/AiSolver/Env` · **Statut :** à faire

## Contexte
Aucun code n'existe encore dans `Source/AiSolver`. Cette tâche pose la classe
`aisolver::HeadlessLevelEnvironment` : son état, son constructeur, `reset(chemin)` (chargement de
niveau et apparition du personnage) et les types de retour de `step` — le corps de `step` lui-même
(l'ordre exact de composition par pas) est TACHE-02. Le patron de spawn est celui déjà établi par
`spawn()` dans `Source/Test/Systeme/test_parcours_complet.cpp` : mêmes composants, mêmes valeurs.

## Travail à réaliser
- **`aisolver::HeadlessLevelEnvironment`** (`Source/AiSolver/Env/HeadlessLevelEnvironment.h`),
  namespace `aisolver` (sibling de `core`/`hmi`) :
  - `[[nodiscard]] bool reset(const std::filesystem::path& levelPath);` — charge le niveau via
    `core::LevelLoader::loadFromFile`, réinitialise `core::World` à un monde vierge, crée l'entité
    joueur avec `core::Transform{core::playerSpawnPosition(entry.column, entry.row), core::
    playerSize(), 0.0f}`, `core::Velocity{}`, `core::Collider{core::playerSize()}`, `core::Player{}`
    (mêmes quatre composants que `spawn()` dans `test_parcours_complet.cpp` — aucun `Sprite` ni
    `Animation`), initialise `jumpsRemaining`/`dashesRemaining` depuis `level.jumpBudget()`/
    `level.dashBudget()`, reconstruit `core::CharacterPhysicsSystem`, `core::BlockController`,
    `core::MechanismController` et `core::DangerController` pour le niveau chargé. Renvoie `false`
    (état neutre, rien de chargé) si `LevelLoadResult::ok()` est faux.
  - `[[nodiscard]] bool loaded() const noexcept;` et `[[nodiscard]] const std::string& loadError()
    const noexcept;` — même patron que `hmi::GameSession::loaded()`/`loadError()`, pour un échec
    récupérable (`EX-NFR-040`) plutôt qu'une exception.
  - `[[nodiscard]] const core::Level& level() const;` — accès en lecture au niveau chargé (nom,
    dimensions, sortie…), utile aux tâches avales (LOT-ANNEXE-06/08).
  - Struct `StepObservation` : `core::LevelOutcome outcome; core::Aabb playerBox; core::Player
    playerState; core::Velocity playerVelocity; int stepIndex = 0;` — le retour de `step`
    (TACHE-02), regroupant issue, boîte et état complet du personnage (position implicite via
    `playerBox`, vitesse, `grounded`/`wallDirection`/timers/budgets via `playerState`).
- **Squelette de `step`** : signature `[[nodiscard]] StepObservation step(const core::PlayerInput&
  input);` déclarée, avec une garde `JADG_ASSERT(loaded(), ...)` (appeler `step` sans
  `reset` réussi au préalable est une erreur de programmation, pas un cas récupérable) ; le corps est
  implémenté en TACHE-02.

## Fichiers impactés
- `Source/AiSolver/Env/HeadlessLevelEnvironment.h` (nouveau).
- `Source/AiSolver/Env/HeadlessLevelEnvironment.cpp` (nouveau, `reset`/accesseurs seulement à ce
  stade).
- Tests : `Source/Test/Unit/AiSolver/Env/test_headless_level_environment.cpp` (nouveau) — ajouté à
  `UnitTests` (TACHE-04 pour le câblage CMake).

## Tests (obligatoires)
- **`reset` réussi** : sur un niveau valide (`demo-deplacement.json`, via `JADG_LEVELS_DIR`),
  `loaded()` devient vrai, `loadError()` est vide, la boîte du personnage est centrée sur la case
  d'entrée du niveau.
- **`reset` échoué** : chemin inexistant → `reset` renvoie `false`, `loaded()` reste faux,
  `loadError()` non vide ; aucune exception.
- **Budgets initiaux** : `level.jumpBudget() == -1`/`dashBudget() == -1` (illimité) se retrouvent
  identiques sur `core::Player::jumpsRemaining`/`dashesRemaining` après `reset` ; idem pour un
  niveau à budget fini (`demo-budget.json`).
- **`reset` répété** : appeler `reset` deux fois sur le même chemin produit un état strictement
  identique (position, vitesse nulle, `stepIndex` remis à zéro) — déterminisme (`EX-NFR-002`).

## Points d'attention
- **Ne pas ajouter `Sprite`/`Animation`** à l'entité joueur : `HeadlessLevelEnvironment` ne rend
  jamais rien, et ces composants n'ont aucun rôle dans la simulation — les ajouter gonflerait
  inutilement chaque `reset` (des dizaines de milliers par run d'entraînement).
- **`reset` doit être appelable plusieurs fois sur la même instance** (rejouer un niveau, ou en
  charger un autre) — pas de ressource acquise en constructeur qui empêcherait un second chargement
  (RAII sans `init()`/`cleanup()` séparés, mais `reset()` n'est pas un `init()` : c'est l'opération
  normale de (re)démarrage d'un épisode, au même titre que `GameSession::reload()`).
- **`core::World` est reconstruit intégralement à chaque `reset`** (`_world = core::World{};`,
  comme `GameSession::loadLevel`), pas réutilisé : plus simple à garantir correct qu'une purge
  sélective des entités du niveau précédent.

## Définition de fait (DoD)
- `HeadlessLevelEnvironment::reset` et ses accesseurs compilent et sont testés (`ctest` vert) ; build
  `/W4 /WX` sans avertissement ; Doxygen à jour. `step` est déclaré mais son corps est laissé à
  TACHE-02 (pas de DoD sur le comportement de simulation ici).

## Notions abordées
@ref guide-annexe-apprentissage-renforcement — agent, environnement, boucle `reset`/`step`, épisode,
propriété de Markov.

## Exigences
`EX-IA-005` (nouvelle, partagée avec TACHE-02 à TACHE-05 de ce lot).
