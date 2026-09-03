# TACHE-01 — Entrées nerveuses : fronts non perdus, focus, sondage manette {#lot-33-tache-01-entrees-nerveuses}

**Lot :** [LOT-33](epic.md) · **Modules :** `Source/HMI/Platform`, `Source/HMI/Input`, `Source/HMI/main.cpp` · **Statut :** terminé

## Contexte
La boucle a toujours supposé un rendu calé sur ~60 Hz. Sur un écran plus rapide (120/144 Hz),
`FixedTimestep::advance` renvoie `0` pas sur une partie des frames. Or `Window::pumpMessages`
appelait `InputState::beginFrame` (recopie courant → précédent, qui **efface** les fronts) à
**chaque** frame de rendu, alors que seuls les pas de simulation lisent ces fronts : un appui
capturé sur une frame sans pas était perdu (`EX-CTRL-020`/`EX-CTRL-021` violées au-delà de 60 Hz).
Deux défauts connexes de la même couche d'entrée : une touche maintenue au moment d'un `Alt+Tab`
restait « collée » (aucun `WM_KEYUP`), et le sondage `XInputGetState` d'un slot **vide** (coûteux)
à chaque frame provoquait des micro-saccades chez un joueur clavier.

## Travail à réaliser
- **Découpler les fronts du rendu** :
  - `Window::pumpMessages` ne fait plus que sonder la manette (`pollGamepad`) et drainer les
    messages Win32 dans l'état **courant** — plus aucun `beginFrame`.
  - Nouvelle `Window::beginInputFrame` : appelle `InputState::beginFrame` (recopie courant →
    précédent). La boucle (`main.cpp`) l'appelle **après chaque pas fixe consommé**, jamais par
    frame de rendu. Un appui capturé sur une frame sans pas survit jusqu'à ce qu'un pas le lise.
- **Relâchement à la perte de focus** :
  - `InputState::releaseAll` : remet à zéro l'état **courant et précédent** de toutes les
    touches/boutons (clavier, manette fusionnée, boutons manette bruts, souris), sans produire de
    front « relâchée ».
  - `Window::handleMessage` traite `WM_KILLFOCUS` en appelant `releaseAll`.
- **Throttler le sondage d'une manette absente** :
  - `Window::pollGamepad` : tant que la manette reste déconnectée, ne re-sonder qu'une frame sur
    `GAMEPAD_DISCONNECTED_POLL_INTERVAL` (≈ 2 s à 60 Hz) ; sondage systématique dès qu'une manette
    est présente (branchement à chaud détecté au plus tard après un intervalle).

## Fichiers impactés
- `Source/HMI/Platform/Window.h`, `Source/HMI/Platform/Window.cpp` : `pumpMessages` sans
  `beginFrame`, nouvelle `beginInputFrame`, `WM_KILLFOCUS`, throttling `pollGamepad`
  (`_gamepadPollCountdown`, `GAMEPAD_DISCONNECTED_POLL_INTERVAL`).
- `Source/HMI/Input/InputState.h`, `Source/HMI/Input/InputState.cpp` : `releaseAll`.
- `Source/HMI/main.cpp` : `beginInputFrame` après chaque pas ; `pumpMessages` documentée.
- `Source/Test/Unit/HMI/Input/test_input_state.cpp` : test de `releaseAll`.

## Tests (obligatoires)
- `InputState::releaseAll` relâche toutes les entrées maintenues (clavier, manette, souris) et ne
  produit **aucun** front « relâchée », ni à la frame courante ni à la suivante.
- Non-régression : toute la suite reste verte (les tests injectant `beginFrame`/`onKeyDown`
  directement sur `InputState` sont inchangés — l'`InputState` lui-même n'a pas changé de sémantique,
  seul son pilotage par la boucle a changé).
- Le découplage boucle ↔ pas (orchestration dans `main.cpp`/`Window`) est vérifié **manuellement**
  à haut framerate : dépendance fenêtre/Win32, non couvert par un test unitaire (comme la boucle
  elle-même depuis `LOT-01`).

## Définition de fait (DoD)
- À 120/144 Hz, aucun appui consommé par un pas n'est perdu ; `Alt+Tab` ne colle plus les touches ;
  aucune micro-saccade sans manette branchée.
- Compile `/W4 /WX`, formaté, API documentée `.h` + `.cpp`.

## Exigences
`EX-CTRL-020`, `EX-CTRL-021`, `EX-CTRL-002`, `EX-NFR-040`, `EX-NFR-001`.
