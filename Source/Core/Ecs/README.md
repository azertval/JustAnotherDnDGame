# Core/Ecs/

Entity-Component-System (implémentation **maison**), cœur de la simulation.

- Entités : handles générationnels.
- Composants : données pures (sparse sets) — sous-dossier `Components/`.
- Systèmes : logique itérant sur les composants — sous-dossier `Systems/`.
- `World` : façade, orchestration au pas de temps fixe.

Implémenté (LOT-03) :
- `Entity` — handle générationnel `{ index, generation }`, `INVALID_ENTITY`.
- `EntityManager` — `create` / `destroy` / `isAlive`, recyclage des index par liste libre.
- `ComponentPool<T>` — sparse set typé (`add` / `get` / `has` / `remove` / `removeIfPresent`), tableau dense contigu, suppression par swap-and-pop.
- `View<Components...>` — vue multi-composants itérant l'intersection (pilotée par la plus petite pool), API `for (auto [entity, ...] : view)` et `view.each(...)`.
- `ISystem` — interface d'un système (`update(World&, float fixedDelta)`).
- `World` — façade : `createEntity` / `destroyEntity` (purge toutes les pools), `addComponent` / `getComponent` / `hasComponent` / `removeComponent`, `view<...>()`, `addSystem` / `update` (exécution ordonnée au pas fixe).
- `Components/` — composants données pures : `Transform` (position, échelle, rotation), `Velocity` (vitesse en unités monde/s), `Sprite` (région d'atlas, couche, teinte — lu par le rendu de `HMI`), `Collider` (boîte AABB de collision), `Actor` (marqueur d'un personnage qui se déplace : orientation et masse, `LOT-06`), `Animation` (clip/image courante du personnage, `EX-REN-012`).
- `Systems/` — systèmes : `MovementSystem` (intègre `position += velocity * fixedDelta` sur les entités `Transform + Velocity`), `TopDownMovementSystem` (déplacement libre en 8 directions d'un `Actor` : intention normalisée → vitesse → balayage continu → position, sans gravité, `LOT-06`), `AnimationSystem` (dérive le clip/l'image d'animation de la vitesse du personnage), `ParticleSystem`.

Réf. specs : `EX-ARCH-010`, `EX-ARCH-011`, `EX-ARCH-012`, `EX-ARCH-100`.
