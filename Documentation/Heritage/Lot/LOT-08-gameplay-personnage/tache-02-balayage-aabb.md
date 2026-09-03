# TACHE-02 — Balayage AABB contre la grille (géométrie pure) {#lot-08-tache-02-balayage-aabb}

**Lot :** [LOT-08](epic.md) · **Emplacement :** `Source/Core/Physics` · **Statut :** à faire

## Contexte
La décision de cadrage retient une **collision continue (swept AABB)** pour interdire toute
traversée à vitesse élevée (`EX-GP-014`) et permettre des niveaux exigeant rapidité et précision.
Cette tâche isole le **cœur géométrique** — une fonction **pure**, sans ECS ni rendu, donc
testable exhaustivement.

## Travail à réaliser
- **Primitive de balayage** : étant donné une boîte AABB mobile (position + demi-tailles), un
  **déplacement** `(dx, dy)` sur le pas, et la grille de tuiles solides (`TileMap`), calculer le
  **temps d'impact** `t ∈ [0, 1]` du premier contact et la **normale** de la surface touchée.
- **Résolution par glissement** : à partir du temps d'impact, avancer la boîte jusqu'au contact
  puis **projeter** le déplacement résiduel le long de la surface (annulation de la composante
  normale), afin que le personnage **glisse** le long des murs/sols au lieu de coller.
- **Balayage contre la grille** : ne tester que les tuiles solides **traversées** par le
  déplacement (parcours de la plage de cellules concernée), pas toute la carte.
- API pressentie : un `sweptMove(aabb, delta, tileMap) -> { Vector2 position; Vector2 normal; bool hit; }`
  (ou équivalent), éventuellement itéré en interne pour gérer plusieurs contacts successifs (mur
  puis sol) sur un même pas.

## Fichiers impactés
- `Source/Core/Physics/Aabb.h`, `SweptCollision.h`/`.cpp` (nouveaux).
- `Source/Core/CMakeLists.txt`, `Source/Test/CMakeLists.txt`.

## Tests (obligatoires)
- **Trajet libre** : aucun solide sur le chemin → déplacement complet, `hit == false`.
- **Butée horizontale** : la boîte s'arrête au contact d'un mur, normale horizontale, pas de
  chevauchement résiduel.
- **Butée verticale** : chute stoppée par un sol, normale verticale.
- **Non-tunneling** : un déplacement d'**une frame plus grand qu'une tuile** vers un mur ne le
  traverse pas (cas que la résolution par axe raterait) — test central de la décision de cadrage.
- **Glissement** : un déplacement diagonal contre un mur vertical conserve la composante verticale
  (la boîte glisse le long du mur).
- **Coin** : arrivée sur un angle rentrant — arrêt propre, sans pénétration.

## Points d'attention
- **Géométrie pure** : `float` et `Vector2` uniquement ; aucun composant ECS, aucun DirectX.
- Cohérence du repère : origine haut-gauche, `y` vers le bas ; une tuile fait **1 unité monde**.
- Attention aux **bords de cellule** et aux déplacements nuls (éviter division par zéro, gérer
  `delta == 0` sur un axe).
- Rester **déterministe** (`EX-NFR-002`) : pas d'aléatoire, comportement identique à entrée égale.

## Définition de fait (DoD)
- Primitive de balayage + glissement fonctionnelle, documentée (Doxygen en en-tête, `//` dans le
  `.cpp`) et **couverte** par les cas ci-dessus (`ctest` vert) ; build `/W4 /WX`.

## Exigences
`EX-GP-002`, `EX-GP-014`, `EX-NFR-002`, `EX-NFR-010`, `EX-ARCH-011`.
