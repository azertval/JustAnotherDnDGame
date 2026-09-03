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
- `Components/` — composants données pures : `Transform` (position, échelle, rotation), `Velocity` (vitesse en unités monde/s), `Sprite` (région d'atlas, couche, teinte — lu par le rendu de `HMI`), `Collider` (boîte AABB de collision), `Player` (marqueur du personnage jouable + état de contact au sol), `Animation` (clip/image courante du personnage, `EX-REN-012`).
- `Systems/` — systèmes : `MovementSystem` (intègre `position += velocity * fixedDelta` sur les entités `Transform + Velocity`), `AnimationSystem` (dérive le clip/l'image d'animation de l'état du personnage), `ParticleSystem`. Le contrôleur de déplacement du personnage a été retiré au `LOT-01` avec le gameplay de plateforme ; son remplaçant en vue de dessus arrive au `LOT-06`.

Réf. specs : `EX-ARCH-010`, `EX-ARCH-011`, `EX-ARCH-012`, `EX-ARCH-100`.
