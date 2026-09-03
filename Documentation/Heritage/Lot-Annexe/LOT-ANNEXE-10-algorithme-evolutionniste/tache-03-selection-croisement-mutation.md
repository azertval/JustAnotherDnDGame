# TACHE-03 — Sélection, croisement, mutation, élitisme {#lot-annexe-10-tache-03-selection-croisement-mutation}

**Lot :** [LOT-ANNEXE-10](epic.md) · **Emplacement :** `Source/AiSolver/Training/Evolutionary` ·
**Statut :** fait

## Contexte
TACHE-02 assigne un fitness à chaque individu d'une génération évaluée. Cette tâche fournit les
opérateurs génétiques qui produisent la génération suivante à partir de celle-ci : choisir des
parents plus aptes (sélection), les combiner (croisement), introduire de la variation
(mutation), et garantir la non-régression du meilleur individu (élitisme).

## Travail à réaliser
- **Sélection par tournoi** (`selectParent(const Population&, Rng&)`) : tire `TOURNAMENT_SIZE`
  individus au hasard via `Rng` (avec remise), retient celui de fitness maximal du groupe. Choix du
  tournoi plutôt que d'une sélection par rang : complexité `O(TOURNAMENT_SIZE)` par tirage, sans
  trier toute la population à chaque appel — suffisant pour une population de taille modeste.
- **Croisement par moyenne** (`crossover(const Individual&, const Individual&, Rng&) ->
  Individual`) : le réseau enfant a, pour chaque poids, la moyenne des poids correspondants des
  deux parents (opération élément par élément sur les `Tensor` de poids, LOT-ANNEXE-01). Le
  `Rng` n'intervient pas dans le croisement lui-même (opération déterministe une fois les deux
  parents choisis) — conservé en paramètre pour une éventuelle variante future (hors périmètre ici).
- **Mutation** (`mutate(Individual&, float mutationRate, float mutationStrength, Rng&)`) : pour
  chaque poids du réseau, avec probabilité `mutationRate`, ajoute un bruit gaussien
  `Rng::nextGaussian(0, mutationStrength)` (API supposée fournie par LOT-ANNEXE-01).
- **Élitisme** : fonction utilitaire `bestIndividual(const Population&)` retournant l'individu de
  fitness maximal — utilisée par TACHE-04 pour recopier cet individu **inchangé** (sans passer par
  sélection/croisement/mutation) dans la génération suivante.
- **`EvolutionaryConfig`** (struct) : regroupe `tournamentSize`, `mutationRate`, `mutationStrength`,
  avec des valeurs par défaut documentées (constantes nommées, ex. `DEFAULT_TOURNAMENT_SIZE = 3`,
  `DEFAULT_MUTATION_RATE = 0.05f`, `DEFAULT_MUTATION_STRENGTH = 0.1f`).

## Fichiers impactés
- `Source/AiSolver/Training/Evolutionary/GeneticOperators.h`/`.cpp` (nouveaux : sélection,
  croisement, mutation, `bestIndividual`).
- `Source/AiSolver/Training/Evolutionary/EvolutionaryConfig.h` (nouveau).
- Tests : `Source/Test/Unit/AiSolver/Training/test_genetic_operators.cpp` (nouveau).

## Tests (obligatoires)
- **Tournoi ne perd jamais un meilleur candidat présent dans le tirage** : avec des fitness connus
  et un tirage contrôlé (seed fixée), la sélection retourne bien le meilleur du groupe tiré, jamais
  un individu de fitness strictement inférieur à un autre membre du même tirage.
- **Déterminisme de la sélection** : à seed `Rng` fixée, deux appels consécutifs de sélection sur
  la même population produisent le même individu sélectionné.
- **Croisement, cas dégénéré** : le croisement de deux parents aux poids identiques produit un
  enfant aux poids identiques (moyenne de deux valeurs égales = cette valeur).
- **Croisement, propriété de borne** : chaque poids de l'enfant est compris entre les poids
  correspondants des deux parents (propriété de la moyenne, vérifiée sur plusieurs poids).
- **Mutation à taux nul** : `mutationRate = 0` ne modifie aucun poids.
- **Mutation à taux plein, reproductible** : `mutationRate = 1` modifie tous les poids ; à seed
  fixée, la perturbation appliquée est strictement identique entre deux exécutions.
- **`bestIndividual`** : retourne bien l'individu du plus haut fitness dans une population aux
  fitness distincts et connus.

## Points d'attention
- **Ces opérateurs ne touchent jamais la topologie**, fixée par LOT-ANNEXE-03 — uniquement les
  poids (pas d'évolution de structure, décision de cadrage de l'épic).
- **Toute la stochasticité passe par l'instance `Rng` transmise en paramètre**, jamais une source
  globale — condition de reproductibilité de tout le lot.
- **Risque de perte de diversité** : le croisement par moyenne, combiné à un taux de mutation trop
  faible, peut faire converger la population vers des poids proches au fil des générations. Ce
  lot ne corrige pas ce risque algorithmiquement (pas d'auto-adaptation, hors périmètre) — à
  surveiller expérimentalement via les CSV de LOT-ANNEXE-09.

## Définition de fait (DoD)
- Opérateurs génétiques disponibles et testés (`ctest` vert) ; build `/W4 /WX` sans avertissement ;
  Doxygen à jour.

## Notions abordées
@ref guide-annexe-algorithmes-evolutionnistes — population, fitness, sélection, croisement,
mutation.

## Exigences
Aucune exigence propre — contribue à `EX-IA-011` (déclarée en TACHE-04).
