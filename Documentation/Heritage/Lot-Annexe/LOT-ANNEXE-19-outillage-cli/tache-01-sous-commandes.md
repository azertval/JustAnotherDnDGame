# TACHE-01 — Sous-commandes train/evaluate/export-replay {#lot-annexe-19-tache-01-sous-commandes}

**Lot :** [LOT-ANNEXE-19](epic.md) · **Emplacement :** `Source/AiSolver/Cli` · **Statut :** fait

## Contexte
Toute la logique nécessaire existe déjà dans `Source/AiSolver` (générations 2 à 4) ; il manque un
exécutable pour l'invoquer sans écrire de code à chaque expérimentation. Cette tâche pose
l'exécutable et ses trois sous-commandes.

## Travail à réaliser
- **`Source/AiSolver/Cli/Main.cpp`** : point d'entrée, analyse la première position (nom de
  sous-commande), délègue à `runTrain`/`runEvaluate`/`runExportReplay` (`Source/AiSolver/Cli/
  Commands.h/.cpp`) ; code de sortie non nul et message sur `stderr` pour toute erreur récupérable
  (niveau introuvable, modèle introuvable, arguments invalides).
- **`runTrain(const TrainArgs&)`** : selon `--algo`, construit `aisolver::training::
  LevelTrainingSession` (évolutionniste, `LOT-ANNEXE-11`) ou l'équivalent de génération 3
  (`PolicyGradient`/`ActorCritic`/`Advanced`, selon la forme exposée par ces lots), lui fournit un
  `TrainingStatsRecorder` construit via `makeTrainingRunPath` (`LOT-ANNEXE-09`), lance
  l'entraînement jusqu'à son critère d'arrêt propre.
- **`runEvaluate(const EvaluateArgs&)`** : charge le `TrainedPolicy` adapté (`LOT-ANNEXE-15`) depuis
  `--model`, appelle `BenchmarkRunner::run`, affiche le résultat sur `stdout` (taux de réussite, pas
  moyen) et écrit optionnellement un rapport CSV (`--report <chemin>`, réutilise `BenchmarkReport`).
- **`runExportReplay(const ExportReplayArgs&)`** : charge le modèle, décode en mode `Argmax`
  (`LOT-ANNEXE-07`) sur `HeadlessLevelEnvironment` (`LOT-ANNEXE-05`), assemble un `ReplayFile` et
  l'écrit via `aisolver::writeReplay` vers `--output`.

## Fichiers impactés
- `Source/AiSolver/Cli/Main.cpp` — nouveau.
- `Source/AiSolver/Cli/Commands.h/.cpp` — nouveau.
- `Source/AiSolver/CMakeLists.txt` — nouvel exécutable `aisolver-cli` (`add_executable`, lié à la
  bibliothèque `AiSolver`).

## Tests (obligatoires)
- **Analyse d'arguments** : chaque sous-commande accepte ses arguments requis et rejette
  (message explicite, code de sortie non nul) un argument manquant ou invalide — testé sur la
  fonction d'analyse isolée (`Source/Test/Unit/AiSolver/Cli/test_argument_parsing.cpp`), pas via
  l'exécutable lui-même.
- **`runTrain` délègue effectivement** : appelé avec `--algo evo` sur un niveau de contrôle simple,
  produit un fichier CSV non vide et, si résolu, un fichier de rejeu valide.
- **`runEvaluate` cohérent avec un appel direct** : sur le même modèle/niveau, `runEvaluate`
  produit un résultat numériquement identique à un appel direct à `BenchmarkRunner::run` (même
  graine).
- **`runExportReplay` produit un rejeu valide** : `aisolver::validateReplay` sur le fichier produit
  renvoie `std::nullopt`.

## Points d'attention
- **`Main.cpp` reste le plus mince possible** (analyse de sous-commande, délégation) : toute
  logique testable doit vivre dans `Commands.h/.cpp` pour rester unitairement testable sans lancer
  le vrai exécutable.
- **Aucune des trois fonctions ne réimplémente une règle déjà cadrée en amont** : un désaccord entre
  le comportement de la CLI et un appel direct à l'API sous-jacente est un bug de ce lot, jamais une
  variante assumée.

## Définition de fait (DoD)
- `aisolver-cli` compile et ses trois sous-commandes fonctionnent sur un cas de bout en bout
  chacune ; tests d'analyse d'arguments verts (`ctest`) ; build `/W4 /WX` sans avertissement ;
  Doxygen à jour.

## Notions abordées
Aucune notion d'apprentissage automatique nouvelle : cette tâche est d'ordre logiciel (format de
fichier, outillage, intégration continue). Le vocabulaire employé (épisode, rejeu, politique, agent)
est défini dans @ref guide-annexe-apprentissage-renforcement.

## Exigences
`EX-IA-020` (nouvelle, partagée avec TACHE-02 du même lot).
