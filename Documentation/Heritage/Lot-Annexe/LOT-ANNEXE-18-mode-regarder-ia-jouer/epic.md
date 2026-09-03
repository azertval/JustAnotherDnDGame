# LOT-ANNEXE-18 — Mode « regarder l'IA jouer » (intégration HMI) {#lot-annexe-18}

> Statut : **fait**. Prérequis : [LOT-ANNEXE-17](@ref lot-annexe-17) (format de rejeu
> validé). Premier lot de la génération 5 et **seule** pièce de tout le programme Lot-Annexe qui
> touche `HMI` — tout le reste (générations 0 à 4) vit exclusivement dans le nouveau module
> `Source/AiSolver`, jamais dans `Core` ni `HMI`.

## Objectif
Le programme Lot-Annexe a, jusqu'ici, produit des fichiers de rejeu validés (`LOT-ANNEXE-17`) mais
aucun moyen de les **jouer** dans le vrai jeu. La décision transverse du programme est ferme :
**rejeu uniquement**, aucune inférence live, aucune nouvelle dépendance d'inférence dans `Core`/
`HMI`. Ce lot tient cette promesse à la lettre : il ne branche **aucun** réseau de neurones sur le
jeu, seulement une séquence de `core::PlayerInput` déjà figée, rejouée par la boucle de simulation
existante — le même mécanisme qui garantit déjà, par construction, qu'un rejeu produit la trajectoire
exacte observée pendant l'entraînement (`EX-NFR-002`, déterminisme au pas fixe de `Core`).

## Périmètre

### Inclus
- **`hmi::ReplayPlayback`** (`Source/HMI/Game`) : charge un fichier de rejeu via
  `aisolver::readReplay` (`LOT-ANNEXE-07`), le fait valider par `aisolver::validateReplay`
  (`LOT-ANNEXE-17`) avant tout usage, puis fournit un `core::PlayerInput` par pas fixe à la demande
  de la boucle de jeu.
- **Nouveau point d'entrée public sur `hmi::GameSession`** : `update(const core::PlayerInput&,
  float fixedDelta)`, exposant directement la logique déjà interne à `GameSession::update` une fois
  l'entrée traduite — l'actuel `update(const InputState&, float)` devient un mince habillage qui
  traduit l'entrée (`toPlayerInput`) puis appelle ce nouveau point d'entrée ; **aucun** comportement
  existant n'est modifié, seule la traduction d'entrée est rendue court-circuitable.
- **Entrée de menu dédiée** (Qt, `Source/HMI/Interface`) : liste les fichiers de rejeu disponibles
  (répertoire connu, ex. `TrainingRuns/**/replay.json` ou un dossier de rejeux « publiés »
  distinct — précisé à l'implémentation), lance une partie pilotée par `ReplayPlayback` au lieu de
  l'entrée clavier/manette habituelle.
- **Test système** : rejeu d'un fichier de rejeu réel (produit par un algorithme d'entraînement)
  aboutit à `core::LevelOutcome::Won`, sur le modèle exact de `test_parcours_complet.cpp` mais à
  partir d'un fichier plutôt que d'un script d'entrée codé en dur.

### Exclus (hors périmètre de ce lot)
- **Toute inférence de réseau de neurones dans `HMI` ou `Core`** : décision ferme, transverse au
  programme entier, rappelée ici avec la plus grande insistance parce que c'est le lot le plus
  susceptible d'être tenté d'y déroger « pour aller plus vite ». `ReplayPlayback` ne charge ni ne
  référence jamais `Source/AiSolver/Nn`, `Autodiff` ni aucun module de `Source/AiSolver/Training`.
  **Amendée, explicitement et localement, par [LOT-ANNEXE-21](@ref lot-annexe-21)** : l'entraînement
  interactif depuis l'IHM (barre de progression, aperçu en direct, arrêt propre) exige un accès
  direct à `AiSolver/Training`/`Nn`/`Optim`, limité au seul espace de noms `HMI/Ai` — cette règle
  reste vraie pour tout le reste de `HMI`, `ReplayPlayback` inclus.
- **Édition ou retouche d'un rejeu depuis l'éditeur de niveaux** (`HMI/Editor`) : ce lot ajoute un
  **mode de jeu**, pas une fonctionnalité d'édition — `HMI/Editor` n'est pas modifié.
- **Rejeu partiel ou navigable** (avance/recul dans la séquence, pause) : un rejeu se joue du
  début à la fin, comme une partie normale — l'ajout d'une scrubbing bar ou d'un contrôle de vitesse
  n'a aucun besoin identifié pour ce lot.
- **Génération ou sélection automatique du « meilleur » rejeu disponible** (ex. parmi plusieurs
  runs sur le même niveau) : la sélection reste manuelle, via la liste de l'entrée de menu — aucune
  heuristique de choix automatique.

## Décisions de cadrage
- **`GameSession` reçoit une nouvelle surface publique, pas une classe parallèle qui dupliquerait sa
  boucle de simulation.** Réutiliser `GameSession` tel quel (moins la seule étape de traduction
  d'entrée) élimine tout risque de divergence entre le jeu joué normalement et le jeu rejoué — le
  scénario que `LOT-ANNEXE-05` a précisément refusé de risquer en gardant son environnement headless
  séparé de `GameSession` ; ici, à l'inverse, c'est `GameSession` lui-même qui est réutilisé, donc
  aucun risque de divergence ne peut apparaître.
- **`Source/HMI` acquiert une nouvelle dépendance vers `Source/AiSolver/Replay` (lecture/validation
  de rejeu), pas vers le reste d'`AiSolver`.** Le sens de dépendance déjà acté (`Core` sans
  dépendance à `HMI`, jamais l'inverse) ne concernait à l'origine que `Core`/`HMI` ; ce lot ajoute
  une arête `HMI → AiSolver` limitée au seul module `Replay` (format de fichier, validation), jamais
  vers `Nn`/`Autodiff`/`Training`/`Optim` — documenté explicitement comme une extension mineure et
  délibérée de la carte de dépendances du projet, pas une entorse silencieuse.
- **La validation (`LOT-ANNEXE-17`) s'exécute systématiquement avant tout rejeu**, jamais
  contournable depuis le menu : un rejeu invalide (niveau modifié depuis l'export) est refusé avec un
  message explicite, jamais joué partiellement ni silencieusement.
- **Le déterminisme de `Core` (`EX-NFR-002`) est la seule raison pour laquelle ce lot fonctionne** :
  aucune synchronisation particulière n'est nécessaire entre l'enregistrement (génération 2/3, en
  environnement headless) et la lecture (ce lot, dans le vrai jeu, rendu compris) — le pas fixe
  produit la même trajectoire quel que soit le contexte d'exécution, pourvu que la séquence
  d'entrées et le niveau soient identiques (garanti par la validation).
- **`ReplayPlayback` ne modifie jamais un fichier de rejeu** : lecture seule stricte, cohérent avec
  le rôle de `HMI` en général vis-à-vis des données de simulation (ne mute jamais ce qu'il affiche
  sans passer par une action explicite de l'utilisateur, ici aucune action de ce type n'existe).

## Notions abordées
Aucune notion d'apprentissage automatique nouvelle : ce lot est de l'intégration logicielle pure
(branchement d'une séquence enregistrée sur la boucle de jeu existante) — le seul principe théorique
en jeu est le déterminisme au pas fixe (`EX-NFR-002`), déjà une propriété de `Core`, pas une notion
introduite par le programme Lot-Annexe.

## Exigences couvertes
- Nouvelle, déclarée dans [la spécification IA](@ref spec-ia) : [`EX-IA-019`](@ref EX-IA-019).
- Réutilisées (inchangées) : `EX-IA-008` (format de rejeu v1), `EX-IA-018` (validation à la
  lecture), `EX-NFR-002` (déterminisme au pas fixe, condition de fidélité du rejeu).

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-lecture-rejeu-alimentation-gamesession.md) | Lecture du rejeu et alimentation de `GameSession` | `Source/HMI/Game` | ✅ |
| [TACHE-02](tache-02-entree-menu-selection-rejeu.md) | Entrée de menu dédiée, sélection d'un rejeu | `Source/HMI/Interface` | ✅ |
| [TACHE-03](tache-03-test-systeme-rejeu.md) | Test système : le rejeu aboutit à `Won` | `Source/Test/Systeme` | ✅ |

## Critères d'acceptation du lot
1. Un fichier de rejeu valide, chargé et joué via le nouveau mode, produit exactement la trajectoire
   observée lors de son enregistrement (même issue finale, mêmes positions à chaque pas).
2. Un fichier de rejeu dont le niveau référencé a changé depuis l'export est refusé avec un message
   explicite, sans plantage, avant qu'aucun pas de simulation ne soit joué.
3. `GameSession::update(const InputState&, float)` existant produit un comportement strictement
   identique à avant ce lot (non-régression, tests existants toujours verts) — seule une nouvelle
   surchage est ajoutée.
4. Aucun symbole de `Source/AiSolver/Nn`, `Autodiff`, `Optim` ou `Training/*` n'est référencé depuis
   `Source/HMI` (vérifiable par les dépendances de compilation de la cible `HMI`).
5. Logique nouvelle **couverte par des tests** (`ctest` vert), Doxygen et lint des exigences verts.
   Build `/W4 /WX` sans avertissement.

## Dépendances
Bâtit sur [LOT-ANNEXE-17](@ref lot-annexe-17) (format de rejeu validé) et, transitivement,
[LOT-ANNEXE-07](@ref lot-annexe-07) (format v1). Réutilise `hmi::GameSession`/`hmi::GameViewport`
(lus, étendus a minima, jamais réécrits). [LOT-ANNEXE-19](@ref lot-annexe-19) (CLI) et
[LOT-ANNEXE-20](@ref lot-annexe-20) (garde-fou CI) supposent l'existence de ce mode pour avoir un
sens pratique (un rejeu qu'on ne peut jamais jouer en jeu n'a pas besoin d'outillage de production
ni de garde-fou de péremption).

## Navigation des tâches
- @subpage lot-annexe-18-tache-01-lecture-rejeu-alimentation-gamesession
- @subpage lot-annexe-18-tache-02-entree-menu-selection-rejeu
- @subpage lot-annexe-18-tache-03-test-systeme-rejeu
