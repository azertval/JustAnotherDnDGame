# TACHE-05 — Tests : formes incompatibles, cas limites, non-régression, reproductibilité du RNG {#lot-annexe-01-tache-05-tests}

**Lot :** [LOT-ANNEXE-01](epic.md) · **Emplacement :** `Source/Test/Unit/AiSolver/Math` · **Statut :** fait

## Contexte
Les quatre tâches précédentes livrent chacune leurs propres cas de test « au fil de l'eau » (listés
dans leurs sections « Tests »). Cette tâche consolide la couverture : elle vérifie que l'ensemble
tient debout **comme bibliothèque**, au-delà des tests unitaires isolés par fichier — cas limites
transverses, non-régression sur des identités mathématiques connues, et la garantie de
reproductibilité du RNG dont dépend tout le programme d'entraînement à venir.

## Travail à réaliser
- **`Source/Test/Unit/AiSolver/Math/test_rng.cpp`**, `test_tensor.cpp`, `test_tensor_ops.cpp`,
  `test_matmul.cpp` : fichiers créés au fil des TACHE-01 à 04 (référencés ici pour mémoire — cette
  tâche n'en réécrit pas le contenu détaillé, déjà spécifié dans chaque tâche).
- **`Source/Test/Unit/AiSolver/Math/test_tensor_integration.cpp`** (nouveau) : cas transverses
  combinant plusieurs briques dans un même test, non couverts par les tests unitaires par fichier :
  - Non-régression sur une **identité mathématique connue** : pour une matrice carrée inversible de
    petite taille dont l'inverse est connu à la main, `matmul(a, inverseConnu)` reproduit
    l'identité à `1e-5` près.
  - **Produits croisés en chaîne** : `matmul(matmul(a, b), c) == matmul(a, matmul(b, c))`
    (associativité), vérifié sur trois petites matrices compatibles — garde-fou contre une
    implémentation de `matmul` qui romprait cette propriété par une erreur d'indexation.
  - **Combinaison vue + opération + réduction** : construire un tenseur, en prendre une vue
    (`view`), lui appliquer une opération élémentaire (TACHE-03), puis réduire (`sum`) — vérifie
    que les trois briques (TACHE-02/03) restent cohérentes utilisées ensemble, pas seulement
    isolément.
- **Revue de la couverture des cas limites** dans les fichiers existants, complétée si un trou est
  identifié pendant cette tâche : tenseur à un seul élément (forme `{1}`), tenseur avec une
  dimension de taille `0` (si autorisé par `Tensor<T>` — sinon, vérifier que la construction avec une
  dimension `0` est explicitement couverte par une assertion documentée), très grandes/très petites
  valeurs flottantes (proches de `std::numeric_limits<float>::max()`/`min()`) sur les opérations
  élémentaires et réductions, sans production de `NaN`/`inf` inattendu.

## Fichiers impactés
- `Source/Test/Unit/AiSolver/Math/test_rng.cpp` (TACHE-01).
- `Source/Test/Unit/AiSolver/Math/test_tensor.cpp` (TACHE-02).
- `Source/Test/Unit/AiSolver/Math/test_tensor_ops.cpp` (TACHE-03).
- `Source/Test/Unit/AiSolver/Math/test_matmul.cpp` (TACHE-04).
- `Source/Test/Unit/AiSolver/Math/test_tensor_integration.cpp` (nouveau).
- `Source/Test/CMakeLists.txt` (ajout de `test_tensor_integration.cpp` à `UnitTests`).

## Tests (obligatoires)
- Voir la liste des cas transverses ci-dessus (« Travail à réaliser »).
- **Suite complète `ctest --tests-regex AiSolver`** (ou filtre GoogleTest équivalent) verte, incluant
  les tests des quatre tâches précédentes.

## Points d'attention
- **Cette tâche ne réécrit pas les tests déjà spécifiés par TACHE-01 à 04** : elle ajoute les cas
  transverses qui ne trouvent naturellement leur place dans aucun fichier isolé, et sert de point de
  vérification final avant de considérer `LOT-ANNEXE-01` terminé.
- **Tolérance flottante `1e-5`** cohérente avec le reste du projet (`Source/Test/Unit/Core/Math/
  test_vector2.cpp` utilise la même tolérance) — pas de tolérance ad hoc différente d'un fichier à
  l'autre dans `AiSolver`.
- **Pas de test de performance** dans cette tâche (cohérent avec l'exclusion de l'épic) : les cas
  limites visent la **correction** numérique, pas le temps d'exécution.

## Définition de fait (DoD)
- Couverture transverse en place, `ctest` vert sur l'ensemble de `Source/Test/Unit/AiSolver/Math`,
  build `/W4 /WX` sans avertissement ; critères d'acceptation de [l'épic](epic.md) vérifiés un par
  un avant de marquer `LOT-ANNEXE-01` terminé.

## Notions abordées
@ref guide-annexe-algebre-tensorielle (tenseurs, forme et *stride*, produit matriciel, réductions,
générateurs pseudo-aléatoires), en particulier sa section 6 (pourquoi une erreur de forme est un bug
et non une erreur à gérer).

## Exigences
Contribue à `EX-IA-001` (déclarée dans [l'épic](epic.md)) ; aucune exigence propre à cette tâche.
