# Source/Elements/

**Assets et éléments statiques** du jeu.

## Périmètre
- `Levels/` — cartes (`.json`), copiées à côté de l'exécutable au build.
- `Localization/` — catalogues de traduction (`.lang`, `.ts`), copiés à côté de l'exécutable au build.
- `Assets/` — images et polices (scènes, PNJ, cartes peintes, illustrations, polices), copiées à
  côté de l'exécutable au build (`LOT-39`) ; voir `Assets/README.md`.
- `Rpg/` — catalogues du jeu de rôle (espèces, classes, historiques, compétences, créatures,
  équipement, règles et leurs schémas), copiés à côté de l'exécutable au build.
- `World/` — atlas du monde (régions, villes, lieux), arène et dialogues, copiés au build.
- `Maps/` — `world-maps.json`, positions relevées sur les cartes peintes (`LOT-94`), copié au build.
- `Credits/` — `credits.json`, les crédits affichés par le jeu.
- `UI/` — assets Qt déclaratifs de l'éditeur : mises en page Qt Designer (`.ui`) et ressource (`.qrc`).
- `Themes/` — feuilles de style Qt (`.qss`) de l'éditeur.

Ces éléments sont consommés par `../Core/` (données de carte et de jeu) et `../HMI/` (rendu, UI Qt).

> Note : les assets binaires volumineux peuvent être gérés à part (Git LFS) selon leur taille.
