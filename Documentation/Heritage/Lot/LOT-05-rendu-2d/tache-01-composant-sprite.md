# TACHE-01 — Composant Sprite (données pures) {#lot-05-tache-01-composant-sprite}

**Lot :** [LOT-05](epic.md) · **Emplacement :** `Source/Core/Ecs/Components` · **Statut :** fait

## Contexte
Le rendu a besoin de savoir, pour chaque entité affichable, **quoi** dessiner et **dans
quel ordre**. Comme le `Transform`, le `Sprite` est une **donnée pure** de `Core`
(`EX-ARCH-011`), lue par le rendu de `HMI` sans mutation (`EX-ARCH-012`).

## Travail à réaliser
- Composant `Sprite` : région source dans l'atlas (rectangle en pixels de texture, via
  `core::Rect` ou un `TextureRect` dédié), **couche** de dessin (entier ou `enum class`),
  et éventuelle **teinte** (couleur RGBA, défaut blanc opaque).
- Rester **agnostique** du backend : pas de type DirectX ni de handle de texture dans le
  composant (l'atlas est résolu côté `HMI`).
- Documenter la convention d'unités de la région (pixels de l'atlas).

## Fichiers impactés
- `Source/Core/Ecs/Components/Sprite.h` (nouveau).
- `Source/Test/Unit/test_sprite.cpp` (nouveau, si logique à tester).

## Tests (obligatoires si logique)
- Valeurs par défaut cohérentes (teinte blanche opaque, couche 0).
- Toute logique utilitaire éventuelle (ex. calcul de coordonnées UV normalisées) testée.

## Points d'attention
- Donnée pure : aucun comportement, sérialisable plus tard avec les niveaux.
- Conventions : `_camelCase` si membres privés (ici struct de données publiques),
  documentation `.h`, `[[nodiscard]]` sur d'éventuels accès.

## Définition de fait (DoD)
- `Sprite` complet, sans dépendance DirectX, documenté ; build `/W4 /WX`, tests verts.

## Exigences
`EX-ARCH-011`, `EX-ARCH-012`, `EX-REN-011`, `EX-REN-014`.
