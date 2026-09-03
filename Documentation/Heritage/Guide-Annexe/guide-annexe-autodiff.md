# Différentiation automatique et rétropropagation {#guide-annexe-autodiff}

**Lot concerné :** [LOT-ANNEXE-02](@ref lot-annexe-02) (moteur d'autodiff maison).
**Prérequis :** [Algèbre tensorielle](@ref guide-annexe-algebre-tensorielle).

## Pourquoi ce chapitre

Un réseau de neurones apprend en ajustant ses poids pour réduire une erreur (une **perte**). Pour
savoir *dans quel sens* et *de combien* ajuster chaque poids, il faut calculer, pour chaque poids,
« si j'augmente ce poids d'une toute petite quantité, la perte augmente-t-elle ou diminue-t-elle,
et de combien ? » — c'est exactement la définition d'une **dérivée**. Ce chapitre explique ce
qu'est une dérivée, pourquoi la « règle de la chaîne » permet de la calculer pour un calcul composé
de plusieurs étapes, et comment automatiser ce calcul pour un réseau entier — c'est la
**rétropropagation** (*backpropagation*), littéralement le mécanisme qui a rendu l'apprentissage
profond praticable.

## 1. La dérivée, intuitivement

La dérivée d'une fonction `f` en un point `x`, notée `f'(x)` ou `df/dx`, mesure la **vitesse de
variation** de `f` autour de `x` : si on augmente `x` d'une petite quantité `ε`, `f(x)` varie
d'environ `f'(x) × ε`.

Exemple : `f(x) = x²`. Sa dérivée est `f'(x) = 2x` (résultat classique, obtenu par la règle de
dérivation des puissances : la dérivée de `xⁿ` est `n·xⁿ⁻¹`). En `x = 3` : `f'(3) = 6` — si on
augmente `x` de `0.01`, `f(x)` doit augmenter d'environ `6 × 0.01 = 0.06`. Vérification :
`f(3) = 9`, `f(3.01) = 9.0601`, différence `≈ 0.0601` — proche de `0.06` (l'écart vient du terme
d'ordre 2, négligeable pour un `ε` petit).

**Dérivée partielle** : quand `f` dépend de **plusieurs** variables, `∂f/∂x` (« dérivée partielle
de `f` par rapport à `x` ») est la dérivée de `f` par rapport à `x` **en gardant toutes les autres
variables constantes**. Exemple : `f(x, y) = x² + xy`. `∂f/∂x = 2x + y` (on traite `y` comme une
constante), `∂f/∂y = x` (on traite `x` comme une constante, et la dérivée de `xy` par rapport à `y`
est `x`).

**Gradient** : le gradient de `f` est simplement le vecteur de toutes ses dérivées partielles,
`∇f = (∂f/∂x, ∂f/∂y, ...)`. Pour un réseau à des milliers de poids, le gradient de la perte est un
vecteur d'autant de nombres — un par poids.

## 2. La règle de la chaîne

Si `f` dépend de `g`, qui dépend elle-même de `x` (on note `f(g(x))`, une **composition**), alors :

```
df/dx = (df/dg) × (dg/dx)
```

C'est la **règle de la chaîne** — le résultat mathématique sur lequel repose tout le reste de ce
chapitre. Intuition : si `g` varie de `dg/dx` par unité de `x`, et que `f` varie de `df/dg` par
unité de `g`, alors `f` varie de `(df/dg) × (dg/dx)` par unité de `x`.

**Exemple travaillé** : `f(x) = (3x + 1)²`. Posons `g(x) = 3x + 1` (donc `f = g²`).
- `dg/dx = 3` (dérivée de `3x + 1`).
- `df/dg = 2g` (dérivée de `g²` par rapport à `g`).
- Par la règle de la chaîne : `df/dx = 2g × 3 = 6g = 6(3x + 1) = 18x + 6`.

En `x = 1` : `g = 4`, `df/dx = 6 × 4 = 24`. Vérification directe : `f(x) = 9x² + 6x + 1`, donc
`f'(x) = 18x + 6`, et `f'(1) = 24`. Les deux méthodes concordent.

**Pourquoi c'est essentiel ici** : un réseau de neurones est une **très longue composition** de
fonctions simples (produits matriciels, additions, activations) empilées en couches. La règle de la
chaîne permet de calculer la dérivée de la perte par rapport à **n'importe quel** poids, aussi
profondément enfoui soit-il dans le réseau, en enchaînant des dérivées locales simples — sans
jamais avoir à écrire à la main l'énorme formule composée.

## 3. Le graphe de calcul

Un calcul composé de plusieurs étapes peut se représenter comme un **graphe** : chaque nœud est une
valeur (une entrée, ou le résultat d'une opération), chaque arête relie une opération à ses
opérandes. Exemple pour `f(x) = (3x + 1)²`, avec `x = 1` :

```
x=1 --(×3)--> a=3 --(+1)--> b=4 --(²)--> f=16
```

Ce graphe est construit **à l'exécution** (*define-by-run*, ou graphe dynamique) : chaque fois
qu'on effectue une opération sur des nœuds, on crée un nouveau nœud qui se souvient de ses parents
(`a` et `b` ci-dessus) et de la règle de dérivation locale de l'opération qui l'a produit — c'est
l'approche retenue par `aisolver::autodiff::Node` (LOT-ANNEXE-02).

## 4. Mode direct vs mode inverse

Il existe deux façons de propager des dérivées à travers un graphe :

- **Mode direct** (*forward mode*) : on propage `∂(chaque nœud)/∂x` en même temps que le calcul
  avance, pour **une** entrée `x` fixée. Efficace quand il y a **peu d'entrées** et **beaucoup de
  sorties**.
- **Mode inverse** (*reverse mode*, la rétropropagation) : on calcule d'abord tout le graphe
  (passe avant), puis on repart de la **sortie** (la perte, un scalaire unique) et on propage
  `∂(perte)/∂(chaque nœud)` en remontant vers les entrées. Efficace quand il y a **beaucoup
  d'entrées** (tous les poids d'un réseau — potentiellement des millions) et **une seule sortie**
  (la perte).

Un réseau de neurones a exactement ce profil (des milliers/millions de poids, une seule perte
scalaire) — c'est pourquoi le mode inverse est la méthode utilisée en apprentissage profond, et
celle que `LOT-ANNEXE-02` implémente (`backward()`, condition : le nœud racine doit être un
scalaire — un tenseur à un seul élément).

## 5. Rétropropagation, pas à pas

Reprenons `f(x) = (3x + 1)²` avec le graphe `x → a=3x → b=a+1 → f=b²`. La rétropropagation part de
`f` (`∂f/∂f = 1`, trivialement) et remonte :

1. **`f = b²`** : `∂f/∂b = 2b`. On multiplie par le gradient déjà accumulé sur `f` (`1`) :
   gradient de `b` reçoit `2b × 1 = 2b`.
2. **`b = a + 1`** : la dérivée d'une addition par rapport à chacun de ses opérandes est `1`
   (`∂b/∂a = 1`). On multiplie par le gradient déjà accumulé sur `b` (`2b`, étape précédente) :
   gradient de `a` reçoit `2b × 1 = 2b`.
3. **`a = 3x`** : `∂a/∂x = 3`. On multiplie par le gradient déjà accumulé sur `a` (`2b`) :
   gradient de `x` reçoit `2b × 3 = 6b`.

Avec `x = 1` : `a = 3`, `b = 4`, gradient final de `x` = `6 × 4 = 24` — exactement le résultat
obtenu à la main au §2 avec la règle de la chaîne. La rétropropagation **n'est rien d'autre** que
l'application mécanique et systématique de la règle de la chaîne, nœud par nœud, en remontant le
graphe.

**Règle générale à chaque nœud** : `gradientDuParent += règleDeDérivationLocale × gradientDéjàAccumuléSurCeNœud`.

## 6. Pourquoi les gradients s'accumulent (`+=`, jamais `=`)

Si un même nœud est utilisé **plusieurs fois** dans le graphe (par exemple un biais partagé par
plusieurs calculs), il reçoit une contribution de **chaque** chemin qui le traverse — ces
contributions doivent s'**additionner**, pas s'écraser. Exemple : `f(x) = x + x` (donc `f = 2x`,
`df/dx = 2`). Si le graphe a deux arêtes distinctes partant de `x` (une vers chaque `+`), chacune
contribue `1` au gradient de `x` (dérivée de l'addition) ; la somme des deux contributions donne
bien `2`, le résultat correct. C'est pourquoi `LOT-ANNEXE-02` accumule toujours par `+=` — écraser
romprait silencieusement tout graphe où un nœud est réutilisé, ce qui arrive **tout le temps** dans
un réseau réel (un même poids intervient dans le calcul de chaque exemple d'un lot, par exemple).

## 7. Parcours topologique inverse

Pour que chaque nœud reçoive **toutes** ses contributions avant que son propre gradient ne soit
propagé à ses parents (sinon on propagerait un gradient incomplet), `backward()` visite les nœuds
dans un ordre précis : l'ordre **topologique inverse** du graphe — un nœud n'est traité qu'une fois
que tous les nœuds qui l'utilisent comme opérande ont déjà été traités. En pratique, pour un graphe
construit par une passe avant classique (chaque nœud créé après tous ses parents), l'ordre inverse
de création est déjà un ordre topologique inverse valide — ce qui simplifie l'implémentation.

## 8. Vérifier qu'un gradient calculé est correct : les différences finies

Comment être sûr que la dérivation implémentée pour une opération est correcte ? On revient à la
**définition** de la dérivée : `f'(x) ≈ (f(x + ε) - f(x - ε)) / (2ε)` pour un `ε` petit (ex.
`0.0001`) — c'est la méthode des **différences finies centrées**. Elle est lente (il faut
recalculer `f` deux fois par paramètre) mais ne dépend d'aucune formule de dérivation — un bon
étalon indépendant. `LOT-ANNEXE-02` (TACHE-04) compare systématiquement le gradient obtenu par
`backward()` à celui obtenu par différences finies, pour **toute nouvelle opération**, avant de lui
faire confiance — condition bloquante avant tout usage en aval.

## 9. Pourquoi une fabrique générique d'opérations (`unaryOp`/`binaryOp`)

Plutôt que de coder en dur, dans `Node`, la liste fermée des opérations possibles (addition,
multiplication, etc.), `LOT-ANNEXE-02` expose une fabrique générique : une fonction qui prend en
argument le calcul de la valeur **et** sa règle de dérivation locale, et produit un nœud capable de
faire les deux. Ajouter une nouvelle opération différentiable (par exemple `sigmoid`, introduite
plus tard par `LOT-ANNEXE-03`) devient alors un **ajout**, jamais une modification du moteur
lui-même — un principe de conception qui revient souvent en génie logiciel sous le nom de « principe
ouvert/fermé » (ouvert à l'extension, fermé à la modification).

## Sources

- Rumelhart, D.E., Hinton, G.E., Williams, R.J. (1986). *Learning representations by
  back-propagating errors*. Nature 323, 533–536. — l'article qui a popularisé la rétropropagation
  pour l'entraînement des réseaux de neurones multicouches.
- Linnainmaa, S. (1970). *The representation of the cumulative rounding error of an algorithm as a
  Taylor expansion of the local rounding errors* (thèse de master, Université d'Helsinki). — la
  description originale du mode inverse de différentiation automatique, antérieure à son
  application aux réseaux de neurones.
- Baydin, A.G., Pearlmutter, B.A., Radul, A.A., Siskind, J.M. (2018). *Automatic Differentiation in
  Machine Learning: a Survey*. Journal of Machine Learning Research 18, 1–43. — synthèse moderne
  distinguant mode direct/mode inverse et situant la rétropropagation comme cas particulier du mode
  inverse appliqué à un graphe de calcul.
- Griewank, A., Walther, A. (2008). *Evaluating Derivatives: Principles and Techniques of
  Algorithmic Differentiation* (2nd ed.). SIAM. — référence approfondie sur la différentiation
  automatique en général (bien au-delà de ce qui est nécessaire au programme Lot-Annexe).
