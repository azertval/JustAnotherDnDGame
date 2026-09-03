# TACHE-01 — Modèle de bindings et persistance JSON {#lot-29-tache-01-modele-bindings}

**Lot :** [LOT-29](epic.md) · **Emplacement :** `HMI/Input` · **Statut :** ✅

## Contexte
Pose les fondations du remappage, indépendamment de tout écran : les classes qui détiennent
l'association action ↔ touche, leur persistance JSON, et un utilitaire d'affichage/capture de
touche partagé par les deux futurs écrans (TACHE-03).

## Travail à réaliser
- **`Source/HMI/Input/GameKeyBindings.h`/`.cpp`** (nouveau) : `enum class GameAction { MoveLeft,
  MoveRight, AimUp, AimDown, Jump, Dash }`. Classe `GameKeyBindings` : valeurs par défaut (`Left`,
  `Right`, `Up`, `Down`, `Space`, `Shift`, cohérentes avec `PlayerInputMapper.cpp` actuel),
  `key(action)`, `setKey(action, key)` (échange avec l'action qui détenait déjà `key`, s'il y en a
  une), `resetToDefaults()`, `load(path)`/`save(path)` (`nlohmann::json`, clés `gauche`/`droite`/
  `haut`/`bas`/`sauter`/`dash`, valeur = code VK entier).
- **`Source/HMI/Input/EditorKeyBindings.h`/`.cpp`** (nouveau) : même mécanique, `enum class
  EditorAction { Save, Undo, Redo, Copy, Paste, Playtest, ToggleGrid, ToggleHelp, Rename }`,
  défauts (`S`, `Z`, `Y`, `C`, `V`, `P`, `F10`, `F1`, `F2`), clés JSON `sauvegarder`/`annuler`/
  `refaire`/`copier`/`coller`/`testRapide`/`grille`/`aide`/`renommer`.
- **`Source/HMI/Input/KeyName.h`/`.cpp`** (nouveau, partagé) :
  - `std::string keyDisplayName(Key)` — nom lisible pour les touches nommées de l'enum (flèches,
    Espace, Maj, F1/F2/F10, etc.) ; repli générique pour toute touche valide non nommée (lettre/
    chiffre → le caractère ; sinon `"Touche 0xNN"`).
  - `std::optional<Key> capturedKey(const InputState& input)` — scrute les 256 codes suivis par
    `InputState` (aucune modification d'`InputState` : `keyPressed(Key)` accepte déjà tout code
    castable), renvoie le premier `keyPressed` de la frame ; `Échap`/`Entrée` exclus (`nullopt` si
    seule une touche réservée est pressée).
- Un seul fichier `Settings/keybindings.json` (créé/lu via `hmi::executableDirectory() /
  "Settings" / "keybindings.json"`, section `"jeu"` pour `GameKeyBindings`, `"editeur"` pour
  `EditorKeyBindings` — deux méthodes `load`/`save` qui lisent/écrivent chacune leur propre
  sous-objet, sans se marcher dessus).

## Fichiers impactés
- `Source/HMI/Input/GameKeyBindings.h`/`.cpp` (nouveaux).
- `Source/HMI/Input/EditorKeyBindings.h`/`.cpp` (nouveaux).
- `Source/HMI/Input/KeyName.h`/`.cpp` (nouveaux).
- `Source/HMI/CMakeLists.txt` (nouveaux fichiers source).
- Tests : `Source/Test/Unit/HMI/Input/test_game_key_bindings.cpp`,
  `test_editor_key_bindings.cpp`, `test_key_name.cpp` (nouveaux).
- `Source/Test/CMakeLists.txt` (nouveaux fichiers de test).

## Tests (obligatoires)
- Valeurs par défaut correctes pour chaque action (jeu et éditeur).
- `setKey` échange correctement quand la touche cible est déjà utilisée par une autre action ;
  n'affecte aucune autre action sinon.
- `resetToDefaults` restaure exactement les valeurs par défaut après un ou plusieurs remaps.
- Aller-retour JSON (`save` puis `load`) préserve un jeu de bindings personnalisé, pour les deux
  classes, sans interférence entre les sections `"jeu"`/`"editeur"` d'un même fichier.
- `load` sur fichier absent, JSON invalide, ou clé d'action inconnue/valeur non castable → valeurs
  par défaut pour l'entrée concernée, aucune exception propagée.
- `keyDisplayName` : au moins une touche nommée (ex. `Key::Left`), une touche lettre non nommée
  explicitement dans l'enum si applicable, une touche totalement inconnue (repli hexadécimal).
- `capturedKey` : renvoie la touche pressée à la frame courante ; renvoie `nullopt` quand seule
  `Échap` ou `Entrée` est pressée ; renvoie `nullopt` si rien n'est pressé.

## Points d'attention
- `GameKeyBindings`/`EditorKeyBindings` ne connaissent ni `InputState` ni les écrans : uniquement
  le stockage et la persistance, testables en isolation (`EX-NFR-010`), sur le même principe que
  `OptionsModel` ne connaît ni `GraphicsDevice` ni `Localization` en écriture.
- La valeur JSON est un code VK brut : un fichier `keybindings.json` édité à la main avec un code
  hors de la plage couverte (`InputState::KEY_COUNT = 256`) doit être rejeté proprement (repli
  défaut), jamais un accès hors bornes.

## Définition de fait (DoD)
- Les trois classes compilent, sont couvertes par des tests unitaires, et n'ont encore **aucun**
  appelant dans le jeu/l'éditeur (branchement réel en TACHE-02) — cette tâche est purement
  additive, zéro risque de régression.

## Exigences
`EX-CTRL-012` (persistance et modèle).
