# TACHE-05 — Opérations différentiables complémentaires {#lot-annexe-03-tache-05-operations-differentiables-complementaires}

**Lot :** [LOT-ANNEXE-03](epic.md) · **Emplacement :** `Source/AiSolver/Math/Autodiff` · **Statut :** fait

## Contexte
`LOT-ANNEXE-02` livre cinq opérations différentiables (`add`, `multiply`, `matmul`, `relu`,
`tanhOp`) et reporte explicitement les autres (division, exponentielle, logarithme) à ce lot, faute
de consommateur à ce moment-là. Les consommateurs existent maintenant : une perte de *policy
gradient* s'écrit `-log π(a_t | s_t) × G_t` (`LOT-ANNEXE-12`, TACHE-03), ce qui demande d'extraire
**une** probabilité d'un vecteur de sortie `softmax` (TACHE-02 de ce lot) puis d'en prendre le
**logarithme** — deux opérations qui n'existent nulle part. Sans elles, `LOT-ANNEXE-12` est
littéralement inécrivable : c'est la dépendance manquante la plus en amont de toute la génération 3.

Cette tâche complète donc l'ensemble d'opérations, **au même endroit et de la même façon** que
`LOT-ANNEXE-02` (fichier `Ops.h`/`Ops.cpp`, construites via `unaryOp`/`binaryOp`, aucune
modification de `Node`) — c'est précisément le test grandeur nature de la fabrique générique posée
par ce lot : ajouter neuf opérations sans toucher au moteur.

## Travail à réaliser
- **`Source/AiSolver/Math/Autodiff/Ops.h`/`.cpp`** (fichiers existants de `LOT-ANNEXE-02`,
  complétés — pas de nouveau fichier) : fonctions libres dans `namespace aisolver::autodiff`,
  chacune construite via `unaryOp`/`binaryOp`.
  - `NodePtr subtract(const NodePtr& a, const NodePtr& b);` — règle : gradient vers `a` = gradient
    de sortie, gradient vers `b` = **opposé** du gradient de sortie.
  - `NodePtr divide(const NodePtr& a, const NodePtr& b);` — règle : gradient vers `a` = gradient de
    sortie ÷ `b->value` ; gradient vers `b` = gradient de sortie × `(−a->value / (b->value)²)`
    (règle du quotient, élément par élément).
  - `NodePtr addScalar(const NodePtr& a, float scalar);` — le scalaire est une **constante** : le
    gradient traverse inchangé vers `a`, le scalaire n'en reçoit aucun (ce n'est pas un paramètre
    entraînable).
  - `NodePtr multiplyScalar(const NodePtr& a, float scalar);` — règle : gradient vers `a` =
    gradient de sortie × `scalar`. C'est cette opération qui porte le facteur `G_t` (retour) ou
    `advantage_t` de la perte de policy gradient : un nombre **détaché** du graphe, jamais une
    grandeur dont on veut le gradient.
  - `NodePtr logOp(const NodePtr& a);` (nommé `logOp`, pas `log`, même raison que `tanhOp` en
    `LOT-ANNEXE-02` : ne pas ombrer `std::log`) — `forward` : `std::log` élément par élément.
    Règle : gradient vers `a` = gradient de sortie ÷ `a->value`.
    `JADG_ASSERT` que tous les éléments d'entrée sont `> 0.0f`.
  - `NodePtr expOp(const NodePtr& a);` — `forward` : `std::exp` élément par élément. Règle :
    gradient vers `a` = gradient de sortie × `outputValue` (la dérivée de `exp` est `exp` lui-même —
    réutiliser la valeur déjà calculée en avant, jamais la recalculer).
  - `NodePtr selectIndex(const NodePtr& a, std::size_t index);` — extrait **un** élément d'un
    tenseur de rang 1 ou de forme `[n, 1]` et renvoie un nœud de forme `[1]` (scalaire).
    `JADG_ASSERT(index < a->value.size())`. Règle : gradient vers `a` = tenseur de la forme
    de `a`, nul partout **sauf** à `index`, où il vaut le gradient de sortie. C'est l'opération qui
    manquait pour aller de la distribution complète `softmax` à la probabilité de l'action
    effectivement jouée, **en restant dans le graphe**.
  - `NodePtr minimum(const NodePtr& a, const NodePtr& b);` — minimum élément par élément. Règle :
    le gradient de sortie va **entièrement** à l'opérande sélectionné pour chaque élément, `0` à
    l'autre ; en cas d'égalité stricte, il va à `a` (convention arbitraire, documentée — c'est un
    sous-gradient, la fonction n'est pas dérivable au point d'égalité).
  - `NodePtr clamp(const NodePtr& a, float low, float high);` — borne chaque élément dans
    `[low, high]`. Règle : gradient de sortie transmis à `a` **uniquement** pour les éléments qui
    étaient strictement à l'intérieur des bornes, `0` pour ceux qui ont été rognés (leur valeur ne
    dépend plus de l'entrée). `JADG_ASSERT(low <= high)`.
- **`Source/AiSolver/Math/TensorOps.h`** (`LOT-ANNEXE-01`) : ajout des primitives non
  différentiables manquantes si nécessaire pour écrire les `forward` ci-dessus (application d'une
  fonction unaire élément par élément) — ne pas dupliquer une boucle de parcours par *stride* dans
  `Ops.cpp`.

## Fichiers impactés
- `Source/AiSolver/Math/Autodiff/Ops.h` (complété).
- `Source/AiSolver/Math/Autodiff/Ops.cpp` (complété — `divide()` ajoute une
  `JADG_ASSERT` sur diviseur nul, au-delà de ce que décrivait le plan initial, pour
  couvrir le test « assertion sur diviseur nul » explicitement demandé plus bas).
- `Source/Test/Unit/AiSolver/Math/test_autodiff_ops.cpp` (complété — fichier créé en
  `LOT-ANNEXE-02`, TACHE-02) : passe avant des neuf opérations, plus la chaîne policy gradient
  bout-en-bout (avec `nn::softmax`, `LOT-ANNEXE-03` TACHE-02).
- `Source/Test/Unit/AiSolver/Math/test_autodiff_gradient_checking.cpp` (complété — fichier créé en
  `LOT-ANNEXE-02`, TACHE-04, non listé initialement ici mais nécessaire au DoD « gradient checking
  systématique »).

## Tests (obligatoires)
- **Gradient checking systématique** : chacune des neuf opérations passe le contrôle par différences
  finies de `LOT-ANNEXE-02` (`GradientCheck.h`, TACHE-04) — c'est la règle posée là-bas : « toute
  opération différentiable ajoutée après ce lot doit passer ce même contrôle avant tout usage en
  aval ». Exceptions documentées : `minimum` et `clamp` ne sont contrôlées qu'en des points
  **strictement** à l'intérieur (respectivement hors égalité et hors bornes), où elles sont
  réellement dérivables.
- **`selectIndex` : gradient localisé** : après `backward()` sur `selectIndex(a, 2)`, le gradient de
  `a` vaut le gradient de sortie à la position `2` et **exactement `0.0f`** partout ailleurs.
- **`logOp`/`expOp` réciproques** : `logOp(expOp(a))` reproduit `a` à `1e-5` près en avant, et
  produit un gradient de `1.0` sur `a` en arrière (composition de dérivées réciproques).
- **`divide` : assertion sur diviseur nul** et absence de `NaN`/`inf` sur un diviseur petit mais
  non nul.
- **`clamp` : gradient nul hors bornes** : une entrée au-delà de `high` reçoit un gradient
  strictement nul, une entrée à l'intérieur reçoit le gradient de sortie inchangé.
- **`minimum` : sélection du bon opérande** : sur deux tenseurs dont on connaît l'ordre élément par
  élément, le gradient ne remonte qu'à l'opérande effectivement minimal.
- **Chaîne représentative de policy gradient** :
  `multiplyScalar(logOp(selectIndex(softmax(sortie), a)), -G)` construit un scalaire dont
  `backward()` produit des gradients non nuls sur les poids du réseau — répétition à l'échelle d'un
  test unitaire de ce que `LOT-ANNEXE-12` (TACHE-03) fera à l'échelle d'une trajectoire.

## Points d'attention
- **`Node` n'est pas modifié** : si l'implémentation de l'une de ces neuf opérations demande de
  toucher à `Node.h`/`Node.cpp` (`LOT-ANNEXE-02`, TACHE-01), c'est le signe que la fabrique
  `unaryOp`/`binaryOp` est insuffisante — le constater et le traiter comme une correction de
  `LOT-ANNEXE-02`, pas comme une exception locale à ce lot.
- **`logOp` sur une probabilité issue de `softmax` est le cas d'usage réel** : `softmax` (TACHE-02)
  garantit des sorties strictement positives, l'assertion `> 0` ne devrait donc jamais se
  déclencher en production — mais un `softmax` numériquement instable (sans soustraction du maximum,
  cf. les points d'attention de TACHE-02) peut produire un `0.0f` exact par sous-dépassement, et
  l'assertion est alors précisément ce qui révèle le vrai bug plutôt qu'un `-inf` silencieux qui
  contaminerait tout l'entraînement.
- **`multiplyScalar` porte volontairement un `float`, pas un `NodePtr`** : les retours et avantages
  de la génération 3 sont des grandeurs **détachées** du graphe (`LOT-ANNEXE-12`/`13`) ; les faire
  entrer comme nœuds ouvrirait la porte à une rétropropagation à travers eux, ce qui est
  sémantiquement faux et notoirement difficile à diagnostiquer ensuite.
- **`minimum`/`clamp` ne servent qu'à PPO** (`LOT-ANNEXE-14`) : elles sont livrées ici parce
  qu'elles appartiennent à la même famille (opérations de la fabrique) et qu'elles se testent avec
  le même outillage, mais leur consommateur n'apparaîtra qu'en génération 3 — si `LOT-ANNEXE-14`
  tranche pour DQN, elles resteront inutilisées, coût assumé de deux fonctions de dix lignes.

## Définition de fait (DoD)
- Les neuf opérations sont disponibles dans `Ops.h`, passent toutes le gradient checking de
  `LOT-ANNEXE-02` (`ctest` vert), et `Node.h`/`Node.cpp` sont **inchangés** par cette tâche ; build
  `/W4 /WX` sans avertissement ; Doxygen à jour (règle de dérivation documentée pour chaque
  opération, convention de sous-gradient explicite pour `minimum`/`clamp`).

## Notions abordées
@ref guide-annexe-reseaux-neurones (neurone, couche dense, fonctions d'activation, initialisation
des poids), ainsi que @ref guide-annexe-autodiff (règle de la chaîne, fabrique générique
d'opérations) : chaque opération ajoutée ici n'est rien d'autre qu'une règle de dérivation locale de
plus.

## Exigences
Contribue à `EX-IA-003` (déclarée dans [l'épic](epic.md)) ; aucune exigence propre à cette tâche.
