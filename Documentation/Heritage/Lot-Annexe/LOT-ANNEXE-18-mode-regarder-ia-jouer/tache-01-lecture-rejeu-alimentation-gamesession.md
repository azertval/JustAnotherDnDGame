# TACHE-01 — Lecture du rejeu et alimentation de GameSession {#lot-annexe-18-tache-01-lecture-rejeu-alimentation-gamesession}

**Lot :** [LOT-ANNEXE-18](epic.md) · **Emplacement :** `Source/HMI/Game` · **Statut :** fait

## Contexte
`hmi::GameSession::update(const InputState&, float fixedDelta)` traduit aujourd'hui l'entrée réelle
(`hmi::toPlayerInput`, `PlayerInputMapper.cpp`) en `core::PlayerInput` avant de piloter la
simulation. Pour rejouer un fichier de rejeu, il faut pouvoir fournir directement ce
`core::PlayerInput` déjà enregistré, sans passer par la traduction d'entrée réelle.

## Travail à réaliser
- **`hmi::GameSession::update(const core::PlayerInput&, float fixedDelta) -> core::LevelOutcome`**
  (nouvelle surcharge publique, `Source/HMI/Game/GameSession.h/.cpp`) : contient la logique
  aujourd'hui interne à `update(const InputState&, float)` (snapshot de position précédente,
  `BlockController::update`, `CharacterPhysicsSystem::update`, sweep des blocs réduits,
  `AnimationSystem`, caméra de salle, `MechanismController::update`, `DangerController::update`,
  `core::evaluateOutcome`) — inchangée pas à pas.
- **`hmi::GameSession::update(const InputState& input, float fixedDelta)`** (existant, modifié) :
  devient `return update(toPlayerInput(input, _gameBindings, _gamepadBindings), fixedDelta);` —
  aucune autre modification de comportement.
- **`hmi::ReplayPlayback`** (`Source/HMI/Game/ReplayPlayback.h/.cpp`) : constructeur
  `ReplayPlayback(const std::filesystem::path& replayPath, const std::filesystem::path&
  levelsDir)` — charge le rejeu via `aisolver::readReplay`, le valide immédiatement via
  `aisolver::validateReplay` (échoue tôt, avant toute lecture de pas, si le rejeu est invalide) ;
  méthode `std::optional<core::PlayerInput> nextInput()` — renvoie le prochain `core::PlayerInput`
  de la séquence, `std::nullopt` une fois la séquence épuisée (signal de fin de rejeu, distinct
  d'une fin de partie par victoire/défaite).
- Point d'intégration côté `hmi::GameViewport` (utilisé par TACHE-02) : en mode rejeu, chaque
  itération de la boucle de pas fixe appelle `ReplayPlayback::nextInput()` plutôt que
  `InputState`/`toPlayerInput`, puis `GameSession::update(core::PlayerInput, fixedDelta)`.

## Fichiers impactés
- `Source/HMI/Game/GameSession.h/.cpp` — modifié (nouvelle surcharge, refactor minimal de
  l'existant en son terme).
- `Source/HMI/Game/ReplayPlayback.h/.cpp` — nouveau.
- `Source/HMI/CMakeLists.txt` — ajout des nouveaux fichiers, lien vers `AiSolver` (module
  `Replay` uniquement, en pratique la bibliothèque `AiSolver` entière puisque CMake ne découpe pas
  en sous-cibles internes — documenté comme dépendance de bibliothèque complète mais d'usage limité
  au sous-espace `aisolver::` de lecture de rejeu, pas une violation du principe de dépendance
  minimale, seulement une contrainte de granularité de `add_library`).

## Tests (obligatoires)
- **Non-régression stricte** : tous les tests existants sur `GameSession::update(const InputState&,
  float)` (`Source/Test/Unit/HMI`, `Source/Test/Integration`) restent verts sans modification de
  leurs assertions.
- **Équivalence des deux surcharges** : pour une `InputState` donnée, `update(inputState, dt)` et
  `update(toPlayerInput(inputState, bindings...), dt)` produisent la même issue et le même état de
  simulation (vérifié par un test dédié construisant les deux appels côte à côte sur un état de
  session identique).
- **`ReplayPlayback` sur un rejeu valide** : `nextInput()` renvoie la séquence enregistrée dans
  l'ordre exact, `std::nullopt` après le dernier élément.
- **`ReplayPlayback` sur un rejeu invalide** (niveau modifié) : le constructeur signale l'échec de
  validation de façon récupérable (exception capturée à la construction, ou statut interrogeable
  avant tout appel à `nextInput` — à trancher à l'implémentation, mais jamais un plantage).

## Points d'attention
- **Le refactor de `GameSession::update` doit être un déplacement de code, pas une réécriture** :
  le risque principal de cette tâche est d'introduire une divergence de comportement en réorganisant
  la méthode — la non-régression stricte (premier test ci-dessus) est la garde-fou direct contre ce
  risque.
- **`ReplayPlayback` ne connaît jamais `hmi::InputState` ni les bindings clavier/manette** : elle
  produit directement des `core::PlayerInput`, cohérent avec le fait qu'un rejeu n'a jamais besoin
  de traduire une intention physique — elle **est** déjà l'intention enregistrée.

## Définition de fait (DoD)
- Nouvelle surcharge de `GameSession::update`, `ReplayPlayback` disponibles et testés (`ctest`
  vert), aucune régression sur les tests existants ; build `/W4 /WX` sans avertissement ; Doxygen à
  jour.

## Notions abordées
Aucune notion d'apprentissage automatique nouvelle : cette tâche est d'ordre logiciel (format de
fichier, outillage, intégration continue). Le vocabulaire employé (épisode, rejeu, politique, agent)
est défini dans @ref guide-annexe-apprentissage-renforcement.

## Exigences
`EX-IA-019` (nouvelle, partagée avec TACHE-02/03 du même lot).
