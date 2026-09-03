# TACHE-03 — Saut au sol + hauteur variable {#lot-09-tache-03-saut-hauteur-variable}

**Lot :** [LOT-09](epic.md) · **Emplacement :** `Source/Core/Ecs/Systems` · **Statut :** à faire

## Contexte
Cœur du lot : ajouter le **saut** au `CharacterPhysicsSystem` (LOT-08), avec la **hauteur
variable** — le levier de *feel* le plus important. La gravité reste **constante** (`EX-GP-011`) ;
la hauteur varie en **coupant** la vitesse ascendante au relâchement, pas en changeant la gravité.

## Travail à réaliser
Dans `CharacterPhysicsSystem::update`, pour chaque personnage, **avant** l'intégration de la
gravité et le balayage :
- **Déclenchement** (`EX-GP-013`) : si `jumpPressed` **et** le personnage est **au sol**
  (`grounded`), appliquer l'impulsion : `velocity.y = -jumpSpeed` (y vers le bas → saut = négatif).
  Le décollage remet `grounded` à `false` (il le sera de toute façon au prochain balayage).
- **Hauteur variable** : si le bouton **n'est plus maintenu** (`!jumpHeld`) alors que le personnage
  **monte encore** (`velocity.y < 0`), **couper** la vitesse ascendante :
  `velocity.y = velocity.y * jumpCutFactor` (ou une borne équivalente). Relâcher tôt ⇒ petit saut.
- La suite (gravité constante, borne de chute, balayage `sweepAabb`, annulation sur axes bloqués,
  recalcul de `grounded`) **reste celle du LOT-08**.

> Le **coyote time** et le **jump buffering** (qui assouplissent la condition « au sol » et
> mémorisent l'appui) sont ajoutés en **TACHE-04** : ici, on implémente le saut « strict au sol »
> et la hauteur variable.

## Fichiers impactés
- `Source/Core/Ecs/Systems/CharacterPhysicsSystem.h`/`.cpp`.
- Tests unitaires/d'intégration du saut.

## Tests (obligatoires)
- **Saut au sol** : au sol, `jumpPressed` → le personnage décolle (`velocity.y < 0`) et **monte**.
- **Pas de saut en l'air** (`EX-GP-013`) : en chute (non `grounded`), `jumpPressed` **n'a aucun
  effet** (pas de double saut).
- **Hauteur pleine** : bouton maintenu tout le saut → hauteur ≈ 2,5 tuiles (tolérance de réglage).
- **Hauteur variable** : relâcher tôt (`jumpHeld` faux pendant la montée) → **apogée plus basse**
  qu'un saut maintenu.
- **Gravité inchangée** : après l'apex, retombée identique au LOT-08 (gravité constante).
- **Déterminisme** : mêmes entrées (dont l'appui) → même trajectoire (`EX-NFR-002`).

## Points d'attention
- **Ordre** dans le pas : traiter le saut/coupe **avant** la gravité et le balayage.
- **Signe** : y vers le bas → impulsion et montée sont **négatives** ; cohérent avec `grounded`
  (`normal.y < 0`).
- **Constance de la gravité** : ne pas moduler `gravity` ; la hauteur variable passe uniquement par
  la coupe de vitesse.
- Réutiliser tel quel le balayage et la résolution du LOT-08 ; n'ajouter que la logique de saut.

## Définition de fait (DoD)
- Saut au sol + hauteur variable fonctionnels, documentés et **testés** (`ctest` vert) ;
  build `/W4 /WX`.

## Exigences
`EX-GP-011`, `EX-GP-013`, `EX-NFR-002`, `EX-ARCH-011`.
