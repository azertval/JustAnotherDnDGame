# TACHE-01 — Données : budget (Player, Level, LevelLoader) {#lot-12-tache-01-donnees}

**Lot :** [LOT-12](epic.md) · **Emplacement :** `Source/Core` · **Statut :** à faire

## Contexte
On déclare le **budget de mouvements** (`EX-GP-024`) en données pures, et on l'expose depuis le
niveau (format + chargement), avant la logique.

## Travail à réaliser
- **`Player`** (Core/Ecs/Components) : `jumpsRemaining` et `dashesRemaining` (`int`, **-1 =
  illimité**), initialisés au spawn depuis le niveau, décomptés par la physique (TACHE-03).
- **`Level`** (Core/Levels) : `jumpBudget` et `dashBudget` (`int`, défaut **-1**), accès en lecture ;
  ajout au constructeur avec **valeurs par défaut** (rétrocompatible).
- **`LevelLoader`** : parser les champs **optionnels** top-level `jumpBudget`/`dashBudget`
  (`root.value("jumpBudget", -1)`), les passer au `Level`.

## Fichiers impactés
- `Source/Core/Ecs/Components/Player.h`, `Source/Core/Levels/Level.h`,
  `Source/Core/Levels/LevelLoader.cpp`.
- Tests unitaires (valeurs par défaut, parsing du budget).

## Tests (obligatoires)
- `Player` par défaut : `jumpsRemaining == -1`, `dashesRemaining == -1` (illimité).
- `Level` par défaut : budgets **-1** ; un `Level` construit avec budgets les restitue.
- `LevelLoader` : un JSON **avec** `jumpBudget`/`dashBudget` les charge ; **sans**, ils valent -1.

## Points d'attention
- **Données pures** ; `-1` = illimité (convention documentée).
- Rétrocompatibilité : les niveaux existants (sans budget) restent valides et **illimités**.

## Définition de fait (DoD)
- Champs + parsing ajoutés, documentés, **testés** (`ctest` vert) ; build `/W4 /WX`.

## Exigences
`EX-GP-024`, `EX-LVL-002`, `EX-LVL-003`, `EX-ARCH-011`, `EX-NFR-010`.
