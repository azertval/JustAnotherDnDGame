# TACHE-01 — Modèle de bindings manette {#lot-30-tache-01-modele-bindings-manette}

**Lot :** [LOT-30](epic.md) · **Emplacement :** `HMI/Input` · **Statut :** ✅

## Contexte
Pose les fondations du remappage manette, indépendamment de tout écran : l'enum des boutons
physiques, la classe qui détient l'association action ↔ bouton, sa persistance JSON, et un
utilitaire d'affichage/capture de bouton — même patron que `GameKeyBindings`/`KeyName` (`LOT-29`).

## Travail à réaliser
- **`Source/HMI/Input/GamepadButton.h`** (nouveau) : `enum class GamepadButton { Up, Down, Left,
  Right, A, B, X, Y, LeftShoulder, RightShoulder }`.
- **`Source/HMI/Input/GamepadBindings.h`/`.cpp`** (nouveau) : même mécanique que
  `GameKeyBindings` — `button(GameAction)`, `setKey(GameAction, GamepadButton)` (échange sur
  conflit), `resetToDefaults()`, `defaultButton(GameAction)` (miroir du câblage actuel de
  `Window::pollGamepad` : Gauche/Droite/Haut/Bas → `Left`/`Right`/`Up`/`Down`, Sauter → `A`, Dash →
  `RightShoulder`), `save`/`load` (section `"manette"` de `Settings/keybindings.json`, fusionnée
  avec les sections `"jeu"`/`"editeur"` existantes comme `GameKeyBindings`/`EditorKeyBindings` le
  font déjà entre elles).
- **`Source/HMI/Input/GamepadButtonName.h`/`.cpp`** (nouveau) :
  - `gamepadButtonDisplayName(GamepadButton)` — nom lisible (« A », « Épaule droite », etc.).
  - `capturedGamepadButton(const InputState&)` — scrute les dix `GamepadButton`, renvoie le
    premier pressé cette frame.
- **`Source/HMI/Input/InputState.h`/`.cpp`** (écart de cadrage : menée ici plutôt qu'en TACHE-02,
  `capturedGamepadButton` ci-dessus en a besoin immédiatement) : nouvelle piste d'état **brute**
  par `GamepadButton`, indépendante de la fusion clavier/manette existante sur `Key`
  (`onGamepadKeyDown`/`Up`, inchangée, toujours utilisée pour la navigation de menu) —
  `onGamepadButtonDown`/`Up`, `gamepadButtonDown`/`Pressed`, alimentées par `beginFrame` comme les
  pistes existantes. `Window::pollGamepad` (TACHE-02) reste à câbler pour les remplir réellement.

## Fichiers impactés
- `Source/HMI/Input/GamepadButton.h`, `GamepadBindings.h`/`.cpp`,
  `GamepadButtonName.h`/`.cpp` (nouveaux).
- `Source/HMI/Input/InputState.h`/`.cpp` (piste `GamepadButton`, écart de cadrage ci-dessus).
- `Source/HMI/CMakeLists.txt`, `Source/Test/CMakeLists.txt` (nouveaux fichiers source/test).
- Tests : `Source/Test/Unit/HMI/Input/test_gamepad_bindings.cpp`,
  `test_gamepad_button_name.cpp` (nouveaux), `test_input_state.cpp` (nouveaux cas `GamepadButton`).

## Tests (obligatoires)
- Valeurs par défaut correctes pour les six actions (miroir du câblage actuel).
- `setKey` échange correctement quand le bouton cible est déjà utilisé par une autre action.
- `resetToDefaults` restaure exactement les valeurs par défaut.
- Aller-retour JSON, cohabitation avec les sections `"jeu"`/`"editeur"` d'un même fichier (les
  trois sections doivent coexister sans interférence, dans les deux sens de sauvegarde).
- Fichier absent/corrompu/bouton hors plage → valeurs par défaut, aucune exception.
- `gamepadButtonDisplayName` : un nom par valeur de `GamepadButton`.
- `InputState` : un bouton manette (piste brute) suit le même cycle pressé/maintenu/relâché
  qu'une touche clavier ou un bouton souris ; indépendant de la fusion `Key` existante (l'un
  n'affecte jamais l'autre).

## Définition de fait (DoD)
- Compile, testé, sans encore aucun appelant dans le jeu (branchement réel en TACHE-02) — tâche
  purement additive.

## Exigences
`EX-CTRL-002`, `EX-CTRL-012`.
