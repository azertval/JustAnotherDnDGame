# TACHE-02 — Hooks d'observation `AiSolver` {#lot-annexe-22-tache-02-hooks-observation}

**Lot :** [LOT-ANNEXE-22](epic.md) · **Emplacement :** `Source/AiSolver/Training`,
`Source/AiSolver/Eval` · **Statut :** fait

## Contexte
Deux grandeurs que l'écran Mode IA doit montrer n'étaient observables par aucun appelant :

- le **compteur de séries stables** de l'algorithme évolutionniste vit dans une variable locale de
  `LevelTrainingSession::run`, et `TrainingResult` ne rapporte à la fin que `solved` — impossible de
  dire, pendant un run, si la session est à une génération de s'arrêter ou si le compteur vient
  d'être remis à zéro ;
- une **campagne d'évaluation** (`BenchmarkRunner::run`) s'exécute jusqu'au bout sans point de
  sortie, ce qui interdit toute progression comme toute annulation.

Même patron additif que les hooks posés par [LOT-ANNEXE-21](@ref lot-annexe-21)
(`setOnRecord`, `shouldStop`, `onGenerationChampion`) : paramètres et observateurs optionnels,
aucune rupture de signature pour les appelants existants.

## Travail à réaliser
- **`LevelTrainingSession::setOnStabilityChanged`** : observateur optionnel appelé après chaque
  génération avec le compteur courant et le seuil exigé. Observation pure — la décision d'arrêt
  reste celle de `run()`.
- **`eval::RepetitionObserver`** et le paramètre `onRepetition` de `BenchmarkRunner::run` /
  `runWithNoise` : appelé après chaque répétition, renvoyer `false` interrompt la campagne, qui
  rapporte alors les seules répétitions déjà jouées. La granularité est la **répétition**, jamais
  le pas de simulation — un appel indirect par image simulée serait payé pour rien.
- `DqnTrainer` n'a besoin d'aucune extension : `currentEpsilon()` et `totalSteps()` existent déjà,
  et le paramètre `dqnStatsCsvPath` de son constructeur n'attendait qu'un appelant.

## Critères de validation
- La séquence de compteurs émise par l'observateur de stabilité correspond à la logique déjà
  testée de `updateConsecutiveStableWins`.
- Une campagne interrompue au bout de *n* répétitions rapporte exactement *n* épisodes.
- Les appelants existants (CLI, tests) compilent sans modification.
