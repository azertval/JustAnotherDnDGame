# TACHE-02 — Fonctions d'activation différentiables (`sigmoid`, `softmax`) {#lot-annexe-03-tache-02-activations}

**Lot :** [LOT-ANNEXE-03](epic.md) · **Emplacement :** `Source/AiSolver/Nn` · **Statut :** fait

## Contexte
`Dense` (TACHE-01) produit une sortie affine brute ; un réseau a besoin d'activations non linéaires
entre ses couches pour approcher des fonctions non triviales. `LOT-ANNEXE-02` livre déjà `relu` et
`tanhOp` ; cette tâche ajoute les deux activations manquantes réellement consommées par ce
programme : `sigmoid` (sortie bornée `]0,1[`, utile pour une probabilité d'action binaire) et
`softmax` (distribution de probabilité sur plusieurs actions discrètes — pertinent dès que l'agent
choisit parmi plusieurs actions possibles, génération 2/3).

## Travail à réaliser
- **`Source/AiSolver/Nn/Activations.h`** : fonctions libres dans `namespace aisolver::nn`, chacune
  construite via `autodiff::unaryOp` (`LOT-ANNEXE-02`), exactement comme `relu`/`tanhOp` l'ont été
  dans `Ops.cpp` — aucune modification de `Node.h`/`Node.cpp`.
  - `autodiff::NodePtr sigmoid(const autodiff::NodePtr& a);` — `forward` : `1 / (1 + exp(-x))`
    élément par élément. Règle de dérivation : gradient vers `a` = gradient de sortie ×
    `outputValue × (1 − outputValue)` (dérivée standard de la sigmoïde, exprimée à partir de la
    sortie déjà calculée, comme `tanhOp`).
  - `autodiff::NodePtr softmax(const autodiff::NodePtr& a);` — `forward` sur un tenseur `[n, 1]`
    (vecteur colonne d'un vecteur de scores/*logits*) : soustrait d'abord le maximum du vecteur à
    chaque élément avant l'exponentielle (stabilité numérique — évite un débordement de `exp` sur de
    grands *logits*), puis normalise par la somme des exponentielles. Règle de dérivation : la
    dérivée du softmax est une matrice jacobienne pleine (`∂softmax_i/∂x_j = softmax_i×(δ_ij −
    softmax_j)`) ; la règle implémentée applique cette jacobienne au gradient de sortie reçu
    (produit jacobien-vecteur), sans jamais matérialiser la matrice `n×n` complète — calcul direct
    élément par élément à partir de `outputValue` et du gradient de sortie.

## Fichiers impactés
- `Source/AiSolver/Nn/Activations.h` (nouveau).
- `Source/AiSolver/Nn/Activations.cpp` (nouveau).
- `Source/AiSolver/CMakeLists.txt` (ajout de `Nn/Activations.cpp`).
- `Source/Test/CMakeLists.txt` (ajout de `Unit/AiSolver/Nn/test_activations.cpp`).
- `Source/Test/Unit/AiSolver/Nn/test_activations.cpp` (nouveau).

## Tests (obligatoires)
- **`sigmoid` bornée** : pour des entrées de grande amplitude (positive et négative), la sortie
  reste strictement dans `]0, 1[`, sans `NaN`/`inf`.
- **`softmax` somme à 1** : pour un vecteur d'entrée quelconque, la somme des éléments de sortie vaut
  `1.0` à `1e-5` près, et chaque élément reste dans `[0, 1]`.
- **`softmax` stable sur de grands *logits*** : une entrée contenant une valeur très grande (ex.
  `1000.0f`) ne produit ni `NaN` ni `inf` (vérifie explicitement la soustraction du maximum) —
  distincte d'une implémentation naïve sans stabilisation, qui échouerait sur ce cas précis.
- **`softmax` sur des entrées égales** : un vecteur constant produit une sortie **uniforme**
  (`1/n` sur chaque élément) — cas limite simple à vérifier à la main.
- **Passage du contrôle de gradient** (`LOT-ANNEXE-02`, `GradientCheck.h`) : `sigmoid` et `softmax`
  passent `checkGradient` sur des entrées aléatoires (`aisolver::Rng` à graine fixe) — condition
  bloquante avant tout usage dans `Dense`/`Network` (TACHE-03), exactement comme pour les cinq
  opérations de `LOT-ANNEXE-02`.

## Points d'attention
- **`softmax` sans stabilisation par soustraction du maximum est un piège classique** : `exp(1000)`
  déborde en `float` bien avant que le vecteur normalisé n'ait de sens — la soustraction du maximum
  ne change pas le résultat mathématique (`softmax(x) == softmax(x − max(x))`) mais rend le calcul
  robuste ; à ne jamais omettre, y compris dans un futur refactor.
- **`softmax` ne matérialise pas sa jacobienne `n×n`** : au-delà de la mémoire gaspillée pour de
  petits vecteurs d'action, ce choix anticipe des vecteurs d'action plus larges (dizaines
  d'actions) sans changer l'implémentation — le produit jacobien-vecteur direct reste `O(n²)`, la
  jacobienne matérialisée aussi, mais sans l'allocation intermédiaire.
- **`sigmoid`/`softmax` réutilisent le même mécanisme (`unaryOp`) que `relu`/`tanhOp`** : aucune
  branche spéciale n'est nécessaire dans `backward()` (`LOT-ANNEXE-02`), confirmant à nouveau que la
  fabrique générique tient sa promesse pour une opération à jacobienne non diagonale (`softmax`),
  cas plus complexe que les activations élément par élément déjà livrées.

## Définition de fait (DoD)
- `sigmoid`/`softmax` disponibles, testées (`ctest` vert), gradient vérifié par `GradientCheck.h` ;
  build `/W4 /WX` sans avertissement ; Doxygen à jour.

## Notions abordées
@ref guide-annexe-reseaux-neurones — neurone, couche dense, fonctions d'activation, initialisation
des poids.

## Exigences
Contribue à `EX-IA-003` (déclarée dans [l'épic](epic.md)) ; aucune exigence propre à cette tâche.
