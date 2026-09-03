# TACHE-02 — Intégration jeu {#lot-30-tache-02-integration-jeu}

**Lot :** [LOT-30](epic.md) · **Emplacement :** `HMI/Input`, `HMI/Platform`, `HMI/Interface` · **Statut :** ✅

## Contexte
Branche le jeu sur `GamepadBindings` (TACHE-01) : `Window::pollGamepad` alimente la piste d'état
brute par `GamepadButton` (déjà ajoutée à `InputState` en TACHE-01, écart de cadrage) depuis le
relevé XInput déjà lu, et `PlayerInputMapper` la consulte à la place du filet de sécurité de
`LOT-29`.

## Travail à réaliser
- **`Source/HMI/Platform/Window.cpp`** (`pollGamepad`) : ajoute, à côté des `setKey(Key::…, …)`
  existants (inchangés), des appels `onGamepadButtonDown/Up(GamepadButton::…, …)` pour les dix
  boutons, à partir du même `XINPUT_STATE`/`stickDirection` déjà calculés.
- **`Source/HMI/Input/PlayerInputMapper.h`/`.cpp`** : signature `toPlayerInput(const InputState&,
  const GameKeyBindings&, const GamepadBindings&)`. Chaque action :
  `input.keyDown(gameKeyBindings.key(action)) ||
  input.gamepadButtonDown(gamepadBindings.button(action))` (ou `keyPressed`/
  `gamepadButtonPressed` pour les fronts). Retire `safeDefaultDown`/`safeDefaultPressed` et
  `GameKeyBindings::isKeyClaimedByOtherAction` (devenus inutiles).
- **`Source/HMI/Interface/GameScreen.h`/`.cpp`** : nouveau membre `const GamepadBindings&
  _gamepadBindings`, transmis à `toPlayerInput`.
- **`Source/HMI/Interface/EditorScreen.h`/`.cpp`** : nouveau membre `const GamepadBindings&
  _gamepadBindings`, relayé au `GameScreen` de l'essai immédiat (`startPlaytest`).
- **`Source/HMI/main.cpp`** : les constructions de `GameScreen`/`EditorScreen` reçoivent
  `gamepadBindings` (voir TACHE-03 pour son chargement au démarrage).

## Fichiers impactés
- `Source/HMI/Platform/Window.cpp`.
- `Source/HMI/Input/PlayerInputMapper.h`/`.cpp`.
- `Source/HMI/Interface/GameScreen.h`/`.cpp`, `EditorScreen.h`/`.cpp`.
- `Source/HMI/main.cpp`.
- Tests : `test_player_input_mapper.cpp` (nouvelle signature, retrait des tests du filet `LOT-29`
  devenu obsolète, nouveaux cas manette).

## Tests (obligatoires)
- `test_player_input_mapper.cpp` : chaque action déclenchée par son bouton manette lié (nouveau
  binding non défaut) ; un remap manette n'affecte pas le clavier et réciproquement ; les tests du
  filet `LOT-29` (`BoutonManetteContinueDeFonctionnerApresRemapClavier`,
  `FiletDeSecuriteNIgnoreQuandToucheDefautReprise`) sont retirés ou réécrits pour vérifier
  directement le nouveau chemin manette (plus besoin de filet, la manette a sa propre touche).
- **Aucune régression** : la suite de tests d'intégration/système existante reste verte avec les
  valeurs par défaut (comportement manette observable inchangé).

## Points d'attention
- Vérifier que retirer le filet de sécurité ne réintroduit pas la régression qu'il corrigeait
  (`EX-CTRL-002`) : avec les valeurs par défaut de `GamepadBindings` (miroir exact de l'ancien
  câblage), le comportement manette observable doit rester strictement identique à avant ce lot.
- `Window::pollGamepad` sonde une seule fois par frame (`EX-CTRL-021`) : la piste `GamepadButton`
  et la piste `Key` existante doivent être alimentées dans le **même** appel, pas deux sondages
  XInput distincts.

## Définition de fait (DoD)
- Jeu fonctionnellement identique à avant ce lot avec les valeurs par défaut ; zéro régression sur
  la suite de tests existante ; build `/W4 /WX` sans avertissement.

## Exigences
`EX-CTRL-002`, `EX-CTRL-010`, `EX-CTRL-012`, `EX-CTRL-021`.
