# TACHE-02 — Intégration jeu/éditeur {#lot-29-tache-02-integration-jeu-editeur}

**Lot :** [LOT-29](epic.md) · **Emplacement :** `HMI/Input`, `HMI/Interface` · **Statut :** ✅

## Contexte
Branche le jeu et l'éditeur sur les classes de TACHE-01, en remplaçant les lectures directes de
`hmi::Key` par une consultation des bindings — le point d'insertion exact anticipé par le
commentaire de `PlayerInputMapper.h` (« un futur remappage… ne toucherait que cette fonction »).
Avec les valeurs par défaut de TACHE-01, le comportement observable doit rester **strictement identique** à avant ce lot.

## Travail à réaliser
- **`Source/HMI/Input/PlayerInputMapper.h`/`.cpp`** : signature `toPlayerInput(const InputState&,
  const GameKeyBindings&)`. Gauche/droite/sauter passent par
  `input.keyDown(bindings.key(GameAction::…)) || input.keyDown(Key::Q/D/W)` (alias fixe conservé,
  voir décision de cadrage de l'épic) ; visée haut/bas et dash passent entièrement par les
  bindings, sans alias.
- **`Source/HMI/Interface/GameScreen.h`/`.cpp`** : nouveau membre `const GameKeyBindings&
  _gameBindings`, ajouté aux deux constructeurs (séquence de niveaux et niveau unique en mémoire),
  transmis à `toPlayerInput`.
- **`Source/HMI/Interface/EditorScreen.h`/`.cpp`** : nouveaux membres `const EditorKeyBindings&
  _editorBindings` et `const GameKeyBindings& _gameBindings` (ce second uniquement pour le
  relayer au `GameScreen` de l'essai immédiat). Les lectures directes suivantes deviennent des
  consultations de `_editorBindings.key(EditorAction::…)`, le préfixe `Ctrl` restant câblé en dur
  où il existait déjà :
  - `Key::S` (+ `Ctrl`) → `EditorAction::Save`.
  - `Key::Z` (+ `Ctrl`) → `EditorAction::Undo`.
  - `Key::Y` (+ `Ctrl`) → `EditorAction::Redo`.
  - `Key::C` (+ `Ctrl`) → `EditorAction::Copy`.
  - `Key::V` (+ `Ctrl`) → `EditorAction::Paste`.
  - `Key::P` → `EditorAction::Playtest`.
  - `Key::F10` → `EditorAction::ToggleGrid`.
  - `Key::F1` → `EditorAction::ToggleHelp`.
  - `Key::F2` → `EditorAction::Rename`.
  - `EditorScreen::startPlaytest()` : le `GameScreen` construit en interne reçoit `_gameBindings`
    (l'essai immédiat doit respecter le remap courant, pas des valeurs par défaut figées).
- **Panneau d'aide** (`EditorScreen::renderHelp`, raccourcis `F1`) : les lignes actuellement
  câblées en dur (« Ctrl+C / Ctrl+V… », « F2 : renommer… », etc.) interpolent désormais
  `keyName(_editorBindings.key(EditorAction::…))`, pour rester correctes après un remap.
- **`Source/HMI/main.cpp`** : les deux constructions de `GameScreen`/`EditorScreen` dans la
  fabrique d'écrans reçoivent les bindings chargés (voir TACHE-03 pour leur construction/chargement
  au démarrage — cette tâche peut temporairement les construire en local si TACHE-03 n'est pas
  encore faite, ou les deux tâches sont menées de front sur ce fichier).

## Fichiers impactés
- `Source/HMI/Input/PlayerInputMapper.h`/`.cpp`.
- `Source/HMI/Interface/GameScreen.h`/`.cpp`.
- `Source/HMI/Interface/EditorScreen.h`/`.cpp`.
- `Source/HMI/main.cpp`.
- Tests : `Source/Test/Unit/HMI/Input/test_player_input_mapper.cpp` (signature mise à jour),
  tests existants d'`EditorScreen`/`GameScreen` s'il y en a qui construisent ces écrans
  directement.

## Tests (obligatoires)
- `test_player_input_mapper.cpp` : tous les cas existants adaptés à la nouvelle signature, plus au
  moins un cas où un binding **non défaut** (ex. `GameAction::Jump` remappé sur une touche
  arbitraire) déclenche bien `jumpPressed`/`jumpHeld` sur cette touche et plus sur `Space`/`W` seul
  si l'alias n'existe pas pour cette action (dash/visée) — et inversement, confirme que l'alias
  fixe (`Q`/`D`/`W`) continue de fonctionner même quand le binding principal a été remappé ailleurs.
- **Aucune régression** : la suite de tests d'intégration jeu/éditeur existante reste verte avec
  des bindings par défaut (comportement observable inchangé).
- Un test (modèle ou intégration léger) confirme qu'`EditorScreen::startPlaytest()` propage bien
  les bindings de jeu courants au `GameScreen` interne (ex. jeu jouable avec une touche de saut
  remappée, depuis l'essai immédiat).

## Points d'attention
- **Écart constaté (demandeur, en revue)** : la manette n'ayant aucune couche de configuration
  (`Window::pollGamepad` câble chaque bouton XInput en dur sur une touche fixe), un remap clavier
  aurait silencieusement désactivé le bouton manette équivalent (ex. remapper Sauter loin
  d'Espace désactive le bouton A). Corrigé en faisant vérifier à `PlayerInputMapper` la touche par
  défaut de chaque action **en plus** de la touche liée (`GameKeyBindings::defaultKey`) — voir la
  décision de cadrage de l'épic pour le détail. Un vrai remappage manette reste hors périmètre
  (différé à un lot dédié).
- **Régression constatée en usage réel, après livraison de TACHE-03** (rapportée par le demandeur :
  « le remapping n'a pas d'effet en jeu ») : les alias fixes `Q`/`D`/`W` (ci-dessus) entraient en
  collision avec un remap d'une **autre** action sur cette même touche — remapper Gauche sur `D`
  neutralisait tout mouvement horizontal (`D` déclenchait à la fois Gauche, via le remap, et
  Droite, via l'alias fixe câblé en dur). Le filet de sécurité manette souffrait du même défaut
  structurel, non encore déclenché mais latent. **Corrigé** : alias fixes retirés entièrement ;
  le filet manette ne revérifie la touche par défaut d'une action que si aucune **autre** action ne
  se l'est appropriée entre-temps (`GameKeyBindings::isKeyClaimedByOtherAction`). Voir la décision
  de cadrage de l'épic (section « Filet de sécurité manette ») pour le détail complet et les tests
  de régression associés.
- **Écart constaté (TACHE-01)** : `GameKeyBindings.cpp`/`EditorKeyBindings.cpp` sont compilés
  directement dans les cibles `JustAnotherDnDGame` et `UnitTests` (comme `PlayerInputMapper.cpp`),
  pas dans la bibliothèque `Core` — `Core` lie `nlohmann_json` en `PRIVATE`, ce lien ne se propage
  donc pas. Corrigé en ajoutant `nlohmann_json::nlohmann_json` directement aux deux cibles
  (`Source/HMI/CMakeLists.txt`, `Source/Test/CMakeLists.txt`).
- **Échap pendant la capture, cadrage définitif** : `Échap` annule la capture en cours (binding
  inchangé), cohérent avec la convention déjà en place ailleurs (`TextInputField`, dialogues de
  confirmation) — voir TACHE-03.
- Cette tâche ne touche **aucune** UI de remappage (TACHE-03) : les bindings sont pour l'instant
  fixes (valeurs par défaut, ou construites en dur temporairement dans `main.cpp` si menée avant
  TACHE-03) — seul le **branchement** de lecture change, pas encore la possibilité de le modifier
  depuis le jeu.
- Vérifier que remplacer les lectures directes ne change **aucun** comportement de bord (ex.
  `input.keyDown(Control) && input.keyPressed(bindings.key(Save))` doit rester équivalent bit à
  bit à `input.keyDown(Control) && input.keyPressed(Key::S)` quand le binding vaut `Key::S`).

## Définition de fait (DoD)
- Jeu et éditeur fonctionnent identiquement à avant ce lot (bindings par défaut) ; zéro régression
  sur la suite de tests existante ; build `/W4 /WX` sans avertissement.

## Exigences
`EX-CTRL-010` (action dissociée de la touche physique, désormais vrai pour les actions couvertes),
`EX-CTRL-012`.
