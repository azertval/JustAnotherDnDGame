# TACHE-05 — Systèmes & World (orchestration au pas fixe) {#lot-03-tache-05-systemes-world}

**Lot :** [LOT-03](epic.md) · **Emplacement :** `Source/Core/Ecs` · **Statut :** fait

## Contexte
Le `World` est la **façade** de l'ECS : il possède les entités (TACHE-02), les pools de composants (TACHE-03), fournit les vues (TACHE-04) et orchestre les **systèmes**. Les systèmes portent la logique et s'exécutent au **pas de temps fixe** (`EX-ARCH-030`).

## Travail à réaliser
- `World` :
  - Délègue le cycle de vie des entités à l'`EntityManager` et gère les `ComponentPool<T>` par type.
  - API composants : `addComponent<T>`, `getComponent<T>`, `hasComponent<T>`, `removeComponent<T>`.
  - API vues : `view<...>()`.
  - `destroyEntity` retire l'entité de **toutes** les pools.
- Systèmes :
  - Interface `ISystem` avec `update(World&, float fixedDelta)`.
  - Enregistrement ordonné des systèmes dans le `World` ; `World::update(float fixedDelta)` les exécute **dans l'ordre**.
- Intégration avec `FixedTimestep` (LOT-01) : l'appelant calcule le nombre de pas et invoque `World::update(fixedDelta)` autant de fois.

## Fichiers impactés
- `Source/Core/Ecs/World.h`, `World.cpp` (nouveau).
- `Source/Core/Ecs/ISystem.h` (nouveau).
- `Source/Test/Unit/test_world.cpp` (nouveau).

## Tests (obligatoires)
- `addComponent`/`getComponent`/`hasComponent`/`removeComponent` cohérents via le `World`.
- `destroyEntity` retire bien tous les composants associés (plus aucune pool ne référence l'entité).
- Les systèmes enregistrés s'exécutent **dans l'ordre** ; un système de test compte ses invocations.
- `update` appelé N fois exécute chaque système N fois (déterminisme du cadencement).

## Points d'attention
- Le `World` **possède** ses ressources (RAII) ; pas d'état global.
- Ordre d'exécution des systèmes **déterministe** et explicite.
- Conventions : documentation `.h` + `.cpp`, dépendances `HMI → Core` respectées (le `World` ignore tout du rendu).

## Définition de fait (DoD)
- Façade complète et testée (`ctest` vert) ; orchestration des systèmes déterministe.
- Compile `/W4 /WX`, formaté, documenté Doxygen.

## Exigences
`EX-ARCH-010`, `EX-ARCH-030`, `EX-NFR-010`, `EX-NFR-002`, `EX-NFR-020`.
