# Descente de gradient et optimiseurs {#guide-annexe-optimisation}

**Lot concerné :** [LOT-ANNEXE-04](@ref lot-annexe-04) (optimiseurs maison).
**Prérequis :** [Autodiff](@ref guide-annexe-autodiff) (le gradient, calculé par `backward()`, est
l'ingrédient que ce chapitre transforme en mise à jour de poids).

## Pourquoi ce chapitre

Le chapitre [Autodiff](@ref guide-annexe-autodiff) explique comment calculer un gradient — pour
chaque poids, la direction et l'intensité dans lesquelles la perte varierait si ce poids changeait.
Mais connaître un gradient ne suffit pas : il faut décider **comment** l'utiliser pour effectivement
changer les poids et réduire la perte. C'est le rôle d'un **optimiseur**.

## 1. L'intuition : descendre une pente

Imaginons la perte comme une **altitude** dans un paysage dont les coordonnées sont les poids du
réseau — l'entraînement cherche le point le plus bas (la perte minimale). Le gradient indique, en
chaque point, la direction de la **montée la plus raide** — pour descendre, il faut donc avancer
dans la direction **opposée** au gradient. C'est exactement la règle de la **descente de gradient**
(*gradient descent*) :

```
poids ← poids - tauxApprentissage × gradient
```

Le **taux d'apprentissage** (*learning rate*, souvent noté `η` ou `α`) contrôle la taille du pas :
- **Trop grand** : le pas dépasse le minimum et peut faire **diverger** (la perte augmente au lieu
  de diminuer, parfois de plus en plus à chaque pas).
- **Trop petit** : la convergence est très lente (des milliers d'itérations pour un progrès
  minime).

Il n'existe pas de valeur universellement bonne — c'est un paramètre d'expérience, ajusté par
essai/erreur (ou, comme le montre §3, en partie automatisé par des optimiseurs plus sophistiqués).

## 2. Descente de gradient stochastique (SGD) et inertie

« Stochastique » (SGD, *Stochastic Gradient Descent*) fait référence au fait qu'en apprentissage
profond, le gradient est en général calculé sur un **échantillon** des données (un exemple, ou un
petit lot) plutôt que sur toutes les données à la fois — dans le programme Lot-Annexe, cela
correspond à calculer le gradient sur un seul épisode de jeu à la fois (voir
[REINFORCE](@ref guide-annexe-reinforce)). Le gradient ainsi obtenu est un peu **bruité** (il varie
d'un échantillon à l'autre), ce qui peut faire « zigzaguer » la descente.

L'**inertie** (*momentum*) atténue ce zigzag par une analogie physique : au lieu de sauter
directement dans la direction du gradient courant, on accumule une **vitesse** qui garde la mémoire
des directions précédentes :

```
vitesse ← momentum × vitesse - tauxApprentissage × gradient
poids ← poids + vitesse
```

`momentum` (typiquement `0.9`) contrôle combien de la vitesse précédente est conservée. Intuition :
comme une bille qui roule dans une vallée — elle continue un peu sur sa lancée même quand la pente
locale change légèrement de direction, ce qui lisse la trajectoire et peut accélérer la convergence
dans une direction constante.

## 3. Adam : un taux d'apprentissage qui s'adapte

SGD (avec ou sans inertie) utilise le **même** taux d'apprentissage pour tous les poids, à tout
moment de l'entraînement. Adam (*Adaptive Moment Estimation*) ajuste ce taux **automatiquement**,
poids par poids, en maintenant deux moyennes mobiles :

- **Premier moment** (`m`) : moyenne mobile du gradient lui-même — approxime sa direction
  « moyenne » récente, lissant le bruit (proche de l'inertie de SGD).
- **Second moment** (`v`) : moyenne mobile du **carré** du gradient — approxime l'amplitude
  récente du gradient (grande si le gradient a été grand ou très variable récemment).

La mise à jour finale divise le premier moment par la racine du second :

```
m ← β₁·m + (1-β₁)·gradient
v ← β₂·v + (1-β₂)·gradient²
poids ← poids - tauxApprentissage × m / (√v + ε)
```

(`β₁ ≈ 0.9`, `β₂ ≈ 0.999`, `ε` un petit nombre pour éviter une division par zéro — voir
`LOT-ANNEXE-04`, TACHE-02 pour les valeurs par défaut exactes et la correction de biais nécessaire
au tout début de l'entraînement, quand `m`/`v` n'ont pas encore eu le temps de « se remplir »).

**Intuition de la division par `√v`** : si un poids a récemment reçu de **grands** gradients
(`v` grand), son pas effectif est **réduit** (on avance prudemment, la direction est peut-être
instable) ; s'il a reçu de **petits** gradients (`v` petit), son pas effectif est **augmenté** (on
peut se permettre d'avancer plus vite, la pente est douce). Ce comportement adaptatif rend Adam
nettement moins sensible au choix exact du taux d'apprentissage que SGD — l'une des raisons de sa
popularité en pratique, et pourquoi `LOT-ANNEXE-04` le retient comme second optimiseur, aux côtés de
SGD comme référence la plus simple.

## 4. Pourquoi tester la convergence sur des fonctions jouets avant tout usage réel

L'apprentissage par renforcement (générations 2/3 du programme) est un contexte où un
entraînement peut échouer à progresser pour de multiples raisons à la fois — mauvaise récompense,
mauvais réseau, mauvais optimiseur, tout en même temps. Pour isoler la responsabilité de
l'optimiseur, `LOT-ANNEXE-04` (TACHE-03) le teste d'abord sur des problèmes **triviaux et connus**
(une quadratique à minimum unique, une régression polynomiale à coefficients connus) — un
optimiseur qui échoue déjà sur un problème aussi simple ne doit jamais être laissé entraîner un
agent de jeu, où l'échec serait indiscernable du bruit inhérent à l'apprentissage par renforcement
lui-même.

## Sources

- Robbins, H., Monro, S. (1951). *A Stochastic Approximation Method*. Annals of Mathematical
  Statistics 22(3), 400–407. — origine mathématique de l'approximation stochastique, fondement
  théorique de la descente de gradient stochastique.
- Rumelhart, D.E., Hinton, G.E., Williams, R.J. (1986). *Learning representations by
  back-propagating errors*. Nature 323, 533–536. — introduit également le terme de *momentum*
  associé à la rétropropagation.
- Kingma, D.P., Ba, J. (2015). *Adam: A Method for Stochastic Optimization*. Proceedings of ICLR
  2015. — article d'origine d'Adam, y compris la dérivation de la correction de biais des moments.
- Ruder, S. (2016). *An overview of gradient descent optimization algorithms*. arXiv:1609.04747. —
  synthèse comparative de SGD, momentum, Adam et variantes, utile pour une vue d'ensemble au-delà de
  ce chapitre.
