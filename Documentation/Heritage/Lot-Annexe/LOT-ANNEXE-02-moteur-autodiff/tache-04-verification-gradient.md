# TACHE-04 — Vérification de gradient par différences finies {#lot-annexe-02-tache-04-verification-gradient}

**Lot :** [LOT-ANNEXE-02](epic.md) · **Emplacement :** `Source/Test/Unit/AiSolver/Math` · **Statut :** fait

## Contexte
`backward()` (TACHE-03) calcule des gradients **analytiques**, dérivés à la main pour chacune des
cinq opérations de TACHE-02. Rien ne garantit encore, de façon **générique** et **automatisable**,
que ces dérivées à la main sont correctes — une erreur de signe ou de facteur dans `Ops.cpp` pourrait
passer inaperçue si seuls des cas particuliers, calculés eux aussi à la main, étaient testés (risque
de reproduire la même erreur des deux côtés). Cette tâche ajoute un **oracle indépendant** : le
gradient **numérique**, obtenu par différences finies centrées, qui ne suppose aucune connaissance
de la formule de dérivation d'une opération donnée. Condition **bloquante** : toute opération
différentiable ajoutée après ce lot (`sigmoid`/`softmax` en `LOT-ANNEXE-03`) doit passer ce même
contrôle avant d'être utilisée par un réseau réel.

## Travail à réaliser
- **`Source/Test/Unit/AiSolver/Math/GradientCheck.h`** (en-tête utilitaire de test, **header-only**,
  inclus directement par les fichiers de test qui en ont besoin — pas de traduction unit séparée,
  donc pas d'entrée propre dans `Source/Test/CMakeLists.txt` au-delà du fichier de test lui-même) :
  - `struct GradientCheckResult { bool passed; float maxAbsoluteError; };`
  - `GradientCheckResult checkGradient(const std::function<autodiff::NodePtr(const
    std::vector<autodiff::NodePtr>&)>& buildGraph, const std::vector<Tensor<float>>& inputs, float
    epsilon = 1e-3f, float tolerance = 1e-2f);` — pour chaque tenseur d'entrée et chacun de ses
    éléments :
    1. Construit deux graphes via `buildGraph`, l'un avec l'élément perturbé de `+epsilon`, l'autre
       de `-epsilon` (les autres entrées inchangées), calcule le gradient **numérique** =
       `(sortiePerturbeePlus − sortiePerturbeeMoins) / (2 × epsilon)` (différence centrée, plus
       précise qu'une différence avant/arrière à ordre de grandeur d'`epsilon` égal).
    2. Construit le graphe **une fois** avec les entrées non perturbées, appelle `autodiff::
       backward()` dessus, lit le gradient **analytique** correspondant sur le `Node` d'entrée
       concerné.
    3. Compare les deux ; l'écart absolu maximal observé sur l'ensemble des éléments est renvoyé
       dans `maxAbsoluteError` ; `passed` est vrai si cet écart reste sous `tolerance`.
- **`Source/Test/Unit/AiSolver/Math/test_autodiff_gradient_checking.cpp`** : applique `checkGradient`
  à chacune des cinq opérations de TACHE-02 (`add`, `multiply`, `matmul`, `relu`, `tanhOp`), sur des
  entrées aléatoires générées via `aisolver::Rng` (`LOT-ANNEXE-01`) à graine fixe (reproductibilité
  du test lui-même).

## Fichiers impactés
- `Source/Test/Unit/AiSolver/Math/GradientCheck.h` (nouveau, header-only, test-only — pas ajouté aux
  sources de `Source/AiSolver/CMakeLists.txt`).
- `Source/Test/CMakeLists.txt` (ajout de `Unit/AiSolver/Math/test_autodiff_gradient_checking.cpp`).
- `Source/Test/Unit/AiSolver/Math/test_autodiff_gradient_checking.cpp` (nouveau).

## Tests (obligatoires)
- **`add`** : `checkGradient` passe pour l'opération d'addition sur des tenseurs `2×2` aléatoires.
- **`multiply`** : idem pour la multiplication élément par élément.
- **`matmul`** : idem pour le produit matriciel, sur des formes `[2,3]`/`[3,2]`.
- **`relu`** : idem, avec des entrées mêlant valeurs strictement positives et strictement négatives
  (éviter les valeurs trop proches de `0`, où le point de non-dérivabilité de `ReLU` rendrait la
  différence finie non représentative — documenté dans le test, pas un défaut de `checkGradient`).
- **`tanhOp`** : idem, sur une plage d'entrées modérée (éviter la saturation extrême où le gradient
  analytique **et** numérique tendent tous deux vers `0`, rendant la comparaison peu discriminante).
- **`checkGradient` détecte une régression volontaire** : test « négatif » — une règle de dérivation
  délibérément fausse (ex. `add` renvoyant `2×` le gradient de sortie au lieu de `1×`, câblée
  localement dans ce seul test via `unaryOp`/`binaryOp` directement, sans toucher à `Ops.cpp`) fait
  échouer `checkGradient` (`passed == false`) — garantit que l'outil détecte effectivement une
  erreur, pas seulement qu'il valide les cas déjà corrects.

## Points d'attention
- **`epsilon` et `tolerance` ont des valeurs par défaut choisies empiriquement** (`1e-3`/`1e-2`) :
  un `epsilon` trop petit fait dominer l'erreur d'arrondi flottant, trop grand fait dominer l'erreur
  de troncature de la différence finie elle-même — ces valeurs sont un point de départ, ajustables
  au cas par cas (ex. `LOT-ANNEXE-03` pourra resserrer la tolérance sur ses propres opérations si le
  besoin s'en fait sentir).
- **`checkGradient` reconstruit le graphe à chaque perturbation** (pas de mutation en place d'un
  graphe existant) : plus coûteux, mais élimine tout risque qu'un état résiduel d'un appel
  précédent (ex. gradient non remis à zéro) fausse la mesure — acceptable ici, cet outil ne tourne
  qu'en test, jamais en entraînement réel.
- **Ce fichier est le gabarit que `LOT-ANNEXE-03` réutilisera tel quel** pour `sigmoid`/`softmax` :
  toute évolution de `GradientCheck.h` après ce lot doit rester rétrocompatible avec cet usage, ou
  être coordonnée avec `LOT-ANNEXE-03`.

## Définition de fait (DoD)
- `GradientCheck.h` disponible et appliqué aux cinq opérations de TACHE-02, `ctest` vert (y compris
  le test négatif de détection de régression) ; build `/W4 /WX` sans avertissement ; Doxygen à jour ;
  critères d'acceptation de [l'épic](epic.md) vérifiés avant de marquer `LOT-ANNEXE-02` terminé.

## Notions abordées
@ref guide-annexe-autodiff (dérivée, règle de la chaîne, graphe de calcul, rétropropagation en mode
inverse), en particulier sa section 8 (vérification par différences finies centrées).

## Exigences
Contribue à `EX-IA-002` (déclarée dans [l'épic](epic.md)) ; aucune exigence propre à cette tâche.
