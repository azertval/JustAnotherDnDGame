# TACHE-01 — Hyperparamètres et drapeaux `aisolver-cli` {#lot-annexe-22-tache-01-parametres-cli}

**Lot :** [LOT-ANNEXE-22](epic.md) · **Emplacement :** `Source/AiSolver/Cli` · **Statut :** fait

## Contexte
`TrainingConfig` porte tous les hyperparamètres des quatre familles d'algorithmes, et
`writeTrainingConfigJson` les journalise tous dans le `config.json` du run. Mais la moitié d'entre
eux n'étaient **atteignables** que par un fichier `--config` écrit à la main : `CommandLineOverrides`
déclarait bien les huit champs DQN, sans que `parseTrainArgs` ne lise le moindre drapeau `--dqn-*`,
et cinq autres paramètres (`hiddenSize`, `tournamentSize`, `mutationStrength`, `maxGenerations`,
`requiredConsecutiveSuccesses`) n'existaient dans aucune des deux structures de surcharge.

Cette tâche est la base commune : l'écran Mode IA et la ligne de commande traversent tous deux
`loadTrainingConfig`, donc élargir les surcharges les sert ensemble.

## Travail à réaliser
- **`CommandLineOverrides`** : ajouter `hiddenSize`, `tournamentSize`, `mutationStrength`,
  `maxGenerations`, `requiredConsecutiveSuccesses` ; étendre `applyOverrides` en conséquence.
  `applyJsonFile` et `writeTrainingConfigJson` couvrent déjà ces clés — ne rien y toucher.
- **`readNumberOption`** (helper de `Commands.cpp`) : le protocole de lecture d'une option
  numérique (absente → défaut ; présente mais mal formée → message d'usage) était recopié pour
  chaque option. L'écrire une fois évite que la trentaine d'options de `train` dérive vers trente
  formulations d'erreur différentes ; les messages produits restent identiques à l'existant.
- **`TrainArgs` + `parseTrainArgs`** : treize nouveaux drapeaux — `--hidden-size`,
  `--tournament-size`, `--mutation-strength`, `--max-generations`, `--required-successes`, et les
  huit `--dqn-*`.
- **`EvaluateArgs` + `parseEvaluateArgs`** : `--max-steps`, `--seed` et
  `--decoding <argmax|stochastic>` couvrent le reste de `eval::BenchmarkConfig`, jusqu'ici figé aux
  défauts du code.
- **`runTrain`** : construire les surcharges par **désignateurs**, jamais par liste positionnelle —
  la structure gagne des champs à chaque hyperparamètre exposé, et un ajout en son milieu
  décalerait silencieusement toutes les valeurs suivantes. Passer `runDir / "dqn_stats.csv"` au
  `DqnTrainer`.
- **`Main.cpp`** : usage regroupé par famille d'algorithme, et rappel de la priorité
  défauts → `--config` → options individuelles.

## Critères de validation
- Chaque nouveau drapeau accepte une valeur valide, refuse une valeur mal formée avec le message
  d'usage de la sous-commande, et laisse le défaut documenté en place quand il est absent.
- La chaîne de priorité complète est vérifiée sur `hiddenSize` : une option individuelle prime sur
  le fichier `--config`, qui prime sur le défaut.
- `aisolver-cli` compile et s'exécute ; `printUsage` liste tous les drapeaux.
