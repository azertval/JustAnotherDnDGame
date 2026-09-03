# TACHE-03 — Opérations élémentaires et réductions {#lot-annexe-01-tache-03-operations-reductions}

**Lot :** [LOT-ANNEXE-01](epic.md) · **Emplacement :** `Source/AiSolver/Math` · **Statut :** fait

## Contexte
`Tensor<T>` (TACHE-02) sait stocker et indexer des valeurs, mais n'offre encore aucun calcul. Cette
tâche ajoute l'arithmétique élémentaire (nécessaire à toute couche de réseau : biais, activations,
normalisation) et les réductions globales (nécessaires à une fonction de perte scalaire).

## Travail à réaliser
- **`Source/AiSolver/Math/TensorOps.h`** : fonctions libres gabarits dans `namespace aisolver`,
  opérant sur `Tensor<T>` (header-only, comme `Tensor.h`).
  - `Tensor<T> add(const Tensor<T>& a, const Tensor<T>& b);`, et de même `subtract`, `multiply`,
    `divide` — élément par élément ; `JADG_ASSERT(a.shape() == b.shape())` (aucune
    diffusion entre deux tenseurs, seule la diffusion **scalaire** ci-dessous est couverte).
  - `Tensor<T> addScalar(const Tensor<T>& a, T scalar);`, et de même `subtractScalar`,
    `multiplyScalar`, `divideScalar` — chaque élément combiné au même scalaire.
  - Opérateurs surchargés `operator+`, `operator-`, `operator*`, `operator/` entre deux `Tensor<T>`
    (délèguent à `add`/`subtract`/`multiply`/`divide`) et entre `Tensor<T>` et `T` (délèguent aux
    variantes scalaires), pour une écriture naturelle côté appelant (`LOT-ANNEXE-02`,
    `LOT-ANNEXE-03`).
  - `sum` : somme de tous les éléments (réduction globale), signature `T sum(const Tensor<T>& a)`.
  - `mean` : `sum(a)` divisé par le nombre d'éléments, avec assertion sur tenseur non vide.
  - `max` : maximum de tous les éléments, avec assertion sur tenseur non vide (un maximum sur un
    tenseur vide n'a pas de sens) — les deux assertions utilisent `JADG_ASSERT`.
- Toutes les fonctions parcourent les éléments **logiquement** (via `at()` ou un itérateur
  équivalent respectant les *strides*), jamais en supposant que le tampon est dense dans l'ordre
  naturel — une vue (`Tensor::view`) reste correcte même si elle ne couvre pas tout le tampon sous-
  jacent dans le cas général futur (sous-vues, hors périmètre de TACHE-02 mais l'implémentation ne
  doit pas fermer la porte).

## Fichiers impactés
- `Source/AiSolver/Math/TensorOps.h` (nouveau, header-only).
- `Source/Test/CMakeLists.txt` (ajout de `Unit/AiSolver/Math/test_tensor_ops.cpp`).
- `Source/Test/Unit/AiSolver/Math/test_tensor_ops.cpp` (nouveau, cf. TACHE-05).

## Tests (obligatoires)
- **Addition/soustraction/multiplication/division élément par élément** : sur deux tenseurs `2×2`
  de valeurs connues, chaque opération produit le résultat attendu calculé à la main.
- **Diffusion scalaire** : `tensor + 1.0f`, `tensor * 2.0f`, etc., appliquent l'opération à chaque
  élément.
- **`sum`/`mean`/`max`** : sur un tenseur de valeurs connues (ex. `{1, 2, 3, 4}`), `sum == 10`,
  `mean == 2.5`, `max == 4`.
- **Assertion sur formes incompatibles** : `add` entre deux tenseurs de formes différentes déclenche
  `JADG_ASSERT`.
- **Cohérence avec `Tensor::view`** : une opération élémentaire appliquée à une vue reshape produit
  le même résultat (aux positions correspondantes) qu'appliquée au tenseur d'origine — garantit que
  les opérations respectent les *strides* et pas seulement l'ordre brut du tampon.

## Points d'attention
- **Pas de diffusion entre deux tenseurs de formes différentes** (au-delà du cas scalaire) : un
  appel à `add` sur des formes incompatibles est un bug appelant, pas une entrée à valider — cohérent
  avec la décision de cadrage de l'épic (`JADG_ASSERT`, pas d'exception).
- **`mean`/`max` sur un tenseur vide sont des erreurs de programmation** (division par zéro,
  maximum non défini), pas des cas à gérer silencieusement — `JADG_ASSERT`, même politique
  que le reste du lot.
- **Ne pas dupliquer la logique de parcours** entre `add`/`subtract`/`multiply`/`divide` : factoriser
  via un petit gabarit interne (fonction binaire appliquée élément par élément) évite quatre copier-
  coller quasi identiques et réduit la surface à corriger si le parcours par *stride* évolue.

## Définition de fait (DoD)
- Opérations élémentaires et réductions disponibles, testées (`ctest` vert), Doxygen à jour ; build
  `/W4 /WX` sans avertissement.

## Notions abordées
@ref guide-annexe-algebre-tensorielle (tenseurs, forme et *stride*, produit matriciel, réductions,
générateurs pseudo-aléatoires), en particulier ses sections 3 (opérations élément par élément,
diffusion scalaire) et 5 (réductions).

## Exigences
Contribue à `EX-IA-001` (déclarée dans [l'épic](epic.md)) ; aucune exigence propre à cette tâche.
