# TACHE-02 — Mécanismes interrupteur/porte (MechanismController) {#lot-12-tache-02-mecanismes}

**Lot :** [LOT-12](epic.md) · **Emplacement :** `Source/Core/Gameplay` · **Statut :** à faire

## Contexte
Les liaisons interrupteur↔porte sont **chargées** depuis le LOT-07 mais **inertes**. Cette tâche
leur donne un **comportement** (`EX-GP-020`, `EX-GP-021`), en logique **pure** et testable : la
collision consommera une **grille reflétant l'état des portes**.

## Travail à réaliser
- **`MechanismController`** (Core/Gameplay), construit depuis un `Level` :
  - conserve une **copie mutable** du `TileMap` (grille de **collision**) où chaque **porte** est
    posée **fermée = `Solid`** au départ ;
  - conserve, par mécanisme, l'**état** de l'interrupteur (ouvert/fermé) et un **front** (le
    personnage était-il déjà sur l'interrupteur au pas précédent).
  - `update(const Aabb& playerBox)` : pour chaque mécanisme, si la boîte du personnage **recouvre**
    la tuile interrupteur **et** qu'il n'y était pas au pas précédent → **basculer** l'état ; puis
    poser la porte : ouverte → `Door` (franchissable), fermée → `Solid`.
  - `collisionMap()` : la grille de collision courante (portes à jour) pour la physique.
  - (optionnel) accès à l'état des portes pour le retour visuel (TACHE-04).

## Fichiers impactés
- `Source/Core/Gameplay/MechanismController.h`/`.cpp` (nouveau), `Source/Core/CMakeLists.txt`.
- Tests unitaires.

## Tests (obligatoires)
- **Ouverture** : la porte est **solide** au départ ; après contact de l'interrupteur, la porte
  devient **franchissable** (`!isSolid`).
- **Bascule sur front** : rester sur l'interrupteur ne **re-bascule pas** ; ressortir puis revenir
  bascule de nouveau.
- **Liaison** : c'est bien la porte **liée** (par position) qui change, pas une autre.
- **Déterminisme** (`EX-NFR-002`).

## Points d'attention
- **Ne pas muter** le `TileMap` du `Level` (source de vérité) : travailler sur la **copie**.
- Recouvrement boîte↔tuile cohérent avec le repère (origine haut-gauche, tuile = 1 unité).
- Front d'activation : mémoriser l'état « sur l'interrupteur » du pas précédent.

## Définition de fait (DoD)
- `MechanismController` fonctionnel, documenté et **testé** (`ctest` vert) ; build `/W4 /WX`.

## Exigences
`EX-GP-020`, `EX-GP-021`, `EX-NFR-002`, `EX-ARCH-011`, `EX-NFR-010`.
