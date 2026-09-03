# TACHE-01 — Intégration manette (XInput) {#lot-20-tache-01-integration-manette}

**Lot :** [LOT-20](epic.md) · **Emplacement :** `HMI/Input`, `HMI/Platform` · **Statut :** fait

## Contexte
`InputState` ne suit que clavier et souris. Ce lot ajoute une source manette (XInput,
`EX-CTRL-002`), **fusionnée** dans les mêmes `Key` déjà lus par tout le reste du code (voir
décisions de cadrage de l'épic) — aucun consommateur (`MenuModel`, `PlayerInputMapper`, etc.) ne
change.

## Travail à réaliser
- **`hmi::InputState`** (`Source/HMI/Input/InputState.h`/`.cpp`) :
  - Deux nouveaux tableaux privés `_gamepadCurrent`/`_gamepadPrevious` (mêmes dimensions que les
    tableaux clavier, indexés par le même `Key`), remis en phase à chaque `beginFrame()` comme
    les tableaux clavier.
  - `void onGamepadKeyDown(Key key) noexcept` / `void onGamepadKeyUp(Key key) noexcept` —
    symétriques à `onKeyDown`/`onKeyUp`, mais écrivent dans les tableaux **manette**, jamais dans
    ceux du clavier.
  - `keyDown`/`keyPressed`/`keyReleased` **combinent** les deux sources : `keyDown(k) =
    clavier(k) || manette(k)` ; `keyPressed(k)` = front montant sur **l'une ou l'autre** source
    (`(cur_clavier && !prev_clavier) || (cur_manette && !prev_manette)`), et symétriquement pour
    `keyReleased`.
  - `void setGamepadConnected(bool connected) noexcept` / `bool gamepadConnected() const
    noexcept` — état de connexion, pour l'affichage informatif du menu d'options (TACHE-02).
- **`hmi::Window`** (`Source/HMI/Platform/Window.h`/`.cpp`) :
  - `#include <Xinput.h>`, fonction privée `pollGamepad()` appelée dans `pumpMessages()`, **après**
    `_input.beginFrame()` (les événements manette doivent atterrir dans le nouvel état courant,
    pas l'ancien).
  - `XInputGetState(0, &state)` (un seul joueur, index 0) : si `ERROR_SUCCESS`, traduit
    `state.Gamepad` en appels `onGamepadKeyDown`/`onGamepadKeyUp` pour chaque bouton/direction
    mappé (table ci-dessous) et appelle `_input.setGamepadConnected(true)` ; sinon
    `_input.setGamepadConnected(false)` et **aucun** appel `onGamepadKeyDown` (déconnectée = state
    manette entièrement relâché, pas de bouton resté « collé »).
  - Zone morte du stick gauche : `XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE` (constante XInput), en
    dessous de laquelle le stick est ignoré (évite un déplacement fantôme au repos, jitter
    matériel).
- **Table de correspondance** (mapping par défaut, `EX-CTRL-002`, non reconfigurable — voir
  périmètre exclu) :

  | Manette | `Key` synthétisé |
  |---------|-------------------|
  | D-pad gauche / stick gauche < -deadzone | `Left` |
  | D-pad droite / stick gauche > +deadzone | `Right` |
  | D-pad haut / stick gauche (axe Y) > +deadzone | `Up` |
  | D-pad bas / stick gauche (axe Y) < -deadzone | `Down` |
  | A | `Enter` **et** `Space` (valider en menu, sauter en jeu) |
  | B | `Escape` |
  | Start | `Escape` |
  | Épaule droite (RB) | `Shift` (dash, `EX-CTRL-013`) |

- **`Source/HMI/CMakeLists.txt`** : ajoute `xinput` à `target_link_libraries(JustAnotherDnDGame
  PRIVATE …)`.
- **`Documentation/Specification/controles.md`** : retire le marqueur « ⚠️ souhaité » d'
  `EX-CTRL-002` (implémentée).

## Fichiers impactés
- `Source/HMI/Input/InputState.h`/`.cpp`.
- `Source/HMI/Platform/Window.h`/`.cpp`.
- `Source/HMI/CMakeLists.txt`.
- `Documentation/Specification/controles.md`.
- Tests : `Source/Test/Unit/HMI/Input/test_input_state.cpp` (nouveaux cas manette — via
  `onGamepadKeyDown`/`onGamepadKeyUp` directement, sans XInput réel).

## Tests (obligatoires)
- Un bouton manette seul (`onGamepadKeyDown`) rend `keyDown`/`keyPressed` vrais, comme au clavier.
- Le clavier et la manette combinés sur la **même** touche ne produisent **pas** de double front
  ni d'incohérence (`keyPressed` vrai une seule fois si les deux sources passent à vrai la même
  frame).
- **Non-stomping** : la manette relâchée (`onGamepadKeyUp`) ne masque pas une touche clavier
  **réellement maintenue** sur le même `Key` (`keyDown` reste vrai si le clavier seul la tient).
  Test décisif de la décision de cadrage « fusion en lecture, jamais en écriture ».
- `gamepadConnected()` reflète le dernier `setGamepadConnected` appelé.
- Aucun test XInput réel (accès matériel non simulable/déterministe, cohérent avec le reste des
  accès Win32 non testés unitairement de `HMI`).

## Points d'attention
- **Ne jamais appeler `onKeyDown`/`onKeyUp` (clavier) depuis le sondage manette** — c'est
  exactement le bug de stomping que la décision de cadrage écarte. Utiliser uniquement
  `onGamepadKeyDown`/`onGamepadKeyUp`.
- **Manette déconnectée en cours de partie** (débranchée) : `XInputGetState` renvoie alors
  `ERROR_DEVICE_NOT_CONNECTED` en continu ; `pollGamepad()` doit relâcher **tous** les boutons
  manette suivis à ce moment (sinon une direction resterait « collée » indéfiniment) — le relâchement
  se fait naturellement en n'appelant `onGamepadKeyDown` pour AUCUN bouton ce pas-ci (l'absence
  d'appel `onGamepadKeyDown` équivaut, après un `beginFrame` puis aucun événement, à un état
  manette resté à sa valeur précédente — il faut donc explicitement appeler `onGamepadKeyUp` pour
  chaque touche potentiellement mappée quand la manette est absente, pas simplement ne rien faire).
- La zone morte s'applique **uniquement** au stick (analogique) ; le D-pad (numérique) n'a pas de
  zone morte à appliquer, un bit est déjà net.

## Définition de fait (DoD)
- Manette fusionnée dans `InputState`, sondée par `Window`, testée (`ctest` vert) sans dépendre
  d'un périphérique réel ; build `/W4 /WX` sans avertissement ; Doxygen à jour ; `EX-CTRL-002`
  marquée implémentée.

## Exigences
`EX-CTRL-002` (déjà déclarée, implémentation).
