# Réseaux de neurones {#guide-annexe-reseaux-neurones}

**Lot concerné :** [LOT-ANNEXE-03](@ref lot-annexe-03) (bibliothèque de réseaux de neurones maison).
**Prérequis :** [Algèbre tensorielle](@ref guide-annexe-algebre-tensorielle),
[Autodiff](@ref guide-annexe-autodiff).

## Pourquoi ce chapitre

Un réseau de neurones est la structure qui va, en fin de programme, décider quelle action jouer à
partir de l'état du jeu observé. Ce chapitre explique ce qu'est un neurone artificiel, pourquoi on
les empile en couches, pourquoi il faut des fonctions non linéaires entre les couches, et pourquoi
le choix des poids de départ (avant tout entraînement) a une réelle importance.

## 1. Le neurone artificiel

Un neurone artificiel calcule une **somme pondérée** de ses entrées, ajoute un **biais**, puis
applique une **fonction d'activation** :

```
sortie = activation(w₁·x₁ + w₂·x₂ + ... + wₙ·xₙ + b)
```

`w₁...wₙ` sont les **poids** (un par entrée), `b` le **biais**. Ce sont ces nombres — les poids et
le biais — que l'entraînement va ajuster ; l'activation, elle, est fixée à l'avance (voir §3).

## 2. La couche dense

Une **couche dense** (*fully connected*) regroupe plusieurs neurones qui partagent tous les mêmes
entrées, chacun avec ses propres poids. Si une couche a `n` entrées et `m` neurones (donc `m`
sorties), ses poids forment une matrice `W` de forme `(m, n)` (une ligne de poids par neurone) et
son biais un vecteur `b` de taille `m`. Pour une entrée `x` (vecteur de taille `n`), la sortie de
toute la couche s'écrit d'un coup, en notation matricielle (chapitre
[Algèbre tensorielle](@ref guide-annexe-algebre-tensorielle)) :

```
sortie = activation(W × x + b)
```

C'est exactement un produit matriciel suivi d'une addition — ce qui explique pourquoi le chapitre
précédent a insisté sur le produit matriciel : c'est le cœur calculatoire de toute couche. En C++
(illustratif, `aisolver::nn::Dense`) :

```cpp
Tensor<float> forward(const Tensor<float>& input) {
    return activation(matmul(weights, input) + bias);
}
```

## 3. Pourquoi une fonction d'activation (et pourquoi non linéaire)

Sans fonction d'activation (ou avec une activation qui serait elle-même linéaire, comme
`activation(x) = x`), empiler plusieurs couches denses ne servirait à **rien** : la composition de
plusieurs transformations linéaires reste une transformation linéaire. Preuve courte : si
`couche1(x) = W₁x + b₁` et `couche2(x) = W₂x + b₂`, alors :

```
couche2(couche1(x)) = W₂(W₁x + b₁) + b₂ = (W₂W₁)x + (W₂b₁ + b₂)
```

C'est encore de la forme `W'x + b'` (avec `W' = W₂W₁` et `b' = W₂b₁ + b₂`) — un **unique** produit
matriciel suivi d'une addition, exactement comme une seule couche. Deux couches linéaires
empilées équivalent à une seule couche linéaire, quel que soit le nombre de couches empilées :
sans non-linéarité, empiler des couches n'augmente jamais la richesse des fonctions représentables.
C'est la **non-linéarité** de l'activation qui permet à un réseau profond de représenter des
fonctions plus riches qu'une seule couche.

## 4. Fonctions d'activation courantes

- **ReLU** (*Rectified Linear Unit*) : `relu(x) = max(0, x)` — vaut `0` pour `x` négatif, `x`
  sinon. Très utilisée car simple et rapide à calculer, et parce que sa dérivée (`0` ou `1`) ne
  fait ni exploser ni disparaître le gradient pour les entrées positives.
- **Tanh** (tangente hyperbolique) : `tanh(x) = (eˣ - e⁻ˣ) / (eˣ + e⁻ˣ)` — comprime toute entrée
  dans l'intervalle `(-1, 1)`, centrée en `0`.
- **Sigmoïde** : `sigmoid(x) = 1 / (1 + e⁻ˣ)` — comprime toute entrée dans `(0, 1)` ; utile pour
  représenter une probabilité d'un seul événement (« oui/non »).
- **Softmax** : généralise la sigmoïde à **plusieurs** sorties qui doivent former une distribution
  de probabilité (somme à `1`). Pour un vecteur `x = (x₁, ..., xₖ)` :
  ```
  softmax(x)ᵢ = eˣⁱ / (eˣ¹ + eˣ² + ... + eˣᵏ)
  ```
  Chaque sortie est positive, et la somme de toutes les sorties vaut exactement `1` — c'est cette
  propriété qui en fait la sortie naturelle d'un réseau qui doit choisir parmi plusieurs actions
  discrètes (voir [espace d'action](@ref lot-annexe-07) et
  [REINFORCE](@ref guide-annexe-reinforce)) : chaque sortie devient la probabilité de choisir
  l'action correspondante.

Chacune de ces fonctions doit avoir une dérivée connue pour être utilisable dans le graphe
d'autodiff (chapitre précédent) — `LOT-ANNEXE-02` fournit `relu`/`tanh`, `LOT-ANNEXE-03` ajoute
`sigmoid`/`softmax` en utilisant la même fabrique générique (`unaryOp`/`binaryOp`), sans modifier
le moteur d'autodiff.

## 5. Pourquoi l'initialisation des poids compte

Avant tout entraînement, les poids doivent être fixés à des valeurs de départ — jamais toutes à
zéro (sans quoi tous les neurones d'une même couche calculeraient exactement la même chose et
recevraient exactement le même gradient, restant identiques indéfiniment — un problème de symétrie
qui empêche tout apprentissage utile) ; jamais trop grandes ni trop petites non plus. Si les poids
sont trop grands, les valeurs traversant le réseau peuvent croître de couche en couche jusqu'à
devenir gigantesques (*exploding*) ; si les poids sont trop petits, les valeurs peuvent au contraire
s'écraser vers zéro (*vanishing*) — dans les deux cas, l'entraînement démarre dans de très
mauvaises conditions numériques.

Deux schémas d'initialisation aléatoire (LOT-ANNEXE-03, via `aisolver::Rng`, voir
[Algèbre tensorielle](@ref guide-annexe-algebre-tensorielle) pour le générateur) visent à maintenir
une amplitude de signal stable à travers les couches :

- **Xavier** (ou *Glorot*) : les poids sont tirés avec une variance
  `2 / (entrées + sorties)` de la couche — calibré pour une activation `tanh`/`sigmoid`, symétrique
  autour de zéro.
- **He** : variance `2 / entrées` — calibré pour `relu`, qui n'est pas symétrique (elle annule
  toutes les valeurs négatives, donc seule la moitié environ des neurones « survit » à chaque
  couche en moyenne, ce que la formule de He compense).

Le choix du schéma dépend donc de l'activation qui **suivra** la couche — une décision explicite à
la construction de chaque couche (voir `LOT-ANNEXE-03`, décisions de cadrage).

## 6. Composer les couches en réseau

Un `aisolver::nn::Network` est simplement une **séquence** de couches (chacune avec son activation
associée) : la sortie d'une couche devient l'entrée de la suivante. La passe avant du réseau entier
est la composition de toutes les passes avant de ses couches — cohérent avec le graphe de calcul du
chapitre précédent : chaque couche ajoute des nœuds au graphe, et `backward()` peut ensuite
propager le gradient à travers **tout** le réseau d'un coup, jusqu'aux poids de la toute première
couche, sans qu'aucune formule composée n'ait été écrite à la main.

## 7. Pourquoi sauvegarder les poids

Un réseau entraîné (générations 2/3 du programme) n'a de valeur que si ses poids peuvent être
réutilisés plus tard — en particulier pour produire le fichier de rejeu déterministe
(@ref lot-annexe-07) qui sera rejoué en jeu (@ref lot-annexe-18), **sans jamais réentraîner ni
recharger un réseau en temps réel dans le jeu lui-même** (décision ferme du programme : rejeu
uniquement). D'où la sérialisation versionnée des poids (`LOT-ANNEXE-03`, TACHE-04) : un format de
fichier propre, minimal, qui permet de recharger exactement le même réseau plus tard.

## Sources

- Goodfellow, I., Bengio, Y., Courville, A. (2016). *Deep Learning*. MIT Press. — référence
  générale sur les réseaux de neurones (architecture, activations, initialisation), consultable
  gratuitement en ligne (deeplearningbook.org) une fois de retour avec accès Internet.
- Glorot, X., Bengio, Y. (2010). *Understanding the difficulty of training deep feedforward neural
  networks*. Proceedings of AISTATS 2010. — article d'origine de l'initialisation Xavier/Glorot.
- He, K., Zhang, X., Ren, S., Sun, J. (2015). *Delving Deep into Rectifiers: Surpassing Human-Level
  Performance on ImageNet Classification*. Proceedings of ICCV 2015. — article d'origine de
  l'initialisation He, calibrée pour ReLU.
- Nair, V., Hinton, G.E. (2010). *Rectified Linear Units Improve Restricted Boltzmann Machines*.
  Proceedings of ICML 2010. — popularisation de ReLU en apprentissage profond.
