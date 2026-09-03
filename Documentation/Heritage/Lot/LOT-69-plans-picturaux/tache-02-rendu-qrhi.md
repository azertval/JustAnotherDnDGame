# TACHE-02 — Rendu sur QRhi et fin de la fenêtre native {#lot-69-tache-02-rendu-qrhi}

**Lot :** [LOT-69](epic.md) · **Emplacement :** `Source/HMI/{Graphics,Game,Interface}` ·
**Statut :** fait

## Contexte
`hmi::GameViewport` est une **`QWindow` native** embarquée par `QWidget::createWindowContainer`
(`Source/HMI/Interface/MainWindow.cpp`). Le commentaire qui accompagne l'écran de pause y documente
**deux défauts réels** payés en `LOT-59` : un widget Qt frère du conteneur natif ne se dessine jamais
de façon fiable **par-dessus** la fenêtre native, quel que soit son `raise()` — l'écran de pause ne
s'affichait pas, la simulation restait figée sans rien à l'écran ; puis le contournement `Qt::Tool` a
cassé `activateWindow()` sur Windows (`SW_SHOWNOACTIVATE`), Entrée et Échap restant sans effet. Le
contournement en vigueur est une fenêtre de **haut niveau** `Qt::Dialog` dont la géométrie est
synchronisée en coordonnées **écran** (`MainWindow::syncOverlayGeometry`).

Le mode création (`TACHE-07`) a besoin de repères **au-dessus** du canevas. Les bâtir sur une fenêtre
native reproduirait cette classe de défauts. `QRhiWidget` (Qt 6.7+) rend dans une **texture d'appui
composée avec le reste de l'interface** : l'empilement redevient celui, ordinaire, des widgets.

La surface Direct3D 11 réelle est bien plus petite qu'il n'y paraît. Sur les 17 fichiers qui
mentionnent D3D11/DXGI, le code GPU véritable tient dans deux : `SpriteBatch.cpp` (buffers, shaders,
états) et `GraphicsDevice.cpp` (device, swap chain, présentation) — de l'ordre de **700 lignes**,
dont une partie **disparaît** puisque Qt possède désormais la cible de rendu. `ComposedScene` et
`DraftRenderer`, qui font le vrai travail de composition, sont **déjà sans GPU** et testables sans
carte graphique (`TextureHandle` est un `void*` opaque) : ce découplage, fait de longue date, est ce
qui rend le portage tractable.

## Travail à réaliser
- Porter `hmi::SpriteBatch` sur **QRhi** : buffers, pipeline, échantillonneur, et shaders recompilés
  en `.qsb` via `qsb` (module `qtshadertools`). L'échantillonnage reste **`Nearest`** et le zoom
  entier (`EX-ARCH-022`).
- **Supprimer** `hmi::GraphicsDevice` : il ne portait qu'un device, une swap chain et une
  présentation dont Qt se charge. Ne pas le porter — le retirer.
- Faire de `GameViewport` un **`QRhiWidget`** (`EX-REN-050`), et retirer `createWindowContainer` de
  `MainWindow`.
- **Ramener `PauseScreen` au rang de widget enfant ordinaire** et supprimer
  `MainWindow::syncOverlayGeometry` ainsi que la synchronisation en coordonnées écran. C'est le
  bénéfice concret de la tâche : la dette du `LOT-59` s'efface au lieu de s'étendre.
- Ajouter `ShaderTools` (et `CanvasPainter` si `TACHE-05` le consomme) aux composants
  `find_package(Qt6 …)`, maintenant qu'ils ont un consommateur.

## Fichiers impactés
`Source/HMI/Graphics/{SpriteBatch,GraphicsDevice,SpriteRenderer,TextureLoader,TextureCache,TextureAtlas,BitmapFont}.{h,cpp}`,
`Source/HMI/Game/GameViewport.{h,cpp}`, `Source/HMI/Interface/MainWindow.{h,cpp}`,
`Source/HMI/CMakeLists.txt`, shaders (nouveaux `.vert`/`.frag` + `.qsb`).

## Tests (obligatoires)
- **Non-régression visuelle** : rendu **hors écran** d'un niveau témoin, comparé à une image de
  référence, pour figer la netteté *nearest* et le zoom entier. C'est le seul test qui protège
  vraiment l'objectif du portage ; sans lui, un rendu flou passerait toute la CI.
- Les tests existants sans GPU (`ComposedScene`, `QuadRecorder`, budget, culling) doivent passer
  **sans retouche** : ils prouvent que le portage n'a pas déplacé la frontière testable.

## Points d'attention
`EX-REN-002` reste satisfaite : QRhi retient **Direct3D 11** par défaut sous Windows. L'exigence est
amendée pour dire *au travers de QRhi* plutôt que *en appelant l'API directement* ; la cible
technique ne change pas.

Le risque dominant du lot est ici : **un portage qui rendrait le pixel art flou est un échec**,
quelle que soit la qualité du reste. Traiter le filtrage et l'arrondi de zoom comme des invariants
vérifiés, pas comme des détails d'implémentation.

`QRhiWidget` peut perdre ses ressources graphiques quand le widget change de fenêtre de haut niveau :
prévoir la recréation plutôt que la supposer impossible.

La vérification du rendu à l'écran reste **manuelle** (`EX-NFR-004` borne ce qui est automatisable) :
netteté, absence de tearing, et le retour à un écran de pause qui s'affiche et prend le focus.

## Definition de fait (DoD)
Plus une ligne de Direct3D 11 appelée directement, `GraphicsDevice` supprimé, `syncOverlayGeometry`
supprimé, `ctest` à 100 %, et le test de non-régression visuelle vert.

## Exigences
`EX-REN-050`, `EX-REN-002` (amendée), `EX-ARCH-022`, `EX-NFR-004`, `EX-NFR-041`.
