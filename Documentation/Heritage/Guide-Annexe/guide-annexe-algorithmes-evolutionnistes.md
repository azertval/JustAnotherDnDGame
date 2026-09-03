# Algorithmes évolutionnistes {#guide-annexe-algorithmes-evolutionnistes}

**Lots concernés :** [LOT-ANNEXE-10](@ref lot-annexe-10)/[LOT-ANNEXE-11](@ref lot-annexe-11)
(algorithme évolutionniste, entraînement niveau-par-niveau).
**Prérequis :** [Réseaux de neurones](@ref guide-annexe-reseaux-neurones) (ce qu'on fait évoluer),
[vocabulaire RL](@ref guide-annexe-apprentissage-renforcement) (épisode, récompense cumulée).

## Pourquoi ce chapitre

Les chapitres [Autodiff](@ref guide-annexe-autodiff) et
[Optimisation](@ref guide-annexe-optimisation) expliquent comment ajuster des poids **à partir d'un
gradient**. Mais calculer un gradient suppose de pouvoir dériver la perte par rapport aux poids —
or, en apprentissage par renforcement, la récompense cumulée d'un épisode dépend d'une longue
séquence de décisions et de la physique du jeu (chutes, collisions, mécanismes) : la relier
directement aux poids par une formule différentiable est loin d'être trivial (les générations 3 du
programme le font malgré tout — voir [REINFORCE](@ref guide-annexe-reinforce)). Ce chapitre
présente une approche radicalement différente, **sans aucun gradient** : faire évoluer une
population de réseaux par sélection, comme la sélection naturelle fait évoluer une population
d'êtres vivants.

## 1. L'idée centrale : sélection artificielle de poids

Un **algorithme évolutionniste** (ici, plus précisément une forme de *neuroévolution* — l'évolution
appliquée directement aux poids d'un réseau de neurones) maintient une **population** d'individus,
chacun un réseau complet aux poids différents. À chaque **génération** :

1. **Évaluation** : chaque individu joue un épisode complet (ici, un niveau du jeu via
   `HeadlessLevelEnvironment`, chapitre [vocabulaire RL](@ref guide-annexe-apprentissage-renforcement)),
   sa **fitness** (aptitude) est sa récompense cumulée sur cet épisode.
2. **Sélection** : les individus les plus aptes ont plus de chances d'être choisis comme
   « parents » de la génération suivante.
3. **Croisement** (*crossover*) : deux parents combinent leurs poids pour produire un enfant.
4. **Mutation** : un peu de bruit aléatoire est ajouté aux poids d'un enfant, pour introduire de la
   nouveauté que le croisement seul ne peut pas produire.
5. **Élitisme** : le meilleur individu de la génération est conservé **inchangé** dans la génération
   suivante — garantit que la fitness du meilleur individu ne peut jamais **régresser** d'une
   génération à l'autre.

On répète sur de nombreuses générations, en espérant voir la fitness du meilleur individu progresser
au fil du temps.

## 2. Sélection : comment choisir les parents

Une méthode simple et efficace est la **sélection par tournoi** : on tire au hasard un petit groupe
d'individus (par exemple 3) et on retient le plus apte du groupe comme parent. Répétée autant de
fois que nécessaire, cette méthode favorise statistiquement les individus les plus aptes sans
jamais éliminer totalement les moins aptes (qui peuvent occasionnellement gagner un tournoi contre
des adversaires encore moins aptes) — un compromis entre exploiter ce qui marche déjà et garder de
la diversité génétique dans la population.

## 3. Croisement : combiner deux réseaux

Pour deux réseaux parents de **même architecture** (mêmes couches, mêmes tailles — condition
indispensable, sinon leurs poids ne se correspondent pas terme à terme), un croisement simple
consiste à faire, pour chaque poids, une **moyenne** des deux parents (ou un tirage au hasard entre
les deux valeurs, poids par poids) :

```
poidsEnfant[i] = (poidsParent1[i] + poidsParent2[i]) / 2
```

Intuition : si les deux parents ont chacun trouvé, indépendamment, un réglage utile pour des poids
différents, l'enfant hérite potentiellement du meilleur des deux.

## 4. Mutation : introduire de la nouveauté

Après le croisement, on ajoute à chaque poids un petit bruit aléatoire, tiré d'une loi normale
(gaussienne) centrée sur zéro :

```
poidsEnfant[i] += Rng.nextGaussian(moyenne=0, écartType=tauxMutation)
```

`tauxMutation` contrôle l'amplitude de la mutation — trop grand, la population perd toute la
qualité acquise génération après génération (l'enfant devient presque aléatoire) ; trop petit, la
population stagne, incapable de sortir d'un optimum local (une configuration de poids qui ne peut
plus s'améliorer par petites variations, mais qui n'est pas la meilleure configuration possible).

## 5. Pourquoi cette approche fonctionne sans gradient

L'algorithme n'a **jamais besoin** de savoir comment la fitness varie précisément en fonction d'un
poids particulier — il se contente de mesurer la fitness de chaque individu **après coup**
(récompense cumulée totale d'un épisode complet) et de favoriser statistiquement ce qui a bien
fonctionné. C'est à la fois sa force (aucune hypothèse de différentiabilité requise — la fitness
peut être calculée par n'importe quel processus, y compris une simulation de jeu complète avec
collisions et mécanismes) et sa faiblesse (il explore « à l'aveugle », sans direction précise,
ce qui le rend généralement moins efficace, à nombre d'essais égal, qu'un algorithme utilisant le
gradient — voir [REINFORCE](@ref guide-annexe-reinforce), qui exploite justement cette direction).
C'est pourquoi le programme retient l'algorithme évolutionniste comme **première ligne de base**
(génération 2, la plus rapide à obtenir un premier agent fonctionnel), avant d'introduire
l'apprentissage par gradient comme amélioration qualitative (génération 3, exigence ferme du
programme).

## 6. Pourquoi la reproductibilité à graine fixée est essentielle ici

Toute la stochasticité de l'algorithme (initialisation des poids, sélection par tournoi, mutation)
passe par le générateur déterministe `aisolver::Rng` (chapitre
[Algèbre tensorielle](@ref guide-annexe-algebre-tensorielle)) — indispensable pour pouvoir
**rejouer** exactement un entraînement qui a bien fonctionné (déboguer un échec, comparer deux
réglages d'hyperparamètres de façon équitable) et pour finalement produire une séquence d'actions
déterministe exportable en rejeu (@ref lot-annexe-07).

## Sources

- Holland, J.H. (1975). *Adaptation in Natural and Artificial Systems*. University of Michigan
  Press. — ouvrage fondateur des algorithmes génétiques (sélection, croisement, mutation appliqués
  à une population de solutions candidates).
- Rechenberg, I. (1973). *Evolutionsstrategie: Optimierung technischer Systeme nach Prinzipien der
  biologischen Evolution*. Frommann-Holzboog. — origine des *stratégies d'évolution* (une famille
  proche des algorithmes génétiques, avec un accent différent sur la mutation plutôt que le
  croisement), en parallèle des travaux de Holland.
- Such, F.P., Madhavan, V., Conti, E., Lehman, J., Stanley, K.O., Clune, J. (2017). *Deep
  Neuroevolution: Genetic Algorithms Are a Competitive Alternative for Training Deep Neural Networks
  for Reinforcement Learning*. arXiv:1712.06567. — démontre directement qu'un algorithme génétique
  simple (mutation gaussienne des poids d'un réseau, sélection du meilleur) rivalise avec des
  méthodes à base de gradient sur des tâches de RL — la justification empirique la plus directe de
  l'approche retenue par `LOT-ANNEXE-10`.
- Salimans, T., Ho, J., Chen, X., Sidor, S., Sutskever, I. (2017). *Evolution Strategies as a
  Scalable Alternative to Reinforcement Learning*. arXiv:1703.03864. — autre démonstration, par une
  équipe différente, de l'efficacité des méthodes évolutionnistes/sans gradient sur des tâches de
  contrôle par renforcement.
