# TACHE-04 — Coyote time + jump buffering {#lot-09-tache-04-coyote-buffering}

**Lot :** [LOT-09](epic.md) · **Emplacement :** `Source/Core/Ecs/Systems` · **Statut :** à faire

## Contexte
Le saut « strict au sol » (TACHE-03) est correct mais **sec** : il punit les erreurs d'une frame.
Deux minuteries corrigent ça et rendent le saut **indulgent**, sans tricher — c'est la signature
d'un platformer réactif. Elles s'appuient sur les champs déclarés en TACHE-01 (`Player::coyoteTimer`,
`Player::jumpBufferTimer`) et sur les fenêtres de `PhysicsConfig`.

## Travail à réaliser
Dans `CharacterPhysicsSystem::update`, au pas fixe `fixedDelta` :
- **Coyote time** : quand le personnage **est au sol**, (re)charger `coyoteTimer = coyoteTime` ;
  sinon le **décrémenter** de `fixedDelta` (borné à 0). La condition de saut de TACHE-03 devient :
  saut permis si `jumpPressed` **et** (`grounded` **ou** `coyoteTimer > 0`). Au décollage, remettre
  `coyoteTimer` à 0 (empêche un second saut dans la même fenêtre).
- **Jump buffering** : à chaque `jumpPressed`, (re)charger `jumpBufferTimer = jumpBufferTime` ;
  sinon le **décrémenter**. Le saut se déclenche si `jumpBufferTimer > 0` **et** le personnage est
  sautable (au sol ou coyote) — ainsi un appui **juste avant** l'atterrissage s'exécute à la pose.
  Consommer le buffer (le remettre à 0) au déclenchement.
- Fusionner proprement les deux avec la logique de TACHE-03 : une **seule** décision « peut sauter
  maintenant ? » combinant buffer (source de l'intention) et coyote/sol (autorisation).

## Fichiers impactés
- `Source/Core/Ecs/Systems/CharacterPhysicsSystem.h`/`.cpp`.
- Tests unitaires/d'intégration.

## Tests (obligatoires)
- **Coyote** : le personnage quitte un bord (devient non `grounded`) ; un `jumpPressed` **dans** la
  fenêtre coyote déclenche le saut ; **au-delà** de la fenêtre, non.
- **Buffering** : `jumpPressed` **avant** l'atterrissage (encore en l'air), puis pose **dans** la
  fenêtre buffer → le saut part à la pose ; au-delà de la fenêtre, non.
- **Pas de double saut** (`EX-GP-013`) : coyote et buffer n'autorisent **jamais** un second saut en
  plein vol hors fenêtre.
- **Décompte déterministe** : les minuteries décroissent de `fixedDelta` par pas (`EX-NFR-002`).

## Points d'attention
- **Au pas fixe uniquement** : décompte en `fixedDelta`, jamais en temps réel de frame.
- **Consommation** : remettre les minuteries à 0 au déclenchement pour éviter les sauts fantômes.
- **Un seul point de décision** : éviter deux chemins de déclenchement concurrents (buffer vs appui
  direct) — les unifier pour un comportement reproductible.
- Garder la logique **dans le système**, l'état **dans le composant** (`EX-ARCH-011`).

## Définition de fait (DoD)
- Coyote time et jump buffering fonctionnels, documentés et **testés** (`ctest` vert) ;
  build `/W4 /WX`.

## Exigences
`EX-CTRL-011`, `EX-GP-013`, `EX-NFR-002`, `EX-ARCH-011`.
