# TACHE-01 — `TrainingStatsRecorder` : interface générique {#lot-annexe-09-tache-01-training-stats-recorder}

**Lot :** [LOT-ANNEXE-09](epic.md) · **Emplacement :** `Source/AiSolver/Stats` · **Statut :** à faire

## Contexte
Chaque algorithme d'entraînement (génération 2 : une génération de population par itération,
génération 3 : un épisode ou un lot d'épisodes) doit pouvoir journaliser sa progression sans
connaître le format de fichier sous-jacent ni dupliquer la logique d'écriture. Cette tâche pose
l'interface que tous consommeront de façon identique.

## Travail à réaliser
- **`struct aisolver::TrainingStatsRow`** (`Source/AiSolver/Stats/TrainingStatsRecorder.h`) :
  `int index; float bestReward; float meanReward; float worstReward; float rewardStdDev; int
  bestStepCount; float successRate; uint64_t seed; std::string levelName;` (l'horodatage et les
  colonnes de moyenne mobile/delta sont calculés par le recorder, pas fournis par l'appelant — voir
  TACHE-03).
- **`class aisolver::TrainingStatsRecorder`** : constructeur `TrainingStatsRecorder(const
  std::filesystem::path& outputCsvPath)`, méthode `void record(const TrainingStatsRow&)`. Ouvre le
  fichier en écriture à la construction (écrit l'en-tête de colonnes immédiatement), chaque appel à
  `record` ajoute une ligne et force l'écriture sur disque (`flush`) — pour qu'un arrêt prématuré du
  processus ne perde pas les lignes déjà journalisées.
- Aucune dépendance à un algorithme particulier : `TrainingStatsRecorder` ne référence ni
  `EvolutionaryTrainer` (`LOT-ANNEXE-10`) ni aucun module de `Source/AiSolver/Training/*`.

## Fichiers impactés
- `Source/AiSolver/Stats/TrainingStatsRecorder.h/.cpp` — nouveau.
- `Source/AiSolver/CMakeLists.txt` — ajout des nouveaux fichiers.

## Tests (obligatoires)
- **Un appel à `record` ajoute exactement une ligne** au fichier CSV, sans altérer les lignes déjà
  écrites.
- **`flush` après chaque `record`** : le contenu du fichier sur disque reflète immédiatement chaque
  appel, vérifiable en relisant le fichier entre deux appels à `record` sans fermer le recorder.
- **Aucune dépendance à un module de `Source/AiSolver/Training`** : vérifié par construction (le
  fichier `.h`/`.cpp` n'inclut aucun en-tête de `Training/*`), pas seulement documenté.

## Points d'attention
- **Le `flush` après chaque ligne a un coût (une écriture disque par génération/épisode)**,
  volontairement accepté : la fréquence d'appel (une fois par génération ou par épisode, jamais par
  pas de simulation) reste largement inférieure à ce qui rendrait ce coût perceptible face au temps
  de calcul d'une génération/d'un épisode complet.
- **Le recorder ne rouvre jamais un fichier existant en ajout** : chaque instance correspond à un
  run neuf, avec son propre fichier créé à la construction — cohérent avec TACHE-04 (un fichier par
  run, nommé par identifiant de run).

## Définition de fait (DoD)
- `TrainingStatsRow`/`TrainingStatsRecorder` disponibles et testés (`ctest` vert) ; build `/W4 /WX`
  sans avertissement ; Doxygen à jour.

## Notions abordées
@ref guide-annexe-apprentissage-renforcement — récompense cumulée (retour), plafond de performance
d'un entraînement.

## Exigences
`EX-IA-010` (nouvelle, partagée avec TACHE-02/03/04/05 du même lot).
