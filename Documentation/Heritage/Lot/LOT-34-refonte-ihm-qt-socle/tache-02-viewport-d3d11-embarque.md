# TACHE-02 — Fenêtre Qt + viewport `QWindow` embarqué → `GraphicsDevice` {#lot-34-tache-02-viewport-d3d11-embarque}

**Lot :** [LOT-34](epic.md) · **Emplacement :** `Source/Editor` · **Statut :** non commencé

## Contexte
`hmi::GraphicsDevice` s'initialise déjà à partir d'un **`HWND`** (`GraphicsDevice(HWND, width,
height)`, appelé aujourd'hui avec le HWND de `hmi::Window`, cf. `main.cpp:152`). Qt peut fournir un
HWND présentable via un **`QWindow`** natif intégré dans la hiérarchie de widgets par
`QWidget::createWindowContainer`. Cette tâche crée la fenêtre principale Qt et le **viewport** dans
lequel Direct3D 11 présentera, sans encore de boucle de jeu (TACHE-03) ni de contenu de niveau
(TACHE-04) : l'objectif est un `GraphicsDevice` vivant, relié au viewport, qui **efface l'écran** à
une couleur et se **redimensionne** proprement.

## Travail à réaliser
- **`GameViewport`** (`Source/Editor/GameViewport.{h,cpp}`) : sous-classe de `QWindow` (surface
  native, `setSurfaceType(QWindow::RawSurface)` — pas de backing store Qt, D3D11 dessine directement).
  - Expose son `HWND` via `reinterpret_cast<HWND>(winId())`.
  - Possède le `hmi::GraphicsDevice` (construit **après** que la fenêtre a un handle natif valide —
    au premier `exposeEvent`, pas dans le constructeur).
  - `resizeEvent(QResizeEvent*)` → `graphics.resize(width, height)` (en pixels physiques :
    `size() * devicePixelRatio()`), en tenant compte du **DPI**.
  - `exposeEvent(QExposeEvent*)` → (re)crée le device si besoin et déclenche un premier rendu.
  - À cette tâche, `render()` se contente de `graphics.clear(...)` + `graphics.present()` (fond de la
    couleur actuelle `0.10, 0.12, 0.16`), pour valider le pont.
- **`MainWindow`** (`Source/Editor/MainWindow.{h,cpp}`) : `QMainWindow` dont le **widget central** est
  `QWidget::createWindowContainer(gameViewport, this)`. Titre, taille par défaut 1280×720 (parité avec
  l'existant).
- **`main`** Qt (`Source/Editor/main_qt.cpp`) : `QApplication app(argc, argv);` → `MainWindow w;
  w.show();` → `return app.exec();`. Réutilise l'initialisation du logger de `main.cpp`
  (`MemoryLogSink`/`ConsoleLogSink`, niveau via env/arg) — factoriser si pertinent.

## Fichiers impactés
- `Source/Editor/GameViewport.{h,cpp}`, `MainWindow.{h,cpp}`, `main_qt.cpp` (nouveaux).
- `Source/Editor/CMakeLists.txt` (enregistrement des sources ; liaison `HmiRuntime`, `Qt6::Widgets`,
  `Qt6::Gui`).

## Tests (obligatoires)
- Intégration **GPU/fenêtre → vérification visuelle** (dépendance D3D11, comme les lots de rendu) :
  la fenêtre Qt s'ouvre et affiche un fond uni ; la redimensionner ne provoque ni étirement ni crash.
- Logique testable isolable : la **conversion taille logique ↔ pixels physiques (DPI)** peut être
  extraite en fonction pure et couverte par un test unitaire sans Qt/GPU.

## Points d'attention
- **Ordre de création** : ne créer `GraphicsDevice` qu'une fois `winId()` valide (premier
  `exposeEvent`), sinon HWND invalide. Gérer la fermeture (destruction ordonnée : device avant
  fenêtre).
- **DPI / `devicePixelRatio`** : la swap chain se dimensionne en **pixels physiques** ; ne pas
  confondre avec les points logiques Qt, sous peine de rendu flou ou tronqué.
- **`createWindowContainer`** : le conteneur est une fenêtre native « étrangère » ; attention à
  l'ordre de rendu/superposition si des widgets Qt le recouvrent (pertinent dès le LOT-35 pour les
  overlays — ici le viewport est seul).
- **Flip model** : `GraphicsDevice` dé-lie la RTV à `Present` et la relie à `clear()` (déjà géré,
  `LOT-33`) — aucun changement, mais le vérifier avec la nouvelle source de fenêtre.

## Définition de fait (DoD)
- Une fenêtre Qt affiche un viewport D3D11 qui s'efface et se présente ; redimensionnement propre,
  fermeture propre. `/W4 /WX` sans warning ; vérification visuelle OK.

## Exigences
`EX-IHM-002` (rendu D3D11 embarqué dans un viewport Qt) ; réutilise `EX-REN-004` (flip model). Prépare
TACHE-03 (boucle/entrées).
