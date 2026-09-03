# LOT-06 — Déplacement top-down 8 directions {#lot-06}

> Statut : **à faire**.
> Prérequis : [LOT-04](@ref lot-04) (couche de collision), [LOT-05](@ref lot-05) (mode
> d'exploration où brancher le système).

## Objectif

Rendre le jeu **jouable à nouveau** : un personnage qui se déplace librement en 8 directions sur
une carte en vue de dessus, sans gravité.

C'est le lot qui referme la parenthèse ouverte par le `LOT-01`, où la physique de plateforme a été
retirée sans remplaçant.

## Ce qui existe déjà, et qu'il ne faut pas réécrire

- **`core::sweepAabb`** (`Source/Core/Physics/SweptCollision.h`) : balayage continu axe par axe
  contre la grille, **sans gravité ni axe privilégié**, avec glissement le long des murs et aucune
  traversée à vitesse élevée. C'est exactement la primitive d'un déplacement top-down. Le `LOT-01`
  l'a débarrassée du suivi de pente ; elle est redevenue un pur balayage de grille.
- **`core::PlayerInput`** porte déjà `moveX` **et** `moveY`, alimentés par `hmi::PlayerInputMapper`
  depuis le clavier et la manette.
- **La normalisation 8 directions existe déjà** : le dash du platformer la faisait
  (`direction.normalized() * speed`). Elle se reprend telle quelle pour la marche.
- La convention de monde est la bonne (`EX-ARCH-020`) : 1 unité = 1 tuile, origine haut-gauche,
  Y vers le bas, 16 px/unité.

Ce lot est donc essentiellement de l'**assemblage**, pas de l'invention.

## Périmètre

- `Source/Core/Ecs/Systems/TopDownMovementSystem.{h,cpp}` (~150 lignes attendues) : intention →
  vitesse → `sweepAabb` → position.
- `Source/Core/Ecs/Components/Actor.h` : remplaçant de `core::Player`, sans ses ~30 champs de
  plateforme (contact au sol, coyote time, jump buffering, dash, combos) devenus inertes au
  `LOT-01`. Porte l'orientation (*facing*), utile au sprite et à l'interaction du `LOT-10`.
- `Source/Core/Physics/TopDownConfig.h` : vitesse, accélération, friction — donnée pure, à côté de
  `PhysicsConfig` dont les champs de saut disparaissent.
- Branchement dans `ExplorationMode`.

## Le piège classique

**La diagonale ne doit pas être plus rapide.** Sans normalisation, aller en diagonale donne
`√2 ≈ 1,41` fois la vitesse cardinale — le défaut le plus courant du genre, et le plus visible en
jeu. C'est un critère d'acceptation, pas un détail de finition.

## Exigences couvertes

Catégorie `EX-EXP-*`, déclarée par ce lot dans `Documentation/Specification/exploration.md` :
déplacement isotrope normalisé, glissement le long des murs, absence
de gravité, orientation conservée à l'arrêt.

## Critères d'acceptation

- Le personnage ne traverse **aucun** mur, même à vitesse élevée (le balayage continu s'en charge).
- **Vitesse diagonale = vitesse cardinale**, vérifié par un test.
- Glissement le long d'un mur pris en biais, sans blocage ni accroche.
- Déterminisme : mêmes entrées → mêmes positions au rejeu (`EX-NFR-002`).
- Testable **headless**, sans fenêtre ni GPU (`EX-ARCH-001`).
