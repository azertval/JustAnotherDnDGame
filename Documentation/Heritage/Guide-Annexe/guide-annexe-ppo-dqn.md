# PPO et DQN {#guide-annexe-ppo-dqn}

**Lot concerné :** [LOT-ANNEXE-14](@ref lot-annexe-14) (algorithme avancé — choix entre les deux
familles ci-dessous, différé à l'ouverture du lot).
**Prérequis :** [REINFORCE](@ref guide-annexe-reinforce), [Acteur-critique](@ref
guide-annexe-acteur-critique) (pour PPO, qui en est une extension directe) ;
[vocabulaire RL](@ref guide-annexe-apprentissage-renforcement) (pour DQN, une famille différente
présentée ici indépendamment).

## Pourquoi ce chapitre

L'acteur-critique (chapitre précédent) réduit la variance de REINFORCE, mais laisse un risque
distinct : une mise à jour de poids trop grande peut faire **s'effondrer** la politique (elle
« oublie » brutalement un comportement qui marchait, parce qu'un pas d'optimisation trop agressif
l'a poussée trop loin). Ce chapitre présente deux façons différentes, développées par des
communautés différentes, de rendre l'apprentissage plus stable : **PPO**, qui limite explicitement
la taille du changement de politique à chaque mise à jour, et **DQN**, qui change complètement
d'approche — apprendre la **valeur** de chaque action plutôt qu'une politique directement.

## Partie 1 — PPO (Proximal Policy Optimization)

### 1.1. Le problème que PPO résout

Après une mise à jour de poids, la politique a changé : les actions qu'elle aurait choisies pour
les états déjà observés dans les trajectoires collectées (chapitre
[REINFORCE](@ref guide-annexe-reinforce)) ne sont plus exactement les mêmes probabilités qu'au
moment de la collecte. Si la mise à jour est trop grande, cette différence devient énorme — la
nouvelle politique n'a presque plus rien à voir avec celle qui a collecté les données, ce qui rend
la mise à jour suivante peu fiable (elle se base sur des données de moins en moins représentatives
de la politique actuelle). PPO limite ce décalage à chaque mise à jour.

### 1.2. Le ratio de probabilité

PPO compare, pour chaque action collectée, la probabilité que lui donnerait la politique **avant**
la mise à jour (`π_ancien`) à celle que lui donne la politique **en cours de mise à jour**
(`π_nouveau`) :

```
ratio(t) = π_nouveau(aₜ|sₜ) / π_ancien(aₜ|sₜ)
```

Un `ratio` proche de `1` signifie que la politique n'a presque pas changé sa probabilité pour cette
action ; un `ratio` très différent de `1` signifie un grand changement.

### 1.3. Le clip : limiter le ratio

PPO borne ce ratio dans un petit intervalle autour de `1` (par exemple `[0.8, 1.2]`, souvent noté
`[1-ε, 1+ε]`) avant de multiplier par l'avantage (chapitre
[Acteur-critique](@ref guide-annexe-acteur-critique)) :

```
perte = - min( ratio(t) × avantage(t), clip(ratio(t), 1-ε, 1+ε) × avantage(t) )
```

Le `min` entre la version non bornée et la version bornée est délibéré : il empêche l'optimiseur de
tirer parti d'un ratio extrême pour gonfler artificiellement la perte dans le sens qui
l'arrangerait, **que l'avantage soit positif ou négatif** — un détail technique de la formule
originale (voir Sources) qui garantit que le « clip » agit réellement comme un frein dans les deux
directions.

### 1.4. Plusieurs passes d'optimisation par lot de trajectoires

Contrairement à REINFORCE/acteur-critique (une seule mise à jour par lot de trajectoires
collectées), PPO peut réutiliser le **même** lot de trajectoires pour plusieurs passes
d'optimisation successives — précisément parce que le clip empêche chaque passe de trop s'éloigner
de la politique qui a collecté les données, ce qui rendrait ces données obsolètes.

## Partie 2 — DQN (Deep Q-Network)

### 2.1. Une approche différente : apprendre une valeur, pas une politique

DQN n'apprend pas directement une politique (une distribution sur les actions). Il apprend une
**fonction de valeur d'action**, `Q(s, a)` : « si je suis dans l'état `s` et que je choisis l'action
`a` maintenant (puis que je joue ensuite du mieux possible), quel retour puis-je espérer ? ». Une
fois `Q` apprise, la politique est **implicite** : à chaque pas, on choisit l'action de plus grand
`Q(s, a)` (ou, pendant l'entraînement, on explore aussi d'autres actions — voir §2.4).

### 2.2. L'équation de Bellman et l'apprentissage par différence temporelle

`Q` doit satisfaire une relation de cohérence interne (l'**équation de Bellman**) : la valeur d'une
action maintenant doit être égale à la récompense immédiate, plus la valeur actualisée (facteur
`γ`, chapitre [REINFORCE](@ref guide-annexe-reinforce), §4) de la **meilleure** action possible à
l'état suivant :

```
Q(sₜ, aₜ) ≈ rₜ + γ × max(sur toutes les actions a') de Q(sₜ₊₁, a')
```

DQN entraîne un réseau à approcher cette relation, en minimisant l'écart (l'**erreur de différence
temporelle**, *TD error*) entre les deux côtés de cette égalité, pour des transitions
`(sₜ, aₜ, rₜ, sₜ₊₁)` réellement vécues :

```
perte = ( Q(sₜ, aₜ) - [ rₜ + γ × max(a') Q(sₜ₊₁, a') ] )²
```

### 2.3. Mémoire de rejeu (*replay buffer*) et réseau cible

Deux dispositifs stabilisent cet apprentissage :
- **Mémoire de rejeu** : les transitions vécues sont stockées dans une mémoire, et
  l'entraînement pioche des lots **au hasard** dedans plutôt que d'utiliser uniquement les
  transitions les plus récentes — casse la corrélation entre transitions consécutives (très
  similaires d'un pas à l'autre), qui déstabiliserait l'apprentissage si elles étaient utilisées
  dans l'ordre.
- **Réseau cible** (*target network*) : le terme `max(a') Q(sₜ₊₁, a')` de la formule ci-dessus
  utilise une **copie figée** du réseau (mise à jour seulement de temps en temps, pas à chaque pas)
  plutôt que le réseau en cours d'entraînement lui-même — sans quoi la « cible » que l'entraînement
  poursuit bougerait à chaque pas d'optimisation, une situation d'instabilité analogue à essayer
  d'atteindre une cible qui se déplace en même temps qu'on vise.

### 2.4. Exploration `ε`-greedy

Pendant l'entraînement, DQN choisit l'action de plus grand `Q` avec une probabilité `1-ε`, et une
action **aléatoire** avec probabilité `ε` (`ε` typiquement décroissant au fil de l'entraînement) —
sans cela, l'agent ne découvrirait jamais qu'une action qu'il croit mauvaise (`Q` sous-estimé, par
manque d'expérience) pourrait en réalité être bonne.

## Quelle famille choisir pour LOT-ANNEXE-14 ?

Ce chapitre présente les deux pour que la décision, prise à l'ouverture de
[LOT-ANNEXE-14](@ref lot-annexe-14) sur la base des résultats mesurés de
[LOT-ANNEXE-13](@ref lot-annexe-13), puisse s'appuyer sur une compréhension réelle des deux
approches plutôt que sur leur seul nom. Repère utile : PPO prolonge directement
l'acteur-critique déjà en place (même structure acteur/critique, formule de perte modifiée) ; DQN
est une **rupture** d'approche (pas de réseau de politique du tout, une fonction de valeur
d'action à la place) mais se prête bien à l'espace d'action **discret et petit** déjà retenu par
[LOT-ANNEXE-07](@ref lot-annexe-07).

## Sources

- Schulman, J., Wolski, F., Dhariwal, P., Radford, A., Klimov, O. (2017). *Proximal Policy
  Optimization Algorithms*. arXiv:1707.06347. — article d'origine de PPO, y compris la formule de
  clip exacte du §1.3.
- Schulman, J., Levine, S., Moritz, P., Jordan, M.I., Abbeel, P. (2015). *Trust Region Policy
  Optimization*. Proceedings of ICML 2015. — algorithme antérieur (TRPO) dont PPO simplifie
  l'implémentation tout en gardant l'idée de limiter le changement de politique par mise à jour.
- Mnih, V., Kavukcuoglu, K., Silver, D., Graves, A., Antonoglou, I., Wierstra, D., Riedmiller, M.
  (2013). *Playing Atari with Deep Reinforcement Learning*. arXiv:1312.5602 (NIPS Deep Learning
  Workshop 2013). — première version de DQN (mémoire de rejeu incluse).
- Mnih, V., Kavukcuoglu, K., Silver, D. et al. (2015). *Human-level control through deep
  reinforcement learning*. Nature 518, 529–533. — version complète de DQN (ajoute le réseau cible,
  §2.3), publiée après démonstration sur un large ensemble de jeux Atari.
- Watkins, C.J.C.H. (1989). *Learning from Delayed Rewards* (thèse de doctorat, Université de
  Cambridge). — origine du **Q-learning**, l'algorithme d'apprentissage par différence temporelle
  dont DQN est l'extension à un réseau de neurones profond (d'où le « Deep » de DQN).
