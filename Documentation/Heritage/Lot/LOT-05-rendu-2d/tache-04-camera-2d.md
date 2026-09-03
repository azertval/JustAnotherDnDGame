# TACHE-04 — Caméra 2D (monde → écran) {#lot-05-tache-04-camera-2d}

**Lot :** [LOT-05](epic.md) · **Emplacement :** `Source/HMI/Graphics` · **Statut :** fait

## Contexte
Les positions des entités sont en **unités monde** (`EX-ARCH-020`). Le rendu doit les
convertir en pixels écran via une **caméra 2D**, en respectant l'échelle **16 px/unité**
(`EX-ARCH-021`), l'origine haut-gauche et l'axe Y vers le bas.

## Travail à réaliser
- `Camera2D` : position (centre, en unités monde), **zoom** (facteur, de préférence
  entier pour la netteté pixel art), et dimensions de la surface de rendu.
- Produire la **matrice de projection** monde → *clip space* (pixels → NDC) utilisée par
  le vertex shader (TACHE-02).
- Conversions utilitaires : monde → écran et écran → monde (utile pour l'édition et les
  entrées plus tard).
- Prise en compte du **redimensionnement** de la fenêtre (mise à jour des dimensions).

## Fichiers impactés
- `Source/HMI/Graphics/Camera2D.h`, `Camera2D.cpp` (nouveau).
- `Source/Test/Unit/test_camera2d.cpp` (nouveau).

## Tests (obligatoires)
- Une entité à l'origine monde se projette au bon pixel selon le centre caméra et le zoom.
- Conversions monde↔écran réciproques (aller-retour ≈ identité).
- L'axe Y est bien orienté vers le bas ; un déplacement caméra décale la scène de façon cohérente.

## Points d'attention
- La caméra est un **objet de présentation** (`HMI`) : elle lit des positions monde mais
  ne modifie pas l'ECS.
- Zoom en facteurs entiers recommandé (`EX-ARCH-022`) ; comparaisons `float` par tolérance.
- Le **suivi** d'une cible et le **bornage** au niveau sont hors périmètre (lot ultérieur).

## Définition de fait (DoD)
- Caméra correcte (projection + conversions), testée (`ctest` vert) ; build `/W4 /WX`, documentée.

## Exigences
`EX-REN-013` (partiel : caméra basique), `EX-ARCH-020`, `EX-ARCH-021`, `EX-ARCH-022`.
