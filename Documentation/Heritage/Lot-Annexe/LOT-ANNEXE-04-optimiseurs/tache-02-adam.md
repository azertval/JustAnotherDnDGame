# TACHE-02 — Adam (moments d'ordre 1/2, correction de biais) {#lot-annexe-04-tache-02-adam}

**Lot :** [LOT-ANNEXE-04](epic.md) · **Emplacement :** `Source/AiSolver/Optim` · **Statut :** à faire

## Contexte
SGD (TACHE-01) exige un taux d'apprentissage bien calibré manuellement — trop grand, il diverge ;
trop petit, il stagne. Adam maintient des moyennes mobiles du gradient (premier moment) et de son
carré (second moment), normalisant la mise à jour par l'amplitude récente du gradient : plus robuste
au choix du taux d'apprentissage, au prix d'un état interne plus riche par paramètre. Choix retenu
comme second optimiseur (`LOT-ANNEXE-04`, épic) pour couvrir les cas où SGD peine à converger,
notamment attendu pour les algorithmes de génération 3 (`LOT-ANNEXE-12`/`13`/`14`).

## Travail à réaliser
- **`aisolver::optim::Adam`** (`Source/AiSolver/Optim/Adam.h/.cpp`) : constructeur `Adam(float
  learningRate = 0.001f, float beta1 = 0.9f, float beta2 = 0.999f, float epsilon = 1e-8f)`.
- État interne par paramètre (table indexée par identité, même mécanisme que `Sgd` momentum,
  TACHE-01) : `m` (moyenne mobile du gradient), `v` (moyenne mobile du carré du gradient),
  compteur de pas global `_stepCount` (partagé par tous les paramètres, incrémenté une fois par
  appel à `step`, pas par paramètre).
- **`step`** : pour chaque paramètre, `m = beta1 * m + (1 - beta1) * grad` ; `v = beta2 * v + (1 -
  beta2) * grad²` (carré élément par élément, via `LOT-ANNEXE-01`) ; correction de biais `mHat = m /
  (1 - beta1^stepCount)`, `vHat = v / (1 - beta2^stepCount)` ; mise à jour `value -= learningRate *
  mHat / (sqrt(vHat) + epsilon)` (toutes opérations élément par élément).
- **`zeroGrad`** : réutilise directement la fonction libre de TACHE-01 (`OptimizerUtils.h`), pas de
  réimplémentation.

## Fichiers impactés
- `Source/AiSolver/Optim/Adam.h/.cpp` — nouveau.
- `Source/AiSolver/CMakeLists.txt` — ajout des nouveaux fichiers.

## Tests (obligatoires)
- **Convergence sur une régression polynomiale connue** : `Adam` atteint une tolérance documentée
  en un nombre d'itérations donné, avec un taux d'apprentissage pour lequel `Sgd` sans inertie
  (TACHE-01) diverge ou stagne sur le même cas — démontre concrètement l'intérêt pratique d'Adam,
  pas seulement sa formule.
- **Correction de biais** : sur le tout premier `step` (compteur = 1), `mHat`/`vHat` correspondent
  à la formule corrigée (pas simplement `m`/`v` bruts), vérifié par calcul de référence.
- **Compteur de pas partagé** : deux paramètres distincts optimisés par la **même** instance
  d'`Adam` partagent le même `_stepCount` (incrémenté une fois par `step`, pas une fois par
  paramètre à l'intérieur d'un même appel).
- **Absence de NaN/inf** : sur un gradient nul persistant (paramètre déjà à l'optimum), `Adam` ne
  produit ni `NaN` ni `inf` malgré la division par `sqrt(vHat) + epsilon` (le terme `epsilon`
  évite la division par une valeur nulle).

## Points d'attention
- **`epsilon` dans le dénominateur n'est pas un détail cosmétique** : sans lui, un paramètre dont
  le gradient est resté nul depuis le début (donc `vHat == 0`) produirait une division par zéro dès
  le premier pas où son gradient redevient non nul avec une magnitude infime.
- **Le compteur de pas est un état de l'optimiseur, pas du paramètre** : une même instance d'`Adam`
  appliquée à plusieurs réseaux distincts partagerait un compteur unique, ce qui serait incorrect —
  chaque réseau optimisé doit avoir sa propre instance d'`Adam` (documenté explicitement, pas
  imposé par le type lui-même).

## Définition de fait (DoD)
- `Adam` disponible et testé (`ctest` vert), convergence démontrée sur un cas où SGD peine ; build
  `/W4 /WX` sans avertissement ; Doxygen à jour.

## Notions abordées
@ref guide-annexe-optimisation — descente de gradient, taux d'apprentissage, inertie, Adam.

## Exigences
`EX-IA-004` (nouvelle, partagée avec TACHE-01/03 du même lot).
