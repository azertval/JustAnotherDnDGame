# LOT-ANNEXE-22 — Mode IA : IHM complète d'entraînement {#lot-annexe-22}

> Statut : **fait**. Dernier lot du programme Lot-Annexe (remplace [LOT-ANNEXE-21](@ref
> lot-annexe-21) à ce titre).

## Objectif
[LOT-ANNEXE-21](@ref lot-annexe-21) a livré l'écran **Mode IA** comme habillage Qt de
`aisolver-cli` ([LOT-ANNEXE-19](@ref lot-annexe-19)). L'audit de cet écran montre que
l'habillage était partiel sur quatre points, et ce lot les ferme :

1. **Des hyperparamètres réellement lus par le moteur n'étaient réglables nulle part** — ni dans
   l'écran, ni en ligne de commande : seulement par un fichier `--config` écrit à la main. Le plus
   coûteux d'entre eux, `hiddenSize`, détermine la topologie sur laquelle un modèle se recharge.
2. **Le champ « Épisodes / générations max » ne pilotait rien pour l'algorithme évolutionniste** :
   `StoppingConfig` restait aux valeurs par défaut quoi qu'affiche l'écran, d'où une barre de
   progression forcée en indéterminé et un manuel qui devait documenter l'écart.
3. **Trois indicateurs de suivi existaient dans le `.ui` sans qu'aucun code ne les alimente**
   (générations stables, epsilon courant, temps restant estimé), et le tableau de statistiques
   n'affichait que quatre des huit colonnes déjà écrites dans le `stats.csv` du run.
4. **L'onglet Validation & sauvegarde était en net retrait de `evaluate`/`export-replay`** :
   évaluation bloquant le thread d'IHM, niveau et modèle imposés par le run choisi, pas d'export
   du rapport CSV, et un « export de rejeu » qui n'était qu'une copie de fichier.

L'objectif est que l'écran Mode IA expose **au moins** tout ce que sait faire `aisolver-cli`, plus
le suivi en direct qu'une ligne de commande ne peut pas offrir.

## Décision de cadrage : `IHM ⊇ CLI`
`LOT-ANNEXE-21` pose qu'« un désaccord de comportement entre l'IHM et `aisolver-cli` est un bug ».
Ce lot en tire la conséquence dans les deux sens : tout hyperparamètre exposé d'un côté l'est de
l'autre. En pratique, l'IHM était **en avance** sur la CLI (les huit hyperparamètres DQN y étaient
réglables, alors que `parseTrainArgs` ne lisait aucun `--dqn-*`) et **en retard** sur elle
(`--config`, `--runs-root`, `--report`, choix libre du niveau et du modèle à évaluer). Les deux
écarts sont corrigés.

Corollaire appliqué aux valeurs par défaut : l'écran au démarrage doit décrire le même run que
`aisolver-cli train` sans option. Deux champs y dérogeaient (taille de population `64` au lieu de
`32`, taux de mutation `0,08` au lieu de `0,05`) et sont alignés sur les constantes documentées de
`EvolutionaryConfig` — le bouton « Réinitialiser aux défauts » lit ces mêmes constantes, jamais une
seconde liste de valeurs inscrite dans l'écran.

## Périmètre

### Inclus
- **Extensions `AiSolver`** (additives, aucune rupture de signature) :
  - `CommandLineOverrides` et `TrainArgs` élargis aux hyperparamètres qui n'étaient accessibles que
    par fichier de configuration ;
  - treize nouveaux drapeaux de `train` et trois de `evaluate` (voir `Cli/README.md`) ;
  - `LevelTrainingSession::setOnStabilityChanged` : le compteur de séries stables ne vivait que
    dans une variable locale de `run()`, invisible à tout appelant ;
  - `BenchmarkRunner::RepetitionObserver` : progression et interruption d'une campagne, à la
    granularité de la répétition — jamais du pas de simulation.
  - Le CSV secondaire DQN (`index,replayBufferSize,epsilon`), que `DqnTrainer` savait déjà écrire
    mais qu'aucun appelant ne demandait, est désormais produit par les deux front-ends.
- **`HMI/Ai/EvaluationWorker`** : l'évaluation passe sur son propre `QThread`, comme
  l'entraînement — mêmes raisons, même patron.
- **Onglet Entraînement** : taille de couche cachée, dossier des runs, taille du tournoi, force de
  mutation, plafond de générations, réussites stables exigées ; tableau à huit colonnes ; trois
  indicateurs de suivi enfin alimentés ; presets de configuration (charger / enregistrer /
  réinitialiser), équivalents IHM de `--config`.
- **Onglet Validation & sauvegarde** : évaluation asynchrone avec progression et annulation, choix
  libre du modèle et du niveau (évaluation croisée), budget de pas, graine et mode de décodage,
  pas moyen sur toutes les répétitions, export du rapport CSV, reprise des réglages d'un run passé,
  et véritable export de rejeu argmax.
- **`MainWindow::confirmLeavingActiveTraining`** : `AiModeScreen::trainingActive()` et
  `stopTrainingIfActive()` existaient depuis `LOT-ANNEXE-21` sans qu'aucun appelant ne les utilise
  — quitter l'écran laissait un entraînement tourner sans plus aucun moyen de le voir ni de
  l'arrêter.

### Exclus (hors périmètre de ce lot)
- **Reprise d'un entraînement interrompu**, **entraînement multi-niveaux** et **file d'attente de
  runs** : décisions de cadrage de `LOT-ANNEXE-21`, non rouvertes ici.
- **Export d'un rejeu que le modèle ne résout pas** : un rejeu **publié** ne peut être qu'une
  réussite validée (décision de cadrage de [LOT-ANNEXE-11](@ref lot-annexe-11), appliquée aussi par
  `aisolver-cli export-replay`). L'écran produit désormais le même diagnostic explicite que la
  ligne de commande au lieu d'un message d'absence de fichier, mais n'écrit toujours rien.
- **Comparaison de plusieurs runs dans une même vue** et **suppression de runs depuis l'écran** :
  hors du périmètre « exposer ce que fait la CLI ».

## Notions abordées
Aucune notion d'apprentissage automatique nouvelle : ce lot reste un habillage IHM, comme
`LOT-ANNEXE-21`. Les hyperparamètres nouvellement exposés sont décrits par les lots qui les ont
introduits ([LOT-ANNEXE-10](@ref lot-annexe-10), [11](@ref lot-annexe-11), [14](@ref
lot-annexe-14), [15](@ref lot-annexe-15)).

## Exigences couvertes
Réutilisées (inchangées) : [`EX-IA-022`](@ref EX-IA-022) (écran Mode IA), et les exigences des lots
exposés (`LOT-ANNEXE-10` à `21`). Aucune exigence nouvelle : ce lot complète une capacité déjà
spécifiée, il n'en ajoute pas.

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-parametres-cli.md) | Hyperparamètres et drapeaux `aisolver-cli` | `Source/AiSolver/Cli` | ✅ |
| [TACHE-02](tache-02-hooks-observation.md) | Hooks d'observation `AiSolver` | `Source/AiSolver/Training`, `Source/AiSolver/Eval` | ✅ |
| [TACHE-03](tache-03-onglet-entrainement.md) | Onglet Entraînement complet | `Source/HMI`, `Source/Elements/UI` | ✅ |
| [TACHE-04](tache-04-onglet-validation.md) | Onglet Validation & sauvegarde | `Source/HMI`, `Source/Elements/UI` | ✅ |

## Critères d'acceptation du lot
1. Tout hyperparamètre lu par une boucle d'entraînement est réglable **et** dans l'écran Mode IA,
   **et** par un drapeau `aisolver-cli train` — aucun n'est accessible par le seul fichier
   `--config`.
2. Un entraînement évolutionniste lancé avec « Générations max » à `N` s'arrête à `N` générations,
   et la barre de progression avance réellement (elle n'est plus indéterminée).
3. À réglages et graine identiques, l'écran et `aisolver-cli train` produisent des `config.json`
   identiques champ pour champ.
4. Pendant un run, « Générations stables » (évolutionniste) et « Epsilon courant » (DQN) évoluent,
   et le temps restant estimé s'affiche dès la deuxième étape.
5. Le tableau de suivi affiche les huit colonnes du `stats.csv` du run.
6. Une évaluation ne gèle jamais la fenêtre, affiche sa progression et reste annulable ; annulée,
   elle rapporte le nombre de répétitions réellement jouées.
7. Un modèle peut être évalué sur un niveau autre que celui de son run, et un modèle extérieur au
   dossier des runs peut être ouvert directement.
8. Quitter l'écran pendant un entraînement demande confirmation et arrête le run.
9. Build `/W4 /WX` sans avertissement, Doxygen et lint des exigences verts.

## Dépendances
Complète [LOT-ANNEXE-21](@ref lot-annexe-21) sans le remplacer : l'écran, ses trois onglets et
`TrainingWorker` restent ceux de ce lot. Expose plus complètement les capacités de
[LOT-ANNEXE-10](@ref lot-annexe-10)/[11](@ref lot-annexe-11), [14](@ref lot-annexe-14),
[15](@ref lot-annexe-15) et [19](@ref lot-annexe-19). Dernier lot du programme : aucun lot
ultérieur n'en dépend.

## Navigation des tâches
- @subpage lot-annexe-22-tache-01-parametres-cli
- @subpage lot-annexe-22-tache-02-hooks-observation
- @subpage lot-annexe-22-tache-03-onglet-entrainement
- @subpage lot-annexe-22-tache-04-onglet-validation
