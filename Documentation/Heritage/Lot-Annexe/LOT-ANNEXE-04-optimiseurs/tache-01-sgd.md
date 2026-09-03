# TACHE-01 — SGD (avec/sans inertie) {#lot-annexe-04-tache-01-sgd}

**Lot :** [LOT-ANNEXE-04](epic.md) · **Emplacement :** `Source/AiSolver/Optim` · **Statut :** à faire

## Contexte
`LOT-ANNEXE-02` accumule un gradient sur chaque `autodiff::Node` utilisé comme paramètre
(`Node::grad()`) ; rien n'existe encore pour transformer ce gradient en mise à jour de poids. SGD
(descente de gradient stochastique) est la règle la plus simple : elle sert de référence de
calibration pour tout optimiseur suivant (`Adam`, TACHE-02) et de premier consommateur réel des
paramètres exposés par `nn::Network` (`LOT-ANNEXE-03`).

## Travail à réaliser
- **`aisolver::optim::IOptimizer`** (`Source/AiSolver/Optim/IOptimizer.h`) : interface abstraite,
  `virtual void step(const std::vector<autodiff::NodePtr>& parameters) = 0;`, `virtual void
  zeroGrad(const std::vector<autodiff::NodePtr>& parameters) = 0;` (implémentation commune non
  virtuelle possible en fonction libre, réutilisée par `Sgd` et `Adam`).
- **`aisolver::optim::Sgd`** (`Source/AiSolver/Optim/Sgd.h/.cpp`) : constructeur `Sgd(float
  learningRate, float momentum = 0.0f)`. `step` : pour chaque paramètre, si `momentum == 0.0f`,
  `value -= learningRate * grad` directement ; sinon, maintient une table `velocity` indexée par
  identité de paramètre (`std::unordered_map<const void*, Tensor<float>>` sur l'adresse du tampon
  de données du nœud, ou tout identifiant stable équivalent) : `velocity = momentum * velocity -
  learningRate * grad ; value += velocity`.
- **`zeroGrad`** (fonction libre partagée, `Source/AiSolver/Optim/OptimizerUtils.h/.cpp`) : parcourt
  les paramètres, remet chaque `Tensor` de gradient à zéro (réutilisée telle quelle par `Adam`,
  TACHE-02, pas dupliquée).

## Fichiers impactés
- `Source/AiSolver/Optim/IOptimizer.h` — nouveau.
- `Source/AiSolver/Optim/Sgd.h/.cpp` — nouveau.
- `Source/AiSolver/Optim/OptimizerUtils.h/.cpp` — nouveau (`zeroGrad` partagé).
- `Source/AiSolver/CMakeLists.txt` — ajout des nouveaux fichiers à la cible `AiSolver`.

## Tests (obligatoires)
- **Mise à jour sans inertie** : un paramètre scalaire de gradient connu, après un `step`, a la
  valeur attendue exactement (`value - learningRate * grad`).
- **Mise à jour avec inertie** : sur deux `step` consécutifs à gradient constant, la vitesse
  s'accumule selon la formule documentée (vérifié par calcul de référence, pas seulement par
  convergence globale).
- **`zeroGrad`** : après appel, `Node::grad()` est le tenseur nul pour tous les paramètres fournis,
  sans effet sur `Node::value()`.
- **Non-effet sur les paramètres non fournis** : `step`/`zeroGrad` appelés sur un sous-ensemble de
  paramètres d'un réseau ne modifient pas les paramètres exclus de l'appel.

## Points d'attention
- **L'identité de paramètre utilisée pour indexer la table de vitesse doit rester stable sur toute
  la durée de l'entraînement.** Un réseau (`LOT-ANNEXE-03`) crée ses paramètres une fois à la
  construction et ne les recrée jamais : l'adresse du tampon de données convient, à condition que
  `nn::Dense` ne réalloue jamais ses `_weights`/`_bias` après construction (vérifié par lecture du
  code de `LOT-ANNEXE-03`, pas reconstruit ici).
- **`momentum == 0.0f` emprunte un chemin sans table de vitesse**, pour que le cas le plus simple
  (SGD pur) n'accumule aucun état inutile — pas une optimisation prématurée, mais une simplicité de
  raisonnement pour le cas de référence utilisé en calibration.

## Définition de fait (DoD)
- `Sgd` (avec et sans inertie) et `zeroGrad` disponibles et testés (`ctest` vert) ; build `/W4 /WX`
  sans avertissement ; Doxygen à jour.

## Notions abordées
@ref guide-annexe-optimisation — descente de gradient, taux d'apprentissage, inertie, Adam.

## Exigences
`EX-IA-004` (nouvelle, partagée avec TACHE-02/03 du même lot).
