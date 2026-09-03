# TACHE-01 — Exécution croisée (modèle de A exécuté sur B) {#lot-annexe-16-tache-01-execution-croisee}

**Lot :** [LOT-ANNEXE-16](epic.md) · **Emplacement :** `Source/AiSolver/Eval` · **Statut :** fait

## Contexte
`aisolver::eval::BenchmarkRunner::run` (`LOT-ANNEXE-15`) accepte déjà indépendamment un
`TrainedPolicy` et un `levelPath` — rien n'empêche techniquement de les faire porter sur deux
niveaux différents, seule l'intention (mesurer un transfert plutôt qu'une fiabilité sur niveau
d'origine) est nouvelle. Cette tâche l'exploite explicitement et outille le rapport pour distinguer
les deux niveaux.

## Travail à réaliser
- **`aisolver::eval::CrossLevelBenchmarkResult`** (`Source/AiSolver/Eval/CrossLevelBenchmark.h/.cpp`) :
  structure `std::string trainedOnLevel; std::string executedOnLevel; aisolver::eval::
  BenchmarkResult result;` — assemble le résultat déjà produit par `BenchmarkRunner::run` (appelé
  sans modification) avec les deux identifiants de niveau.
- **`std::vector<CrossLevelBenchmarkResult> runCrossLevelCampaign(const std::vector<CrossLevelPair>&
  pairs, const BenchmarkConfig&)`** où `CrossLevelPair { std::filesystem::path modelPath; std::string
  trainedOnLevel; std::filesystem::path executedOnLevelPath; std::string executedOnLevel; }` —
  charge le `TrainedPolicy` approprié (réutilise les adaptateurs de `LOT-ANNEXE-15`, TACHE-01, selon
  l'algorithme d'origine du modèle) et appelle `BenchmarkRunner::run` avec `executedOnLevelPath`.
- Extension de `aisolver::BenchmarkReport` (`LOT-ANNEXE-15`, TACHE-02) : surcharge
  `writeCsv` acceptant un `std::vector<CrossLevelBenchmarkResult>`, ajoutant les deux colonnes
  `trainedOnLevel`/`executedOnLevel` (au lieu de la colonne unique `levelName` du rapport
  non-croisé) — implémentée par composition de la sérialisation existante, pas par duplication du
  format de colonnes commun.

## Fichiers impactés
- `Source/AiSolver/Eval/CrossLevelBenchmark.h/.cpp` — nouveau.
- `Source/AiSolver/Eval/BenchmarkReport.h/.cpp` — modifié (surcharge `writeCsv`).
- `Source/AiSolver/CMakeLists.txt` — ajout des nouveaux fichiers.

## Tests (obligatoires)
- **Aucune régression sur le cas non-croisé** : l'ajout de la surcharge de `writeCsv` ne modifie
  pas le comportement de `BenchmarkReport::writeCsv` existant (`LOT-ANNEXE-15`), vérifié par les
  tests déjà existants de ce lot restant verts.
- **Distinction des deux niveaux dans le rapport croisé** : un `CrossLevelBenchmarkResult` où
  `trainedOnLevel != executedOnLevel` produit une ligne CSV avec les deux valeurs dans des colonnes
  séparées, jamais fusionnées.
- **Réutilisation stricte de `BenchmarkRunner::run`** : vérifié par construction (le fichier
  n'introduit aucune nouvelle boucle de simulation, seulement un assemblage autour de l'appel
  existant) — pas un test à proprement parler, mais une contrainte vérifiable par relecture/diff,
  documentée comme condition de revue.
- **Campagne multi-paires** : `runCrossLevelCampaign` sur trois paires distinctes produit trois
  `CrossLevelBenchmarkResult`, un par paire, dans l'ordre fourni.

## Points d'attention
- **Le chargement du bon adaptateur `TrainedPolicy` selon l'algorithme d'origine reste à la charge
  de l'appelant** (`CrossLevelPair` ne devine pas l'algorithme depuis le chemin de modèle) — cohérent
  avec la façon dont `LOT-ANNEXE-15` charge déjà ses quatre adaptateurs explicitement, pas par
  détection automatique de format.
- **`executedOnLevelPath` peut désigner n'importe quel niveau de `Source/Elements/Levels`**, y
  compris celui d'origine du modèle (cas dégénéré valide, équivalent à un appel non-croisé) — aucune
  validation n'interdit ce cas, qui sert d'ailleurs de vérification croisée avec les résultats déjà
  obtenus par `LOT-ANNEXE-15`.

## Définition de fait (DoD)
- `CrossLevelBenchmarkResult`/`runCrossLevelCampaign`/surcharge de `writeCsv` disponibles et
  testés (`ctest` vert) ; build `/W4 /WX` sans avertissement ; Doxygen à jour.

## Notions abordées
@ref guide-annexe-evaluation-rl — généralisation hors du niveau d'entraînement, sur-apprentissage à
un seul environnement.

## Exigences
`EX-IA-017` (nouvelle, partagée avec TACHE-02 du même lot).
