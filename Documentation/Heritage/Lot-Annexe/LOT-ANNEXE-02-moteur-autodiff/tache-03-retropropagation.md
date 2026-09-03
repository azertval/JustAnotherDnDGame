# TACHE-03 — `backward()` : parcours topologique inverse {#lot-annexe-02-tache-03-retropropagation}

**Lot :** [LOT-ANNEXE-02](epic.md) · **Emplacement :** `Source/AiSolver/Math/Autodiff` · **Statut :** fait

## Contexte
`Node` sait enregistrer sa règle de dérivation locale (TACHE-01) et cinq opérations savent
l'utiliser (TACHE-02), mais rien ne **déclenche** encore la propagation : chaque `_backwardFn`
enregistrée reste inerte tant qu'elle n'est pas appelée. Cette tâche ajoute `backward()`, le
parcours qui active toutes les règles de dérivation locales dans le bon ordre — de la racine vers
les feuilles — pour que chaque `Node::grad` reflète la dérivée de la racine par rapport à lui.

## Travail à réaliser
- **`Source/AiSolver/Math/Autodiff/Node.h`/`.cpp`** : ajout de la fonction libre `void
  backward(const NodePtr& root);`.
  - `JADG_ASSERT(root->value.size() == 1)` : la racine doit être **scalaire** (décision de
    cadrage de l'épic).
  - Initialise `root->grad` à un tenseur de même forme (un seul élément) valant `1.0f` — la dérivée
    d'une quantité par rapport à elle-même.
  - **Tri topologique** : parcours en profondeur (DFS) depuis `root`, empilant chaque nœud visité
    **après** avoir visité récursivement tous ses parents (ordre post-fixe) — construit une liste où
    chaque nœud apparaît **après** tous les nœuds qui en dépendent (donc **avant** tous ses propres
    parents). Un ensemble de nœuds déjà visités (`std::unordered_set` sur l'adresse du `Node`, via
    `Node*` brut le temps du parcours — pas de cycle possible : un graphe construit uniquement par
    `unaryOp`/`binaryOp` à la volée ne peut pas créer de référence arrière) évite de revisiter un
    nœud partagé par plusieurs chemins.
  - **Propagation** : parcourt la liste topologique en ordre **inverse** (de la racine vers les
    feuilles) et appelle `_backwardFn()` sur chaque nœud qui en a une (les feuilles créées par
    `variable()` n'en ont pas, cf. TACHE-01) — chaque appel accumule (`+=`) dans le(s) `grad` du(des)
    parent(s) direct(s) du nœud courant.

## Fichiers impactés
- `Source/AiSolver/Math/Autodiff/Node.h` (ajout de la signature `backward`).
- `Source/AiSolver/Math/Autodiff/Node.cpp` (implémentation du tri topologique et de la propagation).
- `Source/Test/CMakeLists.txt` (ajout de `Unit/AiSolver/Math/test_autodiff_backward.cpp`).
- `Source/Test/Unit/AiSolver/Math/test_autodiff_backward.cpp` (nouveau).

## Tests (obligatoires)
- **Cas simple `y = a + b`** : après `backward(y)`, `a->grad` et `b->grad` valent chacun `1.0`
  (dérivée de la somme par rapport à chaque terme).
- **Cas `y = a * b`** : après `backward(y)`, `a->grad == b->value` et `b->grad == a->value` (règle
  du produit), sur des tenseurs à un seul élément.
- **Nœud réutilisé (partagé) dans le graphe** : `y = (a + b) + a` (le nœud `a` apparaît deux fois) ;
  après `backward(y)`, `a->grad` vaut `2.0` (somme des deux contributions), pas `1.0` — vérifie
  explicitement l'accumulation (critère d'acceptation n°2 de l'épic).
- **Chaîne plus profonde** : `y = relu(matmul(w, x) + b)` (motif d'une couche dense) —
  `backward(y)` produit des gradients non nuls sur `w`, `x`, `b`, dont les valeurs sont vérifiées
  contre un calcul de référence posé à la main dans le test (indépendamment de TACHE-04, qui
  vérifiera la même chose de façon générique par différences finies).
- **Assertion sur racine non scalaire** : `backward()` appelé sur un `Node` dont `value.size() > 1`
  déclenche `JADG_ASSERT`.
- **Idempotence de l'appel unique** : appeler `backward()` une seconde fois sur le même graphe
  **sans** `zeroGrad()` entre les deux **accumule** par-dessus les gradients précédents (comportement
  attendu, documenté — c'est à l'appelant, typiquement l'optimiseur de `LOT-ANNEXE-04`, d'appeler
  `zeroGrad()` entre deux passes d'entraînement, exactement comme un `Node` réutilisé dans un même
  graphe accumule ses contributions).

## Points d'attention
- **Le tri topologique utilise des pointeurs bruts (`Node*`) uniquement pour la déduplication par
  adresse**, jamais pour prolonger la durée de vie d'un nœud — les `shared_ptr` (`NodePtr`) restent
  la seule source de propriété ; le parcours ne fait que **lire** un graphe déjà entièrement
  construit et vivant (détenu par l'appelant via `root` et transitivement via les captures de
  `_backwardFn`, TACHE-01).
- **Aucune détection de cycle explicite** : le graphe ne peut pas en contenir par construction
  (`unaryOp`/`binaryOp` ne référencent que des nœuds déjà existants au moment de la construction du
  nouveau nœud) — un `JADG_ASSERT` de profondeur maximale n'est pas ajouté ici, jugé
  inutile pour un graphe qui reflète la profondeur d'un réseau (quelques dizaines de couches au
  plus, `LOT-ANNEXE-03`).
- **`backward()` ne retourne rien** : les gradients se lisent directement sur chaque `Node::grad`
  après l'appel — cohérent avec l'usage prévu (l'optimiseur, `LOT-ANNEXE-04`, lit `grad` sur chaque
  paramètre après un appel à `backward()` sur la perte).

## Définition de fait (DoD)
- `backward()` disponible, testé (`ctest` vert) sur les cas listés, y compris l'accumulation sur nœud
  partagé et l'assertion sur racine non scalaire ; build `/W4 /WX` sans avertissement ; Doxygen à
  jour.

## Notions abordées
@ref guide-annexe-autodiff (dérivée, règle de la chaîne, graphe de calcul, rétropropagation en mode
inverse), en particulier ses sections 5 (rétropropagation pas à pas), 6 (accumulation des gradients
par `+=`) et 7 (parcours topologique inverse).

## Exigences
Contribue à `EX-IA-002` (déclarée dans [l'épic](epic.md)) ; aucune exigence propre à cette tâche.
