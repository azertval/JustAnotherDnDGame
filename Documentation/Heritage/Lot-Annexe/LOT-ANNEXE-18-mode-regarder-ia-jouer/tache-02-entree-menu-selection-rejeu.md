# TACHE-02 — Entrée de menu dédiée, sélection d'un rejeu {#lot-annexe-18-tache-02-entree-menu-selection-rejeu}

**Lot :** [LOT-ANNEXE-18](epic.md) · **Emplacement :** `Source/HMI/Interface` · **Statut :** fait

## Contexte
`ReplayPlayback` (TACHE-01) sait jouer un fichier de rejeu donné ; il manque encore un point d'accès
depuis le menu principal du jeu pour qu'un utilisateur en choisisse un et lance la lecture, symétrique
au parcours existant « Jouer » (`hmi::MainWindow::startGame`, séquence `demo-*.json`).

## Travail à réaliser
- **Entrée de menu « Regarder l'IA jouer »** (`Source/HMI/Interface/MainWindow.h/.cpp` ou un
  nouveau widget dédié, selon l'organisation Qt existante du menu principal) : ouvre une liste des
  fichiers de rejeu disponibles.
- **Emplacement des rejeux « publiés »** : un sous-dossier documenté (ex. `Source/Elements/Replays/`,
  à la manière de `Source/Elements/Levels/` pour les niveaux — décision à documenter précisément à
  l'implémentation) distinct de `/TrainingRuns/` (non versionné, réservé aux runs d'entraînement
  bruts) : un rejeu jugé démonstratif doit être copié manuellement de l'un vers l'autre pour devenir
  sélectionnable dans le menu — pas de mécanisme automatique de « publication ».
- **`GameViewport::startReplay(const std::filesystem::path& replayPath)`** (symétrique à
  `startGame`) : construit un `ReplayPlayback` (TACHE-01), bascule la boucle de pas fixe existante
  (`GameViewport::tick`) vers la source d'entrée « rejeu » plutôt que « clavier/manette », jusqu'à
  ce que `ReplayPlayback::nextInput()` renvoie `std::nullopt` (fin de séquence) ou que
  `GameSession::update` renvoie `Won`/`Lost` — dans les deux cas, retour au menu (pas d'enchaînement
  vers un niveau suivant, un rejeu porte sur un seul niveau).

## Fichiers impactés
- `Source/HMI/Interface/MainWindow.h/.cpp` — modifié (nouvelle entrée de menu).
- `Source/HMI/Game/GameViewport.h/.cpp` — modifié (`startReplay`, aiguillage de la source d'entrée
  dans `tick()`).
- `Source/Elements/Replays/README.md` — nouveau (convention de dossier, à l'image de `Source/
  Elements/Levels/README.md`).

## Tests (obligatoires)
- **Sélection et lancement** : choisir un rejeu dans la liste lance effectivement
  `GameViewport::startReplay` avec le chemin attendu (test d'intégration Qt si l'infrastructure de
  test de `HMI/Interface` le permet, sinon test unitaire sur la fonction de listing/sélection
  isolée de tout widget).
- **Fin de rejeu ramène au menu** : qu'elle se termine par `Won`, `Lost` ou épuisement de la
  séquence, une session de rejeu retourne au menu principal, jamais bloquée ni plantée.
- **Liste vide gérée proprement** : en l'absence de tout fichier sous le dossier de rejeux publiés,
  l'entrée de menu reste accessible mais signale l'absence de rejeu disponible, sans erreur.

## Points d'attention
- **Aucun enchaînement automatique vers un niveau suivant** : contrairement au mode « Jouer »
  (`startGame`, séquence de niveaux), un rejeu porte toujours sur un seul niveau — cohérent avec le
  régime d'entraînement niveau par niveau de tout le programme.
- **La distinction `/TrainingRuns/` (non versionné, brut) vs dossier de rejeux publiés (versionné,
  sélectionné manuellement)** est délibérée : elle évite qu'un menu de jeu livré affiche des
  centaines de runs d'entraînement intermédiaires sans intérêt démonstratif.

## Définition de fait (DoD)
- Entrée de menu et `GameViewport::startReplay` disponibles et testés dans la mesure du possible
  (`ctest` vert pour la partie non-Qt) ; build `/W4 /WX` sans avertissement ; Doxygen à jour.

## Notions abordées
Aucune notion d'apprentissage automatique nouvelle : cette tâche est d'ordre logiciel (format de
fichier, outillage, intégration continue). Le vocabulaire employé (épisode, rejeu, politique, agent)
est défini dans @ref guide-annexe-apprentissage-renforcement.

## Exigences
`EX-IA-019` (nouvelle, partagée avec TACHE-01/03 du même lot).
