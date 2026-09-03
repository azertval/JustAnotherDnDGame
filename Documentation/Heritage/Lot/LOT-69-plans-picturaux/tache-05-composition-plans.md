# TACHE-05 — Composition et rendu des plans {#lot-69-tache-05-composition-plans}

**Lot :** [LOT-69](epic.md) · **Emplacement :** `Source/HMI/Graphics` · **Statut :** fait

## Contexte
`hmi::RenderLayer` est déclaré comme **le seul ordonnancement de calques du projet** (`EX-REN-014`),
et son en-tête interdit explicitement d'en inventer un concurrent. Or le nombre de plans est
**libre** : leur donner une valeur d'énumération chacun figerait dans le rendu ce que le format
déclare variable, et ferait enfler `RENDER_LAYER_COUNT`, donc `LayerVisibility`.

L'échappement existe déjà : `ComposedScene::sort()` trie par calque, puis par rang de texture, puis
par `sortOrder` — ce dernier étant le tri **fin à l'intérieur** d'un calque, exactement l'usage que
faisait le rang d'un décor.

## Travail à réaliser
- `PlaneVisuals.{h,cpp}` (nouveau) : `planeRenderLayer(PlaneDepth)` — `Behind` vers le calque de
  décor, `Front` vers celui de premier plan ; `resolvePlaneTexture` avec repli damier
  (`EX-NFR-040`) ; et `composePlanes(...)`, fonction **pure** testable par `QuadRecorder`, qui émet
  **un quad par plan** couvrant `[0,0]`–`[width, height]` en unités monde, UV pleines, filtrage
  *nearest*.
- Renommer `RenderLayer::Decor` en `RenderLayer::Plane` et mettre à jour la documentation de
  `EX-REN-014`. Laisser un calque nommé « Decor » alors que le mot disparaît du projet serait
  exactement la dette que cette exigence cherche à éviter.
- `PlaneVisibility.h` (nouveau) : visibilité **par rang de plan**, non persistée, avec
  `isolate(rank)` et `showAll()`.
- Résolution disque → GPU aux côtés de celle du fond, avec cache par nom de fichier.
- Ne **rien** émettre en `RenderMode::Physique`, comme le fait déjà la composition du fond.

## Fichiers impactés
`Source/HMI/Graphics/{PlaneVisuals,PlaneVisibility}.{h,cpp}` (nouveaux), `RenderLayer.h`,
`LayerVisibility.h` (documentation), `ComposedScene.{h,cpp}`, `SpriteRenderer.{h,cpp}`,
`DraftRenderer.{h,cpp}`, `Source/HMI/CMakeLists.txt`.

## Tests (obligatoires)
- `test_plane_render.cpp` (nouveau) : **quatre plans composés dans le désordre ressortent dans
  l'ordre du niveau** ; profondeur → calque attendu ; quad couvrant exactement les bornes du niveau ;
  densité 8 → **même** quad et mêmes UV ; rien émis en `Physique` ; plan masqué n'émettant rien ;
  texture absente → damier.
- `test_layer_visibility.cpp` : adapté au renommage, comportement inchangé.

## Points d'attention
**Le piège d'ordre est le point dur de la tâche.** `ComposedScene::sort()` intercale le **rang de
première apparition de texture** entre le calque et le `sortOrder`. Chaque plan portant sa propre
texture, deux plans d'un même calque sont ordonnés par leur ordre de *première apparition*, non par
leur `sortOrder`. C'est correct **à condition** que les plans soient composés en premier et dans
l'ordre de la liste — acquis une fois les décors retirés, puisque le calque ne reçoit plus qu'eux.
Cette propriété est **invisible** à la lecture : sans le test qui la fige, elle se casserait
silencieusement au premier réordonnancement d'appels.

**Pas de découpage en tuiles de texture.** Le culling par boîte englobante n'écarterait aucun quad
— un plan couvre toujours le cadrage — le GPU clippe déjà à l'écran, et découper multiplierait quads
*et* textures liées, donc les passes. Le coût réel n'est pas la mémoire mais le **batch** : chaque
plan est une texture distincte, donc une passe de plus par image, jamais cullée. C'est ce chiffre que
`TACHE-09` doit figer.

Propriété utile à énoncer : le coût des plans est **constant en taille de niveau**, contrairement
aux tuiles.

## Definition de fait (DoD)
Les plans se composent dans l'ordonnancement unique, dans l'ordre déclaré, avec repli sur asset
manquant. `ctest` à 100 %.

## Exigences
`EX-REN-049`, `EX-REN-014`, `EX-DEC-040`, `EX-DEC-041`, `EX-DEC-042`, `EX-DEC-003`, `EX-NFR-040`,
`EX-NFR-005`.
