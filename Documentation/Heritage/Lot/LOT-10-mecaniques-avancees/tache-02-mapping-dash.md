# TACHE-02 — Mapping du dash + direction de visée / orientation {#lot-10-tache-02-mapping-dash}

**Lot :** [LOT-10](epic.md) · **Emplacement :** `Source/HMI/Input` · **Statut :** à faire

## Contexte
Le dash a besoin d'une **action dédiée** (`EX-CTRL-013`) et d'une **direction de visée** à 8
directions. `toPlayerInput` (LOT-08/09) traduit déjà déplacement et saut ; on ajoute le dash et la
visée verticale, sans lier le gameplay aux touches (`EX-CTRL-010`).

## Travail à réaliser
- Étendre l'énumération `Key` : ajouter **`Shift`** (`VK_SHIFT = 0x10`) et les flèches **Up/Down**
  existent déjà (`0x26`/`0x28`).
- Dans `toPlayerInput` :
  - `dashPressed` = front de `Shift` (`keyPressed`) ;
  - `moveY` (visée verticale) = `bas − haut` avec `bas = keyDown(Down)`, `haut = keyDown(Up)` :
    +1 (bas), −1 (haut), 0.
  - `moveX` (déjà présent) sert aussi de visée horizontale.
- L'**orientation** (`Player::facing`) est mise à jour **par le système** au déplacement, pas par le
  mapper (le mapper reste sans état).

## Fichiers impactés
- `Source/HMI/Input/InputState.h` (énumérateur `Shift`), `Source/HMI/Input/PlayerInputMapper.cpp`.
- Test unitaire du mapping.

## Tests (obligatoires)
- `Shift` pressée → `dashPressed == true` ; maintenue sans nouveau front → `false`.
- `Down` → `moveY == +1` ; `Up` → `moveY == −1` ; haut+bas simultanés → `0`.
- Déplacement (`moveX`) et dash restent indépendants (axes distincts).

## Points d'attention
- **Front vs maintenu** : le dash se déclenche au **front** (`keyPressed`), comme le saut.
- **Repère** : `y` vers le bas → `bas = +1`, `haut = −1` (cohérent avec la physique).
- **Pur et testable** : le mapper lit un `InputState` injecté, sans fenêtre (`EX-NFR-010`).

## Définition de fait (DoD)
- Dash et visée mappés, **testés** (`ctest` vert) ; build `/W4 /WX`.

## Exigences
`EX-CTRL-013`, `EX-CTRL-010`, `EX-CTRL-011`, `EX-NFR-010`.
