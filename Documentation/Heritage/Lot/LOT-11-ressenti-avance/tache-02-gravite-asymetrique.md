# TACHE-02 — Gravité asymétrique + apex hang + fast-fall {#lot-11-tache-02-gravite-asymetrique}

**Lot :** [LOT-11](epic.md) · **Emplacement :** `Source/Core/Ecs/Systems` · **Statut :** à faire

## Contexte
Cœur du ressenti (`EX-GP-018`) : la **gravité effective** dépend de la phase du saut. On remplace
le simple `velocity.y += gravity * dt` du `CharacterPhysicsSystem` par une gravité modulée, sans
toucher au reste (saut, wall slide, dash restent compatibles).

## Travail à réaliser
Dans la partie « gravité » de `CharacterPhysicsSystem::update` (hors dash), calculer une gravité
**effective** `g` avant intégration :
- base : `g = _config.gravity` ;
- **chute renforcée** : si `velocity.y > 0` (chute), `g *= fallGravityMultiplier` ;
- **fast-fall** : si en chute **et** `input.moveY > 0` (« bas » maintenu), `g *= fastFallMultiplier` ;
- **apex hang** : si `|velocity.y| < apexThreshold` (proche du sommet), `g *= apexGravityMultiplier` ;
- puis `velocity.y += g * fixedDelta` et la **borne de chute** existante (`maxFallSpeed`) inchangée.

La montée (`velocity.y < 0` hors apex) garde la gravité de base → la chute est **plus rapide** que
la montée.

## Fichiers impactés
- `Source/Core/Ecs/Systems/CharacterPhysicsSystem.h`/`.cpp` (+ `<cmath>` pour `std::abs`).
- Tests d'intégration.

## Tests (obligatoires)
- **Chute plus rapide que la montée** : après un saut, l'accélération verticale en **chute**
  (`Δvy`/pas) est **supérieure** à la décélération en **montée** (hors apex).
- **Apex hang** : près de l'apex (`|vy|` faible), la variation de `vy` par pas est **réduite** par
  rapport à `gravity × dt` (gravité amoindrie).
- **Fast-fall** : en chute, maintenir « bas » (`moveY = 1`) donne une vitesse de chute **plus
  grande** (à pas égal) que sans.
- **Compatibilité** : le saut atteint toujours une hauteur cohérente ; wall slide et dash
  inchangés (leurs tests restent verts).
- **Déterminisme** (`EX-NFR-002`).

## Points d'attention
- **Ordre des multiplicateurs** : appliquer chute → fast-fall → apex sur un même `g` (documenter).
- **Ne pas casser la borne** `maxFallSpeed` ni la suspension de gravité **pendant le dash**.
- La modulation vit **dans le système** ; les réglages restent des **données** (`EX-ARCH-011`).

## Définition de fait (DoD)
- Gravité asymétrique + apex hang + fast-fall fonctionnels et **testés** (`ctest` vert) ;
  build `/W4 /WX`.

## Exigences
`EX-GP-018`, `EX-GP-011`, `EX-NFR-002`, `EX-ARCH-011`.
