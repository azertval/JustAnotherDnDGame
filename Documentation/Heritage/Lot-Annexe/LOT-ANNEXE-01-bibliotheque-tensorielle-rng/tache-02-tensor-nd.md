# TACHE-02 — Conteneur N-D (`Tensor`) {#lot-annexe-01-tache-02-tensor-nd}

**Lot :** [LOT-ANNEXE-01](epic.md) · **Emplacement :** `Source/AiSolver/Math` · **Statut :** fait

## Contexte
`Rng` (TACHE-01) fournit l'aléatoire ; il manque encore le conteneur numérique sur lequel tout le
reste du programme s'appuie (poids de réseau, observations, actions). `Tensor<T>` est un gabarit
**header-only** (comme les types génériques du projet) : forme et *stride* explicites, allocation
contiguë, vues sans copie.

## Travail à réaliser
- **`Source/AiSolver/Math/Tensor.h`** : classe gabarit `template <typename T> class Tensor`.
  - Stockage : `std::shared_ptr<std::vector<T>>` pour le tampon de données — le partage de
    propriété permet aux vues (reshape) de référencer le **même** tampon sans copie.
  - `explicit Tensor(std::vector<std::size_t> shape);` : alloue `product(shape)` éléments,
    initialisés à `T{}` ; calcule les *strides* en ordre ligne (*row-major*, dernière dimension
    contiguë).
  - Constructeur privé supplémentaire prenant tampon partagé + forme + *strides* explicites, utilisé
    en interne par `view()`.
  - `std::size_t rank() const;`, `const std::vector<std::size_t>& shape() const;`,
    `const std::vector<std::size_t>& strides() const;`, `std::size_t size() const;` (nombre total
    d'éléments, `product(shape())`).
  - `T& at(std::initializer_list<std::size_t> indices);` / équivalent `const` : calcule l'offset
    linéaire par produit scalaire indices·strides ; `JADG_ASSERT` si `indices.size() !=
    rank()` ou si un indice dépasse sa dimension.
  - `T* data();` / `const T* data() const;` : accès brut au tampon (premier élément de la vue
    courante — utile pour les futures opérations vectorisées).
  - `Tensor<T> view(std::vector<std::size_t> newShape) const;` : renvoie un nouveau `Tensor<T>`
    partageant le **même** tampon, avec de nouveaux *strides* recalculés pour `newShape` ;
    `JADG_ASSERT` si `product(newShape) != size()` (un reshape ne change jamais le nombre
    total d'éléments).
  - `Tensor<T> clone() const;` : copie profonde (nouveau tampon, mêmes valeurs) — nécessaire dès
    qu'un appelant veut modifier une copie sans affecter les vues partageant l'original.

## Fichiers impactés
- `Source/AiSolver/Math/Tensor.h` (nouveau, header-only — pas d'entrée `.cpp` dans
  `Source/AiSolver/CMakeLists.txt`, un gabarit n'a rien à compiler séparément).
- `Source/Test/CMakeLists.txt` (ajout de `Unit/AiSolver/Math/test_tensor.cpp`).
- `Source/Test/Unit/AiSolver/Math/test_tensor.cpp` (nouveau, cf. TACHE-05).

## Tests (obligatoires)
- **Construction et forme** : `Tensor<float>({2, 3}).shape()` renvoie `{2, 3}`, `.size()` renvoie
  `6`, tous les éléments initialisés à `0.0f`.
- **Indexation** : écrire à `at({i, j})` puis relire à la même position renvoie la valeur écrite ;
  les autres positions restent inchangées.
- **Row-major vérifié explicitement** : pour `Tensor<float>({2, 3})`, l'offset linéaire de `at({1,
  0})` est `3` (et non `1`), confirmant que la dernière dimension est contiguë.
- **Vue partage le tampon** : après `Tensor<float> b = a.view({6});`, écrire dans `b` via `at({k})`
  modifie la valeur lue dans `a` à la position correspondante (même tampon).
- **`clone()` ne partage pas** : après `Tensor<float> c = a.clone();`, écrire dans `c` **ne modifie
  pas** `a`.
- **Assertions sur formes invalides** : `at()` avec un mauvais nombre d'indices ou un indice hors
  bornes, et `view()` avec un nombre d'éléments incompatible, déclenchent
  `JADG_ASSERT` (vérifié en configuration où les assertions ne sont pas strippées).

## Points d'attention
- **`view()` ne copie jamais implicitement** : c'est la source la plus probable de bug (modifier une
  vue en pensant modifier une copie indépendante, ou l'inverse) — documenté explicitement en
  Doxygen sur `view()` et `clone()`, avec un exemple des deux comportements.
- **Les *strides* d'une vue ne sont pas forcément ceux qu'on obtiendrait en construisant un
  `Tensor` de la même forme depuis zéro** dans le cas général (sous-vues non contiguës) — hors
  périmètre de cette tâche (`view()` ne couvre que le reshape à volume constant, pas les sous-vues
  par tranche/*slice*), mais le champ `strides()` reste public pour que ce cas soit ajoutable plus
  tard sans changer la représentation.
- **`shared_ptr` a un coût d'indirection et de comptage atomique** : accepté ici pour la simplicité
  et la sécurité mémoire (RAII) ; la contrainte de performance de ce lot est nulle (cf. exclusions
  de l'épic).

## Définition de fait (DoD)
- `Tensor<float>` disponible, testé (`ctest` vert), Doxygen à jour sur la classe et chaque méthode
  publique ; build `/W4 /WX` sans avertissement.

## Notions abordées
@ref guide-annexe-algebre-tensorielle (tenseurs, forme et *stride*, produit matriciel, réductions,
générateurs pseudo-aléatoires), en particulier sa section 2 (représentation en mémoire : forme,
*stride*, tampon contigu, vue sans copie).

## Exigences
Contribue à `EX-IA-001` (déclarée dans [l'épic](epic.md)) ; aucune exigence propre à cette tâche.
