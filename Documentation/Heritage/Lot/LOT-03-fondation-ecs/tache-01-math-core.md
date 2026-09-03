# TACHE-01 — Types mathématiques de Core {#lot-03-tache-01-math-core}

**Lot :** [LOT-03](epic.md) · **Emplacement :** `Source/Core/Math` · **Statut :** fait

## Contexte
Les composants (à commencer par `Transform`) et la physique ont besoin de types mathématiques 2D. `Core` ne devant pas dépendre de DirectX (`EX-ARCH-040`), ces types sont **propres au projet**. La conversion vers `DirectXMath` se fera plus tard, uniquement dans `HMI`.

## Travail à réaliser
- `Vector2` (composantes `float`) : addition, soustraction, multiplication/division par un scalaire, produit scalaire, longueur, normalisation, égalité.
- `Rect` (position + dimensions en unités monde, origine haut-gauche, Y-bas) : test de **contenance** d'un point, test d'**intersection** avec un autre `Rect`.
- Constantes/utilitaires minimaux si nécessaires (ex. comparaison approchée de `float`).

## Fichiers impactés
- `Source/Core/Math/Vector2.h`, `Vector2.cpp` (nouveau).
- `Source/Core/Math/Rect.h`, `Rect.cpp` (nouveau).
- `Source/Core/CMakeLists.txt` (ajout des sources).
- `Source/Test/Unit/test_vector2.cpp`, `test_rect.cpp` (nouveau).

## Tests (obligatoires)
- Opérations vectorielles : résultats attendus (somme, échelle, produit scalaire, longueur d'un `(3,4)` = 5).
- Normalisation d'un vecteur non nul → longueur ≈ 1 ; cas du vecteur nul géré explicitement.
- `Rect::contains` : points intérieurs, sur les bords, extérieurs.
- `Rect::intersects` : recouvrement, contact par un bord, disjonction.

## Points d'attention
- Types **sans dépendance système** ni DirectX.
- Comparaisons de `float` par tolérance (pas d'`==` brut sur des résultats calculés).
- Conventions : `_camelCase` pour d'éventuels membres, documentation `.h` + `.cpp`, `[[nodiscard]]` sur les accès purs.

## Définition de fait (DoD)
- Types complets et testés (`ctest` vert), sans dépendance externe.
- Compile `/W4 /WX`, formaté clang-format, documenté Doxygen.

## Exigences
`EX-ARCH-040`, `EX-NFR-010`, `EX-NFR-020`.
