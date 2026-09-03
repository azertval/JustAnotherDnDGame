# TACHE-04 — Validation du niveau {#lot-07-tache-04-validation}

**Lot :** [LOT-07](epic.md) · **Emplacement :** `Source/Core/Levels` · **Statut :** à faire

## Contexte
Un fichier de niveau syntaxiquement correct peut rester **incohérent** (tuile hors de la grille,
sans sortie, mécanisme mal lié). Le chargement doit **valider** les données et signaler une
erreur **exploitable** (`EX-LVL-004`), sans planter (`EX-NFR-040`) — un level designer doit
comprendre ce qui ne va pas.

## Travail à réaliser
- Valider, après parsing (TACHE-03) et avant de rendre le `Level` :
  - **Positions dans les bornes** : chaque tuile a `0 ≤ x < width` et `0 ≤ y < height` ; pas de
    **deux tuiles** sur la même case.
  - **Entrée et sortie présentes** : exactement une tuile `entry` et une tuile `exit`
    (unicité retenue pour ce lot).
  - **Liaisons de mécanismes valides** : chaque `door.opensWith` référence un `switch.id`
    **existant** ; les `id` d'interrupteurs sont **uniques**.
- Renvoyer un **message d'erreur descriptif** (quel contrôle a échoué, où) via le résultat du
  chargeur.

## Fichiers impactés
- `Source/Core/Levels/LevelLoader.cpp` (ou `LevelValidator.h`/`.cpp` dédié), tests.
- `Source/Test/CMakeLists.txt`.

## Tests (obligatoires)
- **Tuile hors bornes** (`x ≥ width` ou `y ≥ height`) → rejet avec message.
- **Entrée ou sortie manquante** (ou en double) → rejet.
- **Liaison de mécanisme invalide** (`door.opensWith` sans interrupteur correspondant) → rejet.
- Un **niveau valide** est accepté.

## Points d'attention
- Séparer clairement **erreur de format** (TACHE-03 : JSON/caractère) et **erreur de validité**
  (cette tâche : cohérence métier) ; les deux sont récupérables et remontées de la même façon.
- Messages **compréhensibles par un non-codeur** (l'éditeur, plus tard, réutilisera cette
  validation — `EX-EDIT-007`/`EX-EDIT-010`, hors périmètre de livraison ici).
- Validation **pure** (aucune dépendance rendu) : testable sans fenêtre ni GPU.

## Définition de fait (DoD)
- Règles de validation implémentées, chaque échec testé avec un message exploitable
  (`ctest` vert) ; build `/W4 /WX`, documenté.

## Exigences
`EX-LVL-004`, `EX-NFR-040`, `EX-NFR-010`.
