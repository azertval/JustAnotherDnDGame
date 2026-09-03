# TACHE-01 — Silhouette du personnage dans l'atlas {#lot-17-tache-01-silhouette-personnage}

**Lot :** [LOT-17](epic.md) · **Emplacement :** `HMI/Graphics`, `HMI/Interface` · **Statut :** fait

## Contexte
`GameScreen::spawnPlayer` assigne au personnage `_atlas.tile(1, 1)` — une tuile de couleur unie
(cyan) parmi les seize tuiles générées de la grille 4×4 de `TextureAtlas`. Aucune forme humanoïde
n'existe : ce que le joueur contrôle est visuellement indissociable d'un décor.

## Travail à réaliser
- **`hmi::TextureAtlas`** (`Source/HMI/Graphics/TextureAtlas.h`/`.cpp`) :
  - Nouvelles constantes `PLAYER_REGION_WIDTH = 16`, `PLAYER_REGION_HEIGHT = TILE_SIZE` (16,
    **carrée**, comme une tuile — voir « Points d'attention » : le ratio 1:2 final vient de
    `Transform::scale`, pas de la région).
  - La texture générée grandit verticalement : la grille de tuiles (64×64, inchangée) est
    complétée d'une bande de 16 pixels sous la grille, où vit la région du personnage (colonnes
    0-15) ; le reste de la bande (colonnes 16-63) reste transparent. `width()`/`height()`
    deviennent des membres stockés (`_width`/`_height`, calculés une fois au constructeur) au lieu
    d'une formule figée sur un atlas carré — sur le même principe que `FlagIcons`, déjà non carré.
  - Nouvelle fonction `playerPixel(x, y)` (namespace anonyme) : silhouette par blocs rectangulaires
    (comparaisons d'intervalles, pas de calcul géométrique) — tête (cheveux + peau + nuque),
    torse/manches (chemise), mains (peau) aux extrémités des manches, jambes (pantalon), pieds
    (chaussures plus sombres), **pré-compressée de moitié en hauteur** dans le canevas 16×16 (voir
    « Points d'attention »). Transparent (`pack(0,0,0,0)`) hors silhouette.
  - Nouvel accessseur `core::AtlasRegion playerRegion() const` — région fixe (0, 64, 16, 16).
- **`hmi::GameScreen::spawnPlayer`** (`Source/HMI/Interface/GameScreen.cpp`) : `sprite.region =
  _atlas.playerRegion();` à la place de `_atlas.tile(1, 1)`.

## Fichiers impactés
- `Source/HMI/Graphics/TextureAtlas.h`/`.cpp`.
- `Source/HMI/Interface/GameScreen.cpp`.

## Tests (obligatoires)
- Aucun test unitaire nouveau : `TextureAtlas` crée une ressource Direct3D (nécessite un
  `ID3D11Device`), donc non testable hors GPU (`EX-NFR-010` : la logique pure testable reste dans
  `Core` ; ce lot n'ajoute aucune règle côté `Core`). Vérification par génération locale (Doxygen)
  et par observation visuelle du jeu compilé (silhouette rendue correctement).
- `ctest --preset ninja` reste vert à l'identique (292 tests, aucune régression) — vérifie que
  rien côté `Core`/logique n'est affecté par ce changement purement graphique.

## Points d'attention
- **Ne pas casser les tuiles existantes** : la grille 4×4 (colonnes 0-3, lignes 0-3) doit rester
  bit-à-bit identique après l'agrandissement de la texture — seule sa hauteur relative
  (dénominateur de normalisation UV) change, pas les pixels eux-mêmes ni les régions retournées
  par `tile()`. Vérifié visuellement (niveaux existants inchangés à l'écran).
- **Une seule texture par passe de rendu** (`SpriteRenderer::render`, `_batch->begin(...,
  _atlas->textureView())`) : la région du personnage doit vivre dans la **même** texture que les
  tuiles, pas dans une ressource séparée — sans quoi le personnage et le niveau ne pourraient plus
  être dessinés dans la même passe sans restructurer `SpriteRenderer`.
- **La région doit rester carrée (16×16), ne pas encoder le ratio 1:2 dans ses dimensions.**
  `SpriteRenderer::render` calcule `worldWidth = region.width / PIXELS_PER_UNIT * transform.scale.x`
  (et l'équivalent en hauteur) : la région est donc **déjà** multipliée par `Transform::scale`, qui
  vaut `core::playerSize()` (0,4×0,8, déjà non uniforme). Bug rencontré en implémentant ce lot :
  une première version utilisait une région 16×32 (ratio 1:2 « en dur » dans les pixels), ce qui
  appliquait le ratio **deux fois** — silhouette rendue à 0,4×1,6 au lieu de 0,4×0,8, débordant de
  la boîte de collision et traversant les murs. Corrigé en gardant la région carrée (comme `tile()`)
  et en pré-compressant la silhouette de moitié en hauteur dans `playerPixel` : l'étirement du
  `Transform` restitue les proportions naturelles à l'écran.

## Définition de fait (DoD)
- Silhouette humanoïde visible en jeu, remplaçant le rectangle uni ; build `/W4 /WX` sans
  avertissement ; `ctest` vert (292/292) ; Doxygen local sans sortie.

## Exigences
`EX-REN-011` (partie statique).
