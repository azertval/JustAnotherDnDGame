# TACHE-01 — Graphe de calcul dynamique (`Node`) {#lot-annexe-02-tache-01-graphe-calcul}

**Lot :** [LOT-ANNEXE-02](epic.md) · **Emplacement :** `Source/AiSolver/Math/Autodiff` · **Statut :** fait

## Contexte
Première tâche du lot : elle pose la structure de données porteuse de tout le moteur — le nœud de
graphe — sans encore l'opération d'addition/multiplication ni le parcours `backward()` (TACHE-02 et
TACHE-03). Un nœud porte une valeur (`Tensor<float>`, `LOT-ANNEXE-01`), un gradient accumulé de même
forme, et la connaissance de **comment** le régénérer localement à partir de ses parents — sans quoi
`backward()` (TACHE-03) n'aurait rien à parcourir.

## Travail à réaliser
- **`Source/AiSolver/Math/Autodiff/Node.h`** : classe `aisolver::autodiff::Node`, non copiable
  (référencée exclusivement via `std::shared_ptr`, cf. `using NodePtr = std::shared_ptr<Node>;`),
  RAII.
  - Membres : `Tensor<float> value;` (résultat de la passe avant), `Tensor<float> grad;` (même
    forme que `value`, initialisé à zéro à la construction) et deux membres privés :
    `std::vector<NodePtr> _parents;` et `std::function<void()> _backwardFn;` (règle de dérivation
    locale, capturée en fermeture — appelée par `backward()`, TACHE-03, jamais directement par
    l'appelant).
  - `NodePtr variable(Tensor<float> value);` (fonction libre, pas méthode) : construit une **feuille**
    du graphe — `_parents` vide, `_backwardFn` vide (rien à propager plus loin, c'est un paramètre
    ou une entrée).
  - `void zeroGrad();` : remet `grad` à zéro (même forme que `value`) — nécessaire entre deux passes
    d'entraînement (l'optimiseur de `LOT-ANNEXE-04` l'appellera avant chaque nouvelle passe avant).
- **Fabrique générique** (préparée ici pour TACHE-02, mais l'interface est figée dès cette tâche) :
  `NodePtr unaryOp(const NodePtr& input, std::function<Tensor<float>(const Tensor<float>&)>
  forward, std::function<Tensor<float>(const Tensor<float>&, const Tensor<float>&, const
  Tensor<float>&)> localGrad);` — `localGrad(outputValue, outputGrad, inputValue)` renvoie la
  contribution à accumuler sur `input->grad`. De même `binaryOp(a, b, forward, localGradA,
  localGradB)` pour les opérations à deux parents. Ces deux fabriques construisent le nœud résultat,
  calculent `value` immédiatement (`forward` appelé tout de suite — passe avant **eager**, pas
  différée), et enregistrent `_backwardFn` comme une fermeture qui appelle `localGrad`/
  `localGradA`+`localGradB` et **accumule** (`+=`) le résultat dans le(s) `grad` du (des) parent(s).

## Fichiers impactés
- `Source/AiSolver/Math/Autodiff/Node.h` (nouveau).
- `Source/AiSolver/Math/Autodiff/Node.cpp` (nouveau — implémentation de `variable`, `unaryOp`,
  `binaryOp`, `Node::zeroGrad`).
- `Source/AiSolver/CMakeLists.txt` (ajout de `Math/Autodiff/Node.cpp`).
- `Source/Test/CMakeLists.txt` (ajout de `Unit/AiSolver/Math/test_autodiff_node.cpp`).
- `Source/Test/Unit/AiSolver/Math/test_autodiff_node.cpp` (nouveau).

## Tests (obligatoires)
- **`variable` crée une feuille** : `_parents` vide (indirectement vérifiable via l'absence d'effet
  d'un appel à `backward()` en aval, TACHE-03 — ce test est donc écrit ici comme vérification de
  structure minimale : la valeur est bien celle fournie, le gradient initial est bien nul).
- **`zeroGrad` remet à zéro** : après avoir manuellement écrit une valeur non nulle dans `grad`
  (test white-box), `zeroGrad()` la ramène à zéro, forme inchangée.
- **`unaryOp`/`binaryOp` calculent `value` immédiatement** (passe avant *eager*) : le résultat de
  `forward` est visible dans `value` dès la construction du nœud, sans appel explicite supplémentaire.
- **`unaryOp`/`binaryOp` enregistrent bien `_parents`** : vérifié indirectement en TACHE-03 (le
  parcours topologique doit les retrouver) — cette tâche vérifie uniquement que la construction ne
  lève ni n'assert sur un cas simple (ex. `unaryOp` avec une fonction `forward` identité).

## Points d'attention
- **`_backwardFn` capture ses parents par copie de `shared_ptr`** (pas de référence brute) — un
  `Node` doit rester valide tant qu'un autre `Node` le référence comme parent, même si l'appelant
  d'origine a relâché sa propre référence entre la construction du graphe et l'appel à `backward()`.
- **Pas d'accumulation dans `unaryOp`/`binaryOp` elles-mêmes** : elles ne font qu'**enregistrer** la
  règle de dérivation ; l'accumulation effective dans `grad` des parents n'a lieu que lorsque
  `_backwardFn` est **appelée**, par `backward()` (TACHE-03) — ne pas appeler `localGrad` au moment
  de la construction du nœud, seulement au moment de la propagation.
- **`Tensor<float> grad` initialisé à zéro dès la construction, jamais laissé dans un état
  indéterminé** : un optimiseur (`LOT-ANNEXE-04`) qui lirait `grad` avant tout `backward()` doit
  lire des zéros, pas des valeurs non initialisées.

## Définition de fait (DoD)
- `Node`, `NodePtr`, `variable`, `unaryOp`, `binaryOp` disponibles et testés (`ctest` vert) ; build
  `/W4 /WX` sans avertissement ; Doxygen à jour.

## Notions abordées
@ref guide-annexe-autodiff (dérivée, règle de la chaîne, graphe de calcul, rétropropagation en mode
inverse), en particulier sa section 3 (graphe de calcul construit à l'exécution, *define-by-run*).

## Exigences
Contribue à `EX-IA-002` (déclarée dans [l'épic](epic.md)) ; aucune exigence propre à cette tâche.
