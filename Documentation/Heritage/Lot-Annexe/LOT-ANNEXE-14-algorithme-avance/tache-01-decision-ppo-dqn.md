# TACHE-01 — Décision de cadrage PPO vs DQN et implémentation de l'algorithme retenu {#lot-annexe-14-tache-01-decision-ppo-dqn}

**Lot :** [LOT-ANNEXE-14](epic.md) · **Emplacement :** `Source/AiSolver/Training/Advanced` ·
**Statut :** fait

## Contexte
C'est la tâche charnière du lot : contrairement aux tâches habituelles, elle **commence** par une
décision de cadrage — laquelle des deux familles d'algorithmes (PPO ou DQN) implémenter — avant de
pouvoir écrire la moindre ligne de code. Cette décision n'est **pas prise dans l'épic** : elle
appartient à l'ouverture de cette tâche, sur la base des résultats chiffrés de LOT-ANNEXE-13
(TACHE-04, comparaison REINFORCE vs acteur-critique). Les deux sous-sections ci-dessous décrivent
concrètement le travail attendu pour **chaque** option, afin que la décision, une fois prise, dispose
immédiatement d'un plan de travail détaillé sans nouvelle phase d'analyse.

**Avant d'écrire du code** : relire le résultat consigné dans
`Documentation/Lot-Annexe/LOT-ANNEXE-13-acteur-critique/epic.md` (critère d'acceptation 3) et
trancher explicitement selon les deux critères de la décision de cadrage de cet épic (instabilité
persistante → PPO ; convergence lente mais stable, espace d'action discret favorable → DQN).
Documenter le choix retenu et sa justification en tête de ce fichier (ou dans une sous-section
« Décision retenue ») avant de retirer la sous-section non choisie.

## Travail à réaliser (si PPO)
- **`aisolver::training::PpoTrainer`** (`Source/AiSolver/Training/Advanced/PpoTrainer.h/.cpp`) :
  étend la boucle d'acteur-critique de LOT-ANNEXE-13 — collecte un **batch** de plusieurs
  trajectoires (pas une seule par mise à jour, contrairement à LOT-ANNEXE-12/13) avant chaque mise à
  jour de politique.
- **Ratio de probabilité** : conserve, pour chaque pas du batch, la log-probabilité de l'action sous
  la politique **au moment de la collecte** (`π_ancienne`, valeur détachée, analogue au
  `logProbability` de `TrajectoryStep` en LOT-ANNEXE-12) ; recalcule à chaque époque d'optimisation
  la log-probabilité sous la politique **courante** (`π_nouvelle`, nœud de graphe d'autodiff) ; le
  ratio est `exp(log π_nouvelle − log π_ancienne)`.
- **`aisolver::training::computePpoClippedLoss`**
  (`Source/AiSolver/Training/Advanced/PpoLoss.h/.cpp`) : perte de politique
  `-min(ratio × avantage, clip(ratio, 1−ε_clip, 1+ε_clip) × avantage)` par pas, moyennée sur le
  batch, construite comme graphe d'autodiff. Les quatre opérations nécessaires existent déjà et
  n'ont pas à être réécrites : `expOp` (pour le ratio), `minimum`, `clamp` et `multiplyScalar` sont
  livrées par @ref lot-annexe-03-tache-05-operations-differentiables-complementaires, avec leur
  convention de sous-gradient documentée (`clamp` ne laisse passer aucun gradient sur un élément
  rogné — c'est exactement l'effet stabilisateur recherché par PPO, pas un effet de bord).
- **Plusieurs époques d'optimisation par batch** : la boucle réoptimise la politique (et le critique,
  réutilisé de LOT-ANNEXE-13 sans changement de nature) un nombre configurable de fois (ex. 3 à 10)
  sur le **même** batch de trajectoires collecté, avant de recollecter un nouveau batch — c'est la
  différence structurante avec LOT-ANNEXE-12/13 (une seule passe par trajectoire).
- Réutilise `CriticNetwork`, `computeAdvantages` et `computeCriticLoss` de LOT-ANNEXE-13 sans
  modification pour la partie critique.

## Travail à réaliser (si DQN)
- **`aisolver::nn::QNetwork`** (`Source/AiSolver/Training/Advanced/QNetwork.h/.cpp`) : réseau
  construit avec la bibliothèque de LOT-ANNEXE-03, une sortie par action de l'espace discret de
  LOT-ANNEXE-07 (`Q(s, ·)`, vecteur de valeurs d'action plutôt qu'une distribution de probabilité ou
  un scalaire unique).
- **`aisolver::training::ReplayBuffer`** (`Source/AiSolver/Training/Advanced/ReplayBuffer.h/.cpp`) :
  mémoire circulaire de transitions `(observation, action, récompense, observation_suivante,
  fin_d'épisode)`, capacité configurable, méthode d'ajout (`push`) et d'échantillonnage aléatoire
  d'un mini-lot (`sample`, via le générateur pseudo-aléatoire de LOT-ANNEXE-01).
- **Réseau cible** (`target network`) : copie de `QNetwork`, poids **gelés** entre deux
  synchronisations, mise à jour périodique (copie complète des poids du réseau principal, période
  configurable en nombre de pas ou d'épisodes) — sert à stabiliser la cible de l'erreur de Bellman
  (éviter qu'une cible qui bouge à chaque pas ne fasse diverger l'apprentissage).
- **Exploration `ε`-greedy** : avec probabilité `ε` (décroissante au fil de l'entraînement, borne
  configurable), action aléatoire uniforme sur l'espace d'action ; sinon, action de `Q` maximale
  (argmax) — remplace l'échantillonnage stochastique d'une distribution de politique utilisé par
  REINFORCE/acteur-critique/PPO.
- **`aisolver::training::computeDqnLoss`** (`Source/AiSolver/Training/Advanced/DqnLoss.h/.cpp`) :
  erreur quadratique entre `Q(s_t, a_t)` (réseau principal, nœud de graphe d'autodiff) et la cible de
  Bellman `reward_t + gamma × max_a Q_cible(s_{t+1}, a)` (réseau cible, valeur détachée du graphe —
  jamais rétropropagée), sur un mini-lot échantillonné du `ReplayBuffer`.
- **`aisolver::training::DqnTrainer`** (`Source/AiSolver/Training/Advanced/DqnTrainer.h/.cpp`) :
  boucle qui joue des pas sur `HeadlessLevelEnvironment` (action `ε`-greedy), pousse chaque
  transition dans le `ReplayBuffer`, tire un mini-lot et applique `computeDqnLoss` +
  `backward()` + `step()` (LOT-ANNEXE-04) à intervalle régulier, synchronise le réseau cible
  périodiquement.

## Fichiers impactés
- Selon l'option retenue, un sous-ensemble de : `Source/AiSolver/Training/Advanced/PpoTrainer.h/.cpp`,
  `PpoLoss.h/.cpp`, `QNetwork.h/.cpp`, `ReplayBuffer.h/.cpp`, `DqnLoss.h/.cpp`, `DqnTrainer.h/.cpp`.
- `Source/AiSolver/CMakeLists.txt` (nouveaux fichiers).
- Ce fichier lui-même : sous-section retenue conservée et complétée, sous-section non retenue
  supprimée une fois la décision actée, décision et justification documentées en tête de fichier.

## Tests (obligatoires)
- **Si PPO** : le ratio de probabilité est correctement borné après clipping (test direct sur des
  valeurs synthétiques de log-probabilités ancienne/nouvelle) ; gradient checking de
  `computePpoClippedLoss` ; plusieurs époques d'optimisation sur un même batch font effectivement
  varier les poids à chaque époque (pas une seule mise à jour déguisée).
- **Si DQN** : le `ReplayBuffer` respecte sa capacité (les transitions les plus anciennes sont
  évincées au-delà) et son échantillonnage couvre l'ensemble du buffer sur un grand nombre de tirages
  (pas de biais vers les transitions les plus récentes) ; le réseau cible reste rigoureusement figé
  entre deux synchronisations (ses poids ne changent pas alors que le réseau principal change) ;
  gradient checking de `computeDqnLoss` (uniquement par rapport aux poids du réseau **principal**, la
  cible étant détachée) ; `ε` décroît bien selon le calendrier configuré.

## Points d'attention
- **Ne pas implémenter les deux options** : trancher avant d'écrire du code de production, retirer
  la sous-section non retenue de ce fichier une fois la décision prise et justifiée.
- **Si DQN** : la cible de Bellman doit être **détachée du graphe d'autodiff** (comme l'avantage en
  LOT-ANNEXE-13) — le gradient ne doit jamais se propager à travers `max_a Q_cible`, sans quoi la
  perte deviendrait numériquement instable et sémantiquement incorrecte (la cible doit être traitée
  comme une constante à chaque mise à jour).
- **Si PPO** : un `clip_epsilon` mal calibré (trop petit) peut geler l'apprentissage (le ratio est
  presque toujours saturé) ; trop grand, il annule l'effet stabilisateur recherché — documenter la
  valeur retenue et la raison de son choix.
- Quel que soit le choix, l'espace d'action reste celui déjà défini par LOT-ANNEXE-07 : ce lot ne le
  modifie pas, il change uniquement la façon dont une politique/valeur est apprise sur cet espace.

## Définition de fait (DoD)
- Décision PPO/DQN documentée et justifiée par les résultats de LOT-ANNEXE-13 ; algorithme retenu
  implémenté et testé (`ctest` vert) ; build `/W4 /WX` sans avertissement ; Doxygen à jour.

## Notions abordées
@ref guide-annexe-ppo-dqn — PPO (limitation du pas par *clipping*) et DQN (valeur d'action, rejeu
d'expérience, réseau cible).

## Exigences
`EX-IA-015` (nouvelle, couverte par le lot).
