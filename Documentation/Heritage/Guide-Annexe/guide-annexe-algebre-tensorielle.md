# Algèbre tensorielle et calcul numérique {#guide-annexe-algebre-tensorielle}

**Lot concerné :** [LOT-ANNEXE-01](@ref lot-annexe-01) (bibliothèque tensorielle et RNG maison).

## Pourquoi ce chapitre

Un réseau de neurones (chapitre [Réseaux de neurones](@ref guide-annexe-reseaux-neurones)) n'est,
au fond, qu'une suite de multiplications de matrices et d'additions. Pour l'écrire en C++, il faut
d'abord un type de donnée capable de stocker et manipuler ces matrices (et, plus généralement, des
tableaux à un nombre arbitraire de dimensions — des **tenseurs**) de façon efficace et sans erreur.
Ce chapitre part de zéro : qu'est-ce qu'un vecteur, une matrice, un tenseur, et comment les
représenter en mémoire.

## 1. Scalaire, vecteur, matrice, tenseur

- Un **scalaire** est un nombre seul : `3`, `-1.5`, `0`.
- Un **vecteur** est une liste ordonnée de scalaires : `v = (1, 2, 3)` a 3 **éléments**. On dit
  qu'il a une **dimension** de 3 (pas à confondre avec le nombre d'**axes**, voir plus bas).
- Une **matrice** est un tableau à deux axes (lignes × colonnes) :
  ```
  M = [ 1  2  3 ]
      [ 4  5  6 ]
  ```
  Cette matrice a 2 lignes et 3 colonnes ; on dit que sa **forme** (*shape*) est `(2, 3)`.
- Un **tenseur** généralise cette idée à un nombre quelconque d'**axes** (aussi appelés
  *dimensions* dans le jargon du calcul tensoriel — à ne pas confondre avec la dimension d'un
  vecteur au sens ci-dessus ; le contexte lève l'ambiguïté). Un scalaire est un tenseur à 0 axe, un
  vecteur un tenseur à 1 axe, une matrice un tenseur à 2 axes ; une pile de plusieurs matrices
  identiques en taille est un tenseur à 3 axes, de forme `(profondeur, lignes, colonnes)`, etc.

Dans le programme Lot-Annexe, presque tout est un tenseur : les poids d'une couche de réseau
(matrice), l'observation d'un pas de jeu (vecteur), un lot (*batch*) d'observations (matrice, une
ligne par observation), etc. `aisolver::Tensor<T>` (LOT-ANNEXE-01) est le type C++ qui représente
ceci de façon générique.

## 2. Représenter un tenseur en mémoire : forme, stride, tampon contigu

La mémoire d'un ordinateur est **linéaire** — une longue suite de cases. Un tenseur à plusieurs
axes doit donc être **aplati** dans cette suite linéaire d'une façon cohérente, puis on doit savoir
retrouver, à partir d'indices `(i, j, ...)`, la position exacte dans ce tampon linéaire.

### 2.1. Cas d'une matrice (2 axes)

Pour une matrice de forme `(lignes, colonnes)`, la convention **ligne par ligne** (*row-major*,
celle du C/C++, par opposition à *column-major* du Fortran/MATLAB) range d'abord toute la première
ligne, puis toute la deuxième, etc. :

```
M = [ 1  2  3 ]        tampon linéaire : [1, 2, 3, 4, 5, 6]
    [ 4  5  6 ]                            ligne 0 -- ligne 1
```

Pour retrouver `M[i][j]` dans ce tampon, la formule est :

```
position = i * nombreDeColonnes + j
```

Ici, avec `nombreDeColonnes = 3` : `M[1][2]` (la valeur `6`) est à la position
`1 * 3 + 2 = 5` — et `tampon[5] == 6`. Vérifié.

### 2.2. Généralisation : le *stride*

Pour un tenseur à N axes de forme `(d₀, d₁, ..., dₙ₋₁)`, on généralise la formule ci-dessus avec un
**stride** (pas) par axe : `stride[k]` est le nombre de cases du tampon linéaire à sauter pour
avancer d'une unité sur l'axe `k`. Pour un tampon contigu rangé en *row-major*, le stride se calcule
de la droite vers la gauche :

```
stride[dernier axe] = 1
stride[k] = stride[k+1] * d[k+1]   (pour k allant de l'avant-dernier axe au premier)
```

Exemple pour une forme `(2, 3, 4)` (2 matrices de 3 lignes × 4 colonnes) :
`stride[2] = 1`, `stride[1] = 1 * 4 = 4`, `stride[0] = 4 * 3 = 12`. La position d'un élément
`(i, j, k)` est alors `i*12 + j*4 + k*1`.

**Pourquoi le stride est stocké explicitement plutôt que recalculé** : une fois qu'on a le stride,
on peut représenter une **vue** (*view*) — une réinterprétation d'un tenseur existant sous une
autre forme, ou un sous-ensemble de ses éléments (par exemple une seule ligne d'une matrice) —
**sans copier aucune donnée** : on change juste la forme et le stride associés au même tampon
physique. C'est ce que fait `aisolver::Tensor<T>::view` (TACHE-02 du lot).

### 2.3. Ce que ça donne en C++ (illustratif)

```cpp
struct Tensor {
    std::vector<float> data;    // tampon contigu, partagé par les vues
    std::vector<int> shape;     // ex. {2, 3, 4}
    std::vector<int> stride;    // ex. {12, 4, 1}

    float& at(std::vector<int> indices) {
        int position = 0;
        for (size_t axis = 0; axis < indices.size(); ++axis) {
            position += indices[axis] * stride[axis];
        }
        return data[position];
    }
};
```

## 3. Opérations élémentaires (élément par élément)

L'addition, la soustraction, la multiplication et la division **élément par élément** entre deux
tenseurs de **même forme** produisent un tenseur de cette même forme, où chaque case est le
résultat de l'opération appliquée aux deux cases correspondantes :

```
A = [1, 2, 3]     B = [10, 20, 30]     A + B = [11, 22, 33]
```

Une opération entre un tenseur et un **scalaire** (« diffusion scalaire », *scalar broadcasting*)
applique le même scalaire à chaque case : `A + 1 = [2, 3, 4]`.

> Note de vocabulaire : le *broadcasting* général (façon NumPy — combiner deux tenseurs de formes
> différentes mais compatibles, ex. `(3,1) + (1,4) → (3,4)`) est une notion plus large que la seule
> diffusion scalaire. LOT-ANNEXE-01 ne couvre que le cas scalaire (voir sa section Exclus) — la
> notion générale n'est pas nécessaire au programme Lot-Annexe et n'est donc pas développée ici.

## 4. Produit matriciel

Le produit matriciel (`A × B`, noté `matmul` dans le code) est **différent** de la multiplication
élément par élément — c'est l'opération centrale de tout réseau de neurones (une couche dense,
chapitre suivant, n'est rien d'autre qu'un produit matriciel suivi d'une addition).

**Règle** : pour multiplier une matrice `A` de forme `(m, n)` par une matrice `B` de forme
`(n, p)`, il faut que le nombre de colonnes de `A` égale le nombre de lignes de `B` (ce nombre
commun est `n`). Le résultat `C = A × B` a la forme `(m, p)`, et chaque case se calcule ainsi :

```
C[i][j] = somme, pour k allant de 0 à n-1, de A[i][k] * B[k][j]
```

**Exemple travaillé à la main** — `A` de forme `(2, 2)`, `B` de forme `(2, 2)` :

```
A = [ 1  2 ]        B = [ 5  6 ]
    [ 3  4 ]            [ 7  8 ]

C[0][0] = A[0][0]*B[0][0] + A[0][1]*B[1][0] = 1*5 + 2*7 = 5 + 14 = 19
C[0][1] = A[0][0]*B[0][1] + A[0][1]*B[1][1] = 1*6 + 2*8 = 6 + 16 = 22
C[1][0] = A[1][0]*B[0][0] + A[1][1]*B[1][0] = 3*5 + 4*7 = 15 + 28 = 43
C[1][1] = A[1][0]*B[0][1] + A[1][1]*B[1][1] = 3*6 + 4*8 = 18 + 32 = 50

C = [ 19  22 ]
    [ 43  50 ]
```

**Intuition à retenir** : chaque case du résultat est un **produit scalaire** (somme de produits)
entre une ligne de `A` et une colonne de `B`. C'est exactement ce qui se passe dans une couche
dense : chaque neurone de sortie fait la somme pondérée (produit scalaire) de toutes les entrées
avec ses propres poids.

**Transposition** : la transposée `Aᵀ` d'une matrice `A` de forme `(m, n)` est la matrice de forme
`(n, m)` où `Aᵀ[j][i] = A[i][j]` (lignes et colonnes échangées). Utile en particulier pour la
rétropropagation du gradient à travers un produit matriciel (chapitre
[Autodiff](@ref guide-annexe-autodiff)).

## 5. Réductions

Une **réduction** combine tous les éléments d'un tenseur en une seule valeur :
- **Somme** (`sum`) : `1 + 2 + 3 + 4 = 10`.
- **Moyenne** (`mean`) : somme divisée par le nombre d'éléments — `10 / 4 = 2.5`.
- **Maximum** (`max`) : la plus grande valeur — `4`.

Ces trois réductions servent, entre autres, à calculer une **perte** (*loss*) : un réseau produit
un tenseur de sorties, et une fonction de perte le réduit à un seul scalaire (par exemple une
moyenne d'erreurs) que l'entraînement cherche à minimiser (voir
[Optimisation](@ref guide-annexe-optimisation)).

## 6. Pourquoi les erreurs de forme sont des bugs, pas des erreurs à gérer

Si on essaie de multiplier deux matrices dont les formes ne correspondent pas (ex. `(2,3) ×
(2,3)`), il n'existe **mathématiquement aucun résultat** — ce n'est pas une valeur invalide qu'on
pourrait signaler poliment à un utilisateur, c'est le signe qu'un programme a mal composé son
calcul (une couche mal câblée, par exemple). C'est pourquoi LOT-ANNEXE-01 traite ceci avec
`JADG_ASSERT` (arrêt immédiat en débogage) plutôt qu'avec un code d'erreur — cohérent avec
la distinction déjà appliquée dans tout le projet entre *erreur de programmation* (assertion) et
*erreur récupérable attendue* (résultat optionnel), voir `Documentation/Specification/
conventions.md`.

## 7. Générateurs pseudo-aléatoires déterministes

Tout le programme a besoin d'aléatoire contrôlé : initialiser des poids (chapitre
[Réseaux de neurones](@ref guide-annexe-reseaux-neurones)), muter une population (chapitre
[algorithmes évolutionnistes](@ref guide-annexe-algorithmes-evolutionnistes)), échantillonner une
action selon une distribution (chapitre [REINFORCE](@ref guide-annexe-reinforce)). Un ordinateur ne
produit jamais de « vrai » hasard : un **générateur pseudo-aléatoire** (PRNG) produit une séquence
de nombres qui *paraît* aléatoire (aucun motif exploitable statistiquement) mais qui est en réalité
entièrement déterminée par une valeur de départ, la **graine** (*seed*) — la même graine produit
toujours exactement la même séquence. C'est cette propriété, pas le « hasard » en lui-même, qui
importe le plus ici : elle rend un entraînement **reproductible** (rejouer exactement la même
séquence d'événements aléatoires à partir de la même graine), condition nécessaire pour déboguer un
entraînement ou comparer équitablement deux réglages d'hyperparamètres.

`aisolver::Rng` (LOT-ANNEXE-01) utilise `std::mt19937_64`, une implémentation du générateur
**Mersenne Twister** (voir Sources) déjà fournie par la bibliothèque standard C++ — un choix
délibéré : c'est de la bibliothèque standard, pas un framework d'apprentissage automatique, donc
hors du périmètre de la contrainte « from scratch, sans dépendance » du programme.

**Un piège à connaître : moteur ≠ loi de probabilité.** Un PRNG comme Mersenne Twister produit des
entiers bruts sur 64 bits. Pour en tirer un flottant dans `[0, 1)`, un entier dans `[a, b]` ou un
nombre gaussien, il faut une **loi de probabilité** appliquée par-dessus le moteur. La bibliothèque
standard en fournit (`std::uniform_real_distribution`, `std::uniform_int_distribution`,
`std::normal_distribution`) — mais la norme C++ spécifie **la suite exacte de valeurs produite par
`std::mt19937_64`** et **ne spécifie pas** celle produite par ces distributions : deux compilateurs
(ou deux versions du même) peuvent en donner des résultats différents pour la même graine, tout en
étant parfaitement conformes. Un programme qui veut de la reproductibilité *portable* doit donc
appliquer les lois lui-même, à partir de la sortie brute du moteur — c'est ce que fait
`aisolver::Rng` (LOT-ANNEXE-01, TACHE-01) : conversion sur 24 bits pour `nextFloat`, tirage par
rejet pour `nextInt` (le modulo simple biaise légèrement les dernières valeurs de la plage), et
Box-Muller ci-dessous pour `nextGaussian`.

**Du tirage uniforme au tirage gaussien** : `std::mt19937_64` (comme la plupart des PRNG) produit
nativement des nombres **uniformément** répartis dans un intervalle (chaque valeur a la même
chance d'apparaître). La mutation évolutionniste (chapitre
[algorithmes évolutionnistes](@ref guide-annexe-algorithmes-evolutionnistes)) a besoin, elle, de
nombres suivant une loi **normale/gaussienne** (la plupart des valeurs proches de zéro, de moins en
moins probables à mesure qu'on s'en éloigne — la courbe « en cloche »). La transformation de
**Box-Muller** (voir Sources) convertit deux nombres uniformes `u₁, u₂` (tirés dans `(0, 1]`) en un
nombre gaussien :

```
gaussien = √(-2 × ln(u₁)) × cos(2π × u₂)
```

`aisolver::Rng::nextGaussian` applique cette formule sur deux tirages uniformes successifs du même
générateur sous-jacent.

## Sources

- Harris, C.R., Millman, K.J., van der Walt, S.J. et al. (2020). *Array programming with NumPy*.
  Nature 585, 357–362. — référence pour les conventions de forme/stride/vue sans copie
  (« *strided array* ») reprises ici ; NumPy est l'implémentation la plus largement utilisée de ces
  idées, bien que ce chapitre et LOT-ANNEXE-01 les réimplémentent entièrement en C++ maison.
- Strang, G. (2016). *Introduction to Linear Algebra* (5th ed.). Wellesley-Cambridge Press. —
  référence générale pour l'algèbre linéaire (produit matriciel, transposition) au-delà de ce qui
  est nécessaire au programme.
- Golub, G.H., Van Loan, C.F. (2013). *Matrix Computations* (4th ed.). Johns Hopkins University
  Press. — référence de calcul matriciel numérique, pour approfondir la stabilité numérique des
  opérations si besoin au-delà de ce chapitre.
- Matsumoto, M., Nishimura, T. (1998). *Mersenne Twister: A 623-Dimensionally Equidistributed
  Uniform Pseudo-Random Number Generator*. ACM Transactions on Modeling and Computer Simulation
  8(1), 3–30. — article d'origine de l'algorithme Mersenne Twister, implémenté par
  `std::mt19937_64` de la bibliothèque standard C++, utilisé par `aisolver::Rng`.
- Box, G.E.P., Muller, M.E. (1958). *A Note on the Generation of Random Normal Deviates*. Annals of
  Mathematical Statistics 29(2), 610–611. — origine de la transformation convertissant deux tirages
  uniformes en un tirage gaussien (§7), utilisée par `aisolver::Rng::nextGaussian`.
