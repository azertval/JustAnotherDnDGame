# TACHE-04 — Intégration GameScreen + niveau demo4 + preuves {#lot-12-tache-04-integration-puzzle}

**Lot :** [LOT-12](epic.md) · **Emplacement :** `Source/HMI`, `Source/Elements/Levels` · **Statut :** à faire

## Contexte
Dernière tâche : brancher mécanismes + budget dans le jeu et livrer un **niveau puzzle** prouvé.

## Travail à réaliser
- **`GameScreen`** :
  - détenir un `core::MechanismController` (reconstruit à chaque `loadLevel`) ;
  - chaque pas : exécuter `mechanisms.update(boîte perso)`, puis passer
    `mechanisms.collisionMap()` à la physique (au lieu de `level.tileMap()`) ;
  - au spawn/reset : initialiser `Player::jumpsRemaining`/`dashesRemaining` depuis
    `level.jumpBudget()`/`dashBudget()` ;
  - **retour visuel** des portes : mettre à jour la **teinte** du sprite de chaque tuile porte selon
    son état (fermée opaque / ouverte atténuée).
- **Niveau `demo4.json`** : un tableau **puzzle** — p. ex. une **porte** bloquant la sortie, ouverte
  par un **interrupteur** en détour, avec un **budget** de sauts/dashs serré imposant un plan.
  Ajouté à la **séquence** (`main`), format et validation conformes.
- **Tests** :
  - **intégration** : `demo4` franchissable avec le bon plan (toucher l'interrupteur puis sortir) ;
    et **non** franchissable en ignorant l'interrupteur (porte fermée) ;
  - **système** : la séquence complète (`demo → … → demo4`) est franchie.

## Fichiers impactés
- `Source/HMI/Interface/GameScreen.h`/`.cpp`, `Source/HMI/main.cpp`.
- `Source/Elements/Levels/demo4.json`.
- `Source/Test/Integration/test_physique_personnage.cpp` (ou dédié), `Source/Test/Systeme/test_parcours_complet.cpp`.

## Tests (obligatoires)
- `demo4` : franchissable **avec** l'interrupteur ; bloqué **sans** (porte fermée).
- Parcours **système** complet vert.

## Points d'attention
- **Grille de collision** : la physique doit consommer la grille des mécanismes (portes), pas la
  carte brute — sinon les portes ne bloquent jamais.
- **Budget** au spawn **et** au reset (rejouer le niveau redonne le budget plein).
- **Faisabilité** : le budget doit rester suffisant pour un plan correct (marge), le puzzle vient de
  l'**ordre** (interrupteur avant la porte), pas d'un timing à la frame près.

## Définition de fait (DoD)
- Mécanismes + budget jouables ; `demo4` livré et enchaîné ; preuves intégration + système vertes ;
  vérif visuelle des portes ; build `/W4 /WX`, `CHANGELOG.md` à jour.

## Exigences
`EX-GP-020`, `EX-GP-021`, `EX-GP-024`, `EX-LVL-004`, `EX-LVL-010`, `EX-NFR-002`, `EX-NFR-020`.
