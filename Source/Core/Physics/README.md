# Core/Physics/

Primitives de physique et réglages du déplacement.

Implémenté :
- `Aabb` — boîte englobante alignée aux axes (coins `min`/`max`) et fabrique associée.
- `SweptCollision` — collision **continue** (balayage) d'une AABB contre une grille de tuiles
  solides ; `SweepResult` indique la position finale et l'axe/sens bloqué.
- `PlayerInput` — intention de déplacement du personnage, neutre vis-à-vis de la touche physique
  (produite par `HMI::toPlayerInput`, consommée par le système de déplacement).
- `PlayerSpawn` — taille du personnage jouable et placement centré dans une tuile au spawn.
- `TopDownConfig` — réglages du déplacement en vue de dessus (`LOT-06`) : vitesse,
  accélération, friction. Trois champs là où `PhysicsConfig` en compte vingt-cinq : sans gravité,
  il n'y a ni hauteur de saut, ni coyote time, ni vitesse terminale à régler.
- `PhysicsConfig` — constantes réglables du personnage de plateforme (gravité effective par phase,
  traînée newtonienne, vitesses, budgets). **Sans consommateur** depuis le `LOT-01` ; conservé le
  temps que le RPG dise ce qu'il garde de ce vocabulaire.

Ce dossier ne porte que des **primitives** : elles ignorent la gravité et n'ont aucun axe
privilégié — c'est exactement ce qu'attend un déplacement en vue de dessus, et la raison pour
laquelle le `LOT-06` n'a eu qu'à les assembler (`core::updateTopDownMovement`).

Réf. specs : `EX-GP-011`, `EX-GP-012`, `EX-GP-014`, `EX-GP-018`, `EX-GP-019`, `EX-GP-025`.
