# LOT-34 — Refonte IHM (Qt) : socle applicatif & viewport D3D11 embarqué {#lot-34}

> Statut : **implémenté** (build/tests verts ; vérification IHM manuelle en cours).

## Programme de refonte de l'IHM (LOT-34 → LOT-39)

Ce lot ouvre un **programme pluri-lots** de refonte de l'interface. L'éditeur et l'UI hors-jeu
actuels sont dessinés **entièrement à la main** (quads `SpriteBatch` + police bitmap procédurale,
**aucune bibliothèque d'UI**, **aucune primitive de ligne**). Cela rend illisibles les liaisons
d'interrupteurs (indiquées par simple teinte de case), fragile la liste des niveaux, pénible
l'arrivée des textures, et coûteux à maintenir (`EditorScreen` ≈ 1200 lignes mêlant interaction et
dessin bas niveau).

**Décision structurante (tranchée avec le demandeur)** : migrer **toute l'UI hors-jeu** (éditeur,
menus, options, remappage) vers **Qt** (application desktop, docking `QDockWidget`, layouts réglables
hors code). **Seul le rendu in-game reste DirectX 11**, embarqué dans un viewport Qt. `Core` (modèle,
validation, ECS, physique) **n'est pas touché** : toute la logique d'édition est déjà isolée dans
`core::LevelDraft` (pur, testé) et le format dans `LevelLoader`/`LevelWriter`.

| Lot | Objet | Résout |
|-----|-------|--------|
| **LOT-34** (ce lot) | Socle Qt + viewport D3D11 embarqué, boucle & entrées | Fondation technique ; jeu jouable dans une fenêtre Qt |
| [LOT-35](@ref lot-35) | Éditeur Qt : docking, palette, outils, peinture | Maintenabilité, fenêtres réglables |
| [LOT-36](@ref lot-36) | Gestion des niveaux (liste, recherche, fichiers) | « La liste des niveaux va devenir illisible » |
| [LOT-37](@ref lot-37) | Liens de mécanismes visuels (traits/flèches) | « Le lien des interrupteurs est complexe » |
| [LOT-38](@ref lot-38) | Menus & Options Qt, retrait du legacy UI | Cohérence, une seule techno UI |
| [LOT-39](@ref lot-39) | Textures depuis fichiers (loader + assets) | « Quand j'implémenterai des textures, ce sera compliqué » |

Chaque lot est **livré et vérifié indépendamment** (une PR chacun) ; l'ancien exécutable reste
fonctionnel en parallèle jusqu'à parité (retrait au LOT-38).

## Objectif
Poser la **fondation technique** : une application Qt (`QApplication` + `QMainWindow`) qui possède la
fenêtre principale et la boucle d'événements, dans laquelle le **rendu Direct3D 11 existant est
embarqué** via un `QWindow` natif. À la fin du lot, un niveau se **charge, s'affiche et se joue** dans
la fenêtre Qt, avec exactement le même rendu, le même déterminisme et le même ressenti d'entrées
qu'aujourd'hui — **sans aucune interface d'édition** (introduite au LOT-35). C'est le lot qui dérisque
l'intégration Qt ↔ D3D11 avant d'y bâtir l'IHM.

## Périmètre

### Inclus
- **`HMI` devient une bibliothèque de rendu** (au lieu de l'exécutable) : `GraphicsDevice`,
  `SpriteBatch`, `SpriteRenderer`, `TextureAtlas`, `Camera2D`, `TileVisuals`, `RoomGrid`,
  `BitmapFont`, `FlagIcons` restent tels quels et sont liés par la nouvelle cible.
- **Nouvelle cible exécutable Qt** `JustAnotherDnDGame` (`Source/Editor/`) : `QApplication`,
  `QMainWindow`, un `QWindow` viewport embarqué via `QWidget::createWindowContainer`.
- **Pont D3D11 ↔ Qt** : le `winId()` (HWND) du `QWindow` est passé à `hmi::GraphicsDevice(HWND, w, h)`
  (signature déjà existante) ; `resizeEvent`/`exposeEvent` du viewport → `graphics.resize(...)`.
- **Boucle de rendu pilotée par Qt** rejouant `core::FixedTimestep` : un tick
  (`QWindow::requestUpdate` ou `QTimer`) mesure le temps réel écoulé, appelle `timestep.advance`,
  exécute les pas, appelle `beginInputFrame()` **par pas consommé** (discipline `LOT-33` préservée),
  puis présente une fois par frame avec `interpolationAlpha` (`EX-ARCH-031`).
- **Entrées Qt → `hmi::InputState`** : traduction des événements clavier/souris Qt du viewport vers
  l'état d'entrée existant ; **relâchement global à la perte de focus** (`focusOut`, `EX-CTRL` du
  `LOT-33`). **XInput manette conservé** (sondage dans le tick, throttling `LOT-33` réutilisé).
- **Build** : `find_package(Qt6 COMPONENTS Widgets Gui)`, `CMAKE_AUTOMOC/AUTOUIC/AUTORCC ON` ;
  provisionnement Qt documenté (local + CI, voir Décisions). `HMI` et `Editor` lient `Core`.
- **Coexistence** : l'ancien exécutable `JustAnotherDnDGame` (boucle `main.cpp` + `hmi::Window`) **reste
  compilé et jouable** — aucune régression de jeu pendant toute la migration.
- Documentation (guide d'architecture IHM, provisionnement Qt), tests de la logique nouvelle
  découplée de Qt.

### Exclus (hors périmètre de ce lot)
- **Toute UI d'édition** (palette, outils, docks, peinture) — LOT-35. Ce lot n'affiche que le jeu.
- **Menus / Options / remappage en Qt** — LOT-38 ; l'ancien exécutable les fournit encore.
- **Retrait de `hmi::Window` / de l'ancienne boucle** — reporté au LOT-38 (après parité), pour
  garder un filet de sécurité.
- **Textures depuis fichiers** — LOT-39 ; le `TextureAtlas` procédural reste la source d'images.
- **Portage du rendu du jeu vers une autre API** — le rendu in-game **reste D3D11**, inchangé.

## Décisions de cadrage
- **Qt possède la boucle, D3D11 rend dans un `QWindow` natif** : approche standard des éditeurs de
  jeux — `createWindowContainer` fournit un HWND présentable sans second processus ni fenêtre
  séparée. Alternative écartée : éditeur externe (autre techno) → duplication du rendu d'aperçu.
- **`HMI` scindé en bibliothèque de rendu réutilisable** plutôt que dupliqué : la nouvelle cible Qt
  et l'ancien exécutable partagent le **même** pipeline D3D11, garantissant un rendu identique
  pendant la coexistence.
- **Le déterminisme de la simulation est un invariant** : la boucle Qt reproduit **exactement** la
  discipline pas-fixe / `beginInputFrame` du `LOT-33` (`EX-NFR-002`, `EX-CTRL-020`, `EX-CTRL-021`).
  Qt ne fait que remplacer la source du temps et des événements, jamais la logique.
- **Provisionnement Qt via `aqtinstall`** (local et CI) plutôt que l'installeur GUI officiel ou une
  compilation `vcpkg` (longue) : scriptable, versionné, reproductible sur les runners
  `windows-2022`. Qt 6 en **LGPLv3, lien dynamique** (aucune obligation de publication du source du
  jeu). À documenter dans `External/README.md` et le workflow CI. En **CI** (`ci.yml`, job
  `build-test-coverage`), une étape `jurplel/install-qt-action@v4` (avec `cache: true`) installe Qt
  avant `cmake --preset vs` et exporte `CMAKE_PREFIX_PATH` pour `find_package(Qt6)` ; les jobs
  `lint-exigences` et `docs` (Ubuntu) restent inchangés.
- **Release : l'exe cesse d'être « autonome » au sens strict** (`release.yml`). Le lien dynamique Qt
  impose de **déployer les DLL Qt** à côté du binaire via **`windeployqt`** (Core/Gui/Widgets +
  plugin `platforms/qwindows.dll`) avant de packager l'archive `debug-latest`. À anticiper : c'est le
  changement le plus visible par rapport au modèle `FetchContent` header-only. Le basculement effectif
  de la release sur `JustAnotherDnDGame` (et le retrait de l'ancien exe) est finalisé au
  [LOT-38](@ref lot-38).
- **Coexistence des deux exécutables jusqu'à parité** : ne jamais laisser le jeu injouable entre deux
  lots ; l'ancien chemin est retiré seulement quand le nouveau couvre tout (LOT-38).

## Exigences couvertes
- Nouvelles : `EX-IHM-001` (l'UI hors-jeu repose sur Qt), `EX-IHM-002` (rendu in-game D3D11 embarqué
  dans un viewport Qt), `EX-BUILD-010` (dépendance Qt provisionnée et documentée, local + CI).
- Réutilisées (inchangées) : `EX-NFR-002` (déterminisme), `EX-CTRL-020`/`EX-CTRL-021` (latence et
  échantillonnage des entrées), `EX-ARCH-031` (interpolation de rendu), `EX-REN-004` (présentation
  flip), `EX-NFR-010`/`EX-ARCH-011`/`EX-ARCH-012` (frontière `HMI → Core`, `Core` intact).

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé. Les tâches seront détaillées à l'ouverture du lot.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-provisionnement-qt-build.md) | Provisionnement Qt (local + CI + release) et intégration CMake ; `HMI` en bibliothèque | `External`, `Source/HMI`, CMake, CI | ✅ |
| [TACHE-02](tache-02-viewport-d3d11-embarque.md) | Fenêtre Qt + viewport `QWindow` embarqué → `GraphicsDevice` (HWND, resize) | `Source/Editor` | ✅ |
| [TACHE-03](tache-03-boucle-entrees-qt.md) | Boucle de rendu Qt (pas fixe, interpolation) + entrées Qt → `InputState` + focus + XInput | `Source/Editor` | ✅ |
| [TACHE-04](tache-04-niveau-jouable-doc.md) | Chargement/affichage/jeu d'un niveau dans le viewport ; documentation & vérification | `Source/Editor`, `Documentation` | ✅ |

## Critères d'acceptation du lot
1. `cmake --preset vs` configure et compile la nouvelle cible `JustAnotherDnDGame` **et** l'ancien
   `JustAnotherDnDGame`, Qt6 étant provisionné localement ; le provisionnement est **documenté et
   reproductible** en CI (`windows-2022`).
2. La fenêtre Qt affiche un niveau chargé **au rendu D3D11 identique** à l'exécutable historique
   (mêmes tuiles, même caméra, même interpolation).
3. Le niveau est **jouable** dans la fenêtre Qt : déplacement/saut/dash au clavier et à la manette,
   avec le **même ressenti d'entrées** qu'aujourd'hui (aucun appui avalé à haut framerate).
4. Redimensionner la fenêtre Qt redimensionne proprement la swap chain (pas d'étirement, pas de
   crash) ; `Alt+Tab` ne laisse aucune touche « collée ».
5. La simulation reste **strictement déterministe** : la suite de tests passe à 100 % sans
   changement de résultat (`EX-NFR-002`).
6. Build `/W4 /WX` sans avertissement (code généré `moc` isolé, en-têtes Qt traités en externe),
   Doxygen (binaire CI 1.9.8 vérifié localement) et lint des exigences verts. La logique nouvelle
   découplée de Qt (traduction d'entrées, cadence) est **couverte par des tests** ; l'intégration
   Qt/D3D11 (dépendance GPU) est **vérifiée visuellement**, comme les autres lots de rendu.

## Dépendances
- Réutilise `hmi::GraphicsDevice`/`SpriteBatch`/`SpriteRenderer`/`Camera2D` (`LOT-01`/`05`),
  `core::FixedTimestep` + `interpolationAlpha` (`LOT-01`/`33`), la discipline d'entrées du `LOT-33`,
  et `core::LevelLoader`/`LevelScene` (`LOT-07`…). Introduit la dépendance externe **Qt6**.
- **Prérequis de tous les lots suivants** (LOT-35 → LOT-39) : rien ne peut être bâti dans l'IHM Qt
  tant que ce socle n'est pas livré.

## Navigation des tâches
- @subpage lot-34-tache-01-provisionnement-qt-build
- @subpage lot-34-tache-02-viewport-d3d11-embarque
- @subpage lot-34-tache-03-boucle-entrees-qt
- @subpage lot-34-tache-04-niveau-jouable-doc
