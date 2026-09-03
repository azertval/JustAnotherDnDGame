# TACHE-01 — Exécution automatisée répétée d'un modèle sur son niveau d'origine {#lot-annexe-15-tache-01-execution-automatisee}

**Lot :** [LOT-ANNEXE-15](epic.md) · **Emplacement :** `Source/AiSolver/Eval` · **Statut :** fait

## Contexte
L'entraînement niveau par niveau (`LOT-ANNEXE-10`/`11`, `LOT-ANNEXE-12`/`13`/`14`) ne mesure jamais
qu'**une** chose de façon fiable : la capacité d'un modèle à produire, une fois, une trajectoire
exportable en rejeu déterministe (`LOT-ANNEXE-07`). Rien, jusqu'ici, ne mesure la **fiabilité**
statistique de la politique sous-jacente — à quelle fréquence termine-t-elle le niveau si on la
laisse choisir ses actions selon sa propre distribution (comme à l'entraînement), plutôt que selon
le chemin unique figé dans le rejeu. Cette tâche introduit l'exécution répétée qui répond à cette
question, brique de base du harnais de `LOT-ANNEXE-15`.

## Travail à réaliser
- **`aisolver::eval::TrainedPolicy`** (`Source/AiSolver/Eval/TrainedPolicy.h`) : interface
  abstraite minimale, un adaptateur fin vers un modèle déjà entraîné — `selectAction(const
  Observation& observation, ActionDecodingMode mode, aisolver::Rng& rng) -> core::PlayerInput`. Le
  décodage vers `core::PlayerInput` réutilise, sans le redéfinir, le mécanisme de décodage d'action
  déjà exposé par `LOT-ANNEXE-07`.
- **`aisolver::eval::ActionDecodingMode`** (enum) : `Argmax` (déterministe, une seule sortie
  possible pour une observation donnée) ou `Stochastic` (échantillonnage selon la distribution
  produite par le réseau de politique, via l'instance de `Rng` fournie).
- Quatre implémentations concrètes de `TrainedPolicy`, chacune un adaptateur de chargement vers le
  format de modèle déjà défini par son lot d'entraînement — Eval ne réimplémente aucune logique
  d'inférence, seulement le chargement et l'appel :
  - `EvolutionaryTrainedPolicy` (charge un individu de `Source/AiSolver/Training/Evolutionary`,
    `LOT-ANNEXE-10`/`11`) — n'accepte que `ActionDecodingMode::Argmax` (seul mode que cette famille
    a jamais connu ; un appel en `Stochastic` est un usage incorrect signalé, pas silencieusement
    ignoré).
  - `ReinforceTrainedPolicy` (`Source/AiSolver/Training/PolicyGradient`, `LOT-ANNEXE-12`) — charge
    uniquement le réseau de politique, les deux modes de décodage sont valides.
  - `ActorCriticTrainedPolicy` (`Source/AiSolver/Training/ActorCritic`, `LOT-ANNEXE-13`) — charge
    uniquement l'acteur ; le critique n'est **jamais** chargé ni utilisé en évaluation (il ne sert
    qu'à l'entraînement, `LOT-ANNEXE-13`, décision de cadrage réaffirmée ici).
  - `AdvancedAlgorithmTrainedPolicy` (`Source/AiSolver/Training/...`, algorithme retenu par
    `LOT-ANNEXE-14`) — adaptateur ajouté ou complété une fois `LOT-ANNEXE-14` livré ; la forme exacte
    dépend du choix (PPO/DQN) documenté par ce lot amont.
- **`aisolver::eval::BenchmarkConfig`** (struct, `Source/AiSolver/Eval/BenchmarkConfig.h`) :
  `int repetitions = 30;`, `std::uint64_t rngSeedBase;`, `int maxStepsPerEpisode;`,
  `ActionDecodingMode decodingMode = ActionDecodingMode::Stochastic;`.
- **`aisolver::eval::EpisodeOutcome`** (struct) : `core::LevelOutcome outcome;` (`Playing` en fin de
  budget de pas = timeout), `int stepCount;`.
- **`aisolver::eval::BenchmarkResult`** (struct + méthodes) : `std::vector<EpisodeOutcome>
  episodes;`, `double successRate() const;` (fraction d'épisodes en `LevelOutcome::Won`),
  `double meanStepsAll() const;` (moyenne sur tous les épisodes, échecs/timeouts inclus),
  `double meanStepsOnSuccess() const;` (moyenne sur les seuls épisodes réussis),
  `double stepVariance() const;` (sur `meanStepsAll`, documentée comme telle).
- **`aisolver::eval::BenchmarkRunner::run`** (`Source/AiSolver/Eval/BenchmarkRunner.h/.cpp`) :
  `BenchmarkResult run(TrainedPolicy& policy, const std::filesystem::path& levelPath, const
  BenchmarkConfig& config)`. Pour chaque répétition `i` dans `[0, config.repetitions)` : graine
  dérivée `deriveSeed(config.rngSeedBase, i)`, `HeadlessLevelEnvironment` fraîchement réinitialisé
  (`reset(levelPath)`), boucle `step` jusqu'à `LevelOutcome::Won`/`Lost` ou
  `config.maxStepsPerEpisode` atteint (timeout), action à chaque pas obtenue via
  `policy.selectAction(...)`.
- **`EX-IA-016`** : nouvelle exigence, déclarée dans ce fichier (section Exigences ci-dessous) —
  écrite ici plutôt que dans les spécifications (`Documentation/Specification`) car ce lot ne touche
  qu'à `Documentation/Lot-Annexe` à ce stade de rédaction ; à recopier dans
  `Documentation/Specification/ia.md` (ou équivalent) au moment de l'implémentation si ce fichier de
  spécifications existe alors.

## Fichiers impactés
- `Source/AiSolver/Eval/TrainedPolicy.h`.
- `Source/AiSolver/Eval/ActionDecodingMode.h` (ou intégré à `TrainedPolicy.h`).
- `Source/AiSolver/Eval/EvolutionaryTrainedPolicy.h/.cpp`.
- `Source/AiSolver/Eval/ReinforceTrainedPolicy.h/.cpp`.
- `Source/AiSolver/Eval/ActorCriticTrainedPolicy.h/.cpp`.
- `Source/AiSolver/Eval/AdvancedAlgorithmTrainedPolicy.h/.cpp`.
- `Source/AiSolver/Eval/BenchmarkConfig.h`.
- `Source/AiSolver/Eval/BenchmarkResult.h`.
- `Source/AiSolver/Eval/BenchmarkRunner.h/.cpp`.
- Tests : `Source/Test/Unit/AiSolver/Eval/test_benchmark_runner.cpp`.
- Test d'intégration (niveau JSON réel) : `Source/Test/Integration/test_benchmark_multi_algorithmes.cpp`.

## Tests (obligatoires)
- **Reproductibilité stricte** : deux appels à `BenchmarkRunner::run` avec la même
  `BenchmarkConfig` (même `rngSeedBase`) produisent des `BenchmarkResult` identiques (mêmes issues,
  mêmes nombres de pas, répétition par répétition).
- **Graines dérivées distinctes** : `deriveSeed(base, i) != deriveSeed(base, j)` pour `i != j`, sur
  un échantillon de valeurs (test direct de la fonction de dérivation).
- **Modèle évolutionniste : variance nulle** : sur `EvolutionaryTrainedPolicy`
  (`ActionDecodingMode::Argmax` uniquement), les `N` répétitions d'un même modèle sur un même
  niveau sont **strictement identiques** (même issue, même nombre de pas) — sert de garde de
  cohérence croisée avec la
  décision de cadrage de `LOT-ANNEXE-10`.
- **Politique stochastique simulée** : sur un `TrainedPolicy` factice à comportement aléatoire connu
  (ex. une « pièce » biaisée simulée pour le choix d'action), le `successRate()` mesuré sur un grand
  nombre de répétitions converge vers la probabilité théorique, à une tolérance documentée (graine
  fixée pour un test déterministe malgré la stochasticité simulée).
- **Distinction `meanStepsAll` / `meanStepsOnSuccess`** : un jeu d'épisodes mêlant succès rapides et
  échecs plafonnés à `maxStepsPerEpisode` donne des valeurs **différentes** pour les deux moyennes,
  vérifiées indépendamment.
- **Troncature sans boucle infinie** : un `TrainedPolicy` factice qui ne termine jamais le niveau est
  correctement arrêté à `maxStepsPerEpisode`, comptabilisé en timeout (`LevelOutcome::Playing` à la
  coupure), sans dépassement.
- **Refus explicite du mode `Stochastic` pour l'évolutionniste** :
  `EvolutionaryTrainedPolicy::selectAction(..., ActionDecodingMode::Stochastic, ...)` signale une
  erreur récupérable (`EX-NFR-040`), ne plante pas.
- **Intégration (niveau réel)** : sur un petit niveau JSON de test avec une politique scriptée
  triviale connue pour réussir à tous les coups, `BenchmarkResult::successRate() == 1.0`.

## Points d'attention
- **Ne jamais mélanger `meanStepsAll` et `meanStepsOnSuccess` sans le dire** : un échec plafonné à
  `maxStepsPerEpisode` gonfle artificiellement une moyenne « tous épisodes » non distinguée — les
  deux méthodes existent précisément pour éviter cette confusion dans les rapports (`TACHE-02`).
- **Le harnais ne modifie ni ne réentraîne jamais un modèle** : chargement strictement en lecture
  seule dans les quatre `TrainedPolicy`, aucune mise à jour de poids déclenchable depuis
  `Source/AiSolver/Eval`.
- **Isolation stricte entre répétitions** : chaque répétition repart d'un `HeadlessLevelEnvironment`
  fraîchement construit (`reset`) — un état résiduel d'une répétition à l'autre biaiserait la mesure
  de fiabilité par un effet d'ordre.
- **Le critique d'un modèle acteur-critique n'est jamais chargé en évaluation** — seul l'acteur
  produit les actions ; un chargement du critique serait un couplage inutile avec un composant qui
  n'a de sens qu'à l'entraînement (`LOT-ANNEXE-13`).
- **Coût CPU non négligeable** : `N` répétitions par (algorithme, niveau) pour plusieurs algorithmes
  peut devenir long à exécuter séquentiellement — accepté comme compromis de ce lot (voir Exclus de
  l'épic), pas un défaut à corriger ici.

## Définition de fait (DoD)
- `TrainedPolicy` (et ses quatre adaptateurs), `BenchmarkConfig`, `BenchmarkResult` et
  `BenchmarkRunner` disponibles et testés (`ctest` vert) ; build `/W4 /WX` sans avertissement ;
  Doxygen à jour ; `EX-IA-016` déclarée.

## Notions abordées
@ref guide-annexe-evaluation-rl — variance entre exécutions, graines multiples, mesure honnête d'un
agent entraîné.

## Exigences
`EX-IA-016` (nouvelle, déclarée dans `epic.md` — partagée avec TACHE-02/03 du même lot).
