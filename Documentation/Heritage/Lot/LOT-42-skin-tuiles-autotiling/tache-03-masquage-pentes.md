# TACHE-03 — Masquage alpha automatique des pentes et arrondis {#lot-42-tache-03-masquage-pentes}

**Lot :** [LOT-42](epic.md) · **Emplacement :** `Source/HMI/Graphics` · **Statut :** fait

## Contexte
Douze types de tuiles ont une **silhouette non carrée** : pentes montantes et descendantes, arrondis
convexes et concaves, au sol et au plafond (`EX-GP-003`, `EX-GP-004`, `EX-GP-006`, `EX-GP-007`).
L'atlas procédural les dessine déjà correctement, en calculant leur masque de forme à partir des
**mêmes fonctions** que la physique — `core::slopeSurfaceHeight`, `core::ceilingSlopeHeight`,
`core::isCeilingSlope` — pour que l'apparence colle exactement à la hitbox.

Un skin fourni par l'auteur est, lui, une image carrée. L'afficher tel quel sur une pente donnerait
un carré plein là où le personnage traverse : la lecture du niveau serait fausse.

## Travail à réaliser
- **Détourage au chargement** : lorsqu'un asset est assigné à l'un des douze types à silhouette, en
  produire une variante dont les pixels situés **hors** de la forme sont rendus transparents, en
  réutilisant les fonctions de géométrie de `Core` déjà employées par
  *slopeShapePixel* (`ProceduralAtlas.cpp`).
- **Une seule fois, puis en cache** : le détourage porte sur 16×16 pixels, son coût est négligeable,
  mais il ne doit pas être refait à chaque image. Le résultat est une entrée du *TextureCache*
  (LOT-40), invalidée comme les autres.
- **Fonction pure** : le calcul du masque (pour un type et une taille donnés, quels pixels sont
  dedans) est séparé de toute manipulation de texture GPU, donc testable.
- Ces types restent en mode `single` : le raccord automatique ne s'applique pas à une silhouette.

## Fichiers impactés
- `Source/HMI/Graphics/SlopeMask.{h,cpp}` (nouveau) — masque pur.
- `Source/HMI/Graphics/TextureCache.{h,cpp}` (variante détourée mise en cache).
- `Source/Test/Unit/HMI/Graphics/test_slope_mask.cpp` (nouveau).

## Tests (obligatoires)
- Pour chacun des douze types : le masque calculé correspond à la silhouette attendue (pixels
  intérieurs opaques, extérieurs transparents), aux mêmes positions que celles produites par
  `ProceduralAtlas` pour le même type.
- Un type **sans** silhouette (solide, bloc, danger) n'est pas détouré.
- Test **pur**, sans GPU.

## Points d'attention
- **La source de vérité reste `Core`.** Ne jamais réimplémenter la géométrie des pentes côté `HMI` :
  toute divergence entre le masque affiché et la surface suivie par la physique produirait
  précisément le bug que ce projet évite depuis `LOT-22` (l'affichage doit coller à la hitbox).
- La transparence doit être **franche** (dedans/dehors), pas anticrénelée : le rendu est en pixel
  art avec filtrage *nearest* (`EX-ARCH-022`), un bord adouci créerait un halo.
- Vérifier que l'asset d'origine reste disponible non détouré s'il est aussi assigné à un type
  carré : la mise en cache doit distinguer les deux variantes.

## Définition de fait (DoD)
- Les douze types à silhouette affichent leur skin découpé à la bonne forme, identique à la
  silhouette de la hitbox ; le masque est calculé une fois et mis en cache ; tests purs verts ;
  `/W4 /WX` propre.

## Exigences
`EX-EDIT-042` (association type → texture) ; réutilise `EX-GP-003`/`EX-GP-004`/`EX-GP-006`/
`EX-GP-007` (silhouettes de pentes et arrondis), `EX-ARCH-022` (*nearest*), `EX-NFR-010` (testable
sans GPU).
