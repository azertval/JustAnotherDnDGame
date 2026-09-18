# Source/Test/

Tests du projet, organisés par **niveau de test**.

| Sous-dossier | Portée |
|--------------|--------|
| `Unit/` | Tests unitaires : une fonction / une classe isolée (`../Core/`, `../HMI/`), sans dépendances externes. |
| `Integration/` | Tests d'intégration : interaction entre plusieurs modules (ex. Core ↔ Elements, Core ↔ HMI). |
| `Systeme/` | Tests système : le jeu complet, bout en bout (boucle, rendu, entrées, niveaux). |

## Convention
- Un fichier de test par module testé, nommé d'après la cible (ex. `test_world.cpp`).
- Les tests référencent les specs (`../../Documentation/Specification/`) et les critères d'acceptation des lots (`../../Documentation/Lot/`).
