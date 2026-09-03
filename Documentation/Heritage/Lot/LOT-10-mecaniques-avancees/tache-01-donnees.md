# TACHE-01 — Données des mécaniques (PlayerInput, Player, PhysicsConfig) {#lot-10-tache-01-donnees}

**Lot :** [LOT-10](epic.md) · **Emplacement :** `Source/Core` · **Statut :** à faire

## Contexte
Comme aux lots précédents, on déclare d'abord l'**état** des nouvelles mécaniques en **données
pures** (`EX-ARCH-011`), avant toute logique. Trois mécaniques : double saut, wall jump/slide, dash.

## Travail à réaliser
- **`PlayerInput`** (Core/Physics) :
  - `dashPressed` — front de l'action dash (`EX-CTRL-013`) ;
  - `moveY` — intention verticale de **visée** (−1 haut, +1 bas, 0), pour la direction du dash
    (le déplacement reste horizontal ; `moveY` ne sert qu'à viser).
- **`Player`** (Core/Ecs/Components) :
  - `airJumpsRemaining` — sauts aériens restants (double saut) ;
  - `facing` — orientation courante (−1 gauche, +1 droite), mise à jour au déplacement ;
  - `wallDirection` — sens du mur au contact (−1 mur à gauche, +1 à droite, 0 aucun) ;
  - `dashAvailable` — le dash est-il disponible (rechargé au sol) ;
  - `dashTimer` — durée restante du dash en cours (secondes).
- **`PhysicsConfig`** (Core/Physics) — réglages ⚠️ à affiner :
  - `airJumps` (défaut 1) ;
  - `wallSlideSpeed` (vitesse de descente plafonnée le long d'un mur) ;
  - `wallJumpSpeedX`, `wallJumpSpeedY` (impulsion diagonale du wall jump) ;
  - `dashSpeed`, `dashDuration`.

## Fichiers impactés
- `Source/Core/Physics/PlayerInput.h`, `Source/Core/Physics/PhysicsConfig.h`,
  `Source/Core/Ecs/Components/Player.h`.
- Tests unitaires des valeurs par défaut.

## Tests (obligatoires)
- Valeurs par défaut cohérentes : `PlayerInput::dashPressed == false`, `moveY == 0` ;
  `Player::airJumpsRemaining == 0`, `dashAvailable == false`, `dashTimer == 0`, `wallDirection == 0`,
  `facing` valeur neutre définie ; `PhysicsConfig` avec réglages non nuls et plausibles
  (`airJumps >= 1`, vitesses > 0).

## Points d'attention
- **Données pures** : aucune logique ni type DirectX ; unités monde et **secondes** documentées.
- N'ajouter que des **champs** (rétrocompatible LOT-09) ; ne pas décompter ici (rôle des systèmes).
- `facing` doit avoir une valeur initiale déterministe (p. ex. +1, vers la droite).

## Définition de fait (DoD)
- Champs ajoutés, documentés (Doxygen en en-tête) et testés (`ctest` vert) ; build `/W4 /WX`.

## Exigences
`EX-CTRL-013`, `EX-GP-015`, `EX-GP-016`, `EX-GP-017`, `EX-ARCH-011`, `EX-NFR-010`.
