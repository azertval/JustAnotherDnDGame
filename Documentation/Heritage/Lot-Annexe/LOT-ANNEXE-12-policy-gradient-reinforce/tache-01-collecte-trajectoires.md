# TACHE-01 — Collecte de trajectoires sur un épisode complet {#lot-annexe-12-tache-01-collecte-trajectoires}

**Lot :** [LOT-ANNEXE-12](epic.md) · **Emplacement :** `Source/AiSolver/Training/PolicyGradient` ·
**Statut :** fait

## Contexte
`HeadlessLevelEnvironment` (LOT-ANNEXE-05) expose déjà un cycle observation → action → pas de
simulation → récompense/fin d'épisode, sur un niveau chargé sans fenêtre ni GPU. La bibliothèque de
réseaux (LOT-ANNEXE-03) permet de construire un réseau de politique qui transforme une observation
encodée (LOT-ANNEXE-06) en distribution sur l'espace d'action (LOT-ANNEXE-07). Ce qui manque encore :
la boucle qui, à partir de ces briques, **rejoue un épisode complet** et **conserve** tout ce dont
REINFORCE aura besoin ensuite (TACHE-02 et TACHE-03) — pas seulement la récompense, mais aussi la
log-probabilité de l'action effectivement choisie, indispensable à la perte.

## Travail à réaliser
- **`aisolver::training::TrajectoryStep`** (`Source/AiSolver/Training/PolicyGradient/Trajectory.h`) :
  agrégat simple — observation encodée au pas (référence ou copie légère selon le format de
  LOT-ANNEXE-06), indice de l'action échantillonnée, `float logProbability` (log-probabilité de
  cette action sous la distribution produite par la politique **au moment** de l'échantillonnage),
  `float reward` (récompense immédiate renvoyée par l'environnement pour ce pas).
- **`aisolver::training::Trajectory`** (même fichier) : `std::vector<TrajectoryStep> steps;` plus un
  indicateur de fin d'épisode (`EpisodeOutcome` réutilisé de LOT-ANNEXE-08 : victoire, échec,
  timeout).
- **Le « réseau de politique » n'est pas un type nouveau** : c'est un `aisolver::nn::Network`
  (LOT-ANNEXE-03, TACHE-03) dont la couche de sortie a `outputSize` égal au nombre d'actions de
  l'espace discret (LOT-ANNEXE-07) et pour activation `nn::softmax` (LOT-ANNEXE-03, TACHE-02) — sa
  sortie est donc déjà une distribution de probabilité sur les actions. Aucune classe
  `PolicyNetwork` dédiée n'est introduite : la composition suffit, et le critique de LOT-ANNEXE-13
  réutilise la même bibliothèque avec une autre configuration.
- **`aisolver::training::TrajectoryCollector`**
  (`Source/AiSolver/Training/PolicyGradient/TrajectoryCollector.h/.cpp`) : méthode
  `Trajectory collectEpisode(HeadlessLevelEnvironment& env, nn::Network& policy, aisolver::Rng& rng)` —
  boucle jusqu'à fin d'épisode : encode l'observation courante, passe **avant** (forward) dans le
  réseau de politique pour obtenir la distribution sur l'espace d'action, **échantillonne** une
  action selon cette distribution avec le générateur pseudo-aléatoire de LOT-ANNEXE-01 (jamais
  argmax pendant la collecte, cf. décision de cadrage de l'épic), conserve la log-probabilité de
  l'action tirée, applique l'action sur l'environnement, ajoute un `TrajectoryStep`.
- Le passage avant qui produit la distribution reste un graphe du moteur d'autodiff (LOT-ANNEXE-02) :
  `logProbability` stocké dans `TrajectoryStep` est la **valeur** de ce nœud à l'instant du tirage,
  mais le graphe lui-même (pour la rétropropagation ultérieure) est reconstruit à l'identique en
  TACHE-03 à partir des observations et actions enregistrées — la trajectoire ne retient que des
  données simples (valeurs), pas des nœuds de graphe vivants, pour rester un objet trivialement
  copiable/sérialisable.

## Fichiers impactés
- `Source/AiSolver/Training/PolicyGradient/Trajectory.h` (nouveau).
- `Source/AiSolver/Training/PolicyGradient/TrajectoryCollector.h` (nouveau).
- `Source/AiSolver/Training/PolicyGradient/TrajectoryCollector.cpp` (nouveau).
- `Source/AiSolver/CMakeLists.txt` (nouveaux fichiers dans la cible du module).

## Tests (obligatoires)
- **Longueur de trajectoire cohérente avec l'issue** : un épisode se terminant par timeout produit
  une trajectoire de longueur égale au nombre maximal de pas de LOT-ANNEXE-08 ; un épisode gagné ou
  perdu avant le timeout produit une trajectoire plus courte, terminée au pas exact de fin.
- **Log-probabilités valides** : chaque `logProbability` stocké est `≤ 0` (log d'une probabilité
  dans `]0, 1]`) et correspond bien à l'action enregistrée dans le même `TrajectoryStep` (recalcul
  indépendant à partir de la distribution et comparaison à la tolérance flottante usuelle).
- **Déterminisme à graine fixée** : deux collectes avec le même `aisolver::Rng` initialisé à la
  même graine et le même réseau de politique produisent des trajectoires **identiques** pas à pas.
- **Couverture de l'espace d'action** : sur un grand nombre d'épisodes avec une politique proche de
  l'uniforme (poids initiaux), toutes les actions de l'espace (LOT-ANNEXE-07) sont échantillonnées
  au moins une fois — vérifie que l'échantillonnage n'est pas dégénéré (toujours la même action).

## Points d'attention
- **Ne jamais échantillonner par argmax pendant la collecte d'entraînement** : REINFORCE a besoin
  d'exploration, et un argmax donnerait un gradient de log-probabilité nul en pratique dès que la
  politique devient un peu confiante — piège documenté dans la décision de cadrage de l'épic.
- La **graine du générateur** doit être injectée depuis l'appelant (jamais générée en interne par
  `TrajectoryCollector`), pour que la boucle d'entraînement (TACHE-04) contrôle la reproductibilité
  d'un run complet, pas seulement d'un épisode isolé.
- `TrajectoryStep` reste des **données pures** (`EX-ARCH-011`) : aucune logique de calcul de retour
  ou de perte ne doit s'y glisser, c'est le rôle de TACHE-02/TACHE-03.

## Définition de fait (DoD)
- `Trajectory`, `TrajectoryStep` et `TrajectoryCollector` disponibles et testés (`ctest` vert) ;
  build `/W4 /WX` sans avertissement ; Doxygen à jour.

## Notions abordées
@ref guide-annexe-reinforce — policy gradient, trajectoire, retour actualisé, algorithme REINFORCE.

## Exigences
`EX-IA-013` (nouvelle, couverte par le lot).
