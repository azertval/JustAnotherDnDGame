# TACHE-04 — Onglet Validation & sauvegarde {#lot-annexe-22-tache-04-onglet-validation}

**Lot :** [LOT-ANNEXE-22](epic.md) · **Emplacement :** `Source/HMI`, `Source/Elements/UI` ·
**Statut :** fait

## Contexte
L'onglet appelait `evaluateModel` **directement depuis le thread d'IHM** : une campagne rejoue le
niveau `repetitions` fois, chacune jusqu'à `maxStepsPerEpisode` pas, et figeait donc la fenêtre
aussi sûrement qu'un entraînement — alors que l'entraînement, lui, avait son travailleur depuis
`LOT-ANNEXE-21`. Le niveau était déduit du nom du dossier de run et le modèle imposé
(`runDir/model.bin`) : ni évaluation croisée, ni modèle extérieur. Le rapport CSV de
`evaluate --report` n'avait pas d'équivalent. Enfin, « Exporter comme rejeu » était une
`copy_file` du `replay.json` du run — inopérante dès que ce fichier manque, alors que le modèle
sauvegardé reste rejouable.

## Travail à réaliser
- **`HMI/Ai/EvaluationWorker`** : `QObject` déplacé sur un `QThread` par l'écran, même patron que
  `TrainingWorker`. Délègue à `evaluateModel` ; le rappel de progression sert aussi d'interruption.
- **`EvaluationRequest`/`EvaluationOutcome`** : la requête couvre tout `eval::BenchmarkConfig`
  (répétitions, budget de pas, graine, mode de décodage), avec les **mêmes défauts** que lui ; le
  résultat gagne `meanStepsAll` (que la CLI affichait déjà) et `repetitionsRun` (nécessaire dès
  qu'une campagne est annulable).
- **`writeEvaluationReport`** et **`exportModelReplay`**, placés dans `HMI/Ai` : `HMI/Interface` ne
  référence pas `AiSolver/Eval`/`Training` (amendement de `LOT-ANNEXE-18`, limité à `HMI/Ai`), et
  un CSV formaté à la main dans l'écran divergerait du rapport de la ligne de commande.
- **`exportModelReplay`** est le véritable équivalent de `export-replay` : recharge le modèle,
  rejoue en argmax, écrit le rejeu. Il conserve le refus d'exporter un rejeu non résolu (décision
  de cadrage de [LOT-ANNEXE-11](@ref lot-annexe-11)), mais le **diagnostique** désormais
  explicitement au lieu de signaler un fichier absent.
- **Formulaire** : modèle (avec parcours de fichiers), niveau libre, budget de pas, graine, mode de
  décodage, barre de progression et bouton d'annulation.
- **`runCombo`** : libellé enrichi de l'algorithme du run, relu dans son `config.json`. Choisir un
  run **pré-remplit** modèle et niveau sans les y figer — l'évaluation croisée est précisément ce
  que la ligne de commande permettait et que cet onglet interdisait.
- **« Reprendre les réglages de ce run »** : recharge le `config.json` du run dans l'onglet
  Entraînement, par le même chemin de code que les presets.

## Critères de validation
- Une évaluation de 100 répétitions laisse la fenêtre réactive, montre sa progression, et s'annule.
- Une évaluation annulée rapporte le nombre de répétitions réellement jouées.
- Un modèle peut être mesuré sur un autre niveau que celui de son run.
- Le CSV exporté a le même en-tête et les mêmes colonnes que `evaluate --report`.
- L'export d'un rejeu que le modèle ne résout pas affiche le diagnostic et n'écrit aucun fichier.
