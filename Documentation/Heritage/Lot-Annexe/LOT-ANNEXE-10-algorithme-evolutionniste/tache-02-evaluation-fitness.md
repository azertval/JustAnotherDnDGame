# TACHE-02 — Évaluation de fitness {#lot-annexe-10-tache-02-evaluation-fitness}

**Lot :** [LOT-ANNEXE-10](epic.md) · **Emplacement :** `Source/AiSolver/Training/Evolutionary` ·
**Statut :** fait

## Contexte
TACHE-01 fournit une population d'individus aux poids indépendants, mais sans aucun moyen de les
comparer. Cette tâche transforme chaque individu en un score : le faire jouer une fois, en entier,
sur `HeadlessLevelEnvironment` (LOT-ANNEXE-05), et cumuler la récompense telle que définie par
LOT-ANNEXE-08.

## Travail à réaliser
- **`aisolver::training::evolutionary::evaluateFitness(Individual&, HeadlessLevelEnvironment&)`** :
  réinitialise l'environnement (`reset()`), boucle jusqu'à fin d'épisode (succès, échec ou
  dépassement du nombre maximal de pas, tel que défini par LOT-ANNEXE-08) ; à chaque pas, encode
  l'observation courante (LOT-ANNEXE-06), effectue la propagation avant du réseau de l'individu
  (LOT-ANNEXE-03), **décode l'action en `argmax`** (jamais d'échantillonnage stochastique — décision
  de cadrage de l'épic), applique l'action à l'environnement et cumule la récompense retournée.
  Écrit le total dans `Individual::fitness` à la fin de l'épisode.
- **`Population::evaluateAll(HeadlessLevelEnvironment&)`** : appelle `evaluateFitness` pour chaque
  individu de la population, séquentiellement, sur la **même** instance d'environnement réutilisée
  d'un individu à l'autre (pas de parallélisation, décision de cadrage de l'épic) — chaque appel
  commence par un `reset()`, aucun état ne fuit d'un individu au suivant.
- Le fitness reste **exactement** la récompense cumulée telle que produite par LOT-ANNEXE-08 :
  aucune repondération, normalisation ou troncature locale à cette tâche — une seule source de
  vérité pour la sémantique de la récompense.

## Fichiers impactés
- `Source/AiSolver/Training/Evolutionary/FitnessEvaluator.h`/`.cpp` (nouveaux).
- `Source/AiSolver/Training/Evolutionary/Population.h`/`.cpp` (ajout de `evaluateAll`).
- Tests : `Source/Test/Unit/AiSolver/Training/test_fitness_evaluator.cpp` (nouveau).

## Tests (obligatoires)
- **Déterminisme d'une évaluation** : deux évaluations successives du même individu (poids
  inchangés) sur le même niveau produisent un fitness strictement identique (conséquence du
  décodage `argmax` et du déterminisme de `HeadlessLevelEnvironment`).
- **Ordre de fitness cohérent** : avec un réseau construit à la main pour produire une action fixe
  connue (fixture de test, pas d'entraînement), un individu qui progresse effectivement vers la
  sortie obtient un fitness supérieur à un individu qui reste immobile ou recule — vérifie le sens
  de la comparaison sans dépendre d'un entraînement réel.
- **Terminaison garantie** : l'évaluation d'un individu produisant en boucle une action « ne rien
  faire » se termine bien avant le plafond de pas défini par LOT-ANNEXE-08 (pas de boucle infinie).
- **Aucune fuite d'état entre individus** : `Population::evaluateAll` sur deux individus
  différents utilisant la même instance d'environnement produit pour chacun le même fitness que si
  chacun était évalué isolément sur une instance fraîche.

## Points d'attention
- **Une seule instance d'environnement réutilisée** (pas N instances) : cohérent avec l'évaluation
  séquentielle décidée dans l'épic ; chaque `evaluateFitness` doit donc commencer par un `reset()`
  explicite, jamais supposer un état initial implicite.
- **Le fitness n'est pas réinterprété ici** : le signe, l'échelle et les composantes de la
  récompense appartiennent entièrement à LOT-ANNEXE-08 ; cette tâche se contente de les cumuler sur
  un épisode.
- **Hypothèse héritée de LOT-ANNEXE-05** : `HeadlessLevelEnvironment` ne comporte aucune source
  d'aléa propre (les dangers mobiles/temporisés du jeu sont déterministes par construction, cf.
  `Source/Elements/Levels/README.md`) — si ce n'était pas le cas, deux évaluations du même individu
  pourraient diverger et casseraient la reproductibilité exigée par TACHE-05 ; à vérifier au
  moment de l'intégration avec LOT-ANNEXE-05, pas re-décidé ici.

## Définition de fait (DoD)
- Évaluation de fitness disponible, déterministe et testée (`ctest` vert) ; build `/W4 /WX` sans
  avertissement ; Doxygen à jour.

## Notions abordées
@ref guide-annexe-algorithmes-evolutionnistes — population, fitness, sélection, croisement,
mutation.

## Exigences
Aucune exigence propre — contribue à `EX-IA-011` (déclarée en TACHE-04).
