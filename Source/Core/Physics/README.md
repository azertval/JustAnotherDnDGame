# Core/Physics/

Physique de plateforme.

Implémenté :
- `Aabb` — boîte englobante alignée aux axes (coins `min`/`max`) et fabrique associée.
- `SweptCollision` — collision **continue** (balayage) d'une AABB contre une grille de tuiles
  solides ; `SweepResult` indique la position finale et l'axe/sens bloqué.
- `PlayerInput` — intention de déplacement du personnage, neutre vis-à-vis de la touche physique
  (produite par `HMI::toPlayerInput`, consommée par le système de déplacement).
- `PlayerSpawn` — taille du personnage jouable et placement centré dans une tuile au spawn.
- `PhysicsConfig` — constantes réglables (gravité effective par phase, traînée newtonienne,
  vitesses, budgets) regroupées pour faciliter le *tuning*.

Ce dossier ne porte que des **primitives** : elles ignorent la gravité et n'ont aucun axe
privilégié. Le système de déplacement du personnage, qui les consomme, a été retiré au `LOT-01`
avec le gameplay de plateforme ; son remplaçant en vue de dessus arrive au `LOT-06`.

Réf. specs : `EX-GP-011`, `EX-GP-012`, `EX-GP-014`, `EX-GP-018`, `EX-GP-019`, `EX-GP-025`.
