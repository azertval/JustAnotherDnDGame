# TACHE-01 — Provisionnement Qt (local + CI/release) & intégration CMake ; `HMI` en bibliothèque {#lot-34-tache-01-provisionnement-qt-build}

**Lot :** [LOT-34](epic.md) · **Emplacement :** `External`, `Source/HMI`, `Source/Editor`, CMake, `.github/workflows` · **Statut :** non commencé

## Contexte
Le projet gère aujourd'hui ses dépendances par **`FetchContent`** (GoogleTest, nlohmann/json dans
`External/CMakeLists.txt`) — sans dépendance externe préinstallée. Qt **ne se prête pas** à
`FetchContent` (SDK volumineux, compilation longue) : il faut le **provisionner** (installer) puis le
localiser via `find_package(Qt6)`. Cette tâche pose ce socle de build **avant** toute ligne d'UI Qt :
intégration CMake, provisionnement local reproductible, provisionnement **CI** (job requis
`build-test-coverage`) et **release** (déploiement des DLL Qt), et scission de `HMI` en bibliothèque
réutilisable par la future cible Qt tout en gardant l'exécutable historique.

## Travail à réaliser

### 1. `HMI` : exécutable → bibliothèque + exécutable legacy conservé
- Dans `Source/HMI/CMakeLists.txt`, extraire le **rendu et la plateforme** réutilisables dans une
  **bibliothèque statique** (p. ex. `HmiRuntime`) : `GraphicsDevice`, `SpriteBatch`, `SpriteRenderer`,
  `TextureAtlas`, `Camera2D`, `BitmapFont`, `FlagIcons`, `SaveIcon`, `RoomGrid`, `TileVisuals`,
  `InputState`, `…KeyBindings`, `Localization`, `ExecutableDirectory`. Elle lie `Core`, `nlohmann_json`
  et `d3d11 dxgi d3dcompiler xinput`.
- L'exécutable **historique** `JustAnotherDnDGame` (`main.cpp`, `hmi::Window`, `ScreenManager`, `…Screen`)
  **reste** et lie désormais `HmiRuntime` — aucune régression de jeu (coexistence, cf. epic).
- Le copier-après-build des assets (`Localization`, `Levels`) est préservé (déplacé au besoin sur la
  cible qui produit l'exe final).

### 2. Intégration Qt dans CMake
- `find_package(Qt6 REQUIRED COMPONENTS Widgets Gui)` (dans `Source/Editor/CMakeLists.txt`).
- `set(CMAKE_AUTOMOC ON)`, `set(CMAKE_AUTOUIC ON)`, `set(CMAKE_AUTORCC ON)` **sur la cible Qt
  uniquement** (ne pas polluer `Core`/`HmiRuntime`).
- Documenter dans `External/README.md` la stratégie Qt (provisionné, non `FetchContent`) et la
  **licence LGPLv3 / lien dynamique**.
- La cible Qt `JustAnotherDnDGame` est déclarée ici (vide/minimale à cette tâche : un `main` Qt qui
  ouvre une fenêtre nue), remplie par TACHE-02/03/04.

### 3. Provisionnement local
- Documenter (guide de build) l'installation via **`aqtinstall`** (`pip install aqtinstall` puis
  `aqt install-qt windows desktop 6.8.1 win64_msvc2022_64 --outputdir C:\Qt`) **ou** l'installeur
  officiel, et le passage de `CMAKE_PREFIX_PATH`/`Qt6_DIR` au preset `vs` (variable d'environnement ou
  ajout ciblé au `CMakePresets.json`, sans casser un poste sans Qt tant que la cible Qt est optionnelle
  — voir Points d'attention).

### 4. Provisionnement CI (`.github/workflows/ci.yml`, job `build-test-coverage`)
- Ajouter **avant** `cmake --preset vs` une étape d'installation Qt, avec cache :
  ```yaml
  - name: Install Qt
    uses: jurplel/install-qt-action@v4
    with:
      version: '6.8.1'
      host: windows
      arch: win64_msvc2022_64
      cache: true
  ```
- L'action exporte `QT_ROOT_DIR` + `CMAKE_PREFIX_PATH`/PATH → `find_package(Qt6)` se résout dans le
  preset `vs`. Runner **`windows-2022`** conservé (générateur VS 2022, cf. contrainte projet).
- Les jobs `lint-exigences` et `docs` (Ubuntu) **ne changent pas** (aucun Qt).
- Vérifier que `ctest --preset vs` et la couverture (`UnitTests.exe`) restent verts : `UnitTests` **ne
  doit pas linker Qt** (logique pure + contrôleurs découplés), donc aucune DLL Qt requise à l'exécution
  des tests.

### 5. Provisionnement Release (`.github/workflows/release.yml`)
- La release produit un exe **autonome `/MTd`**. Qt (LGPL) impose le **lien dynamique** : ajouter une
  étape **`windeployqt`** (`windeployqt build\vs\bin\Debug\JustAnotherDnDGame.exe`) pour copier les
  DLL Qt (Core, Gui, Widgets, plugins `platforms/qwindows.dll`, styles) à côté de l'exe **avant** de
  packager l'archive `debug-latest`. Documenter que « exe autonome » devient « exe + DLL Qt ».
- (Le retrait de l'ancien exe et le passage de la release sur `JustAnotherDnDGame` se finalisent au
  [LOT-38](@ref lot-38), après parité ; à cette tâche, la release peut continuer à packager
  `JustAnotherDnDGame`.)

## Fichiers impactés
- `Source/HMI/CMakeLists.txt` (scission bibliothèque `HmiRuntime` + exe legacy).
- `Source/Editor/CMakeLists.txt` (nouveau : cible Qt, `find_package(Qt6)`, AUTOMOC/UIC/RCC).
- `Source/CMakeLists.txt` (`add_subdirectory(Editor)`).
- `External/README.md`, `CMakePresets.json` (doc/prefix path Qt).
- `.github/workflows/ci.yml` (étape Install Qt), `.github/workflows/release.yml` (`windeployqt`).
- `Documentation/Guide/` (guide de build : provisionnement Qt local).

## Tests (obligatoires)
- **Non-régression build** : `cmake --preset vs` + `cmake --build --preset vs` compilent `Core`,
  `HmiRuntime`, `JustAnotherDnDGame` (legacy), `JustAnotherDnDGame` (nu) et `UnitTests`.
- `ctest --preset vs` **vert** (suite existante inchangée) ; couverture générée comme avant.
- Vérifier en CI (sur la PR du lot) que l'étape Install Qt + cache fonctionne sur `windows-2022`.
- Aucune logique métier nouvelle ici → pas de test unitaire nouveau (tâche d'infrastructure) ; les
  tests de logique arrivent aux TACHE-03/04.

## Points d'attention
- **Ne pas casser un poste/CI sans Qt involontairement** : `find_package(Qt6 REQUIRED)` sur la cible
  Qt fait échouer la config si Qt est absent. Choix à assumer (Qt devient dépendance de build du
  dépôt) — le documenter clairement ; option de repli : rendre la cible Qt conditionnelle à une option
  CMake `BUILD_EDITOR_QT` (ON par défaut) le temps de la transition.
- **`/W4 /WX`** : isoler le code généré `moc`/`uic` (traité comme externe) ; les en-têtes Qt sont déjà
  couverts par `/external:anglebrackets /external:W0`. Vérifier qu'aucun warning n'échappe.
- **CRT** : Qt msvc2022_64 est en CRT dynamique ; l'exe historique release est `/MTd`. La cible Qt
  utilisera la CRT par défaut du preset — vérifier la cohérence au packaging (windeployqt gère les
  redistribuables).
- **Cache CI** : `cache: true` de l'action évite le re-téléchargement (~1–2 Go) ; clé implicite sur
  version/arch.

## Définition de fait (DoD)
- `HMI` scindé en bibliothèque `HmiRuntime` + exe legacy fonctionnel ; cible Qt `JustAnotherDnDGame`
  déclarée et configurable via `find_package(Qt6)`. CI (`build-test-coverage`) installe Qt et reste
  verte ; provisionnement local et release (`windeployqt`) **documentés**. `/W4 /WX`, Doxygen et lint
  verts.

## Exigences
`EX-BUILD-010` (Qt provisionné et documenté, local + CI + release) ; prépare `EX-IHM-001`/`EX-IHM-002`
(TACHE-02→04). Réutilise `EX-NFR-031` (dépendances épinglées) — Qt épinglé sur `6.8.1` (l'archi `win64_msvc2022_64` requiert Qt 6.8+ ; 6.7 n'expose que `win64_msvc2019_64`).
