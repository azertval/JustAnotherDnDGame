# TACHE-06 — Tests : reproductibilité, stabilité numérique {#lot-annexe-03-tache-06-tests}

**Lot :** [LOT-ANNEXE-03](epic.md) · **Emplacement :** `Source/Test/Unit/AiSolver/Nn` · **Statut :** fait

## Contexte
TACHE-01 à 04 introduisent chacune leurs cas de test locaux (couche dense, activations,
composition, sérialisation) ; cette tâche consolide les vérifications transversales du réseau
complet — reproductibilité de bout en bout et absence de sorties aberrantes — condition
d'acceptation 3/4/5 de l'épic.

## Travail à réaliser
- **`Source/Test/Unit/AiSolver/Nn/test_network_reproducibility.cpp`** : construit deux
  `aisolver::nn::Network` identiques (mêmes poids, chargés depuis le même fichier sérialisé via
  TACHE-04), leur soumet la même entrée, vérifie une sortie **identique** bit à bit (tolérance
  nulle, pas seulement approchée — les deux réseaux effectuent exactement les mêmes opérations
  flottantes dans le même ordre).
- **`Source/Test/Unit/AiSolver/Nn/test_network_stability.cpp`** : soumet un `Network` composé de
  couches `Dense`+`relu`/`tanh`/`sigmoid`/`softmax` mêlées (TACHE-01/02/03) à des entrées
  d'amplitude extrême (valeurs très grandes, très petites, nulles) et vérifie l'absence de `NaN`/
  `inf` en sortie, sur chaque couche intermédiaire (pas seulement la sortie finale).
- **Test de bout en bout gradient + réseau** : un `Network` mêlant plusieurs couches et
  activations, une perte scalaire construite dessus, `autodiff::backward()` (`LOT-ANNEXE-02`)
  appliqué — vérifie que **tous** les paramètres du réseau (`Network::parameters()`) reçoivent un
  gradient non nul pour une entrée/perte non dégénérée (condition d'acceptation 1 de l'épic).

## Fichiers impactés
- `Source/Test/Unit/AiSolver/Nn/test_network_reproducibility.cpp` — nouveau.
- `Source/Test/Unit/AiSolver/Nn/test_network_stability.cpp` — nouveau.
- `Source/Test/CMakeLists.txt` — ajout des nouveaux fichiers à la cible `UnitTests`.

## Tests (obligatoires)
- **Reproductibilité stricte** : deux réseaux aux poids identiques (chargés depuis le même
  fichier) produisent une sortie bit-à-bit identique sur la même entrée.
- **Sauvegarde/rechargement** : un réseau sauvegardé (TACHE-04) puis rechargé produit une sortie
  identique à l'original, pour la même entrée (répété ici en test dédié, au-delà du test unitaire
  local de TACHE-04, pour couvrir un réseau à plusieurs couches mêlées).
- **Absence de `NaN`/`inf`** : sur des entrées d'amplitude extrême, aucune couche intermédiaire ni
  la sortie finale ne produit de valeur non finie.
- **Gradient non nul sur tous les paramètres** : après `backward()` sur une perte non dégénérée,
  chaque paramètre de `Network::parameters()` a un gradient accumulé différent de zéro.
- **`sigmoid`/`softmax` passent le gradient checking** (`LOT-ANNEXE-02`, `GradientCheck.h`) dans le
  contexte d'un `Network` complet, pas seulement isolées (déjà couvert localement par TACHE-02,
  revérifié ici en composition).

## Points d'attention
- **La reproductibilité stricte exige un ordre d'opérations flottantes rigoureusement identique**
  entre les deux réseaux comparés : construits depuis le même fichier sérialisé (TACHE-04), pas
  reconstruits indépendamment avec des poids « équivalents » calculés autrement — deux chemins de
  calcul différents pourraient légitimement diverger au dernier bit flottant sans que ce soit un
  bug.
- **Ces tests exercent le réseau en composition, pas chaque couche isolément** (déjà fait par
  TACHE-01/02/03) : leur valeur ajoutée est spécifiquement de vérifier que l'assemblage ne
  introduit pas de problème qui n'existerait pas au niveau d'une couche seule.

## Définition de fait (DoD)
- Les deux suites de tests vertes (`ctest`) ; build `/W4 /WX` sans avertissement ; Doxygen à jour ;
  `EX-IA-003` déclarée dans l'`epic.md` du lot.

## Notions abordées
@ref guide-annexe-reseaux-neurones — neurone, couche dense, fonctions d'activation, initialisation
des poids.

## Exigences
`EX-IA-003` (nouvelle, du même lot).
