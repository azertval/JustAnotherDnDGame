# TACHE-01 — Boucle d'entraînement pour un niveau et critère d'arrêt {#lot-annexe-11-tache-01-boucle-entrainement}

**Lot :** [LOT-ANNEXE-11](epic.md) · **Emplacement :** `Source/AiSolver/Training` · **Statut :**
fait

## Contexte
LOT-ANNEXE-10 fournit `EvolutionaryTrainer::runGeneration()`, une mécanique générique agnostique de
toute notion de « niveau résolu ». Cette tâche l'applique à un fichier de niveau précis, avec un
critère d'arrêt métier réel : c'est ici que la décision transverse « un run = un niveau » prend sa
forme la plus concrète du programme Lot-Annexe.

## Travail à réaliser
- **`aisolver::training::LevelTrainingSession`** : construite à partir d'un **unique** chemin de
  fichier niveau (`std::filesystem::path`), d'une `EvolutionaryConfig` (LOT-ANNEXE-10), d'une
  configuration d'arrêt (`requiredConsecutiveSuccesses`, `maxGenerations`), d'une seed `Rng` et d'un
  chemin de sortie pour `TrainingStatsRecorder` (LOT-ANNEXE-09).
- **Construction** : charge le niveau via `core::LevelLoader` (déjà existant dans `Core`, aucune
  modification), construit **un seul** `HeadlessLevelEnvironment` (LOT-ANNEXE-05) pour ce niveau,
  réutilisé (`reset()`) pour toute la session — jamais reconstruit par génération.
- **`run() -> TrainingResult`** : boucle sur `EvolutionaryTrainer::runGeneration()`. Après chaque
  génération, détermine si le meilleur individu courant a résolu le niveau (booléen de succès
  d'épisode fourni par LOT-ANNEXE-08, distinct du score de fitness continu) et s'il est resté le
  même individu invaincu (aucun autre ne l'a dépassé) depuis la dernière rupture de séquence —
  maintient un compteur de générations consécutives satisfaisant les deux conditions ; toute
  rupture (nouveau champion, ou champion non résolvant) remet le compteur à zéro. S'arrête dès que
  le compteur atteint `requiredConsecutiveSuccesses`, ou dès que `maxGenerations` est atteint.
- **`TrainingResult`** (struct) : `bool solved`, `unsigned generationsRun`, `Individual
  bestIndividual` — permet à l'appelant de distinguer un arrêt par résolution d'un arrêt par
  plafond, et donne accès au meilleur individu pour TACHE-02.
- `LevelTrainingSession` ne connaît **jamais** de liste de niveaux ni de mécanisme de progression :
  un seul appel à `core::LevelLoader::load` par session, aucune structure d'itération sur plusieurs
  fichiers niveau nulle part dans cette classe.

## Fichiers impactés
- `Source/AiSolver/Training/LevelTrainingSession.h`/`.cpp` (nouveaux).
- `Source/AiSolver/Training/TrainingResult.h` (nouveau).
- Tests : `Source/Test/Unit/AiSolver/Training/test_level_training_session.cpp` (nouveau).

## Tests (obligatoires)
- **Arrêt par résolution** : sur un niveau trivial, la session s'arrête via le critère « résolu N
  fois d'affilée » avant d'atteindre un plafond de générations volontairement large ;
  `TrainingResult::solved == true`.
- **Arrêt par plafond** : avec un plafond de générations fixé artificiellement bas dans le test (sur
  un niveau ou une configuration ne permettant pas de résoudre à temps), la session s'arrête par le
  plafond ; `TrainingResult::solved == false`.
- **Réinitialisation du compteur de succès consécutifs** : un scénario où le champion change avant
  d'avoir atteint le seuil (fitness/succès contrôlés via une fixture de test) remet bien le compteur
  à zéro plutôt que de le laisser progresser à tort.
- **Un seul chargement de niveau** : `core::LevelLoader::load` n'est appelé qu'une fois par session
  (vérifiable par compteur d'appels ou identité d'objet), pas une fois par génération.

## Points d'attention
- **« N fois d'affilée » n'est pas une répétition de tirages aléatoires** : l'évaluation et le
  rejeu sont déterministes (LOT-ANNEXE-10) — répéter l'évaluation du même individu inchangé donnerait
  toujours le même résultat. Le critère est donc une exigence de **stabilité inter-générationnelle**
  (le champion reste invaincu et résolvant pendant N générations), pas une garantie statistique
  contre un aléa d'évaluation qui n'existe pas ici (voir décision de cadrage de l'épic).
- **Le plafond de générations doit être réellement atteignable en test**, pas une valeur symbolique
  jamais exercée — au moins un test doit effectivement sortir par ce chemin pour que le cas
  « la ligne de base n'apprend pas ce niveau » soit couvert.
- **`LevelTrainingSession` ne fait aucune hypothèse sur le contenu du fichier niveau** (pas de
  chemin en dur vers un niveau de démonstration particulier) — reste utilisable sur tout niveau
  valide du jeu, y compris ceux qui n'existent pas encore.

## Définition de fait (DoD)
- `LevelTrainingSession` disponible et testée (`ctest` vert), les deux chemins d'arrêt exercés ;
  build `/W4 /WX` sans avertissement ; Doxygen à jour.

## Notions abordées
@ref guide-annexe-algorithmes-evolutionnistes — boucle générationnelle, élitisme, reproductibilité
d'un entraînement.

## Exigences
Aucune exigence propre — contribue à `EX-IA-012` (déclarée en TACHE-03).
