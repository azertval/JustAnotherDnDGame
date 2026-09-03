# TACHE-04 — Boucle de génération {#lot-annexe-10-tache-04-boucle-generation}

**Lot :** [LOT-ANNEXE-10](epic.md) · **Emplacement :** `Source/AiSolver/Training/Evolutionary` ·
**Statut :** fait

## Contexte
TACHE-01 à TACHE-03 fournissent les briques élémentaires (population, évaluation de fitness,
opérateurs génétiques) mais aucune ne les assemble en une boucle exécutable. Cette tâche fournit ce
point d'assemblage — la première chose qu'un appelant externe (LOT-ANNEXE-11) manipule réellement —
et la connecte à `TrainingStatsRecorder` (LOT-ANNEXE-09) pour rendre chaque génération observable.

## Travail à réaliser
- **`aisolver::training::evolutionary::EvolutionaryTrainer`** : construit à partir d'une
  `EvolutionaryConfig` (TACHE-03, incluant la taille de population de TACHE-01), d'un
  `HeadlessLevelEnvironment&` (LOT-ANNEXE-05), d'un `Rng` seedé explicitement et d'un
  `TrainingStatsRecorder&` (LOT-ANNEXE-09). Possède une `Population` interne.
- **`runGeneration()`** : (1) `population.evaluateAll(environment)` (TACHE-02) ; (2) enregistre les
  statistiques de la génération courante via `recorder.record(...)` (indice de génération, meilleur
  fitness, fitness moyen — schéma exact propriété de LOT-ANNEXE-09) ; (3) construit la génération
  suivante dans un buffer distinct (double tampon, jamais de mutation en place) : copie l'élite
  (`bestIndividual`, TACHE-03) sans réévaluation à l'indice 0, puis remplit le reste par sélection +
  croisement + mutation (TACHE-03) ; (4) échange les deux buffers.
- **`bestIndividual() const`** : accès en lecture seule au meilleur individu connu de la génération
  courante — utilisé plus tard par LOT-ANNEXE-11/TACHE-02 pour le rejeu final.
- Aucune notion de critère d'arrêt métier dans cette classe : `runGeneration()` exécute
  **exactement une** génération par appel ; c'est à l'appelant (LOT-ANNEXE-11) de décider combien
  de fois l'invoquer et pourquoi s'arrêter (décision de cadrage de l'épic).

## Fichiers impactés
- `Source/AiSolver/Training/Evolutionary/EvolutionaryTrainer.h`/`.cpp` (nouveaux).
- Tests : `Source/Test/Unit/AiSolver/Training/test_evolutionary_trainer.cpp` (nouveau).

## Tests (obligatoires)
- **Taille de population préservée** : après `runGeneration()`, la population contient toujours
  exactement `N` individus.
- **Non-régression de l'élite** : sur plusieurs générations consécutives, `bestIndividual().fitness`
  n'est jamais strictement inférieur à sa valeur à la génération précédente.
- **Indice de génération correct** : les appels à `TrainingStatsRecorder::record` reçoivent un
  indice de génération strictement croissant, sans saut ni répétition, correspondant au nombre
  d'appels effectifs à `runGeneration()`.
- **Cohérence de `bestIndividual()`** : à tout instant après un appel à `runGeneration()`, l'individu
  retourné a bien le fitness maximal de la population courante.

## Points d'attention
- **L'élite n'est pas réévaluée** : son fitness de la génération précédente reste valable tant que
  ses poids ne changent pas (elle est recopiée, pas régénérée) — économise un run d'environnement
  complet par génération ; ne pas réintroduire un appel à `evaluateFitness` sur l'élite par erreur.
- **Double tampon obligatoire** : construire la génération suivante « en place » dans le même
  buffer risquerait qu'un individu sélectionné tardivement comme parent ait déjà été écrasé par un
  enfant d'une opération précédente de la **même** génération.
- **`EvolutionaryTrainer` ne sait pas ce qu'est un niveau « résolu »** : uniquement le fitness
  numérique, jamais un booléen de succès — ce booléen (nécessaire au critère d'arrêt de
  LOT-ANNEXE-11) provient de LOT-ANNEXE-08, pas de cette classe.

## Définition de fait (DoD)
- Boucle de génération disponible, testée (`ctest` vert), journalisée via `TrainingStatsRecorder` ;
  build `/W4 /WX` sans avertissement ; Doxygen à jour ; `EX-IA-011` déclarée.

## Notions abordées
@ref guide-annexe-algorithmes-evolutionnistes — population, fitness, sélection, croisement,
mutation.

## Exigences
`EX-IA-011` (nouvelle).
