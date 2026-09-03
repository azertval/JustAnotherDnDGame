# TACHE-05 — Tests : écriture/relecture, stabilité du format {#lot-annexe-09-tache-05-tests}

**Lot :** [LOT-ANNEXE-09](epic.md) · **Emplacement :** `Source/Test/Unit/AiSolver/Stats` · **Statut :** à faire

## Contexte
TACHE-01 à 04 introduisent chacune leurs cas de test locaux (interface, format de colonnes, moyenne
mobile, chemin de fichier) ; cette tâche consolide un test de bout en bout du recorder complet, qui
vérifie que l'ensemble produit un fichier CSV réellement exploitable une fois toutes les briques
assemblées.

## Travail à réaliser
- **`Source/Test/Unit/AiSolver/Stats/test_training_stats_recorder.cpp`** : construit un
  `TrainingStatsRecorder` sur un fichier temporaire, y enregistre une séquence de
  `TrainingStatsRow` synthétiques (y compris un scénario de progression puis de plateau), relit le
  fichier produit avec un analyseur CSV minimal dédié au test, et vérifie : nombre de lignes exact,
  en-tête conforme, valeurs de chaque colonne cohérentes avec ce qui a été enregistré, colonnes de
  moyenne mobile/delta cohérentes avec `MovingAverageTracker` appliqué indépendamment à la même
  séquence.
- **`Source/Test/Unit/AiSolver/Stats/test_training_run_path.cpp`** : cas de TACHE-04
  (construction de chemin, génération d'identifiant de run).

## Fichiers impactés
- `Source/Test/Unit/AiSolver/Stats/test_training_stats_recorder.cpp` — nouveau.
- `Source/Test/Unit/AiSolver/Stats/test_training_run_path.cpp` — nouveau.
- `Source/Test/CMakeLists.txt` — ajout des nouveaux fichiers à la cible `UnitTests`.

## Tests (obligatoires)
- **Bout en bout** : la séquence enregistrée puis relue correspond exactement à ce qui a été
  fourni à `record`, colonnes calculées incluses.
- **Fichier interrompu simulé** : un recorder détruit après `k` appels sur une séquence de `k+m`
  valeurs prévues laisse un fichier valide de `k` lignes (pas de ligne partielle, pas de corruption).
- Voir aussi TACHE-04 pour les cas de construction de chemin.

## Points d'attention
- **L'analyseur CSV utilisé pour relire dans les tests est un utilitaire de test, jamais promu en
  code de production** : un analyseur complet (gestion générale de l'échappement, types multiples)
  n'est pas nécessaire côté écriture (TACHE-02, un seul champ textuel à risque) ni côté lecture pour
  ce lot — un futur lot consommateur (génération 4, `LOT-ANNEXE-15`) définira sa propre lecture si
  et quand il en a besoin, sans qu'un couplage prématuré ne soit introduit ici.

## Définition de fait (DoD)
- Les deux suites de tests vertes (`ctest`) ; build `/W4 /WX` sans avertissement ; Doxygen à jour ;
  `EX-IA-010` déclarée dans l'`epic.md` du lot.

## Notions abordées
@ref guide-annexe-apprentissage-renforcement — récompense cumulée (retour), plafond de performance
d'un entraînement.

## Exigences
`EX-IA-010` (nouvelle, du même lot).
