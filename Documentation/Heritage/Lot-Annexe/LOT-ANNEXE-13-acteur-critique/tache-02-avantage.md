# TACHE-02 — Avantage remplaçant le retour brut dans la perte de politique {#lot-annexe-13-tache-02-avantage}

**Lot :** [LOT-ANNEXE-13](epic.md) · **Emplacement :** `Source/AiSolver/Training/ActorCritic` ·
**Statut :** fait

## Contexte
Le critique (TACHE-01) sait désormais estimer la valeur d'un état. Cette tâche exploite cette
estimation pour calculer, à chaque pas d'une trajectoire, l'**avantage** — `retour_t − valeur_estimée_t`
— et branche ce nouveau signal dans la perte de politique de LOT-ANNEXE-12 à la place du retour brut
`G_t`. C'est le cœur de la réduction de variance : l'avantage est positif quand l'action a fait
**mieux que ce à quoi on s'attendait** depuis cet état, négatif dans le cas contraire — un signal
beaucoup plus discriminant que le retour brut de tout l'épisode.

## Travail à réaliser
- **`aisolver::training::computeAdvantages`**
  (`Source/AiSolver/Training/ActorCritic/AdvantageCalculator.h/.cpp`) : fonction
  `std::vector<float> computeAdvantages(const std::vector<float>& returns, training::CriticNetwork& critic, const Trajectory& trajectory)` —
  pour chaque pas, appelle `critic.forward` sur l'observation du pas (valeur détachée du graphe,
  `float`, pour ce calcul de diagnostic/vecteur), calcule `advantage_t = returns[t] − valeur_t`.
- **`aisolver::training::computeActorCriticLoss`**
  (`Source/AiSolver/Training/ActorCritic/ActorCriticLoss.h/.cpp`) : variante de
  `computeReinforceLoss` (LOT-ANNEXE-12, TACHE-03) qui prend un `std::vector<float>` d'avantages à
  la place du `std::vector<float>` de retours — même construction de graphe d'autodiff
  (`-log π(a_t|s_t) × advantage_t`, moyenné sur l'épisode), même appel à `policy.forward` rejoué par
  pas. Réutilise la politique (LOT-ANNEXE-03) sans modification.
- La fonction de LOT-ANNEXE-12 (`computeReinforceLoss`) n'est **pas dupliquée entièrement** :
  factoriser la partie commune (construction du graphe `-log π(a_t|s_t) × poids_t`, où `poids_t` est
  soit le retour brut soit l'avantage selon l'appelant) dans une fonction interne partagée entre
  `Source/AiSolver/Training/PolicyGradient` et `Source/AiSolver/Training/ActorCritic`, pour éviter
  que les deux lots divergent silencieusement sur la formule de perte.

## Fichiers impactés
- `Source/AiSolver/Training/ActorCritic/AdvantageCalculator.h` (nouveau).
- `Source/AiSolver/Training/ActorCritic/AdvantageCalculator.cpp` (nouveau).
- `Source/AiSolver/Training/ActorCritic/ActorCriticLoss.h` (nouveau).
- `Source/AiSolver/Training/ActorCritic/ActorCriticLoss.cpp` (nouveau).
- `Source/AiSolver/Training/PolicyGradient/ReinforceLoss.h/.cpp` (factorisation de la formule de
  perte commune, sans changement de comportement pour LOT-ANNEXE-12).
- `Source/AiSolver/CMakeLists.txt` (nouveaux fichiers).

## Tests (obligatoires)
- **Signe de l'avantage** : un pas dont le retour dépasse la valeur estimée produit un avantage
  positif ; l'inverse produit un avantage négatif ; égalité produit un avantage nul.
- **Avantage nul dégénéré** : si le critique est initialisé pour prédire exactement les retours
  observés (cas construit pour le test), tous les avantages calculés sont proches de zéro — le
  gradient de la perte de politique correspondante l'est alors aussi.
- **Non-régression de la formule REINFORCE** : après factorisation, `computeReinforceLoss`
  (LOT-ANNEXE-12) produit des résultats **strictement identiques** à ceux d'avant cette tâche, sur
  les mêmes cas de test déjà existants (gradient checking de LOT-ANNEXE-12 toujours vert, sans
  modification de ses assertions).
- **Gradient checking de `computeActorCriticLoss`** : par rapport aux poids de la politique
  uniquement (le critique est traité comme une donnée détachée dans ce graphe, cf. points
  d'attention) — même méthodologie que LOT-ANNEXE-12 (TACHE-03/TACHE-05).

## Points d'attention
- **L'avantage utilisé dans la perte de politique est une valeur détachée du graphe du critique**
  (calculée via `forward` puis convertie en `float`, pas gardée comme nœud d'autodiff connecté) : le
  gradient de la perte de politique ne doit **pas** se propager dans les poids du critique — les deux
  réseaux sont optimisés par des pertes et des passages de rétropropagation strictement séparés
  (cohérent avec la décision de cadrage « pas de poids partagés »). Une fuite de gradient ici serait
  un bug silencieux (le critique influencerait l'acteur d'une façon non voulue, difficile à détecter
  sans ce point d'attention explicite).
- **Ne pas réintroduire de normalisation d'avantage inter-épisodes** ici non plus, pour la même
  raison qu'en LOT-ANNEXE-12 (mise à jour épisode par épisode, pas de lot d'épisodes).
- La **factorisation avec LOT-ANNEXE-12** doit rester un simple partage de la formule de graphe, pas
  une dépendance de `PolicyGradient` vers `ActorCritic` ou l'inverse dans un sens qui romprait la
  hiérarchie : privilégier un utilitaire commun à un niveau partagé (`Source/AiSolver/Training/`)
  plutôt qu'un sous-module qui dépend de l'autre.

## Définition de fait (DoD)
- `computeAdvantages` et `computeActorCriticLoss` disponibles et testés (`ctest` vert), y compris
  non-régression de LOT-ANNEXE-12 ; build `/W4 /WX` sans avertissement ; Doxygen à jour.

## Notions abordées
@ref guide-annexe-acteur-critique — variance du gradient, fonction de valeur, critique, avantage.

## Exigences
`EX-IA-014` (nouvelle).
