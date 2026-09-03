# LOT-ANNEXE-05 — Environnement de simulation headless {#lot-annexe-05}

> Statut : **fait**. Prérequis : aucun lot annexe (peut démarrer dès que la génération 0
> fournit de quoi compiler `AiSolver`, sans dépendance **fonctionnelle** sur elle). Premier lot de
> la génération 1 : donne à tout algorithme d'apprentissage à venir un moyen de **jouer** un niveau
> sans fenêtre ni GPU.

## Objectif
Aucun algorithme d'apprentissage (génération 2 : recherche évolutionniste, génération 3 :
apprentissage par gradient) ne peut s'exécuter sans un moyen de faire avancer un niveau, pas à pas,
des dizaines de milliers de fois par seconde, sans Direct3D 11 ni Qt. Cette orchestration existe déjà
— mais **écrite deux fois** : `hmi::GameSession::update` (`Source/HMI/Game/GameSession.cpp`, utilisée
en jeu) et `playLevel()` (`Source/Test/Systeme/test_parcours_complet.cpp`, sa reproduction headless
qui sert de test système de non-régression). Ce lot introduit une **troisième** implémentation, dans
`Source/AiSolver/Env`, fidèle aux deux premières et pensée dès le départ pour l'usage qu'en feront
les générations 2 et 3 : rejouable des millions de fois, sans aucun état de rendu, derrière une API
`reset`/`step` minimale.

## Périmètre

### Inclus
- `aisolver::HeadlessLevelEnvironment` (`Source/AiSolver/Env/HeadlessLevelEnvironment.h/.cpp`) :
  `reset(chemin)` charge un niveau et fait apparaître le personnage à l'entrée ; `step(core::
  PlayerInput)` avance d'un pas fixe et renvoie l'issue, la boîte et l'état du personnage.
- Réplication **exacte** de l'ordre de composition par pas déjà établi par `hmi::GameSession::update`
  et repris par `playLevel()` : mécanismes (lecture de la grille de collision) → blocs (poussée/chute
  puis grille complétée) → physique du personnage → sweep boîte-boîte des blocs à taille réduite
  (`EX-GP-005`) → mécanismes (mise à jour de l'état + grille) → dangers (mise à jour de l'état) →
  `core::evaluateOutcome`.
- Un budget de pas dur et une mesure de progression exposée (distance à la sortie, pas écoulés
  depuis le dernier progrès) : matière première pour la détection de blocage — la **politique** de
  fin d'épisode (à partir de quand déclarer un blocage) reste hors de ce lot (LOT-ANNEXE-08).
- Cible CMake `AiSolver`, liée uniquement à `Core`.
- Garde de fidélité permanente en CI : rejeu de toute la table de niveaux scriptés de
  `test_parcours_complet.cpp` à travers `HeadlessLevelEnvironment`, avec assertion **pas-à-pas**.

### Exclus (hors périmètre de ce lot)
- **Tout rendu.** `HeadlessLevelEnvironment` n'ajoute à l'entité joueur que les quatre composants
  nécessaires à la physique (`core::Transform`, `core::Velocity`, `core::Collider`, `core::Player`)
  — ni `Sprite` ni `Animation`, exactement comme `playLevel()`.
- **Refactorer `hmi::GameSession::update` pour partager le code avec `HeadlessLevelEnvironment`.**
  Cela inverserait un sens de dépendance (`HMI` dépendrait alors d'`AiSolver`), une décision
  d'architecture qui dépasse le périmètre d'un lot d'observabilité. `HeadlessLevelEnvironment` est
  donc une réplique **indépendante mais testée fidèle** (TACHE-05), pas un partage littéral de code.
- **La politique de fin d'épisode** (à partir de quand un blocage devient une fin d'épisode, quelle
  récompense en résulte) : LOT-ANNEXE-08, qui consomme les compteurs bruts exposés ici.
- **L'encodage tenseur de l'observation** : LOT-ANNEXE-06, qui consomme `HeadlessLevelEnvironment`
  en lecture seule.

## Décisions de cadrage
- **Aucune ligne de `Core` n'est modifiée.** `HeadlessLevelEnvironment` n'appelle que des méthodes
  publiques déjà exposées : `core::World`, `core::CharacterPhysicsSystem::update`, `core::
  BlockController::update`/`collisionMap`/`boxAt`/`scales`, `core::MechanismController::update`/
  `collisionMap`, `core::DangerController::update`, `core::evaluateOutcome`.
- **Composition indépendante de `GameSession`, testée fidèle plutôt que partagée** (voir Exclus) : le
  risque de divergence entre deux implémentations maintenues séparément est réel — c'est précisément
  ce que TACHE-05 transforme en échec de CI plutôt qu'en bug silencieux découvert en entraînement.
- **`DangerController` inclus dès ce lot, à la différence de `playLevel()`.** `playLevel()` n'en
  construit pas, parce qu'aucun de ses scripts n'a jamais besoin de mourir d'un danger mobile/
  commuté/temporisé (`demo-dangers-avances.json` n'y est traversé que par son couloir principal, sans
  jamais viser une alcôve à danger, par construction du scénario). Un environnement d'entraînement,
  lui, doit pouvoir faire mourir un agent qui s'aventure dans ces alcôves : sans `DangerController`,
  ce niveau serait structurellement inapprenable en headless.
- **La grille de collision n'est jamais mutée sur le `Level`.** Comme `GameSession`/`playLevel()`,
  `HeadlessLevelEnvironment` recompose une `TileMap` de collision à chaque pas
  (`mechanisms.collisionMap()` puis `blocks.collisionMap(...)`) ; le `Level` chargé reste la source
  de vérité immuable — nécessaire pour qu'un `reset` répété sur le même chemin reparte d'un état
  strictement identique (déterminisme, `EX-NFR-002`).
- **Budget de pas et mesure de progression exposés, jamais interprétés.**
  `HeadlessLevelEnvironment` reste une enveloppe de simulation pure : elle expose des compteurs
  (`stepIndex`, `stepsSinceProgress`, `bestProgress`) mais ne décide jamais elle-même qu'un épisode
  est « bloqué » — cette décision dépend de l'algorithme et du niveau (LOT-ANNEXE-08), une seule
  responsabilité par module.

## Notions abordées
Voir @ref guide-annexe-apprentissage-renforcement (agent, environnement, boucle
`reset`/`step`/observation/action/récompense). Source directe de la forme de l'interface : Brockman
et al. (2016, *OpenAI Gym*, popularise l'interface `reset`/`step`) — `HeadlessLevelEnvironment` s'en
inspire dans sa forme uniquement, sans aucune dépendance à cette bibliothèque.

## Exigences couvertes
- Nouvelle, déclarée dans [la spécification IA](@ref spec-ia) : [`EX-IA-005`](@ref EX-IA-005).
- Réutilisées (inchangées) : `EX-GP-001` à `EX-GP-025`/`050`-`053` (modèle de niveau et mécaniques,
  consommées en lecture seule), `EX-NFR-002` (déterminisme au pas fixe), `EX-NFR-004` (vérification
  sans GPU), `EX-ARCH-011`/`EX-ARCH-030` (données pures, orchestration par systèmes).

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-headless-level-environment.md) | `HeadlessLevelEnvironment` : squelette, `reset`, entité joueur | `Source/AiSolver/Env` | ✅ |
| [TACHE-02](tache-02-replication-ordre-pas.md) | `step` : réplication exacte de l'ordre de composition par pas | `Source/AiSolver/Env` | ✅ |
| [TACHE-03](tache-03-budget-pas-detection-blocage.md) | Budget de pas et mesure de progression (blocage) | `Source/AiSolver/Env` | ✅ |
| [TACHE-04](tache-04-cible-cmake-aisolver.md) | Cible CMake `AiSolver` | `Source/AiSolver`, `Source/CMakeLists.txt` | ✅ |
| [TACHE-05](tache-05-garde-fidelite-parcours-complet.md) | Garde de fidélité pas-à-pas (CI permanente) | `Source/Test/Systeme` | ✅ |

## Critères d'acceptation du lot
1. `HeadlessLevelEnvironment::reset` charge n'importe quel niveau JSON valide de
   `Source/Elements/Levels` et positionne le personnage à l'entrée, budgets de saut/dash inclus.
2. `HeadlessLevelEnvironment::step` fait avancer la simulation d'exactement un pas fixe (`1/60 s`) et
   renvoie une issue cohérente avec `core::evaluateOutcome`.
3. Aucune ligne de `Source/Core` n'est modifiée par ce lot (vérifiable par diff).
4. Le budget de pas dur arrête toute boucle d'entraînement automatisée en un nombre de pas borné,
   quel que soit le niveau ou le script d'entrée.
5. La garde de fidélité (TACHE-05) rejoue l'intégralité de la table de `ParcoursCompletSysteme` et
   échoue au premier pas où position, vitesse ou issue divergent entre l'orchestration de référence
   et `HeadlessLevelEnvironment` — pas seulement sur l'issue finale.
6. `AiSolver` compile en cible séparée, liée uniquement à `Core`, sans dépendance Qt/HMI/D3D11.
7. Build `/W4 /WX` sans avertissement, Doxygen et lint des exigences verts.

## Dépendances
Enveloppe, en lecture seule, du `Core` existant : `Level`/`TileMap`/`LevelLoader` (LOT-07),
`CharacterPhysicsSystem` (LOT-08), `MechanismController` (LOT-12) étendu par la masse et la plaque
de pression (LOT-19), `BlockController` (LOT-21) et les blocs à taille fractionnaire (LOT-24),
`DangerController` (LOT-31), le pas fixe déterministe (LOT-33). Réplique fidèlement l'orchestration
de `hmi::GameSession::update` et du test système `test_parcours_complet.cpp` (séquence de niveaux
`demo-*.json`, LOT-25). Aucun autre lot annexe en amont.

## Navigation des tâches
- @subpage lot-annexe-05-tache-01-headless-level-environment
- @subpage lot-annexe-05-tache-02-replication-ordre-pas
- @subpage lot-annexe-05-tache-03-budget-pas-detection-blocage
- @subpage lot-annexe-05-tache-04-cible-cmake-aisolver
- @subpage lot-annexe-05-tache-05-garde-fidelite-parcours-complet
