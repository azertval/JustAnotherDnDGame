# TACHE-02 — Configuration des hyperparamètres et traçabilité {#lot-annexe-19-tache-02-configuration-tracabilite}

**Lot :** [LOT-ANNEXE-19](epic.md) · **Emplacement :** `Source/AiSolver/Cli` · **Statut :** fait

## Contexte
TACHE-01 pose les sous-commandes avec leurs arguments minimaux (niveau, modèle, algorithme) ; cette
tâche ajoute le réglage fin des hyperparamètres propres à chaque algorithme et garantit que la
configuration **effectivement utilisée** — valeurs par défaut incluses — reste toujours retrouvable
après coup, condition de reproductibilité d'un run passé (critère d'acceptation 5 de l'épic).

## Travail à réaliser
- **`aisolver::cli::TrainingConfig`** (`Source/AiSolver/Cli/TrainingConfig.h/.cpp`) : structure
  agrégeant les hyperparamètres pertinents selon l'algorithme choisi (taille de population, taux de
  mutation, élitisme pour l'évolutionniste ; taux d'apprentissage, `gamma`, choix d'optimiseur pour
  les algorithmes de gradient) — chaque champ porte une valeur par défaut documentée.
- **Chargement** : `TrainingConfig loadTrainingConfig(const std::optional<std::filesystem::path>&
  configFile, const CommandLineOverrides&)` — charge un fichier JSON optionnel (`--config`), puis
  applique les surcharges d'arguments individuels (`--population-size`, `--mutation-rate`, etc.),
  chaque source (défaut → fichier → argument) documentée par ordre de priorité croissante.
- **Traçabilité** : `runTrain` (TACHE-01) sérialise la `TrainingConfig` **résolue** (après
  application des trois sources) dans les métadonnées du CSV (`TrainingStatsRecorder`, colonne ou
  fichier annexe `config.json` dans le même dossier de run, `LOT-ANNEXE-09` TACHE-04) et dans les
  métadonnées du fichier de rejeu final (`ReplayFile`, `LOT-ANNEXE-07`) si l'entraînement aboutit à
  un export.

## Fichiers impactés
- `Source/AiSolver/Cli/TrainingConfig.h/.cpp` — nouveau.
- `Source/AiSolver/Cli/Commands.cpp` — modifié (utilise `loadTrainingConfig`, écrit la
  configuration résolue).
- `Source/AiSolver/CMakeLists.txt` — ajout des nouveaux fichiers.

## Tests (obligatoires)
- **Priorité des sources** : un hyperparamètre présent à la fois dans un fichier de configuration
  et en argument individuel retient la valeur de l'argument (priorité la plus haute) ; un
  hyperparamètre absent des deux retient sa valeur par défaut documentée.
- **Fichier de configuration absent** : `loadTrainingConfig(std::nullopt, overrides)` produit une
  configuration entièrement issue des défauts et des arguments, sans erreur.
- **Traçabilité effective** : après un `runTrain`, la configuration résolue (valeurs réellement
  utilisées, pas seulement ce qui a été explicitement passé) est retrouvable dans le dossier de run
  produit (`config.json` ou équivalent), avec des valeurs correspondant exactement à celles
  utilisées pendant l'entraînement (vérifié en relisant la config et en comparant aux paramètres
  effectivement passés aux constructeurs internes).

## Points d'attention
- **La configuration résolue écrite doit être complète**, jamais une simple copie du fichier/des
  arguments fournis par l'utilisateur (qui peuvent être partiels) — c'est précisément la valeur
  ajoutée de cette tâche par rapport à un simple passage d'arguments : reconstituer un run passé ne
  doit jamais nécessiter de connaître les valeurs par défaut de la version du code utilisée à
  l'époque.
- **Le format du fichier de configuration reste propre à `AiSolver`**, distinct du format de niveau
  et du format de rejeu (décision de cadrage de l'épic) — pas de schéma partagé par commodité.

## Définition de fait (DoD)
- `TrainingConfig`/`loadTrainingConfig` disponibles et testés (`ctest` vert), traçabilité vérifiée
  de bout en bout ; build `/W4 /WX` sans avertissement ; Doxygen à jour ; `EX-IA-020` déclarée.

## Notions abordées
Aucune notion d'apprentissage automatique nouvelle : cette tâche est d'ordre logiciel (format de
fichier, outillage, intégration continue). Le vocabulaire employé (épisode, rejeu, politique, agent)
est défini dans @ref guide-annexe-apprentissage-renforcement.

## Exigences
`EX-IA-020` (nouvelle, du même lot).
