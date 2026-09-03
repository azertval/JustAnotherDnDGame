# TACHE-03 — Stockage de composants (sparse set typé) {#lot-03-tache-03-stockage-composants}

**Lot :** [LOT-03](epic.md) · **Emplacement :** `Source/Core/Ecs` · **Statut :** fait

## Contexte
Chaque type de composant est stocké dans un **sparse set** : un tableau dense de composants (itération cache-friendly) doublé d'un tableau creux indexé par l'entité (accès en temps constant).

## Travail à réaliser
- `ComponentPool<T>` (sparse set) :
  - `add(Entity, T)` / `remove(Entity)` / `has(Entity)` / `get(Entity) → T&`.
  - Tableau **dense** des composants + correspondance entité ↔ position dense.
  - Suppression par *swap-and-pop* (maintien de la densité).
- Registre de pools typés dans le `World` (une pool par type de composant, créée à la demande).
- Cohérence avec la destruction d'entité (TACHE-02) : détruire une entité retire ses composants de toutes les pools.

## Fichiers impactés
- `Source/Core/Ecs/ComponentPool.h` (nouveau, template).
- Intégration dans `World` (voir TACHE-05).
- `Source/Test/Unit/test_component_pool.cpp` (nouveau).

## Tests (obligatoires)
- `add` puis `get` renvoie la valeur stockée ; `has` est cohérent.
- `remove` (y compris d'un élément au milieu, via swap-and-pop) laisse les autres composants **accessibles et corrects**.
- `get`/`remove` sur une entité absente est géré selon le contrat (assertion de précondition, cf. politique d'erreurs).
- Le stockage dense reste contigu après suppressions.

## Points d'attention
- Composants = **données pures** (`EX-ARCH-011`), sans logique — condition pour la sérialisation future.
- Attention à l'invalidation des références après *swap-and-pop* (documenter le contrat).
- Conventions : `_camelCase`, `[[nodiscard]]` sur `get`/`has`, documentation `.h`.

## Définition de fait (DoD)
- Pool correcte et dense, testée (`ctest` vert).
- Compile `/W4 /WX`, formaté, documenté Doxygen.

## Exigences
`EX-ARCH-011`, `EX-ARCH-012`, `EX-NFR-010`, `EX-NFR-020`.
