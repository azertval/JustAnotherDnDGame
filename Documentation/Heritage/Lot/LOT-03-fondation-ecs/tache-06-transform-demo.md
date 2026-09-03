# TACHE-06 — Composant Transform + système de mouvement (démo) {#lot-03-tache-06-transform-demo}

**Lot :** [LOT-03](epic.md) · **Emplacement :** `Source/Core/Ecs` · **Statut :** fait

## Contexte
Tâche d'assemblage : prouver que l'ECS fonctionne de bout en bout avec un premier composant réel et un système qui le met à jour. Sert aussi d'exemple de référence pour les futurs systèmes de gameplay.

## Travail à réaliser
- Composant `Transform` (données pures) : `position` (`Vector2`), `scale` (`Vector2`), `rotation` (`float`), en **unités monde** (cf. `EX-ARCH-020`).
- Composant `Velocity` (données pures) : `value` (`Vector2`, unités monde par seconde).
- `MovementSystem` (implémente `ISystem`) : pour chaque entité ayant `Transform` **et** `Velocity`, applique `position += velocity * fixedDelta`.
- Démonstration : créer quelques entités, enregistrer le système dans un `World`, exécuter plusieurs pas fixes et vérifier les positions.

## Fichiers impactés
- `Source/Core/Ecs/Components/Transform.h`, `Velocity.h` (nouveau).
- `Source/Core/Ecs/Systems/MovementSystem.h`, `MovementSystem.cpp` (nouveau).
- `Source/Test/Unit/test_movement_system.cpp` (nouveau).

## Tests (obligatoires)
- Une entité `Transform + Velocity` avance de `velocity * fixedDelta` par pas ; après N pas, position attendue (déterministe).
- Une entité sans `Velocity` **ne bouge pas**.
- Deux entités de vitesses différentes évoluent indépendamment.

## Points d'attention
- Les composants restent des **données pures** ; toute la logique est dans `MovementSystem`.
- Déterminisme : même état initial + mêmes pas → mêmes positions (`EX-NFR-002`).
- Conventions : documentation `.h` + `.cpp`.

## Définition de fait (DoD)
- Chaîne complète entité → composant → système → résultat vérifiée par tests (`ctest` vert).
- Tous les critères d'acceptation de l'[epic](epic.md) satisfaits.
- Build `/W4 /WX` sans avertissement, **CI verte**, Doxygen à jour, `CHANGELOG.md` mis à jour.

## Exigences
`EX-ARCH-010`, `EX-ARCH-011`, `EX-ARCH-030`, `EX-NFR-002`, `EX-NFR-020`.
