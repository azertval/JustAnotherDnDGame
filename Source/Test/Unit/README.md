# Source/Test/Unit/

Tests **unitaires** : une fonction ou une classe isolée, sans dépendances externes (fichiers, rendu, fenêtre).

Cible principale : la logique de `../../Core/` (règles du jeu de rôle, combat, cartes, monde) et la
logique pure de `../../HMI/`.

## Arborescence

Les tests **reflètent l'arborescence des sources** : le test de `Source/<Module>/<X>` vit sous
`Unit/<Module>/test_<x>.cpp` (p. ex. `Source/Core/Ecs/World.h` → `Unit/Core/Ecs/test_world.cpp`).
Sous-dossiers actuels : `Core/` (`Combat`, `Data`, `Diagnostics`, `Ecs`, `Gameplay`, `Levels`,
`Math`, `Resources`, `Rpg`, `Time`, `World`) et `HMI/` (`Audio`, `Editor`, `Graphics`, `Input`,
`Interface`, `Localization`, `Platform`, `Presentation`, `Runtime`).
