# TACHE-02 — Rapport comparatif CSV par niveau × algorithme {#lot-annexe-15-tache-02-rapport-comparatif-csv}

**Lot :** [LOT-ANNEXE-15](epic.md) · **Emplacement :** `Source/AiSolver/Eval` · **Statut :** fait

## Contexte
TACHE-01 produit, pour **un** modèle donné, `N` répétitions et leurs mesures agrégées (taux de
réussite, pas moyen, variance). Cette tâche agrège ces mesures, pour **plusieurs** modèles exécutés
dans une même campagne, en un rapport comparatif — condition pour répondre objectivement à « quel
algorithme se comporte le mieux sur quel niveau ? ».

## Travail à réaliser
- **`aisolver::BenchmarkReport`** (`Source/AiSolver/Eval/BenchmarkReport.h/.cpp`) : accumule des
  `BenchmarkResult` (sortie de TACHE-01, un par modèle exécuté) au fil d'une campagne, expose
  `void writeCsv(const std::filesystem::path&) const`.
- **Réutilisation du format de colonnes de `TrainingStatsRecorder`** (`LOT-ANNEXE-09`,
  `Source/AiSolver/Stats/CsvFormat.h`) : mêmes conventions de colonnes/échappement, colonnes
  supplémentaires propres à ce rapport (`algorithmName`, `levelName`, `successRate`,
  `meanStepCount`, `stepCountStdDev`) — pas une réimplémentation indépendante de l'échappement CSV.
- Une ligne par couple (algorithme, niveau) exécuté dans la campagne ; pas d'agrégation
  supplémentaire au-delà (aucun classement automatique — décision de cadrage de l'épic : le rapport
  reste descriptif).

## Fichiers impactés
- `Source/AiSolver/Eval/BenchmarkReport.h/.cpp` — nouveau.
- `Source/AiSolver/CMakeLists.txt` — ajout des nouveaux fichiers.

## Tests (obligatoires)
- **Une ligne par modèle accumulé** : un `BenchmarkReport` recevant des résultats pour trois couples
  (algorithme, niveau) distincts produit un CSV de trois lignes de données (plus l'en-tête).
- **Réutilisation de l'échappement CSV** : un `levelName` contenant une virgule est échappé de la
  même façon que dans `TrainingStatsRecorder` (même fonction partagée, pas une seconde
  implémentation qui pourrait diverger).
- **Stabilité de colonnes** : deux campagnes différentes (nombre de modèles différent) produisent
  des fichiers avec le **même** en-tête.

## Points d'attention
- **Ce rapport ne modifie ni ne relit aucun fichier `stats.csv` de `TrainingStatsRecorder`** : il
  consomme les résultats déjà calculés par TACHE-01 (mesures post-entraînement), pas les données
  d'entraînement elles-mêmes — deux préoccupations distinctes (progression pendant l'entraînement
  vs comparaison après entraînement) qui ne doivent pas être mélangées dans un seul fichier.
- **Aucun jugement de « meilleur algorithme » encodé dans le format** (pas de colonne de rang ou de
  score composite) — cohérent avec la décision de cadrage de l'épic : le rapport reste des chiffres
  bruts, à lire par un humain.

## Définition de fait (DoD)
- `BenchmarkReport` disponible et testé (`ctest` vert) ; build `/W4 /WX` sans avertissement ;
  Doxygen à jour.

## Notions abordées
@ref guide-annexe-evaluation-rl — variance entre exécutions, graines multiples, mesure honnête d'un
agent entraîné.

## Exigences
`EX-IA-016` (nouvelle, partagée avec TACHE-01/03 du même lot).
