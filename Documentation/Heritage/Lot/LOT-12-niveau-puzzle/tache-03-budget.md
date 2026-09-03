# TACHE-03 — Budget de sauts/dashs dans la physique {#lot-12-tache-03-budget}

**Lot :** [LOT-12](epic.md) · **Emplacement :** `Source/Core/Ecs/Systems` · **Statut :** à faire

## Contexte
Le `CharacterPhysicsSystem` doit **respecter** le budget (`EX-GP-024`) : refuser un saut ou un dash
quand le quota est épuisé et **décompter** chaque usage. `-1` = illimité (rétrocompatible).

## Travail à réaliser
Dans `CharacterPhysicsSystem::update`, aux points de déclenchement :
- **Saut** (toutes sources : sol/coyote, mur, aérien) : n'autoriser que si
  `player.jumpsRemaining != 0` ; à chaque saut effectif, **décrémenter** `jumpsRemaining` s'il est
  **limité** (`> 0`). `-1` reste illimité.
- **Dash** : n'autoriser que si `player.dashesRemaining != 0` ; à chaque dash, **décrémenter** s'il
  est limité.
- Ne pas toucher aux autres compteurs (coyote, sauts aériens, minuteries) : le budget est une
  **contrainte globale** du tableau, distincte des règles de rechargement au sol.

## Fichiers impactés
- `Source/Core/Ecs/Systems/CharacterPhysicsSystem.h`/`.cpp`.
- Tests d'intégration.

## Tests (obligatoires)
- **Budget de sauts** : avec `jumpsRemaining = 1`, un saut fonctionne, le suivant est **refusé**.
- **Budget de dashs** : avec `dashesRemaining = 1`, un dash fonctionne, le suivant est **refusé**.
- **Illimité** : `-1` → aucun refus (comportement des lots précédents inchangé, tests verts).
- **Déterminisme** (`EX-NFR-002`).

## Points d'attention
- **Distinguer** budget (global, ne se recharge pas au sol) et sauts aériens (rechargés au sol) :
  un saut consomme **un** budget **et** le compteur de source approprié.
- Convention `-1` = illimité appliquée partout.

## Définition de fait (DoD)
- Budget respecté et décompté, **testé** (`ctest` vert) ; build `/W4 /WX`.

## Exigences
`EX-GP-024`, `EX-NFR-002`, `EX-ARCH-011`.
