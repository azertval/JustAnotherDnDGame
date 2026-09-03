# TACHE-01 — Données du saut : PlayerInput, Player, PhysicsConfig {#lot-09-tache-01-donnees-saut}

**Lot :** [LOT-09](epic.md) · **Emplacement :** `Source/Core` · **Statut :** à faire

## Contexte
Avant la logique de saut, il faut décrire son **état** en données pures (`EX-ARCH-011`) : l'intention
d'entrée, les minuteries de *game feel* et les réglages. Séparer ces données de la logique garde la
simulation déterministe et testable (comme au LOT-08).

## Travail à réaliser
- **`PlayerInput`** (Core/Physics) : ajouter l'intention de **saut** à deux signaux (`EX-CTRL-011`) :
  - `jumpPressed` — le saut vient d'être **pressé** cette frame (front montant) : déclenche le saut
    et alimente le *buffering* ;
  - `jumpHeld` — le bouton de saut est **maintenu** : sert à la hauteur variable.
- **`Player`** (Core/Ecs/Components) : ajouter les **minuteries** de *game feel*, exprimées en
  secondes et décomptées par la physique au pas fixe :
  - `coyoteTimer` — temps restant pendant lequel un saut est encore permis après avoir quitté le sol ;
  - `jumpBufferTimer` — temps restant de validité d'un saut pré-appuyé avant l'atterrissage.
- **`PhysicsConfig`** (Core/Physics) : ajouter les **réglages** (⚠️ à affiner) :
  - `jumpSpeed` — vitesse verticale de l'impulsion (calée pour ~2,5 tuiles / apex ~0,35 s) ;
  - `coyoteTime` (~0,08 s), `jumpBufferTime` (~0,12 s) ;
  - `jumpCutFactor` — fraction de vitesse ascendante conservée au relâchement (hauteur variable).

## Fichiers impactés
- `Source/Core/Physics/PlayerInput.h`, `Source/Core/Physics/PhysicsConfig.h`,
  `Source/Core/Ecs/Components/Player.h`.
- Tests unitaires associés (valeurs par défaut).

## Tests (obligatoires)
- Valeurs par défaut cohérentes : `PlayerInput::jumpPressed/jumpHeld == false`,
  `Player::coyoteTimer == 0` et `jumpBufferTimer == 0`, `PhysicsConfig` avec des réglages non nuls
  et plausibles.

## Points d'attention
- **Données pures** : aucun type DirectX, aucune logique ; unités monde et **secondes** documentées.
- Ne pas encore décompter les minuteries ici (c'est le rôle de TACHE-03/04) : cette tâche **déclare**.
- Rester rétrocompatible avec le LOT-08 : n'ajouter que des champs, sans casser l'existant.

## Définition de fait (DoD)
- Champs ajoutés, documentés (Doxygen en en-tête) et testés (`ctest` vert) ; build `/W4 /WX`.

## Exigences
`EX-CTRL-011`, `EX-ARCH-011`, `EX-NFR-010`.
