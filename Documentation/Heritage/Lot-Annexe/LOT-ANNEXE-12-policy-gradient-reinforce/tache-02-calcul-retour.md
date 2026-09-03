# TACHE-02 — Calcul du retour actualisé par pas {#lot-annexe-12-tache-02-calcul-retour}

**Lot :** [LOT-ANNEXE-12](epic.md) · **Emplacement :** `Source/AiSolver/Training/PolicyGradient` ·
**Statut :** fait

## Contexte
Une `Trajectory` collectée (TACHE-01) ne porte, pour chaque pas, que la récompense **immédiate**.
REINFORCE a besoin, pour chaque pas `t`, du **retour** `G_t` — la somme actualisée des récompenses
**à partir de** `t` jusqu'à la fin de l'épisode — puisque c'est ce signal qui pondère le gradient de
log-probabilité de l'action prise à `t`. Le calcul est un simple parcours **arrière** de la
trajectoire, rendu possible par le fait qu'un épisode se termine toujours (LOT-ANNEXE-08 garantit un
critère de fin, y compris le timeout).

## Travail à réaliser
- **`aisolver::training::computeReturns`**
  (`Source/AiSolver/Training/PolicyGradient/ReturnCalculator.h/.cpp`) : fonction pure
  `std::vector<float> computeReturns(const Trajectory& trajectory, float gamma)` — parcourt les pas
  de la trajectoire **de la fin vers le début**, accumule `G_t = reward_t + gamma × G_{t+1}` (avec
  `G_T = reward_T` au dernier pas), renvoie un vecteur de même longueur que `trajectory.steps`,
  indexé dans le même ordre (chronologique, pas inversé).
- `gamma` est un paramètre explicite de la fonction (pas une constante globale) : la boucle
  d'entraînement (TACHE-04) le lit depuis sa configuration et le transmet à chaque appel.
- Validation légère en entrée : `gamma` attendu dans `[0, 1]` — assertion de développement
  (`assert`), pas de lancer d'exception (donnée de configuration, cf. gestion d'erreurs des
  conventions du projet).

## Fichiers impactés
- `Source/AiSolver/Training/PolicyGradient/ReturnCalculator.h` (nouveau).
- `Source/AiSolver/Training/PolicyGradient/ReturnCalculator.cpp` (nouveau).
- `Source/AiSolver/CMakeLists.txt` (nouveaux fichiers).

## Tests (obligatoires)
- **`gamma = 1`** (pas d'actualisation) : le retour au premier pas égale la somme brute de toutes
  les récompenses de l'épisode ; le retour au dernier pas égale sa propre récompense.
- **`gamma = 0`** (myope) : le retour à chaque pas égale exactement sa récompense immédiate,
  indépendamment des pas suivants.
- **`gamma` intermédiaire (ex. `0.9`)** : vérification numérique directe de la récurrence sur une
  trajectoire de récompenses connues à la main (3-4 pas), comparaison à la tolérance flottante
  usuelle.
- **Trajectoire d'un seul pas** : `G_0 = reward_0`, aucune récursion à tort.
- **Monotonie de la longueur** : le vecteur renvoyé a exactement la même taille que
  `trajectory.steps`, quelle que soit la longueur de l'épisode (victoire rapide, timeout).

## Points d'attention
- **Parcours arrière obligatoire** : calculer `G_t` par une somme directe des récompenses futures à
  chaque pas (double boucle) serait correct mais quadratique en longueur d'épisode ; la récurrence
  arrière est linéaire et doit être l'implémentation retenue, pas une optimisation ultérieure.
- **Aucune normalisation ici** (centrage/réduction) : décision de cadrage de l'épic — ce lot met à
  jour la politique épisode par épisode, sans population de retours sur laquelle normaliser.
- Le retour est une **fonction pure** sur des données déjà collectées : ne dépend d'aucun état de
  l'environnement ni du réseau — testable entièrement en isolation, sans `HeadlessLevelEnvironment`.

## Définition de fait (DoD)
- `computeReturns` disponible et testée (`ctest` vert) ; build `/W4 /WX` sans avertissement ;
  Doxygen à jour.

## Notions abordées
@ref guide-annexe-reinforce — policy gradient, trajectoire, retour actualisé, algorithme REINFORCE.

## Exigences
`EX-IA-013` (nouvelle, couverte par le lot).
