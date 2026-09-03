# TACHE-02 — Opérations différentiables de base {#lot-annexe-02-tache-02-operations-differentiables}

**Lot :** [LOT-ANNEXE-02](epic.md) · **Emplacement :** `Source/AiSolver/Math/Autodiff` · **Statut :** fait

## Contexte
`Node`, `unaryOp` et `binaryOp` (TACHE-01) posent la mécanique générique mais ne fournissent encore
aucune opération concrète. Cette tâche ajoute les cinq opérations différentiables nécessaires à une
couche dense avec activation (`LOT-ANNEXE-03`) : addition, multiplication élément par élément,
produit matriciel, `relu`, `tanh` — chacune construite **au-dessus** de `unaryOp`/`binaryOp`, sans
toucher à `Node.h`.

## Travail à réaliser
- **`Source/AiSolver/Math/Autodiff/Ops.h`** : fonctions libres dans `namespace aisolver::autodiff`,
  chacune implémentée via `binaryOp`/`unaryOp` (TACHE-01) plutôt qu'en manipulant `Node`
  directement.
  - `NodePtr add(const NodePtr& a, const NodePtr& b);` — `forward` : `aisolver::add(a->value,
    b->value)` (`LOT-ANNEXE-01`). Règle de dérivation : le gradient de sortie se propage
    **inchangé** aux deux parents (`d(a+b)/da = 1`, `d(a+b)/db = 1`).
  - `NodePtr multiply(const NodePtr& a, const NodePtr& b);` — `forward` :
    `aisolver::multiply(a->value, b->value)`. Règle : gradient vers `a` = gradient de sortie ×
    `b->value` (élément par élément), et symétriquement vers `b` (règle du produit).
  - `NodePtr matmul(const NodePtr& a, const NodePtr& b);` — `forward` : `aisolver::matmul(a->value,
    b->value)` (`LOT-ANNEXE-01`). Règle : gradient vers `a` = `matmul(gradientDeSortie,
    transpose(b->value))` ; gradient vers `b` = `matmul(transpose(a->value), gradientDeSortie)` —
    dérivation matricielle standard du produit.
  - `NodePtr relu(const NodePtr& a);` — `forward` : chaque élément `max(0, x)` (implémenté ici,
    n'existe pas déjà dans `TensorOps`). Règle : gradient vers `a` = gradient de sortie **si**
    l'élément d'entrée était `> 0`, sinon `0` (dérivée de `ReLU` : `1` si `x > 0`, `0` sinon ; le
    point `x = 0` est traité comme `0`, choix arbitraire mais documenté, sans conséquence pratique
    — probabilité nulle de tomber exactement sur `0.0f` avec des entrées flottantes réelles).
  - `NodePtr tanhOp(const NodePtr& a);` (nommé `tanhOp`, pas `tanh`, pour ne pas ombrer
    `std::tanh`) — `forward` : `std::tanh` élément par élément. Règle : gradient vers `a` = gradient
    de sortie × `(1 − tanh(x)²)` (dérivée standard de la tangente hyperbolique), calculée à partir
    de `outputValue` déjà connu (`tanh(x)` déjà calculé en avant, pas recalculé).

## Fichiers impactés
- `Source/AiSolver/Math/Autodiff/Ops.h` (nouveau).
- `Source/AiSolver/Math/Autodiff/Ops.cpp` (nouveau).
- `Source/AiSolver/CMakeLists.txt` (ajout de `Math/Autodiff/Ops.cpp`).
- `Source/Test/CMakeLists.txt` (ajout de `Unit/AiSolver/Math/test_autodiff_ops.cpp`).
- `Source/Test/Unit/AiSolver/Math/test_autodiff_ops.cpp` (nouveau).

## Tests (obligatoires)
- **Passe avant correcte** pour chacune des cinq opérations : sur des `Node` construits depuis des
  valeurs connues, `value` du résultat correspond au calcul de référence (fait à la main dans le
  test).
- **`relu` : zéro sous le seuil** : pour une entrée contenant des valeurs négatives, positives et
  nulle, la sortie de `relu` vaut respectivement `0`, la valeur inchangée, et `0`.
- **`tanhOp` bornée** : pour des entrées de grande amplitude (positive et négative), la sortie reste
  strictement dans `]-1, 1[`, sans `NaN`/`inf`.
- **Composition d'opérations** : un petit graphe combinant `matmul` puis `add` (biais) puis `relu`
  (motif d'une couche dense complète, anticipant `LOT-ANNEXE-03`) calcule la bonne valeur en avant,
  sans qu'aucun appel à `backward()` ne soit nécessaire pour ce test (couvert par TACHE-03/04).
- Note : la **correction du gradient** de chacune des cinq opérations est vérifiée en TACHE-04
  (gradient checking), pas ici — cette tâche ne teste que la passe avant et l'enregistrement correct
  des parents.

## Points d'attention
- **`tanhOp` (pas `tanh`)** : évite tout risque de masquage (*shadowing*) de `std::tanh` par
  *argument-dependent lookup* dans le code appelant qui utiliserait `using namespace
  aisolver::autodiff;` — nommage délibérément différent.
- **Chaque opération construite via `unaryOp`/`binaryOp`, jamais en instanciant `Node` directement**
  : vérifie concrètement la décision de cadrage de l'épic (fabrique générique) — une revue de code
  doit pouvoir constater qu'`Ops.cpp` n'inclut aucune connaissance des détails internes de `Node`
  au-delà de l'interface publique de TACHE-01.
- **`relu`/`tanhOp` recalculent `outputValue` une seule fois** (dans `forward`), réutilisé tel quel
  dans la règle de dérivation (capturé par la fermeture) — ne pas recalculer `tanh(x)` une seconde
  fois dans la passe arrière, coût inutile et risque de léger écart numérique entre les deux calculs.

## Définition de fait (DoD)
- `add`, `multiply`, `matmul`, `relu`, `tanhOp` disponibles, testés (`ctest` vert) sur la passe
  avant et la composition ; build `/W4 /WX` sans avertissement ; Doxygen à jour.

## Notions abordées
@ref guide-annexe-autodiff (dérivée, règle de la chaîne, graphe de calcul, rétropropagation en mode
inverse), en particulier ses sections 2 (règle de la chaîne) et 9 (fabrique générique d'opérations).

## Exigences
Contribue à `EX-IA-002` (déclarée dans [l'épic](epic.md)) ; aucune exigence propre à cette tâche.
