# TACHE-05 — Dash 8 directions (burst, durée, recharge au sol) {#lot-10-tache-05-dash}

**Lot :** [LOT-10](epic.md) · **Emplacement :** `Source/Core/Ecs/Systems` · **Statut :** à faire

## Contexte
Troisième mécanique (`EX-GP-017`) : une **ruée** à vitesse élevée dans l'une des **8 directions**,
sur une **courte durée**, **disponible une fois** puis **rechargée au contact du sol**. C'est la
mécanique la plus « impactante » : pendant le dash, la gravité est **suspendue** pour une
trajectoire nette.

## Travail à réaliser
Dans `CharacterPhysicsSystem::update` :
- **Orientation** : mettre à jour `Player::facing` selon `moveX` quand il est non nul.
- **Recharge** : au **contact du sol**, `dashAvailable = true`.
- **Déclenchement** : si `dashPressed` **et** `dashAvailable` **et** pas déjà en dash :
  - direction = `(moveX, moveY)` **normalisée** ; si nulle, `(facing, 0)` (dash horizontal par
    défaut) ;
  - `velocity = direction * _config.dashSpeed` ; `dashTimer = _config.dashDuration` ;
    `dashAvailable = false`.
- **Pendant le dash** (`dashTimer > 0`) : **maintenir** `velocity = direction * dashSpeed`,
  **suspendre la gravité** et la vitesse horizontale d'entrée, décrémenter `dashTimer` de
  `fixedDelta`. À l'expiration, reprendre la physique normale (la vitesse acquise se poursuit puis
  la gravité reprend).
- Résoudre le déplacement du dash par le **balayage** habituel (le dash ne traverse pas les murs).

## Fichiers impactés
- `Source/Core/Ecs/Systems/CharacterPhysicsSystem.h`/`.cpp`.
- Tests d'intégration.

## Tests (obligatoires)
- **Dash horizontal** : au sol, `dashPressed` avec orientation droite → forte vitesse **+X** sur la
  durée ; la distance parcourue dépasse nettement un pas normal.
- **Dash diagonal** : `moveX = +1, moveY = -1` → vitesse **+X/−Y** (haut-droite).
- **Une seule ruée en l'air** : après un dash en l'air, un second `dashPressed` **ne fait rien**
  tant que le sol n'est pas retouché ; après atterrissage, le dash est de nouveau disponible.
- **Gravité suspendue** : pendant le dash horizontal, la composante verticale ne « tombe » pas.
- **Pas de traversée** : un dash vers un mur s'arrête au mur (balayage).
- **Déterminisme** (`EX-NFR-002`).

## Points d'attention
- **Normalisation** : une direction diagonale ne doit pas dasher plus vite qu'une cardinale
  (normaliser `(moveX, moveY)`).
- **Priorité** : gérer proprement l'ordre dash / saut / gravité — pendant un dash actif, ignorer la
  gravité et l'entrée de déplacement.
- **Recharge** : uniquement au **sol** (pas de recharge en l'air), conformément à la décision de
  cadrage.
- Réutiliser le balayage du LOT-08 ; ne pas réimplémenter la collision.

## Définition de fait (DoD)
- Dash 8 directions fonctionnel et **testé** (`ctest` vert) ; build `/W4 /WX`.

## Exigences
`EX-GP-017`, `EX-GP-014`, `EX-NFR-002`, `EX-ARCH-011`.
