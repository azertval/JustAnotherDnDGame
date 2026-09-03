# TACHE-02 — Écran AiModeScreen (trois onglets) {#lot-annexe-21-tache-02-ai-mode-screen}

**Lot :** [LOT-ANNEXE-21](epic.md) · **Emplacement :** `Source/HMI/Interface`, `Source/Elements/UI` ·
**Statut :** à faire

## Contexte
`TACHE-01` fournit `TrainingWorker` ; cette tâche construit l'écran Qt qui l'utilise, avec les
onglets Validation & sauvegarde et Rejeu. Maquettes de référence validées avant implémentation
(revue utilisateur, trois itérations).

## Travail à réaliser
- **`Elements/UI/AiModeScreen.ui`** : `QTabWidget` trois onglets, portée identité (`#AiModeScreen`
  dans `theme.qss`, même patron que `#OptionsPage`/`#LevelSelectScreen`).
  - **Entraînement** : sélecteurs niveau/algorithme, champs hyperparamètres (`QSpinBox`/
    `QDoubleSpinBox`, validation native), `QStackedWidget` pour la bascule Rejeu 3D/Statistiques,
    `QTableWidget` pour les statistiques, bouton Lancer/Arrêter.
  - **Validation & sauvegarde** : sélecteur de run, répétitions, bouton Lancer l'évaluation,
    affichage des résultats (`BenchmarkResult`), boutons Sauvegarder le modèle / Exporter comme
    rejeu publié.
  - **Rejeu** : `QListWidget`/`QTableWidget` des rejeux de `Elements/Replays/`, bouton Lancer.
- **`HMI/Interface/AiModeScreen.h/.cpp`** : construit/possède le `TrainingWorker` et son `QThread`
  (créé à la demande, jamais avant `Lancer l'entraînement` ; détruit proprement après `finished`/
  `requestStop`), connecte ses signaux aux widgets, gère la bascule Rejeu 3D (relance
  `GameViewport::startReplay` sur `previewReady`) et Statistiques (ajoute une ligne au tableau sur
  `progress`).
- **Infobulles** (`setToolTip`) sur chaque champ/bouton des onglets Entraînement et Validation &
  sauvegarde, expliquant son rôle (voir maquettes pour le texte de référence).

## Fichiers impactés
- `Source/Elements/UI/AiModeScreen.ui` — nouveau.
- `Source/HMI/Interface/AiModeScreen.h/.cpp` — nouveau.
- `Source/Elements/Themes/theme.qss` — modifié (`#AiModeScreen`).
- `Source/Elements/Localization/en.lang`/`fr.lang` — modifiés (nouvelles clés).
- `Source/HMI/CMakeLists.txt` — modifié (nouveaux fichiers).

## Tests (obligatoires)
- **Bascule Rejeu 3D/Statistiques sans interrompre le run** : basculer de vue pendant un
  entraînement actif ne redémarre ni n'interrompt le `TrainingWorker`.
- **Validation & sauvegarde cohérente avec un appel direct** : même critère que
  `LOT-ANNEXE-19`/TACHE-01 CLI — résultat numériquement identique à `BenchmarkRunner::run` appelé
  directement dans les mêmes conditions.
- **Rejeu invalide signalé** : un rejeu dont le niveau référencé a changé affiche un message
  d'erreur explicite, jamais un plantage ni un écran de jeu partiellement affiché.

## Points d'attention
- **Le `QThread` de `TrainingWorker` est toujours arrêté proprement avant la destruction de
  l'écran** (`requestStop` puis `wait()`) : une fermeture de fenêtre pendant un run actif ne doit
  jamais laisser un thread orphelin.
- **Un seul `TrainingWorker` actif à la fois** : `Lancer l'entraînement` est désactivé tant qu'un
  run est en cours (même règle que `aisolver-cli`).

## Définition de fait (DoD)
- Écran fonctionnel de bout en bout (test manuel guidé par les maquettes) ; build `/W4 /WX` sans
  avertissement ; Doxygen à jour.

## Notions abordées
Aucune notion d'apprentissage automatique nouvelle : cette tâche est d'ordre IHM (Qt, disposition,
threading). Le vocabulaire employé est défini dans @ref guide-annexe-apprentissage-renforcement.

## Exigences
`EX-IA-022` (nouvelle, partagée avec TACHE-01/03 du même lot).
