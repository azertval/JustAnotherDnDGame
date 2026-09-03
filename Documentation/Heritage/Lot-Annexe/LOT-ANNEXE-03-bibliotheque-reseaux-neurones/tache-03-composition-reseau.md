# TACHE-03 — Composition en réseau (`Network`) {#lot-annexe-03-tache-03-composition-reseau}

**Lot :** [LOT-ANNEXE-03](epic.md) · **Emplacement :** `Source/AiSolver/Nn` · **Statut :** fait

## Contexte
`Dense` (TACHE-01) et les activations (TACHE-02, plus `relu`/`tanhOp` de `LOT-ANNEXE-02`) existent
séparément ; cette tâche les compose en une séquence utilisable comme un seul objet — le réseau
complet que consommeront les algorithmes d'apprentissage (génération 2/3) et l'optimiseur
(`LOT-ANNEXE-04`).

## Travail à réaliser
- **`Source/AiSolver/Nn/Network.h`** : classe `aisolver::nn::Network`, non copiable.
  - Type `using ActivationFn = std::function<autodiff::NodePtr(const autodiff::NodePtr&)>;` —
    n'importe laquelle des fonctions d'activation existantes (`autodiff::relu`, `autodiff::tanhOp`,
    `nn::sigmoid`, `nn::softmax`) ou `nullptr` pour « aucune activation » (couche de sortie linéaire,
    cas fréquent en régression).
  - `void addLayer(std::unique_ptr<Dense> layer, ActivationFn activation = nullptr);` — ajoute une
    couche à la séquence ; `Network` **possède** ses couches (`std::vector<std::unique_ptr<Dense>>`
    en interne), cohérent avec RAII (pas de gestion manuelle de durée de vie côté appelant).
  - `autodiff::NodePtr forward(const autodiff::NodePtr& input);` — enchaîne
    `couche->forward(courant)` puis `activation(résultat)` si non nulle, pour chaque couche dans
    l'ordre d'ajout ; la sortie d'une couche devient l'entrée de la suivante.
  - `std::vector<autodiff::NodePtr> parameters() const;` — concatène `parameters()` (TACHE-01) de
    toutes les couches, dans l'ordre d'ajout — c'est cette liste que consommera l'optimiseur
    (`LOT-ANNEXE-04`).
  - `std::size_t layerCount() const;` — nombre de couches, utilisé par la sérialisation (TACHE-04)
    pour écrire/valider l'en-tête du fichier.

## Fichiers impactés
- `Source/AiSolver/Nn/Network.h` (nouveau).
- `Source/AiSolver/Nn/Network.cpp` (nouveau).
- `Source/AiSolver/CMakeLists.txt` (ajout de `Nn/Network.cpp`).
- `Source/Test/CMakeLists.txt` (ajout de `Unit/AiSolver/Nn/test_network.cpp`).
- `Source/Test/Unit/AiSolver/Nn/test_network.cpp` (nouveau).

## Tests (obligatoires)
- **Forward bout-en-bout** : un `Network` à trois couches (`Dense` + `relu`, `Dense` + `sigmoid`,
  `Dense` sans activation) transforme une entrée `[4,1]` en une sortie de la forme attendue par la
  dernière couche, valeurs cohérentes avec un calcul manuel sur un cas simple.
- **`parameters()` couvre toutes les couches** : le nombre de `NodePtr` renvoyés vaut `2 ×
  layerCount()` (poids + biais par couche, TACHE-01), sans doublon ni omission.
- **`backward()` de bout en bout** : `autodiff::backward()` sur une perte scalaire construite à
  partir de `Network::forward()` produit des gradients non nuls sur **chaque** paramètre de
  **chaque** couche, y compris la première (vérifie que la rétropropagation traverse correctement
  toute la profondeur du réseau, pas seulement la dernière couche).
- **Activation `nullptr` laisse la sortie linéaire** : une couche ajoutée sans activation ne subit
  aucune transformation après `Dense::forward()`.
- **Ordre des couches respecté** : deux réseaux construits avec les mêmes couches mais ajoutées dans
  un ordre différent produisent des sorties différentes sur la même entrée (garde-fou contre un
  stockage qui ignorerait l'ordre d'ajout, ex. structure non séquentielle par erreur).

## Points d'attention
- **`Network` ne connaît aucune fonction de perte** (cf. décision de cadrage de l'épic) : elle
  s'arrête à `forward()`, la perte et son calcul de gradient (`autodiff::backward()`) restent à la
  charge de l'appelant (génération 2/3).
- **`ActivationFn` est un `std::function`, donc une petite indirection à chaque appel** : jugé
  négligeable ici (pas d'objectif de performance pour la génération 0, cf. `LOT-ANNEXE-01`) ; à
  reconsidérer seulement si un profilage futur l'identifie comme un point chaud réel.
- **Une couche ajoutée via `addLayer` change de forme d'entrée attendue à la couche suivante** :
  aucune vérification de compatibilité des dimensions n'est faite à l'ajout (`addLayer` ne connaît
  pas la forme de sortie de la couche précédente indépendamment d'un appel à `forward()`) ; une
  incompatibilité se manifestera par l'assertion de `matmul` (`LOT-ANNEXE-01`) au premier
  `forward()` — comportement jugé suffisant, une vérification anticipée dupliquerait cette logique
  sans bénéfice pour un réseau construit par du code (pas par une configuration utilisateur externe
  à valider).

## Définition de fait (DoD)
- `Network` disponible et testé (`ctest` vert) pour la composition, l'accès aux paramètres et la
  rétropropagation de bout en bout ; build `/W4 /WX` sans avertissement ; Doxygen à jour.

## Notions abordées
@ref guide-annexe-reseaux-neurones — neurone, couche dense, fonctions d'activation, initialisation
des poids.

## Exigences
Contribue à `EX-IA-003` (déclarée dans [l'épic](epic.md)) ; aucune exigence propre à cette tâche.
