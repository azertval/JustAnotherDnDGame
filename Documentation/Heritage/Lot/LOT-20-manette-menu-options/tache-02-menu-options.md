# TACHE-02 — Menu d'options {#lot-20-tache-02-menu-options}

**Lot :** [LOT-20](epic.md) · **Emplacement :** `HMI/Interface`, `HMI/Graphics` · **Statut :** fait

## Contexte
Aucun écran ne centralise de réglages : la langue se change via une icône en coin du seul menu
principal, et le V-Sync est fixé en dur (`GraphicsDevice::present`, `Present(1, 0)`). Ce lot ajoute
un écran d'options (`ScreenId::Options`), accessible comme 4ᵉ option du menu principal.

## Travail à réaliser
- **`hmi::GraphicsDevice`** (`Source/HMI/Graphics/GraphicsDevice.h`/`.cpp`) : nouveau membre
  `bool _vsyncEnabled = true;`, accesseurs `bool vsyncEnabled() const` / `void
  setVSyncEnabled(bool enabled)` ; `present()` utilise `_swapChain->Present(_vsyncEnabled ? 1 : 0,
  0)` au lieu de la valeur `1` figée.
- **`hmi::ScreenId`** (`Source/HMI/Interface/IScreen.h`) : nouvelle valeur `Options`.
- **`hmi::OptionsAction`** (nouvel enum, `Source/HMI/Interface/OptionsModel.h`) : `ToggleVSync`,
  `Back`.
- **`hmi::OptionsModel`** (nouveau, `.h`/`.cpp`, logique **pure**, sur le modèle exact de
  `MenuModel`) :
  - `OPTION_COUNT = 2` : V-Sync (bascule), Retour.
  - Constructeur `OptionsModel(const Localization& localization, bool vsyncEnabled)` — snapshot de
    l'état V-Sync courant, pour afficher le bon libellé (« Activé »/« Désactivé ») dès la première
    frame.
  - `std::optional<OptionsAction> update(const InputState& input)` : navigation clavier/manette
    (`↑`/`↓`, fusionnés avec la manette via `InputState`, TACHE-01) et souris (survol + clic,
    `optionAtPoint`, réutilise `MenuModel::MARGIN_X`/`OPTIONS_TOP`/`OPTION_SPACING`/
    `OPTION_SCALE` — pas de constantes dupliquées) ; renvoie l'action de l'option confirmée.
  - `void setVSyncEnabled(bool enabled)` — resynchronise le libellé affiché juste après une
    bascule (l'écran reste le même objet le temps de la visite, pas de reconstruction).
  - `optionLabel(int index)`, `optionWidth(int index)`, `selectedIndex()` — mêmes signatures que
    `MenuModel`.
- **`hmi::OptionsScreen`** (nouveau, `.h`/`.cpp`, implémente `IScreen`) :
  - Construit avec `Localization&`, `GraphicsDevice&`.
  - `update` : délègue à `OptionsModel::update` ; sur `ToggleVSync`, appelle
    `_graphics.setVSyncEnabled(!_graphics.vsyncEnabled())` puis `_model.setVSyncEnabled(...)` avec
    la nouvelle valeur, reste sur l'écran (`ScreenTransition::none()`) ; sur `Back`, retourne
    `ScreenTransition::switchTo(ScreenId::Menu)`. Traite aussi le bouton de langue en coin
    (`hmi::LanguageSelector`, réutilisé **tel quel**, même geste que `MenuScreen`) et capture
    `input.gamepadConnected()` dans un membre (pour l'affichage informatif, `render` ne reçoit pas
    `InputState`).
  - `render` : titre, les deux options (surbrillance de la sélection, même style que
    `MenuScreen`), une ligne d'état « Manette : connectée »/« non connectée » (`options.manette_*`),
    le bouton de langue en coin.
- **`hmi::MenuModel`** : `OPTION_COUNT` passe à 4 ; nouvelle option "Options" (clé
  `menu.options`) menant à `ScreenTransition::switchTo(ScreenId::Options)`.
- **`Source/HMI/main.cpp`** : construit `OptionsScreen` dans la fabrique d'écrans (capture
  `graphics`, déjà utilisée par ailleurs dans la boucle principale, par référence).
- **Catalogues de traduction** (`Source/Elements/Localization/fr.lang`, `en.lang`) : nouvelles
  clés `menu.options`, `options.titre`, `options.vsync_on`, `options.vsync_off`,
  `options.retour`, `options.manette_connectee`, `options.manette_absente`.

## Fichiers impactés
- `Source/HMI/Graphics/GraphicsDevice.h`/`.cpp`.
- `Source/HMI/Interface/IScreen.h`.
- `Source/HMI/Interface/OptionsModel.h`/`.cpp` (nouveau).
- `Source/HMI/Interface/OptionsScreen.h`/`.cpp` (nouveau).
- `Source/HMI/Interface/MenuModel.h`/`.cpp`.
- `Source/HMI/main.cpp`.
- `Source/HMI/CMakeLists.txt` (nouveaux fichiers).
- `Source/Elements/Localization/fr.lang`, `en.lang`.
- Tests : `Source/Test/Unit/HMI/Interface/test_options_model.cpp` (nouveau),
  `test_menu_model.cpp` (option supplémentaire).

## Tests (obligatoires)
- `OptionsModel` : navigation clavier (`↑`/`↓` avec bouclage), confirmation (`Entrée`) renvoie
  `ToggleVSync` pour l'option 0 et `Back` pour l'option 1 ; survol/clic souris équivalents
  (`optionAtPoint`) ; `setVSyncEnabled` change le libellé retourné par `optionLabel(0)`.
- `MenuModel` : la 4ᵉ option (« Options ») produit `ScreenTransition::switchTo(ScreenId::Options)`
  une fois confirmée ; les tests existants (3 options) sont mis à jour pour `OPTION_COUNT = 4`
  sans changer leur intention.
- `GraphicsDevice` non testé unitairement (GPU requis, comme le reste de `HMI/Graphics`) —
  vérification par usage direct dans l'application (TACHE-03).

## Points d'attention
- **Ne pas dupliquer `MARGIN_X`/`OPTIONS_TOP`/`OPTION_SPACING`/`OPTION_SCALE`** : `OptionsModel`
  réutilise directement les constantes publiques de `MenuModel` (déjà `static constexpr`,
  accessibles depuis n'importe quelle classe) — un seul jeu de constantes de mise en page pour
  tous les écrans à liste verticale (menu principal, options).
- **`OptionsScreen` est reconstruit à chaque visite** (`ScreenManager` détruit l'écran sortant à
  chaque transition, comme documenté pour `EditorScreen`/LOT-15) : le snapshot V-Sync au
  constructeur (`_graphics.vsyncEnabled()`) est donc **toujours** à jour à l'entrée sur l'écran,
  sans mécanisme de synchronisation supplémentaire à maintenir.
- Le bouton de langue en coin (`LanguageSelector`) est un **second** moyen de changer la langue,
  redondant avec l'icône déjà présente sur le menu principal — c'est voulu (cohérence visuelle,
  le réglage doit être trouvable depuis l'écran qui en parle) plutôt qu'une régression à corriger.

## Définition de fait (DoD)
- Écran d'options fonctionnel et testé (`ctest` vert) ; build `/W4 /WX` sans avertissement ;
  Doxygen à jour ; catalogues FR/EN à jour.

## Exigences
Aucune nouvelle exigence propre — réutilise `EX-REN-022` (V-Sync activable), `EX-REN-033`
(catalogue de traduction).
