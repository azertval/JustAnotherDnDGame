# TACHE-01 — Couche dense (poids + biais, forward/backward) {#lot-annexe-03-tache-01-couche-dense}

**Lot :** [LOT-ANNEXE-03](epic.md) · **Emplacement :** `Source/AiSolver/Nn` · **Statut :** fait

## Contexte
Première tâche du lot, et premier fichier du module `Source/AiSolver/Nn` (n'existe pas encore).
`autodiff::matmul`/`add` (`LOT-ANNEXE-02`) suffisent en théorie à exprimer `y = W·x + b`, mais
composer ces deux opérations à la main pour chaque couche d'un réseau serait répétitif et fragile.
`Dense` encapsule ce motif comme une classe réutilisable, porteuse de ses propres paramètres.

## Travail à réaliser
- **`Source/AiSolver/Nn/Dense.h`** : classe `aisolver::nn::Dense`, non copiable (les paramètres sont
  des `autodiff::NodePtr`, partagés par référence, jamais dupliqués implicitement).
  - Constructeur `Dense(std::size_t inputSize, std::size_t outputSize, WeightInitScheme scheme,
    Rng& rng);` (`WeightInitScheme` défini en TACHE-04, dépendance interne à la tâche — l'énumération
    est déclarée dès cette tâche dans `WeightInit.h` pour que le constructeur compile, son
    **implémentation** d'initialisation effective vient en TACHE-04). Alloue `_weights` de forme
    `[outputSize, inputSize]` et `_bias` de forme `[outputSize, 1]`, tous deux enveloppés en
    `autodiff::NodePtr` via `autodiff::variable()` (`LOT-ANNEXE-02`).
  - `autodiff::NodePtr forward(const autodiff::NodePtr& input);` — `input` de forme `[inputSize,
    1]` : calcule `autodiff::add(autodiff::matmul(_weights, input), _bias)`, forme de sortie
    `[outputSize, 1]`. Aucune activation appliquée ici (responsabilité de `Network`/`Activations.h`,
    TACHE-02/03) — `Dense` reste une transformation affine pure.
  - `std::vector<autodiff::NodePtr> parameters() const;` — renvoie `{_weights, _bias}`, pour que
    `Network::parameters()` (TACHE-03) et, en aval, l'optimiseur (`LOT-ANNEXE-04`) puissent itérer
    sur tous les paramètres entraînables sans connaître la structure interne de `Dense`.
  - Accesseurs `const Tensor<float>& weights() const;`/`const Tensor<float>& bias() const;` (lecture
    de la **valeur** courante, pas du `NodePtr`) — utilisés par la sérialisation (TACHE-04).
- **`Source/AiSolver/Nn/Dense.cpp`** : implémentation.

## Fichiers impactés
- `Source/AiSolver/Nn/Dense.h` (nouveau).
- `Source/AiSolver/Nn/Dense.cpp` (nouveau).
- `Source/AiSolver/Nn/WeightInit.h`/`.cpp` (nouveaux — `enum class WeightInitScheme { Xavier, He };`
  et `initializeWeights()` complète, cf. points d'attention : livrée ici plutôt qu'en TACHE-04).
- `Source/AiSolver/CMakeLists.txt` (ajout de `Nn/Dense.cpp`, `Nn/WeightInit.cpp`).
- `Source/Test/CMakeLists.txt` (ajout de `Unit/AiSolver/Nn/test_dense.cpp`).
- `Source/Test/Unit/AiSolver/Nn/test_dense.cpp` (nouveau).

## Tests (obligatoires)
- **Forme de sortie** : `Dense(4, 3, ...).forward(entrée [4,1])` produit une sortie de forme `[3,
  1]`.
- **Paramètres exposés** : `parameters()` renvoie exactement deux `NodePtr` (poids puis biais, dans
  cet ordre, documenté), de formes `[outputSize, inputSize]` et `[outputSize, 1]`.
- **`forward` différentiable de bout en bout** : `autodiff::backward()` appliqué à une perte scalaire
  construite sur la sortie de `forward()` produit des gradients **non nuls** sur `_weights` et
  `_bias` (démontre que `Dense` s'intègre correctement au graphe de `LOT-ANNEXE-02`, sans encore
  vérifier leur exactitude numérique — couvert par TACHE-06 via `GradientCheck.h`).
- **Deux couches indépendantes ont des poids différents** (avec une graine de `Rng` différente, ou
  la même graine consommée séquentiellement) : garde-fou contre une initialisation accidentellement
  constante.

## Points d'attention
- **`Dense` ne connaît aucune activation** : appliquer `relu`/`sigmoid`/etc. après `forward()` est la
  responsabilité de l'appelant (`Network`, TACHE-03) — une couche dense reste une transformation
  affine, cohérent avec la séparation couche/activation habituelle et avec le fait qu'une même
  couche pourrait en théorie être suivie de différentes activations selon le contexte.
- **`_weights`/`_bias` sont créés une seule fois, à la construction** : ne jamais les recréer dans
  `forward()`, sous peine de perdre le lien avec les gradients accumulés d'une itération
  d'entraînement à l'autre (cf. décision de cadrage de l'épic).
- **`WeightInitScheme` et `initializeWeights` sont livrés complets dès cette tâche**, contrairement
  au découpage envisagé à la planification (contrat ici, corps en TACHE-04) : le DoD de TACHE-01
  exige un test (« deux couches ont des poids différents ») qui ne peut pas passer sans une
  initialisation réellement aléatoire — un corps différé aurait laissé `Dense` non testable telle
  quelle. TACHE-04 se recentre donc sur la seule sérialisation ; ses propres tests statistiques
  (bornes Xavier, plausibilité He, reproductibilité par graine) restent dans son périmètre.

## Définition de fait (DoD)
- `Dense` disponible et testé (`ctest` vert) pour la forme, les paramètres exposés et
  l'intégration au graphe d'autodiff ; build `/W4 /WX` sans avertissement ; Doxygen à jour.

## Notions abordées
@ref guide-annexe-reseaux-neurones — neurone, couche dense, fonctions d'activation, initialisation
des poids.

## Exigences
Contribue à `EX-IA-003` (déclarée dans [l'épic](epic.md)) ; aucune exigence propre à cette tâche.
