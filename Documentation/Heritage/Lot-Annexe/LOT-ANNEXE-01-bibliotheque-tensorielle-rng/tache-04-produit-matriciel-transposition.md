# TACHE-04 — Produit matriciel et transposition {#lot-annexe-01-tache-04-produit-matriciel-transposition}

**Lot :** [LOT-ANNEXE-01](epic.md) · **Emplacement :** `Source/AiSolver/Math` · **Statut :** fait

## Contexte
Les opérations élémentaires (TACHE-03) ne suffisent pas à exprimer une couche dense (`y = W·x + b`)
: il manque le produit matriciel. Cette tâche l'ajoute, restreint aux tenseurs de rang 2 (matrices)
— seule forme réellement consommée par `LOT-ANNEXE-03`.

## Travail à réaliser
- **`Source/AiSolver/Math/Matmul.h`** : fonctions libres gabarits dans `namespace aisolver`
  (header-only).
  - `Tensor<T> matmul(const Tensor<T>& a, const Tensor<T>& b);` : produit matriciel standard entre
    `a` de forme `[m, k]` et `b` de forme `[k, n]`, résultat `[m, n]`. `JADG_ASSERT(a.rank()
    == 2 && b.rank() == 2 && a.shape()[1] == b.shape()[0])`. Implémentation triple boucle directe
    (`i`, `j`, accumulation sur `k`) — aucune optimisation par blocs, cohérent avec l'exclusion de
    performance de l'épic.
  - `Tensor<T> transpose(const Tensor<T>& a);` : pour `a` de forme `[m, n]`, renvoie une **copie**
    de forme `[n, m]` avec `résultat.at({j, i}) == a.at({i, j})`. `JADG_ASSERT(a.rank() ==
    2)`. Copie (pas une vue à *strides* permutés) : plus simple à raisonner pour ce lot, et évite
    qu'une opération élémentaire ultérieure (TACHE-03) suppose à tort un parcours row-major strict
    sur le résultat.

## Fichiers impactés
- `Source/AiSolver/Math/Matmul.h` (nouveau, header-only).
- `Source/Test/CMakeLists.txt` (ajout de `Unit/AiSolver/Math/test_matmul.cpp`).
- `Source/Test/Unit/AiSolver/Math/test_matmul.cpp` (nouveau, cf. TACHE-05).

## Tests (obligatoires)
- **Produit par l'identité** : `matmul(a, identité[n,n]) == a` pour une matrice `a` quelconque de
  forme `[m, n]`.
- **Produit croisé connu** : sur deux petites matrices `2×2`/`2×3` de valeurs fixées à la main, le
  résultat de `matmul` correspond au calcul de référence posé dans le test (commentaire montrant le
  calcul).
- **Assertion sur dimensions incompatibles** : `matmul` entre `[2,3]` et `[4,5]` (dimension interne
  `3 ≠ 4`) déclenche `JADG_ASSERT`.
- **Assertion sur rang invalide** : `matmul`/`transpose` appelés sur un tenseur de rang `1` ou `3`
  déclenchent `JADG_ASSERT`.
- **`transpose` involutive** : `transpose(transpose(a)) == a` (mêmes valeurs, même forme).
- **`transpose` puis `matmul`** : `matmul(a, transpose(a))` sur une matrice non carrée produit une
  matrice carrée symétrique de la bonne forme, valeurs vérifiées sur un petit cas fixé à la main.

## Points d'attention
- **Complexité cubique naïve** : acceptable pour ce lot (pas d'objectif de performance), mais à
  documenter en Doxygen (`@note complexité O(m·k·n), sans optimisation par blocs`) pour que
  l'absence de vectorisation ne soit pas prise pour un oubli lors d'une revue future.
- **`matmul`/`transpose` restreints au rang 2** : un usage à rang différent est une erreur de
  programmation (couche mal composée), pas une entrée à valider — même politique
  `JADG_ASSERT` que le reste du lot.
- **Ne pas anticiper le produit matriciel par lots (*batched*)** : `LOT-ANNEXE-03` empilera des
  appels `matmul` 2D plutôt que d'exiger une troisième dimension ici (cf. exclusions de l'épic) —
  résister à la tentation de généraliser avant qu'un consommateur réel n'existe.

## Définition de fait (DoD)
- `matmul`/`transpose` disponibles, testés (`ctest` vert) sur cas connus et cas limites, Doxygen à
  jour (complexité documentée) ; build `/W4 /WX` sans avertissement.

## Notions abordées
@ref guide-annexe-algebre-tensorielle (tenseurs, forme et *stride*, produit matriciel, réductions,
générateurs pseudo-aléatoires), en particulier sa section 4 (produit matriciel, exemple travaillé à
la main, transposition).

## Exigences
Contribue à `EX-IA-001` (déclarée dans [l'épic](epic.md)) ; aucune exigence propre à cette tâche.
