# TACHE-02 — Entités : handles générationnels & cycle de vie {#lot-03-tache-02-entites}

**Lot :** [LOT-03](epic.md) · **Emplacement :** `Source/Core/Ecs` · **Statut :** fait

## Contexte
Le cœur de l'ECS est l'entité : un simple **identifiant**. Pour éviter les références pendantes (une entité détruite dont l'index est recyclé), on utilise un **handle générationnel** (index + génération).

## Travail à réaliser
- Type `Entity` : structure légère `{ index, generation }` (copiable, comparable), avec une valeur **invalide** conventionnelle.
- `EntityManager` :
  - `create()` → renvoie une `Entity` valide (réutilise un index libéré en incrémentant sa génération).
  - `destroy(Entity)` → libère l'index et invalide les handles existants.
  - `isAlive(Entity)` → vrai seulement si l'index est vivant **et** la génération correspond.
- Recyclage des index libérés (liste libre) pour borner la mémoire.

## Fichiers impactés
- `Source/Core/Ecs/Entity.h` (nouveau).
- `Source/Core/Ecs/EntityManager.h`, `EntityManager.cpp` (nouveau).
- `Source/Core/CMakeLists.txt`.
- `Source/Test/Unit/test_entity_manager.cpp` (nouveau).

## Tests (obligatoires)
- `create` renvoie des entités vivantes et distinctes.
- Après `destroy`, `isAlive` est faux pour l'ancien handle.
- Un index recyclé produit une **génération différente** : l'ancien handle reste invalide, le nouveau est valide.
- L'entité invalide conventionnelle n'est jamais vivante.

## Points d'attention
- Aucune allocation par entité au-delà des tableaux internes ; opérations en temps constant amorti.
- Conventions : RAII (le manager possède ses tableaux), `_camelCase`, documentation `.h` + `.cpp`.

## Définition de fait (DoD)
- Cycle de vie correct, détection fiable des handles périmés, testé (`ctest` vert).
- Compile `/W4 /WX`, formaté, documenté Doxygen.

## Exigences
`EX-ARCH-010`, `EX-NFR-010`, `EX-NFR-020`.
