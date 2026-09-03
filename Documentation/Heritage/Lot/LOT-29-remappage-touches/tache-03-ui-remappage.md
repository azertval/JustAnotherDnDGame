# TACHE-03 — UI de remappage et câblage {#lot-29-tache-03-ui-remappage}

**Lot :** [LOT-29](epic.md) · **Emplacement :** `HMI/Interface`, `HMI/main.cpp` · **Statut :** ✅

## Contexte
Rend le remappage accessible depuis le menu Options : deux nouveaux écrans listant respectivement
les actions de jeu et d'éditeur, sur le même patron que `OptionsScreen`/`OptionsModel` (liste
navigable au clavier/souris, action confirmée par Entrée/clic).

## Travail à réaliser
- **`Source/HMI/Interface/GameKeybindingsModel.h`/`.cpp`** (nouveau) : logique testable, patron
  `OptionsModel` — liste de 8 lignes (6 actions + « Réinitialiser » + « Retour »), navigation
  Haut/Bas/survol/clic (réutilise les constantes publiques de `MenuModel`, comme `OptionsModel`).
  Sélectionner puis confirmer une ligne d'action bascule un état `_capturing` (aucune navigation
  tant qu'actif) ; la frame suivante consulte `KeyName::capturedKey`, applique
  `GameKeyBindings::setKey` (échange géré par la classe elle-même, TACHE-01), déclenche
  `save(path)`, sort de la capture. La ligne « Réinitialiser » appelle `resetToDefaults()` +
  `save`. La ligne « Retour » produit une action de sortie (interprétée par l'écran comme
  `ScreenTransition::switchTo(ScreenId::Options)`).
- **`Source/HMI/Interface/GameKeybindingsScreen.h`/`.cpp`** (nouveau) : habillage `IScreen` du
  modèle ci-dessus (patron `OptionsScreen`) — dessine le titre, les 8 lignes (celle sélectionnée
  en évidence, la touche actuelle affichée via `keyDisplayName`), et le texte « Appuyez sur une
  touche… » pendant la capture.
- **`Source/HMI/Interface/EditorKeybindingsModel.h`/`.cpp`** et
  **`EditorKeybindingsScreen.h`/`.cpp`** (nouveaux) : même patron, 11 lignes (9 actions +
  Réinitialiser + Retour), sur `EditorKeyBindings`.
- **`Source/HMI/Interface/OptionsModel.h`/`.cpp`** : `OPTION_COUNT` `2` → `4` ; deux nouvelles
  valeurs `OptionsAction::OpenGameKeybindings`/`OpenEditorKeybindings` ; `actionFor`/`optionLabel`
  mis à jour (ordre proposé : V-Sync, Touches de jeu, Touches de l'éditeur, Retour).
- **`Source/HMI/Interface/OptionsScreen.cpp`** : les deux nouvelles actions produisent
  `ScreenTransition::switchTo(ScreenId::GameKeybindings)`/`switchTo(ScreenId::EditorKeybindings)`.
- **`Source/HMI/Interface/IScreen.h`** : `ScreenId` gagne `GameKeybindings`, `EditorKeybindings`.
- **`Source/HMI/Interface/ScreenManager.cpp`** : `screenName()` gagne les deux nouveaux cas.
- **`Source/HMI/main.cpp`** : construit `hmi::GameKeyBindings gameBindings =
  hmi::GameKeyBindings::load(settingsPath)` et `hmi::EditorKeyBindings editorBindings =
  hmi::EditorKeyBindings::load(settingsPath)` une fois au démarrage (à côté de `localization`),
  capturées par référence dans la fabrique d'écrans ; nouveaux `case` pour
  `ScreenId::GameKeybindings`/`EditorKeybindings` ; les `case Game`/`Editor` existants passent
  désormais `gameBindings`/`editorBindings` (branchement TACHE-02 finalisé ici si pas déjà fait).
- **Localisation** (`Source/Elements/Localization/fr.lang`/`en.lang`) : `options.touches_jeu`,
  `options.touches_editeur`, `keybindings.titre_jeu`, `keybindings.titre_editeur`,
  `keybindings.reinitialiser`, `keybindings.appuyez_touche`, libellés des 6 + 9 actions.

## Fichiers impactés
- `Source/HMI/Interface/GameKeybindingsModel.h`/`.cpp`,
  `GameKeybindingsScreen.h`/`.cpp` (nouveaux).
- `Source/HMI/Interface/EditorKeybindingsModel.h`/`.cpp`,
  `EditorKeybindingsScreen.h`/`.cpp` (nouveaux).
- `Source/HMI/Interface/OptionsModel.h`/`.cpp`, `OptionsScreen.cpp`.
- `Source/HMI/Interface/IScreen.h`, `ScreenManager.cpp`.
- `Source/HMI/main.cpp`.
- `Source/HMI/CMakeLists.txt` (nouveaux fichiers source).
- `Source/Elements/Localization/fr.lang`/`en.lang`.
- Tests : `Source/Test/Unit/HMI/Interface/test_options_model.cpp` (mis à jour),
  `test_game_keybindings_model.cpp`, `test_editor_keybindings_model.cpp` (nouveaux).

## Tests (obligatoires)
- `OptionsModel` : les 4 options, navigation, et les deux nouvelles actions renvoyées
  correctement.
- `GameKeybindingsModel`/`EditorKeybindingsModel` : navigation identique au patron `OptionsModel` ;
  confirmer une action entre en capture ; une touche capturée valide met à jour le binding
  correspondant et sort de la capture ; `Échap`/`Entrée` pendant la capture ne modifient rien et la
  laissent ouverte (sauf si le modèle choisit `Échap` = annuler la capture sans modifier — à
  trancher en implémentant, cohérent avec la convention « Échap annule » déjà en place ailleurs
  dans l'éditeur, ex. `TextInputField`) ; « Réinitialiser » restaure les défauts ; « Retour »
  renvoie l'action de sortie.
- Un test bout-en-bout léger (ou revue manuelle documentée ici) : ouvrir Options → Touches de jeu
  → remapper Sauter → Retour → Retour → Jouer, confirme que la nouvelle touche saute bien.

## Points d'attention
- **`Échap` pendant la capture, tranché** : annule la capture (binding inchangé), géré directement
  dans `GameKeybindingsModel`/`EditorKeybindingsModel::update` (avant même d'appeler
  `capturedKey`, qui reste inchangée : elle ignore `Échap`/`Entrée` comme touche **cible**, ce qui
  est un besoin distinct). Cohérent avec `TextInputField` (`Échap` = annuler une saisie en cours).
- **Pas de sortie par `Échap` hors capture** : contrairement à `MenuScreen`/`GameScreen`/
  `EditorScreen`, les deux nouveaux écrans (comme `OptionsScreen`, déjà ainsi) ne quittent que par
  la ligne « Retour » sélectionnée + validée — `OptionsModel` ne gère déjà pas `Échap` non plus,
  décision reconduite à l'identique plutôt que réinventée.
- **Mise en page dédiée**, pas celle de `MenuModel` : `MenuModel::OPTIONS_TOP`/`OPTION_SPACING`
  (pensés pour 2 à 4 lignes) déborderait largement une fenêtre 720p avec 8 ou 11 lignes —
  `GameKeybindingsModel` définit ses propres constantes compactes (`ROWS_TOP=110`,
  `ROW_SPACING=40`, `ROW_SCALE=2.4`), réutilisées telles quelles par `EditorKeybindingsModel`
  (même principe qu'`OptionsModel` réutilisant `MenuModel::MARGIN_X`). Seul `MenuModel::MARGIN_X`
  (alignement horizontal) est repris tel quel des deux côtés.
- Les deux nouveaux écrans ne dessinent **pas** le bouton de langue (`LanguageSelector`) —
  contrairement à `OptionsScreen`, ce ne sont pas des écrans de premier niveau.
- Écart de build `nlohmann_json`/cible `JustAnotherDnDGame` : voir TACHE-02 (constaté au premier build
  complet incluant `main.cpp`, pas spécifique à cette tâche).

## Définition de fait (DoD)
- Les deux sous-menus sont accessibles, fonctionnels, et persistent leurs changements ; navigation
  complète Options → sous-menu → remap → retour testée manuellement ; build `/W4 /WX` sans
  avertissement.

## Exigences
`EX-CTRL-012`.
