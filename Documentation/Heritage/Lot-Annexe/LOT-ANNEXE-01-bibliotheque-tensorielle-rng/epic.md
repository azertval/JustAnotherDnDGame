# LOT-ANNEXE-01 — Bibliothèque tensorielle et RNG maison {#lot-annexe-01}

> Statut : **fait**. Premier lot du programme [Lots annexes](@ref lots-annexe) — aucun
> prérequis. Pose la brique de calcul numérique (tenseurs, opérations, RNG déterministe) sur
> laquelle s'appuient tous les lots suivants, de l'autodiff (`LOT-ANNEXE-02`) à l'entraînement
> (générations 2 et 3).

## Objectif
`Source/AiSolver` n'existe pas encore : aucune brique de calcul numérique n'est disponible dans le
projet en dehors des types géométriques minces de `Core` (`core::Vector2`, `core::Rect`), impropres
à porter des matrices de poids ou des lots d'observations. Le programme d'IA maison a besoin de deux
choses avant toute chose : un conteneur numérique N-dimensionnel sur lequel exprimer matrices et
vecteurs (couches de réseau, observations, actions), et une source d'aléatoire **déterministe et
reproductible** (initialisation des poids, mutation évolutionniste en génération 2, échantillonnage
stochastique en génération 3) — sans quoi chaque lot ultérieur réimplémenterait sa propre variante,
au risque de résultats non reproductibles d'un lot à l'autre. Contrainte dure du programme : **aucun
framework de calcul ni bibliothèque tierce** (pas de LibTorch, pas de BLAS externe, pas de Python) —
tout est écrit à la main, en C++20 standard.

## Périmètre

### Inclus
- **RNG déterministe** (`aisolver::Rng`) : seed explicite obligatoire, `std::mt19937_64` comme
  moteur (bibliothèque standard C++, pas un framework de calcul numérique — autorisée malgré la
  contrainte « from scratch »), API `nextFloat()`/`nextFloat(min, max)`/`nextGaussian(mean,
  stddev)`/`nextInt(min, max)`, chacune dérivée **à la main** de la sortie brute du moteur
  (cf. décision de cadrage ci-dessous).
- **`Tensor<T>`** : conteneur N-D à forme et *stride* explicites, allocation contiguë, indexation
  multi-dimensionnelle et vues (reshape/sous-vues sans copie de données).
- **Opérations élémentaires** : addition, soustraction, multiplication, division composante à
  composante entre deux tenseurs de même forme, et avec un scalaire (diffusion scalaire uniquement).
- **Réductions** : somme, moyenne, maximum sur l'ensemble des éléments d'un tenseur (réduction
  globale, pas par axe).
- **Produit matriciel** (`matmul`) et **transposition**, restreints aux tenseurs de rang 2
  (matrices) — les seules formes réellement consommées par les couches denses de `LOT-ANNEXE-03`.
- Le nouveau module CMake `Source/AiSolver` (bibliothèque statique, sibling de `Core`/`HMI`/
  `Elements`/`Test`), sa cible de tests dédiée sous `Source/Test/Unit/AiSolver/Math`.

### Exclus (hors périmètre de ce lot)
- **Diffusion (broadcasting) générale façon NumPy** (ex. `[3,1] + [1,4] → [3,4]`) : seule la
  diffusion **scalaire** (tenseur ⊕ nombre) est couverte. Le besoin réel des lots suivants (couches
  denses de `LOT-ANNEXE-03`, traitées par lots de taille fixe) ne le demande pas ; l'ajouter
  maintenant ferait grossir `TensorOps` sans consommateur pour le valider.
- **Réductions par axe** (ex. somme le long de la dimension `batch`) : seule la réduction globale
  (scalaire) est couverte. `LOT-ANNEXE-03` (fonction de perte, softmax) précisera le besoin réel
  avant qu'une API par axe soit figée.
- **`matmul` par lots (*batched*) ou N-D** : seul le cas matrice × matrice (rang 2) est couvert. Le
  traitement par lot des observations (génération 1/2) empile des appels 2D plutôt que d'exiger une
  troisième dimension dans `matmul` lui-même.
- **Optimisation SIMD/vectorisation, multi-threading** : ce lot vise la **correction**, pas la
  vitesse — `EX-NFR-001` (60 img/s) ne s'applique pas à `AiSolver`, qui tourne hors boucle de rendu,
  y compris en entraînement hors ligne (génération 2/3). Une passe de performance reste possible
  plus tard sans changer l'API.
- **Sérialisation de `Tensor` sur disque** : le besoin réel (sauvegarder des poids entraînés) est
  couvert au niveau réseau, pas tenseur isolé, par `LOT-ANNEXE-03` (TACHE-04).
- **Autodiff (rétropropagation)** : `Tensor<T>` reste un conteneur de valeurs pur, sans graphe de
  calcul ni gradient — c'est tout l'objet de `LOT-ANNEXE-02`.

## Décisions de cadrage
- **Stockage contigu, ordre ligne (*row-major*, convention C), buffer partagé par pointeur
  intelligent.** Les vues (reshape, sous-vues) partagent le même tampon plutôt que de copier les
  données — nécessaire pour que les couches de `LOT-ANNEXE-03` puissent réinterpréter un tenseur
  (ex. aplatir un lot d'observations) sans coût de copie à chaque passe avant.
- **Le gabarit `Tensor<T>` est générique, mais seule l'instanciation `Tensor<float>` est testée et
  utilisée en pratique.** Le reste du programme (autodiff, réseaux) manipule exclusivement des
  `float` — cohérent avec la précision déjà en usage dans `Core` (`core::Vector2` en `float`).
  Instancier `Tensor<int>` reste possible (indices, compteurs) mais n'est pas un objectif de ce lot.
- **Les erreurs de forme (tailles incompatibles pour une opération élémentaire ou un `matmul`) sont
  des erreurs de programmation, pas des erreurs récupérables** : elles sont signalées par
  `JADG_ASSERT` (stripé en Release), pas par une exception ni un `std::optional` — même
  politique que les autres invariants internes de `Core` (`Documentation/Specification/conventions.md`,
  catégorie « erreur de programmation »). Une forme incompatible ne peut résulter que d'un bug dans
  le code appelant (réseau mal composé), jamais d'une entrée utilisateur.
- **Les lois de probabilité sont calculées à la main, pas déléguées à `std::…_distribution`.** La
  norme C++ spécifie la suite de valeurs produite par `std::mt19937_64` (bit à bit, quelle que soit
  l'implémentation), mais **pas** celle produite par `std::uniform_real_distribution`,
  `std::uniform_int_distribution` ou `std::normal_distribution` : deux bibliothèques standard
  peuvent légitimement en donner des suites différentes pour la même graine. Comme la
  reproductibilité à partir de la seule graine est la raison d'être de cette classe — et qu'elle est
  vérifiée en intégration continue sur des rejeux exportés (LOT-ANNEXE-20) —, `Rng` dérive
  lui-même ses tirages : mapping entier→`float` sur 24 bits, tirage entier par rejet,
  transformation de Box-Muller pour la loi normale. Coût : quelques lignes d'arithmétique de plus,
  toutes couvertes par les sources citées dans @ref guide-annexe-algebre-tensorielle.
- **`aisolver::Rng` est mono-thread, sans garantie de sécurité concurrente** : le programme
  d'entraînement (génération 2/3) reste séquentiel (`EX-ARCH-060`, boucle mono-thread déjà actée
  pour le jeu) ; paralléliser l'entraînement plus tard impliquerait une instance de `Rng` par
  thread avec des graines dérivées, pas un `Rng` partagé verrouillé.
- **`aisolver::Rng` est indépendant de tout état du jeu** : il ne réutilise ni ne remplace aucune
  source d'aléatoire de `Core`/`HMI` (il n'en existe aucune) ; ses graines sont fournies
  explicitement par l'appelant (jamais une graine par défaut basée sur l'horloge), pour que chaque
  run d'entraînement soit rejouable à l'identique à partir de sa seule graine.

## Notions abordées
Ce lot est entièrement pédagogique dans son thème : voir @ref guide-annexe-algebre-tensorielle
(tenseurs, stride, produit matriciel, réductions) et sa section 7 (générateurs pseudo-aléatoires
déterministes) pour le RNG. Sources directes des notions implémentées ici : Harris et al. (2020,
conventions forme/stride/vue) ; Matsumoto & Nishimura (1998, Mersenne Twister, `std::mt19937_64`) ;
Box & Muller (1958, tirage gaussien à partir de deux tirages uniformes) — bibliographie complète
dans le chapitre.

## Exigences couvertes
- Nouvelle, déclarée dans [la spécification IA](@ref spec-ia) : [`EX-IA-001`](@ref EX-IA-001).
- Réutilisées : `EX-NFR-010`/`EX-NFR-012`/`EX-NFR-013` (testabilité headless, conventions,
  `/W4 /WX`), `EX-NFR-020` (couverture par tests unitaires), `EX-ARCH-001` (sens de dépendance
  `HMI → Core`, étendu ici à `AiSolver → Core`, jamais l'inverse).

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-rng-deterministe.md) | RNG déterministe (`Rng`) | `Source/AiSolver/Math` | ✅ |
| [TACHE-02](tache-02-tensor-nd.md) | `Tensor<T>` : conteneur N-D | `Source/AiSolver/Math` | ✅ |
| [TACHE-03](tache-03-operations-reductions.md) | Opérations élémentaires et réductions | `Source/AiSolver/Math` | ✅ |
| [TACHE-04](tache-04-produit-matriciel-transposition.md) | Produit matriciel et transposition | `Source/AiSolver/Math` | ✅ |
| [TACHE-05](tache-05-tests.md) | Tests : formes incompatibles, cas limites, non-régression, reproductibilité du RNG | `Source/Test/Unit/AiSolver/Math` | ✅ |

## Critères d'acceptation du lot
1. `Source/AiSolver` existe comme bibliothèque CMake compilable indépendamment, liée uniquement à
   `Core` (jamais l'inverse), sans nouvelle dépendance tierce dans `External/CMakeLists.txt`.
2. `aisolver::Rng` produit la **même** séquence de valeurs pour une même graine, sur les quatre
   méthodes (`nextFloat`, `nextFloat(min,max)`, `nextGaussian`, `nextInt`), vérifié par test.
3. `Tensor<T>` construit, indexe et redimensionne (vues) correctement sur des cas connus (tenseur
   identité, produits croisés attendus), avec assertion sur toute forme incompatible.
4. Les opérations élémentaires, réductions, `matmul` et transposition produisent des résultats
   numériquement corrects sur des cas de référence calculés à la main (tolérance flottante `1e-5`).
5. Toute la logique nouvelle est **couverte par des tests** (`ctest` vert), déterministe, sans GPU.
   Build `/W4 /WX` sans avertissement, Doxygen et `scripts/lint_exigences.py` verts.

## Dépendances
Aucune (premier lot du programme annexe). Introduit le nouveau module `Source/AiSolver`, lié à
`Core` en lecture seule (`target_link_libraries(AiSolver PRIVATE Core project_warnings
project_options)`, jamais l'inverse — même sens de dépendance qu'`EX-ARCH-001` pour `HMI`), bien
que ce lot en particulier n'utilise encore aucun symbole de `Core` : la liaison est posée dès
maintenant pour que les lots suivants (génération 1, pont avec le jeu) n'aient pas à retoucher la
configuration CMake du module.

## Navigation des tâches
- @subpage lot-annexe-01-tache-01-rng-deterministe
- @subpage lot-annexe-01-tache-02-tensor-nd
- @subpage lot-annexe-01-tache-03-operations-reductions
- @subpage lot-annexe-01-tache-04-produit-matriciel-transposition
- @subpage lot-annexe-01-tache-05-tests
