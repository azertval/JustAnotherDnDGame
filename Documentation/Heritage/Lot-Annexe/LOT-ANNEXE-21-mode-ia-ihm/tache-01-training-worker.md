# TACHE-01 — Hooks AiSolver + TrainingWorker {#lot-annexe-21-tache-01-training-worker}

**Lot :** [LOT-ANNEXE-21](epic.md) · **Emplacement :** `Source/AiSolver`, `Source/HMI/Ai` ·
**Statut :** en cours

## Contexte
Aucune des boucles d'entraînement existantes (`LevelTrainingSession`, `ReinforceTrainer`,
`ActorCriticTrainer`, `DqnTrainer`) n'expose de point d'observation ou d'interruption : chacune
s'exécute jusqu'à son critère d'arrêt naturel, exactement ce qu'il faut pour `aisolver-cli`
(synchrone, jamais interrompu) mais insuffisant pour une IHM qui doit rester réactive. Cette tâche
ajoute des points d'extension minimaux à `AiSolver`, puis un exécuteur Qt (`TrainingWorker`) qui les
utilise sans dupliquer la moindre règle d'apprentissage.

## Travail à réaliser
- **`TrainingStatsRecorder::setOnRecord`** (`Source/AiSolver/Stats`) : observateur optionnel,
  appelé à la fin de chaque `record()` — point unique déjà traversé par les quatre familles
  d'algorithmes, aucune modification des boucles elles-mêmes nécessaire pour la progression.
- **`shouldStop` optionnel** sur `LevelTrainingSession::run`, `ReinforceTrainer::run`,
  `ActorCriticTrainer::run`, `DqnTrainer::run` : vérifié en tête de chaque génération/épisode,
  arrêt propre (résultat partiel) si vrai. Paramètre par défaut vide : aucun changement de
  comportement pour les appelants existants (`aisolver-cli`, tests).
- **`onGenerationChampion` optionnel** sur `LevelTrainingSession::run` : seul point d'accès externe
  au champion courant (le `_trainer` interne reste privé) — nécessaire à l'aperçu en direct
  évolutionniste, jamais à une décision d'arrêt.
- **`AiSolver/Training/ArgmaxRollout`** : extraction sans changement de comportement de la fonction
  homonyme, jusqu'ici piégée dans l'espace de noms anonyme de `Cli/Commands.cpp` — partagée entre
  la CLI et `TrainingWorker`.
- **`HMI/Ai/TrainingWorker`** (`QObject`, déplacé sur un `QThread` par l'appelant) : dispatch
  d'algorithme calqué sur `aisolver::cli::runTrain`, mêmes types de construction
  (`TrainingConfig`/`CommandLineOverrides`, `LOT-ANNEXE-19`). Signaux `progress`, `previewReady`,
  `finished`, `failed` ; slot `requestStop` (indicateur atomique, thread-safe).

## Fichiers impactés
- `Source/AiSolver/Stats/TrainingStatsRecorder.h/.cpp` — modifié (`setOnRecord`).
- `Source/AiSolver/Training/LevelTrainingSession.h/.cpp` — modifié (`shouldStop`,
  `onGenerationChampion`).
- `Source/AiSolver/Training/PolicyGradient/ReinforceTrainer.h/.cpp` — modifié (`shouldStop`).
- `Source/AiSolver/Training/ActorCritic/ActorCriticTrainer.h/.cpp` — modifié (`shouldStop`).
- `Source/AiSolver/Training/Advanced/DqnTrainer.h/.cpp` — modifié (`shouldStop`).
- `Source/AiSolver/Training/ArgmaxRollout.h/.cpp` — nouveau (extrait de `Cli/Commands.cpp`).
- `Source/AiSolver/Cli/Commands.cpp` — modifié (utilise `training::argmaxRollout`).
- `Source/HMI/Ai/TrainingWorker.h/.cpp` — nouveau.
- `Source/AiSolver/CMakeLists.txt`, `Source/HMI/CMakeLists.txt` — modifiés (nouveaux fichiers).

## Tests (obligatoires)
- **`shouldStop` interrompt avant le critère naturel** : sur chacune des quatre boucles, un
  `shouldStop` renvoyant `true` dès le premier appel produit un résultat après une seule
  génération/un seul épisode (jamais jusqu'au plafond).
- **`setOnRecord` observe sans effet de bord** : le CSV produit reste identique bit à bit avec ou
  sans observateur enregistré ; l'observateur reçoit exactement une ligne par appel à `record`.
- **`onGenerationChampion` reçoit le vrai champion** : le réseau transmis produit, rejoué en mode
  Argmax, la même séquence que celle obtenue en interrogeant `TrainingResult::bestIndividual` à
  l'issue d'un run équivalent sans interruption.
- **`argmaxRollout` inchangé après extraction** : même résultat, à niveau/modèle/graine identiques,
  qu'avant l'extraction (non-régression, comparé aux tests déjà existants de `LOT-ANNEXE-19`).

## Points d'attention
- **Chaque extension reste additive** (paramètre par défaut, comportement inchangé si non fourni) :
  aucun appelant existant (`aisolver-cli`, tests unitaires/d'intégration) ne doit changer.
- **`TrainingWorker` ne réimplémente aucune règle** : toute divergence de comportement entre un run
  lancé depuis l'IHM et le même lancé via `aisolver-cli train` (mêmes arguments, même graine) est
  un bug de cette tâche.

## Définition de fait (DoD)
- Hooks `AiSolver` et `TrainingWorker` fonctionnels et testés (`ctest` vert) ; build `/W4 /WX` sans
  avertissement ; Doxygen à jour.

## Notions abordées
Aucune notion d'apprentissage automatique nouvelle : cette tâche est d'ordre logiciel (threading,
observation, interruption). Le vocabulaire employé (génération, épisode, champion, politique) est
défini dans @ref guide-annexe-apprentissage-renforcement.

## Exigences
`EX-IA-022` (nouvelle, partagée avec TACHE-02/03 du même lot).
