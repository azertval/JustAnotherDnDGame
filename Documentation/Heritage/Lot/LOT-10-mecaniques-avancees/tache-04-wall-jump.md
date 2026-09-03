# TACHE-04 — Wall jump + wall slide {#lot-10-tache-04-wall-jump}

**Lot :** [LOT-10](epic.md) · **Emplacement :** `Source/Core/Ecs/Systems` · **Statut :** à faire

## Contexte
Deuxième mécanique (`EX-GP-016`) : contre un mur en l'air, **glisser** (chute ralentie) et pouvoir
**sauter à l'opposé** du mur. Le contact mural est déjà connu — le balayage (`sweepAabb`) renvoie
une **normale horizontale** non nulle quand le déplacement est bloqué latéralement.

## Travail à réaliser
Dans `CharacterPhysicsSystem::update` :
- **Détection du mur** : après le balayage, si le personnage n'est **pas au sol** et qu'un contact
  **horizontal** a eu lieu (normale X non nulle) alors qu'il **pousse vers ce mur** (`moveX` de même
  sens), positionner `Player::wallDirection` (−1 mur à gauche, +1 à droite) ; sinon 0.
- **Wall slide** : si `wallDirection != 0` et que le personnage **descend** (`velocity.y > 0`),
  **plafonner** la vitesse de chute à `_config.wallSlideSpeed` (descente douce).
- **Wall jump** : si un saut est demandé alors que `wallDirection != 0` (et pas au sol), appliquer
  une **impulsion diagonale opposée** au mur :
  `velocity.x = -wallDirection * wallJumpSpeedX`, `velocity.y = -wallJumpSpeedY`. Consommer le buffer.
  Optionnel : recharger le double saut au wall jump (à décider à l'implémentation, documenter).
- Ordonner proprement avec le saut au sol/coyote/aérien (TACHE-03) : le wall jump est une source
  d'autorisation supplémentaire.

## Fichiers impactés
- `Source/Core/Ecs/Systems/CharacterPhysicsSystem.h`/`.cpp`.
- Tests d'intégration.

## Tests (obligatoires)
- **Wall slide** : collé à un mur en chute, la vitesse de descente est **plafonnée** (plus lente
  qu'une chute libre).
- **Wall jump** : contre un mur à droite, un saut envoie le personnage **vers la gauche et le haut**
  (`velocity.x < 0`, `velocity.y < 0`) ; symétrique pour un mur à gauche.
- **Pas de wall jump sans mur** : en l'air sans contact mural, la logique de wall jump ne s'active
  pas.
- **Déterminisme** (`EX-NFR-002`).

## Points d'attention
- **Pousser vers le mur** : ne considérer le contact mural que si l'intention va vers le mur (évite
  de « coller » en s'en éloignant).
- **Repère** : `y` bas → monter = négatif ; `wallDirection` = sens du mur, l'impulsion part à
  l'opposé.
- Ne pas confondre contact **mural** (normale X) et contact **sol** (normale Y) — `grounded` prime.

## Définition de fait (DoD)
- Wall slide + wall jump fonctionnels et **testés** (`ctest` vert) ; build `/W4 /WX`.

## Exigences
`EX-GP-016`, `EX-GP-011`, `EX-NFR-002`, `EX-ARCH-011`.
