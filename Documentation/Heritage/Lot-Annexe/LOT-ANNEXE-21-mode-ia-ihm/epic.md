# LOT-ANNEXE-21 — Mode IA (IHM) {#lot-annexe-21}

> Statut : **fait**. Dernier lot du programme Lot-Annexe (remplace [LOT-ANNEXE-20](@ref
> lot-annexe-20) à ce titre).

## Objectif
`LOT-ANNEXE-19` expose l'entraînement, l'évaluation et l'export de rejeu via `aisolver-cli`, une
ligne de commande — un outil d'expérimentation, jamais pensé pour un joueur ou un testeur sans
terminal. Ce lot ajoute un écran **« Mode IA »** au menu principal du jeu, IHM Qt complète pour les
mêmes trois capacités, avec deux façons de suivre un entraînement : **avec IHM** (rejeu en direct du
meilleur individu courant dans la scène du niveau) ou **sans IHM** (tableau de statistiques,
confirmant la progression sans plantage). L'entrée de menu historique « Regarder l'IA jouer »
(`LOT-ANNEXE-18`) est absorbée par l'onglet Rejeu de ce nouvel écran.

## Amendement à une décision de cadrage antérieure
`LOT-ANNEXE-18` (epic.md, section Exclus) pose une règle transverse, rappelée avec insistance :
« Toute inférence de réseau de neurones dans `HMI` ou `Core` » est hors périmètre — `HMI` ne
référence jamais `Source/AiSolver/Nn`, `Autodiff` ni `Training`. Cette règle reste vraie pour tout
le reste de `HMI` (`ReplayPlayback`, `GameSession`, `GameViewport` en dehors de ce lot).

**Ce lot l'amende, explicitement et localement** : l'entraînement d'un modèle **doit** rester
réactif à l'IHM (barre de progression, aperçu en direct, bouton Arrêter) — un simple lancement
bloquant, comme `aisolver-cli train`, gèlerait la fenêtre pendant potentiellement plusieurs minutes.
Un sous-processus externe (`QProcess` autour de `aisolver-cli`) aurait préservé la règle d'origine,
mais au prix d'un arrêt brutal (aucune sauvegarde du dernier état avant `kill()`) et d'un aperçu en
direct nécessitant que le sous-processus écrive lui-même un fichier de rejeu intermédiaire —
complexité et fragilité jugées supérieures au bénéfice de la séparation stricte.

**Portée de l'amendement, strictement limitée** :
- Seul `Source/HMI/Ai/` (espace de noms isolé, ce lot) référence
  `AiSolver/Training`/`Nn`/`Optim`/`Eval` (au-delà de `AiSolver/Replay`, déjà autorisé depuis
  `LOT-ANNEXE-18`) — jamais `HMI/Game`, `HMI/Editor` ni `HMI/Interface` directement.
- Aucune règle d'apprentissage n'est réimplémentée : `HMI/Ai/TrainingWorker` délègue aux mêmes
  types que `aisolver::cli::runTrain` (`LevelTrainingSession`, `ReinforceTrainer`,
  `ActorCriticTrainer`, `DqnTrainer`), avec deux points d'extension ajoutés **dans `AiSolver`**
  (jamais dans `HMI`) pour rester observable/interruptible : `TrainingStatsRecorder::setOnRecord`
  et un paramètre `shouldStop` optionnel sur chaque `run()` — voir leurs en-têtes respectifs.
- L'entraînement continue de s'exécuter sur un `QThread` séparé du thread d'IHM (jamais bloquant),
  et un seul run à la fois (même règle que `aisolver-cli`, `LOT-ANNEXE-19`).

## Périmètre

### Inclus
- **Entrée de menu unique « Mode IA »**, remplaçant « Regarder l'IA jouer » dans `MainMenu`.
- **Écran `AiModeScreen`**, trois onglets (`QTabWidget`, même portée identité que `MainMenu`) :
  - **Entraînement** : niveau, algorithme (`evo`/`pg`/`ac`/`avance`), hyperparamètres — infobulles
    sur chaque champ/bouton expliquant son rôle (`setToolTip`, styles du châssis identité). Bascule
    « Rejeu 3D » (aperçu du champion courant rejoué dans `GameViewport`, rafraîchi périodiquement)
    / « Statistiques » (tableau de générations) pendant un run actif ; bouton Arrêter.
  - **Validation & sauvegarde** : sélection d'un run, évaluation (`BenchmarkRunner::run`, même
    résultat qu'un appel direct — critère hérité de `LOT-ANNEXE-19`), sauvegarde du modèle, export
    comme rejeu publié (`Elements/Replays/`, alimente l'onglet Rejeu).
  - **Rejeu** : liste des rejeux publiés (`Elements/Replays/*.json`), lecture via
    `GameViewport::startReplay` — reprend le rôle de l'ancien écran « Regarder l'IA jouer ».
- **`HMI/Ai/TrainingWorker`** : dispatch d'algorithme calqué sur `aisolver::cli::runTrain`, sur
  `QThread`, progression/interruption/aperçu par signaux Qt.
- **Extensions `AiSolver`** (additives, jamais de rupture de compatibilité) :
  `TrainingStatsRecorder::setOnRecord`, `shouldStop` sur `LevelTrainingSession::run`/
  `ReinforceTrainer::run`/`ActorCriticTrainer::run`/`DqnTrainer::run`, `onGenerationChampion` sur
  `LevelTrainingSession::run` (accès au champion courant, nécessaire à l'aperçu évolutionniste).
  `AiSolver/Training/ArgmaxRollout` (extrait de `Cli/Commands.cpp`, sans changement de
  comportement) : partagé entre la CLI et `TrainingWorker`, jamais dupliqué.

### Exclus (hors périmètre de ce lot)
- **Toute modification de la logique d'entraînement, d'évaluation ou d'export elle-même** : ce lot
  reste un habillage IHM, même principe que `LOT-ANNEXE-19` pour la CLI — un désaccord de
  comportement entre l'IHM et `aisolver-cli` est un bug de ce lot, jamais une variante assumée.
- **Reprise d'un entraînement interrompu** : « Arrêter » produit un résultat partiel (meilleur
  individu/modèle au moment de l'arrêt, sauvegardé), jamais un point de reprise ultérieur.
- **Entraînement multi-niveaux ou file d'attente de runs** : un seul run actif à la fois (décision
  déjà actée par `LOT-ANNEXE-19`), enchaîner plusieurs niveaux reste manuel.

## Notions abordées
Aucune notion d'apprentissage automatique nouvelle : ce lot est un habillage IHM (Qt, threading) des
capacités déjà cadrées par les lots amont — voir leurs sections « Notions abordées » respectives.

## Exigences couvertes
- Nouvelle, déclarée dans [la spécification IA](@ref spec-ia) : [`EX-IA-022`](@ref EX-IA-022).
- Réutilisées (inchangées) : les exigences de tous les lots exposés (`LOT-ANNEXE-10` à `20`).

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-training-worker.md) | Hooks `AiSolver` + `TrainingWorker` | `Source/AiSolver`, `Source/HMI/Ai` | ✅ |
| [TACHE-02](tache-02-ai-mode-screen.md) | Écran `AiModeScreen` (trois onglets) | `Source/HMI/Interface`, `Source/Elements/UI` | ✅ |
| [TACHE-03](tache-03-integration-menu.md) | Intégration au menu principal | `Source/HMI/Interface` | ✅ |

## Critères d'acceptation du lot
1. Depuis le menu principal, « Mode IA » ouvre un écran à trois onglets ; « Regarder l'IA jouer »
   n'existe plus comme entrée séparée.
2. Un entraînement lancé depuis l'onglet Entraînement ne bloque jamais l'IHM (la fenêtre reste
   réactive, redimensionnable, fermable) ; la vue « Statistiques » se met à jour à chaque
   génération/épisode.
3. La vue « Rejeu 3D » affiche le meilleur individu courant rejoué dans la scène du niveau,
   rafraîchi périodiquement pendant le run.
4. « Arrêter l'entraînement » interrompt le run avant son critère d'arrêt naturel, sauvegarde le
   modèle et les statistiques obtenues jusque-là, sans plantage.
5. L'onglet Validation & sauvegarde produit un résultat d'évaluation numériquement identique à un
   appel direct à `BenchmarkRunner::run` dans les mêmes conditions (même critère que
   `LOT-ANNEXE-19`).
6. L'onglet Rejeu liste les rejeux de `Elements/Replays/` et les rejoue via `GameViewport`,
   identique au comportement de l'ancien `MainWindow::watchAiPlay`.
7. Chaque champ/bouton des onglets Entraînement et Validation & sauvegarde porte une infobulle au
   survol expliquant son rôle.
8. Build `/W4 /WX` sans avertissement, Doxygen et lint des exigences verts.

## Dépendances
Expose, sans les dupliquer, les capacités de [LOT-ANNEXE-10](@ref lot-annexe-10)/[11](@ref
lot-annexe-11) (génération 2), [LOT-ANNEXE-12](@ref lot-annexe-12) à [14](@ref lot-annexe-14)
(génération 3), [LOT-ANNEXE-15](@ref lot-annexe-15) (harnais de benchmark), [LOT-ANNEXE-17](@ref
lot-annexe-17)/[18](@ref lot-annexe-18) (format de rejeu, intégration jeu) et
[LOT-ANNEXE-19](@ref lot-annexe-19) (mêmes types de configuration `TrainingConfig`/`TrainArgs`).
Amende explicitement la décision de cadrage de `LOT-ANNEXE-18` sur l'isolation `HMI`/`AiSolver`
(voir section dédiée ci-dessus). Dernier lot du programme : aucun lot ultérieur n'en dépend.

## Navigation des tâches
- @subpage lot-annexe-21-tache-01-training-worker
- @subpage lot-annexe-21-tache-02-ai-mode-screen
- @subpage lot-annexe-21-tache-03-integration-menu
