# TACHE-04 — Un fichier CSV par run, sous /TrainingRuns/ {#lot-annexe-09-tache-04-fichier-par-run}

**Lot :** [LOT-ANNEXE-09](epic.md) · **Emplacement :** `Source/AiSolver/Stats` · **Statut :** à faire

## Contexte
`TrainingStatsRecorder` (TACHE-01) prend un chemin de fichier CSV en construction ; cette tâche
définit comment ce chemin est construit pour que deux runs (même niveau ou niveaux différents) ne
se marchent jamais dessus, et où ces fichiers générés vivent dans l'arborescence du dépôt.

## Travail à réaliser
- **`std::filesystem::path aisolver::makeTrainingRunPath(const std::filesystem::path&
  trainingRunsRoot, std::string_view levelName, std::string_view runId)`** (`Source/AiSolver/Stats/
  TrainingRunPath.h/.cpp`) : construit `trainingRunsRoot / levelName / runId / "stats.csv"`, crée
  les dossiers intermédiaires s'ils n'existent pas (`std::filesystem::create_directories`).
- **`std::string aisolver::generateRunId()`** : identifiant de run par horodatage ISO 8601
  compact (ex. `20260729-143512`), suffixé si nécessaire pour éviter une collision dans la même
  seconde (compteur incrémental en repli).
- Constante documentée `constexpr const char* kDefaultTrainingRunsRoot = "TrainingRuns";` (chemin
  relatif à la racine du dépôt, cohérent avec l'entrée `.gitignore` `/TrainingRuns/` déjà en place)
  — l'appelant (CLI, `LOT-ANNEXE-19`) résout ce chemin relatif en absolu selon son propre répertoire
  de travail, cette tâche ne fait aucune hypothèse sur le répertoire courant du processus.

## Fichiers impactés
- `Source/AiSolver/Stats/TrainingRunPath.h/.cpp` — nouveau.
- `Source/AiSolver/CMakeLists.txt` — ajout des nouveaux fichiers.

## Tests (obligatoires)
- **Construction du chemin** : `makeTrainingRunPath` produit le chemin attendu et crée
  effectivement les dossiers intermédiaires (vérifié avec un répertoire temporaire de test, pas le
  vrai `/TrainingRuns/` du dépôt).
- **Absence de collision** : deux appels à `generateRunId()` dans la même seconde produisent des
  identifiants distincts.
- **Isolation par niveau** : deux runs sur des `levelName` différents produisent des chemins
  disjoints, y compris leurs dossiers intermédiaires.

## Points d'attention
- **`kDefaultTrainingRunsRoot` reste un nom relatif** : cette tâche ne fixe jamais un chemin absolu
  en dur, pour rester portable d'une machine à l'autre (même principe que `levelPath` du format de
  rejeu, `LOT-ANNEXE-07`).
- **Le dossier `/TrainingRuns/` est déjà exclu de Git** (`.gitignore`, ajouté par le cadrage général
  du programme) — cette tâche ne modifie pas `.gitignore`, elle produit seulement des fichiers dans
  un dossier déjà ignoré.

## Définition de fait (DoD)
- `makeTrainingRunPath`/`generateRunId` disponibles et testés (`ctest` vert) ; build `/W4 /WX` sans
  avertissement ; Doxygen à jour.

## Notions abordées
@ref guide-annexe-apprentissage-renforcement — récompense cumulée (retour), plafond de performance
d'un entraînement.

## Exigences
`EX-IA-010` (nouvelle, partagée avec TACHE-01/02/03/05 du même lot).
