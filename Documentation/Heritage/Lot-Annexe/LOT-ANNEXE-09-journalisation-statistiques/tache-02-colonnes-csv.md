# TACHE-02 — Colonnes CSV {#lot-annexe-09-tache-02-colonnes-csv}

**Lot :** [LOT-ANNEXE-09](epic.md) · **Emplacement :** `Source/AiSolver/Stats` · **Statut :** à faire

## Contexte
`TrainingStatsRecorder` (TACHE-01) sait qu'il doit écrire une ligne par appel ; cette tâche définit
le format exact de cette ligne — l'en-tête, l'ordre des colonnes, l'échappement — condition pour que
le harnais de benchmark (`LOT-ANNEXE-15`) puisse plus tard lire ces fichiers de façon fiable.

## Travail à réaliser
- **En-tête de colonnes** (`Source/AiSolver/Stats/CsvFormat.h/.cpp`, fonction `std::string
  csvHeader()`), fixe : `index,bestReward,meanReward,worstReward,rewardStdDev,bestStepCount,
  successRate,seed,levelName,timestampIso8601,movingAverageReward,rewardDelta` (les deux dernières
  colonnes, calculées, sont ajoutées par TACHE-03).
- **Sérialisation d'une ligne** (`std::string csvRow(const TrainingStatsRow&, float
  movingAverage, float delta, std::string_view timestampIso8601)`) : valeurs séparées par virgules,
  `levelName` entre guillemets si elle contient une virgule (échappement CSV minimal mais correct,
  pas une bibliothèque CSV complète — un seul champ textuel dans tout le schéma).
- `TrainingStatsRecorder::TrainingStatsRecorder` (TACHE-01) écrit `csvHeader()` immédiatement à la
  construction ; chaque `record` construit sa ligne via `csvRow(...)`.

## Fichiers impactés
- `Source/AiSolver/Stats/CsvFormat.h/.cpp` — nouveau.
- `Source/AiSolver/Stats/TrainingStatsRecorder.cpp` — modifié (utilise `csvHeader`/`csvRow`).
- `Source/AiSolver/CMakeLists.txt` — ajout des nouveaux fichiers.

## Tests (obligatoires)
- **En-tête exact** : `csvHeader()` produit la liste de colonnes documentée, dans l'ordre exact.
- **Échappement** : un `levelName` contenant une virgule est correctement entouré de guillemets
  dans `csvRow`, et se relit comme une seule valeur (test round-trip avec un analyseur CSV minimal
  dédié au test, pas une dépendance de production).
- **Stabilité inter-lignes** : deux appels à `csvRow` avec des `TrainingStatsRow` différentes
  produisent des lignes du **même** nombre de champs, dans le même ordre.

## Points d'attention
- **Aucune dépendance à une bibliothèque CSV tierce** : le format réel n'a qu'un seul champ textuel
  à risque d'échappement (`levelName`) — une fonction dédiée de quelques lignes suffit, conforme à
  la contrainte « from scratch, aucune dépendance tierce nouvelle » du programme.
- **L'horodatage est au format ISO 8601** (`AAAA-MM-JJThh:mm:ssZ`), trié lexicographiquement dans
  le même ordre que chronologiquement — utile si plusieurs runs sont un jour agrégés (génération 4).

## Définition de fait (DoD)
- `csvHeader`/`csvRow` disponibles et testés (`ctest` vert) ; build `/W4 /WX` sans avertissement ;
  Doxygen à jour.

## Notions abordées
@ref guide-annexe-apprentissage-renforcement — récompense cumulée (retour), plafond de performance
d'un entraînement.

## Exigences
`EX-IA-010` (nouvelle, partagée avec TACHE-01/03/04/05 du même lot).
