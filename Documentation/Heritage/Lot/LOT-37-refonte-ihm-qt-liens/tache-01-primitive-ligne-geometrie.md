# TACHE-01 — Primitive de ligne/flèche (pipeline) + géométrie des traits (logique testable) {#lot-37-tache-01-primitive-ligne-geometrie}

**Lot :** [LOT-37](epic.md) · **Emplacement :** `Source/HMI/Graphics`, `Source/HMI/Editor` · **Statut :** fait

> Réalisé dans `Source/HMI/Graphics/SpriteBatch.{h,cpp}` (`hmi::LineQuad` + `draw(const LineQuad&)`,
> même tampon/index buffer que `draw(SpriteQuad)`) et `Source/HMI/Editor/LinkGeometry.{h,cpp}`
> (chemin réel, pas `Source/Editor` comme cadré initialement — voir note en tête de l'épic).

## Contexte
Le rendu 2D actuel (`hmi::SpriteBatch`) ne dessine que des **quads texturés** : il n'existe **aucune
primitive de ligne**, d'où l'affichage des liaisons par teinte de case. Pour dessiner des
**traits/flèches** reliant un déclencheur à sa cible, il faut une primitive de segment. Cette tâche
l'ajoute au pipeline et isole la **géométrie** (segment + tête de flèche entre deux cases) dans une
fonction **pure testable**.

## Travail à réaliser
- **Primitive de segment** : ajouter à `hmi::SpriteBatch` (ou un utilitaire dédié) le tracé d'un
  segment d'épaisseur `w` entre deux points monde — implémenté comme un **quad orienté** (rectangle
  fin aligné sur la direction), technique déjà employée pour la grille de repère (`renderGrid`
  dessine des lignes via quads fins). Option flèche : deux petits segments formant la tête.
  - Signature type : `drawLine(Vector2 a, Vector2 b, float thickness, Color)` et
    `drawArrow(Vector2 a, Vector2 b, ...)`.
  - Rester dans le batching existant (même texture blanche 1×1 / région unie) pour ne pas casser les
    performances.
- **Géométrie des liens** (`Source/Editor/LinkGeometry.{h,cpp}`, pur, sans GPU) :
  - `linkSegment(GridPosition trigger, GridPosition target, tileSize) -> {Vector2 a, Vector2 b}` :
    centres des cases (origine haut-gauche, cohérent `TileMap`).
  - Géométrie de la tête de flèche (points) pour une longueur/angle donnés.
  - Éventuel léger décalage si plusieurs liens partagent une case (déclencheur → plusieurs cibles) pour
    éviter la superposition parfaite.

## Fichiers impactés
- `Source/HMI/Graphics/SpriteBatch.{h,cpp}` (primitive segment/flèche) ou
  `Source/HMI/Graphics/PrimitiveBatch.{h,cpp}` (nouveau, si séparation préférée).
- `Source/Editor/LinkGeometry.{h,cpp}` (nouveau, pur).
- `Source/Test/Unit/Editor/test_link_geometry.cpp` (nouveau).

## Tests (obligatoires)
- **`LinkGeometry` pur** (sans GPU) : centres de cases corrects pour diverses positions/tailles ;
  segment horizontal/vertical/diagonal ; tête de flèche orientée du bon côté ; décalage anti-
  superposition déterministe. Origine haut-gauche respectée.
- **Vérification manuelle** de la primitive : un segment/flèche s'affiche net à l'écran (épaisseur,
  couleur), au bon endroit au zoom/pan courant.

## Points d'attention
- **Réutiliser l'astuce quad-fin** de `renderGrid` plutôt qu'un nouveau pipeline (pas de nouveau
  shader) : cohérent avec l'existant, un seul état de rendu.
- **Épaisseur constante à l'écran** vs en unités monde : décider si l'épaisseur suit le zoom
  (recommandé : épaisseur en pixels écran pour rester lisible à tout zoom) — le documenter.
- **Origine haut-gauche** (`EX-ARCH-020`) : pas d'inversion d'axe dans les centres de cases.

## Définition de fait (DoD)
- Primitive segment/flèche disponible dans le pipeline ; `LinkGeometry` livré et **testé** sans GPU ;
  `/W4 /WX` propre ; primitive vérifiée visuellement.

## Exigences
Prépare `EX-IHM-030` (liens par traits/flèches) ; `EX-NFR-010` (géométrie testable sans GPU) ;
réutilise `EX-ARCH-020` (origine haut-gauche).
