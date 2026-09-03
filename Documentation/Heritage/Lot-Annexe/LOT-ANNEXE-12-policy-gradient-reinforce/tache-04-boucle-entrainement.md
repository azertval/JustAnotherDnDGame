# TACHE-04 — Boucle d'entraînement par épisodes et journalisation {#lot-annexe-12-tache-04-boucle-entrainement}

**Lot :** [LOT-ANNEXE-12](epic.md) · **Emplacement :** `Source/AiSolver/Training/PolicyGradient` ·
**Statut :** fait

## Contexte
TACHE-01 à TACHE-03 fournissent les briques (collecte, retour, perte) ; il manque l'assemblage qui
les enchaîne épisode après épisode, applique effectivement les mises à jour de poids via un
optimiseur de LOT-ANNEXE-04, et journalise la progression via `TrainingStatsRecorder`
(LOT-ANNEXE-09) — sans quoi rien n'est observable ni comparable à l'évolutionniste de LOT-ANNEXE-10.

## Travail à réaliser
- **`aisolver::training::ReinforceTrainer`**
  (`Source/AiSolver/Training/PolicyGradient/ReinforceTrainer.h/.cpp`) : construit à partir d'un
  niveau chargé (`HeadlessLevelEnvironment`, LOT-ANNEXE-05), d'un réseau de politique
  (LOT-ANNEXE-03), d'un optimiseur (`optim::Sgd` ou `optim::Adam`, LOT-ANNEXE-04), d'une
  configuration (`gamma`, nombre d'épisodes, graine initiale) et d'un `TrainingStatsRecorder`
  (LOT-ANNEXE-09).
- Méthode `void run(std::size_t episodeCount)` : pour chaque épisode — `TrajectoryCollector::collectEpisode`
  (TACHE-01), `computeReturns` (TACHE-02), `computeReinforceLoss` (TACHE-03), `loss.backward()`
  (LOT-ANNEXE-02), `optimizer.step()` (LOT-ANNEXE-04) sur les paramètres du réseau de politique,
  remise à zéro des gradients accumulés avant l'épisode suivant, puis enregistrement d'une ligne de
  statistiques (numéro d'épisode, récompense totale de l'épisode, longueur de l'épisode, issue —
  victoire/échec/timeout, valeur de la perte).
- La graine du générateur pseudo-aléatoire est **dérivée déterministiquement** de la graine
  initiale et du numéro d'épisode (ex. `seed_t = seedBase + episodeIndex`), pour qu'un run complet
  soit intégralement reproductible d'un bout à l'autre à graine de base fixée.
- Un seul niveau à la fois : `ReinforceTrainer` prend un `HeadlessLevelEnvironment` déjà construit
  pour un niveau donné, aucune notion de changement de niveau en cours de run (cohérent avec le
  régime niveau-par-niveau de tout le programme).

## Fichiers impactés
- `Source/AiSolver/Training/PolicyGradient/ReinforceTrainer.h` (nouveau).
- `Source/AiSolver/Training/PolicyGradient/ReinforceTrainer.cpp` (nouveau).
- `Source/AiSolver/CMakeLists.txt` (nouveaux fichiers).

## Tests (obligatoires)
- **Non-régression de progression sur un niveau de contrôle** : sur un niveau simple et
  déterministe (peu de tuiles, chemin direct vers la sortie), la récompense **moyenne glissante**
  sur les derniers épisodes d'un run est significativement supérieure à celle des premiers épisodes
  — signe d'apprentissage effectif, pas seulement d'absence de crash.
- **CSV bien formé** : après un run court, le fichier produit par `TrainingStatsRecorder` contient
  exactement `episodeCount` lignes, colonnes cohérentes avec les runs des autres lots
  (LOT-ANNEXE-09, LOT-ANNEXE-10).
- **Reproductibilité intégrale** : deux runs avec la même graine de base et la même configuration
  produisent des CSV **identiques** ligne à ligne.
- **Remise à zéro des gradients entre épisodes** : le gradient accumulé sur les poids avant le
  calcul de la perte d'un nouvel épisode est nul (pas de fuite d'un épisode à l'autre).
- **Robustesse à un épisode de longueur nulle** (cas limite : échec au tout premier pas) : la boucle
  ne plante pas et journalise correctement une trajectoire d'un seul pas.

## Points d'attention
- **Une seule passe de politique par trajectoire collectée** (pas plusieurs époques d'optimisation
  sur le même lot de données) : c'est la différence structurante avec PPO (LOT-ANNEXE-14), qui
  réoptimise plusieurs fois sur le même batch — introduire ça ici anticiperait sur ce lot.
- **Ordre des opérations par épisode strict** : collecte (poids figés pendant tout l'épisode) →
  retour → perte (poids **au moment du calcul de perte**, identiques à ceux de la collecte puisqu'
  aucune mise à jour n'intervient entre les deux) → `backward()` → `step()`. Mettre à jour les poids
  **avant** la fin de la collecte casserait la cohérence entre l'action jouée et le gradient qui la
  concerne.
- Le format des lignes CSV doit rester **compatible** avec celui déjà utilisé par LOT-ANNEXE-09/10,
  pour que la comparaison chiffrée envisagée en LOT-ANNEXE-13 (TACHE-04) puisse superposer les
  courbes sans transformation de données.

## Définition de fait (DoD)
- `ReinforceTrainer` disponible et testé (`ctest` vert), progression observée sur le niveau de
  contrôle ; build `/W4 /WX` sans avertissement ; Doxygen à jour ; `EX-IA-013` déclarée dans
  l'épic.

## Notions abordées
@ref guide-annexe-reinforce — policy gradient, trajectoire, retour actualisé, algorithme REINFORCE.

## Exigences
`EX-IA-013` (nouvelle).
