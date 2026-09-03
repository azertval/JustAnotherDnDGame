# TACHE-02 — Modèle de tuiles et de niveau {#lot-07-tache-02-modele-niveau}

**Lot :** [LOT-07](epic.md) · **Emplacement :** `Source/Core/Levels` · **Statut :** à faire

## Contexte
Avant de charger un fichier, il faut un **modèle en mémoire** du niveau. Le gameplay (collisions,
mécanismes) et le rendu s'appuieront tous sur cette représentation : une **grille de tuiles
typées** (`EX-GP-001`) et les données du niveau (`EX-LVL-002`). Le modèle est **donnée pure**
(`EX-ARCH-011`), sans dépendance rendu ni fichier.

## Travail à réaliser
- **`TileType`** (`enum class`) : `Empty`, `Solid`, `Danger`, `Entry`, `Exit`, `Switch`, `Door`.
- **`TileMap`** : grille `width × height` de `TileType`, origine haut-gauche ; accès
  `tile(column, row)`, dimensions, et un utilitaire de **solidité** (`isSolid`) pour le futur
  gameplay (`EX-GP-002`). Coordonnées en `[colonne, ligne]`.
- **`Mechanism`** : liaison d'un **interrupteur** (position) à une **porte** (position).
- **`Level`** : `TileMap` + position d'**entrée** + position de **sortie** + liste de
  `Mechanism` + nom. Accès en lecture ; construit par le chargeur (TACHE-03).

## Fichiers impactés
- `Source/Core/Levels/TileType.h`, `TileMap.h`/`.cpp`, `Level.h`/`.cpp` (nouveau).
- `Source/Core/CMakeLists.txt`, `Source/Test/CMakeLists.txt`.

## Tests (obligatoires)
- Construction d'une `TileMap` de dimensions données ; lecture/écriture d'une tuile à
  `(colonne, ligne)` ; dimensions correctes.
- `isSolid` vrai pour `Solid` (et les types bloquants retenus), faux pour `Empty`.
- Un `Level` restitue son entrée, sa sortie et ses mécanismes.

## Points d'attention
- **Donnée pure** : aucun type DirectX ni accès fichier ici ; testable sans fenêtre ni GPU.
- Repère du projet : origine **haut-gauche**, `[colonne, ligne]` (cohérent avec `niveaux.md` et
  `EX-ARCH-020`).
- Ne pas mélanger état de **simulation** (positions de mécanismes ouverts/fermés) avec le
  **modèle statique** chargé : ce lot ne fait que représenter, le comportement viendra plus tard.

## Définition de fait (DoD)
- `TileType`/`TileMap`/`Level` fonctionnels et testés (`ctest` vert) ; build `/W4 /WX`, documentés.

## Exigences
`EX-GP-001`, `EX-GP-002`, `EX-LVL-002`, `EX-ARCH-010`, `EX-ARCH-011`.
