# TACHE-02 — Intégration au harnais d'entraînement existant {#lot-annexe-14-tache-02-integration-harnais}

**Lot :** [LOT-ANNEXE-14](epic.md) · **Emplacement :** `Source/AiSolver/Training/Advanced` ·
**Statut :** fait

## Contexte
TACHE-01 produit un algorithme entraînable (`PpoTrainer` ou `DqnTrainer`) mais potentiellement
autonome, avec sa propre boucle de collecte. Cette tâche s'assure qu'il s'intègre au **même** harnais
que tous les lots précédents de la génération 3 : `HeadlessLevelEnvironment` (LOT-ANNEXE-05) comme
seule source d'observations/récompenses, `TrainingStatsRecorder` (LOT-ANNEXE-09) comme seul canal de
journalisation — sans court-circuit ni format de sortie parallèle qui rendrait la comparaison de
TACHE-03 plus difficile qu'elle ne devrait l'être.

## Travail à réaliser
- Vérifier/adapter que le trainer retenu (TACHE-01) consomme `HeadlessLevelEnvironment` exactement
  comme `TrajectoryCollector` (LOT-ANNEXE-12) ou la boucle par pas de DQN — un seul niveau chargé à
  la fois, aucune notion de changement de niveau en cours de run (régime niveau-par-niveau, décision
  transverse à tout le programme).
- **Journalisation** : chaque épisode (PPO) ou chaque intervalle de pas significatif (DQN, où la
  notion d'épisode existe aussi mais les mises à jour sont plus fréquentes que la fin d'épisode)
  produit une ligne exploitable par `TrainingStatsRecorder`, avec des colonnes cohérentes avec
  celles déjà utilisées par LOT-ANNEXE-09/12/13 (numéro d'épisode, récompense, longueur, issue) plus,
  le cas échéant, des colonnes spécifiques à l'algorithme (perte clippée moyenne pour PPO ; taille du
  `ReplayBuffer`, valeur courante de `ε` pour DQN).
- **Configuration** : les hyperparamètres propres à l'algorithme retenu (`clip_epsilon` et nombre
  d'époques par batch pour PPO ; capacité du `ReplayBuffer`, période de synchronisation du réseau
  cible et calendrier de décroissance de `ε` pour DQN) sont exposés par une structure de
  configuration dédiée, du même esprit que celle de `ReinforceTrainer`/`ActorCriticTrainer`
  (LOT-ANNEXE-12/13), pas des constantes codées en dur dans le trainer.
- **Reproductibilité** : comme pour LOT-ANNEXE-12/13, la graine du générateur pseudo-aléatoire est un
  paramètre explicite de configuration, dérivée déterministiquement pour chaque épisode/mini-lot, de
  sorte qu'un run complet soit intégralement reproductible à graine de base fixée.

## Fichiers impactés
- Le ou les fichiers de trainer de TACHE-01 (`PpoTrainer.h/.cpp` ou `DqnTrainer.h/.cpp`), complétés
  pour la journalisation et la configuration.
- Éventuel fichier de configuration dédié (`Source/AiSolver/Training/Advanced/AdvancedTrainerConfig.h`,
  nouveau) si non déjà couvert par TACHE-01.

## Tests (obligatoires)
- **CSV bien formé** : après un run court, le fichier produit par `TrainingStatsRecorder` contient un
  nombre de lignes cohérent avec la configuration du run, colonnes compatibles avec le format déjà
  utilisé par les lots amont de la génération 3 (colonnes communes identiques, colonnes spécifiques
  additionnelles).
- **Reproductibilité intégrale** : deux runs avec la même graine de base et la même configuration
  produisent des CSV identiques ligne à ligne.
- **Respect strict du régime niveau-par-niveau** : le trainer n'accepte qu'un seul
  `HeadlessLevelEnvironment` construit pour un seul niveau à la fois ; aucune API ne permet de lui en
  substituer un autre en cours de run.
- **Configuration effective** : faire varier un hyperparamètre (ex. `clip_epsilon` ou capacité du
  `ReplayBuffer`) via la structure de configuration change effectivement le comportement observé du
  trainer (pas une valeur ignorée en pratique).

## Points d'attention
- **Ne pas dupliquer `TrainingStatsRecorder`** ni introduire un format de journal parallèle : toute
  colonne supplémentaire propre à l'algorithme s'ajoute au schéma existant, elle ne le remplace pas —
  condition nécessaire pour que TACHE-03 puisse comparer les runs sans transformation de données.
- **DQN n'a pas de notion d'« épisode collecté puis rejoué » comme PPO/REINFORCE/acteur-critique** :
  la journalisation doit néanmoins rester alignée sur la fin de chaque épisode de jeu (victoire,
  échec, timeout selon LOT-ANNEXE-08), même si les mises à jour de poids surviennent à une fréquence
  différente (par mini-lot, pas par épisode) — bien distinguer, dans le code et les commentaires, «
  pas de mise à jour » et « pas d'épisode ».
- Garder la structure de configuration **simple et explicite** (pas de valeurs par défaut cachées non
  documentées) : ce sont précisément les hyperparamètres qui seront ajustés lors de la comparaison de
  TACHE-03.

## Définition de fait (DoD)
- Trainer retenu intégré au harnais existant, testé (`ctest` vert), CSV compatible avec les lots
  amont ; build `/W4 /WX` sans avertissement ; Doxygen à jour.

## Notions abordées
@ref guide-annexe-ppo-dqn — PPO (limitation du pas par *clipping*) et DQN (valeur d'action, rejeu
d'expérience, réseau cible).

## Exigences
`EX-IA-015` (nouvelle).
