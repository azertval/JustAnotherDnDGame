# Acteur-critique {#guide-annexe-acteur-critique}

**Lot concerné :** [LOT-ANNEXE-13](@ref lot-annexe-13) (réduction de variance : critique et
avantage).
**Prérequis :** [REINFORCE](@ref guide-annexe-reinforce) (ce chapitre en réutilise directement la
perte, en la modifiant).

## Pourquoi ce chapitre

REINFORCE (chapitre précédent) fonctionne, mais souffre d'un défaut connu : la **variance** élevée
de son gradient estimé. Ce chapitre explique d'où vient cette variance, et comment un second réseau
— le **critique** — la réduit, sans changer la direction moyenne dans laquelle l'apprentissage
progresse.

## 1. D'où vient la variance de REINFORCE

REINFORCE multiplie le gradient de log-probabilité par le **retour complet** `Gₜ` de l'épisode
(chapitre précédent, §4). Or ce retour dépend de **tout** ce qui se passe après le pas `t` — y
compris des éléments que l'action `aₜ` elle-même n'a pas causés (un danger mobile qui, par hasard,
se trouvait ailleurs ce coup-ci ; un pas suivant légèrement différent). Deux épisodes où la
politique a fait exactement les mêmes choix jusqu'au pas `t` peuvent donc recevoir des `Gₜ` très
différents à cause d'événements ultérieurs sans rapport avec la qualité de l'action `aₜ` elle-même
— c'est cette variabilité qui rend le signal d'apprentissage **bruyant**, et l'entraînement plus
lent/instable qu'il ne le serait avec un signal plus propre.

## 2. L'idée : soustraire une base de comparaison (*baseline*)

Plutôt que d'utiliser le retour brut `Gₜ`, on soustrait une **estimation de ce à quoi on pouvait
s'attendre en moyenne** depuis l'état `sₜ`, notée `V(sₜ)` (la **valeur** de l'état `sₜ`) :

```
avantage(t) = Gₜ - V(sₜ)
```

L'**avantage** répond à une question plus précise que le retour brut : « cette action a-t-elle fait
**mieux ou moins bien que la moyenne attendue** depuis cet état, plutôt que simplement bien ou mal
dans l'absolu ? ». Si `V(sₜ)` est une bonne estimation, l'avantage isole la contribution propre de
l'action `aₜ`, en retirant la part de variabilité qui aurait existé de toute façon, quelle que soit
l'action choisie.

**Pourquoi ceci ne biaise pas l'apprentissage** (résultat clé, admis ici — la preuve complète est
dans les sources) : soustraire une quantité qui ne dépend **que** de l'état `sₜ` (et non de
l'action choisie) à l'intérieur de la formule de REINFORCE ne change **pas**, en moyenne sur de
nombreux épisodes, la direction du gradient — seulement sa variance, qu'elle réduit. C'est
pourquoi `V(sₜ)` est appelée une *baseline* : un point de comparaison neutre, pas une correction de
direction.

## 3. Le critique : un réseau qui apprend à estimer `V(s)`

Comme `V(sₜ)` (la valeur « vraie » d'un état) n'est pas connue à l'avance, on l'**apprend** avec un
second réseau — le **critique** (chapitre [Réseaux de neurones](@ref guide-annexe-reseaux-neurones)
pour sa structure, identique à celle de l'acteur si ce n'est une seule sortie scalaire plutôt qu'une
distribution sur les actions). Le critique est entraîné à **prédire** le retour réellement observé,
par une perte d'erreur quadratique classique :

```
perteCritique = (V(sₜ) - Gₜ)²
```

Minimiser cette perte pousse `V(sₜ)` à se rapprocher, en moyenne, du retour réellement observé
depuis `sₜ` — exactement ce qu'on veut d'une bonne baseline. Les deux réseaux (acteur et critique)
sont entraînés **simultanément** : l'acteur avec la perte de politique du chapitre précédent
(retour remplacé par l'avantage), le critique avec sa propre perte d'erreur quadratique — d'où le
nom « acteur-critique » (*actor-critic*) : un réseau qui **agit** (l'acteur), un réseau qui **juge**
la qualité des états traversés (le critique), chacun avec sa propre fonction de perte et son propre
optimiseur (chapitre [Optimisation](@ref guide-annexe-optimisation)).

## 4. Ce qui ne change pas par rapport à REINFORCE

La perte de l'acteur reste rigoureusement la même formule que REINFORCE (chapitre précédent, §3),
seul `Gₜ` est remplacé par `avantage(t)` :

```
perteActeur = - Σ (pour chaque pas t) [ log π(aₜ|sₜ;θ) × avantage(t) ]
```

Tout le reste (échantillonnage stochastique de l'action pendant l'entraînement, rétropropagation
via le moteur d'autodiff, `argmax` uniquement à l'export final) reste identique — l'acteur-critique
n'est pas un algorithme entièrement nouveau, c'est un **raffinement ciblé** de REINFORCE.

## 5. Pourquoi le critique n'est jamais utilisé à l'évaluation ou à l'export

Le critique n'a **qu'un seul rôle** : réduire la variance du signal d'entraînement de l'acteur. Une
fois l'entraînement terminé, seul l'acteur décide des actions (il **est** la politique) — le
critique n'a plus aucune utilité et n'est jamais chargé pour évaluer un modèle
([LOT-ANNEXE-15](@ref lot-annexe-15)) ni pour produire un rejeu ([LOT-ANNEXE-07](@ref
lot-annexe-07)) : il ne participe en aucune façon à la décision d'une action, seulement à
l'ajustement des poids de l'acteur pendant l'entraînement.

## Sources

- Sutton, R.S., McAllester, D., Singh, S., Mansour, Y. (1999/2000). *Policy Gradient Methods for
  Reinforcement Learning with Function Approximation*. Advances in NIPS 12. — établit formellement
  que soustraire une *baseline* ne dépendant que de l'état ne biaise pas le gradient de politique
  (preuve de l'invariance mentionnée au §2).
- Konda, V.R., Tsitsiklis, J.N. (2000). *Actor-Critic Algorithms*. Advances in NIPS 12. — article
  de référence introduisant formellement l'architecture acteur-critique et son analyse de
  convergence.
- Sutton, R.S., Barto, A.G. (2018). *Reinforcement Learning: An Introduction* (2nd ed.). MIT Press,
  chapitre 13.4 (« Policy Gradient with Baseline ») et chapitre 6 (fonctions de valeur, apprentissage
  par différence temporelle — utile pour situer `V(s)` dans un cadre plus large que celui présenté
  ici). — présentation pédagogique des *baselines* et de l'acteur-critique.
