# TACHE-03 — UI de remappage manette et câblage {#lot-30-tache-03-ui-remappage-manette}

**Lot :** [LOT-30](epic.md) · **Emplacement :** `HMI/Interface`, `HMI/main.cpp` · **Statut :** ✅

## Contexte
Rend le remappage manette accessible depuis Options : un nouveau sous-menu listant les six actions
de jeu avec le bouton manette lié, même patron que `GameKeybindingsModel`/`Screen` (`LOT-29`).

## Travail à réaliser
- **`Source/HMI/Interface/GamepadBindingsModel.h`/`.cpp`** (nouveau) : logique testable, patron
  `GameKeybindingsModel` — six lignes d'action + « Réinitialiser » + « Retour », réutilisant les
  constantes de mise en page de `GameKeybindingsModel` (`ROWS_TOP`, `ROW_SPACING`, `ROW_SCALE`,
  `MenuModel::MARGIN_X`). Capture : `capturedGamepadButton` (TACHE-01) au lieu de `capturedKey`.
  Si `InputState::gamepadConnected()` est faux, la ligne sélectionnée ne peut pas entrer en
  capture ; affiche une invite dédiée à la place (« Connectez une manette »).
- **`Source/HMI/Interface/GamepadBindingsScreen.h`/`.cpp`** (nouveau) : habillage `IScreen`, patron
  `GameKeybindingsScreen`.
- **`Source/HMI/Interface/OptionsModel.h`/`.cpp`** : `OPTION_COUNT` `4` → `5` ; nouvelle valeur
  `OptionsAction::OpenGamepadBindings` ; `actionFor`/`optionLabel` mis à jour.
- **`Source/HMI/Interface/OptionsScreen.cpp`** : la nouvelle action produit
  `ScreenTransition::switchTo(ScreenId::GamepadBindings)`.
- **`Source/HMI/Interface/IScreen.h`** : `ScreenId` gagne `GamepadBindings`.
- **`Source/HMI/Interface/ScreenManager.cpp`** : `screenName()` gagne le nouveau cas.
- **`Source/HMI/main.cpp`** : construit `hmi::GamepadBindings gamepadBindings =
  hmi::GamepadBindings::load(keybindingsPath)` au démarrage (même fichier que `gameBindings`/
  `editorBindings`) ; nouveau `case` pour `ScreenId::GamepadBindings` ; les `case Game`/`Editor`
  passent désormais `gamepadBindings` en plus (branchement TACHE-02 finalisé ici si pas déjà fait).
- **Localisation** (`fr.lang`/`en.lang`) : `options.touches_manette`,
  `keybindings.titre_manette`, libellés des dix `GamepadButton`, invite « manette non connectée ».

## Fichiers impactés
- `Source/HMI/Interface/GamepadBindingsModel.h`/`.cpp`, `GamepadBindingsScreen.h`/`.cpp`
  (nouveaux).
- `Source/HMI/Interface/OptionsModel.h`/`.cpp`, `OptionsScreen.cpp`.
- `Source/HMI/Interface/IScreen.h`, `ScreenManager.cpp`.
- `Source/HMI/main.cpp`.
- `Source/HMI/CMakeLists.txt`.
- `Source/Elements/Localization/fr.lang`/`en.lang`.
- Tests : `Source/Test/Unit/HMI/Interface/test_options_model.cpp` (mis à jour, 5 options),
  `test_gamepad_bindings_model.cpp` (nouveau).

## Tests (obligatoires)
- `OptionsModel` : les 5 options, la nouvelle action renvoyée correctement.
- `GamepadBindingsModel` : navigation identique au patron `GameKeybindingsModel` ; capture d'un
  bouton valide met à jour le binding ; sans manette connectée, la capture ne démarre pas (ou
  n'aboutit jamais) et l'état/l'invite le reflète ; « Réinitialiser »/« Retour » comme `LOT-29`.

## Points d'attention
- Le sous-menu manette ne peut pas se tester par simulation clavier seule pour la capture
  elle-même (`capturedGamepadButton` dépend de l'état manette de `InputState`) — les tests
  unitaires injectent directement `onGamepadButtonDown` sur un `InputState`, sans manette réelle
  (même principe que les tests existants de fusion manette, `LOT-20`).
- **Écart constaté, signalé par le demandeur** : passer `OptionsModel::OPTION_COUNT` de 4 à 5
  (`LOT-29` en avait déjà porté 2 à 4) a fait déborder le menu Options d'une fenêtre 720p — à
  720p, `MenuModel::OPTIONS_TOP`/`OPTION_SPACING` (pensés pour 2 à 4 lignes) ne font tenir que 4
  lignes sans défilement, la 5ᵉ (« Retour ») se retrouvant hors écran, inaccessible à la souris.
  Corrigé en dotant `OptionsModel`/`OptionsScreen` d'un vrai défilement (fenêtre visible + barre de
  défilement), sur le même patron que `LevelPicker` (liste plate, pas l'accordéon de
  `TilePalette`) : `visibleOptionCount`/`scrollOffset`, le clavier fait suivre la sélection dans la
  fenêtre visible, la molette défile sans la changer. `OptionsModel::update` gagne un paramètre
  `viewportHeight` (même principe que `LevelPicker::update`).

## Définition de fait (DoD)
- Sous-menu accessible, fonctionnel, persiste ses changements ; navigation complète testée
  manuellement si une manette est disponible ; build `/W4 /WX` sans avertissement.

## Exigences
`EX-CTRL-002`, `EX-CTRL-012`.
