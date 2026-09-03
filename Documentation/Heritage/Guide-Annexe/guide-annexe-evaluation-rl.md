# Évaluation et reproductibilité en RL {#guide-annexe-evaluation-rl}

**Lots concernés :** [LOT-ANNEXE-15](@ref lot-annexe-15)/[LOT-ANNEXE-16](@ref lot-annexe-16)
(harnais de benchmark, évaluation hors-niveau).
**Prérequis :** [vocabulaire RL](@ref guide-annexe-apprentissage-renforcement) (politique
stochastique vs déterministe).

## Pourquoi ce chapitre

Un modèle entraîné (génération 2 ou 3) a produit, au moins une fois, un rejeu déterministe qui
termine le niveau (@ref lot-annexe-07, @ref lot-annexe-11). Il est tentant d'en conclure que « le
modèle sait terminer ce niveau » — ce chapitre explique pourquoi cette conclusion est **prématurée**
et comment mesurer plus honnêtement ce qu'un modèle sait réellement faire.

## 1. Une seule réussite ne prouve pas une compétence fiable

Une politique **stochastique** peut, par chance, réussir un niveau lors d'une exécution donnée sans
que cela signifie qu'elle réussirait systématiquement — de la même façon qu'un joueur humain
occasionnel peut réussir un niveau difficile une fois par chance sans être capable de le
reproduire à volonté. C'est pourquoi [LOT-ANNEXE-15](@ref lot-annexe-15) exécute une politique
**plusieurs fois** (répétitions, chacune avec une graine différente dérivée d'une graine de base)
et mesure un **taux de réussite** plutôt que de se contenter d'un unique succès observé.

## 2. Le taux de réussite n'est qu'une des mesures utiles

Deux politiques peuvent avoir le même taux de réussite tout en étant très différentes en pratique :
l'une termine toujours le niveau rapidement, l'autre erre longtemps avant de réussir de justesse.
D'où l'intérêt de mesurer aussi le **nombre de pas** (distinctement pour tous les épisodes et pour
les seuls épisodes réussis — mélanger les deux fausserait la moyenne, un échec plafonné au budget de
pas maximal gonflerait artificiellement la moyenne « tous épisodes ») et la **variance** de ces
mesures d'une répétition à l'autre.

## 3. Un cas particulier instructif : la politique évolutionniste

Une politique produite par l'algorithme évolutionniste (chapitre
[algorithmes évolutionnistes](@ref guide-annexe-algorithmes-evolutionnistes)) n'a jamais connu
d'autre mode de décision que déterministe (`argmax` — cette famille n'a jamais appris de
distribution de probabilité au sens de REINFORCE). Répéter son exécution plusieurs fois produit
donc, logiquement, une **variance nulle** : exactement la même trajectoire à chaque répétition. Ce
n'est pas une anomalie du harnais de benchmark, c'est une propriété attendue et vérifiable de cette
famille d'algorithmes — un bon exemple de la différence entre « déterministe » (une seule sortie
possible) et « stochastique » (une distribution, chapitre
[vocabulaire RL](@ref guide-annexe-apprentissage-renforcement)) vue en pratique sur des mesures
réelles.

## 4. Robustesse : un modèle qui « triche » en mémorisant

Un modèle peut réussir de façon très fiable sur son niveau d'entraînement tout en n'ayant, en
réalité, appris qu'à suivre une trajectoire précise plutôt que des réflexes transférables — un peu
comme mémoriser la solution d'un puzzle précis plutôt que comprendre le principe qui le résout. Une
façon de détecter cela : perturber légèrement ce que la politique **perçoit** (l'observation, sans
jamais toucher l'état réellement simulé) et observer si son taux de réussite s'effondre — un
effondrement marqué face à un bruit léger est un signe de fragilité, pas de robustesse
([LOT-ANNEXE-15](@ref lot-annexe-15), test de robustesse au bruit).

## 5. Le transfert entre niveaux comme mesure, pas comme objectif

Le régime d'entraînement du programme est strictement niveau par niveau (aucune généralisation
recherchée) — mais on peut légitimement se demander ce qu'un modèle entraîné sur un niveau produit,
une fois exécuté sur un niveau qu'il n'a jamais vu ([LOT-ANNEXE-16](@ref lot-annexe-16)). Sans
pression d'entraînement à généraliser, un transfert faible est l'issue **attendue**, pas un signe
d'échec du programme — un point qui a valu, dans le milieu de la recherche en RL, plusieurs travaux
alertant sur la difficulté à comparer des résultats d'un article à l'autre sans mesures répétées et
sans rigueur méthodologique (voir Sources).

## Sources

- Henderson, P., Islam, R., Bachman, P., Pineau, J., Precup, D., Meger, D. (2018). *Deep
  Reinforcement Learning that Matters*. Proceedings of AAAI 2018. — étude largement citée montrant
  qu'un même algorithme de RL peut donner des résultats très différents selon la graine aléatoire,
  l'implémentation exacte ou les hyperparamètres — justification directe de la nécessité de
  répéter les exécutions (§1) plutôt que de se fier à un seul essai.
- Henderson, P. et al. (voir ci-dessus), section sur les intervalles de confiance et le nombre
  d'exécutions nécessaires pour une comparaison statistiquement significative entre deux
  algorithmes — pertinent pour interpréter les rapports comparatifs de
  [LOT-ANNEXE-15](@ref lot-annexe-15).
- Sutton, R.S., Barto, A.G. (2018). *Reinforcement Learning: An Introduction* (2nd ed.). MIT Press.
  — pour le rappel de la distinction déterministe/stochastique (chapitre
  [vocabulaire RL](@ref guide-annexe-apprentissage-renforcement)) appliquée ici à l'évaluation.
