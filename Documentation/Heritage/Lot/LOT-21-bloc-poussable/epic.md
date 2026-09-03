# LOT-21 — Bloc poussable {#lot-21}

> Statut : **terminé**. Nouveau mécanisme de puzzle : le **bloc poussable**, spécifié dès
> `gameplay.md` (`EX-GP-022`) mais jamais livré jusqu'ici.

## Objectif
`EX-GP-022` demande un bloc déplaçable horizontalement par le personnage et retombant sous
gravité. Aucun code de gameplay ne le gère : `TileType` ne connaît que des tuiles à **état**
(porte ouverte/fermée) ou statiques, jamais une tuile qui change de **position** en jeu. Ce lot
introduit ce comportement, sur le modèle déjà établi par `core::MechanismController`
(LOT-12/LOT-19) plutôt qu'en réinventant une architecture séparée.

## Périmètre

### Inclus
- **`TileType::Block`**, nouvelle tuile plaçable depuis la palette de l'éditeur comme n'importe
  quelle autre (pas de liaison à établir, contrairement à `Switch`/`PressurePlate`↔`Door`).
- **`core::BlockController`** (nouveau, `Core/Gameplay`) : logique **pure**, résolue chaque pas
  fixe, **avant** la physique du personnage —
  - **poussée** horizontale d'une case si le personnage touche le bloc du côté vers lequel il se
    déplace et que la case suivante est libre (ni mur, ni autre bloc) ;
  - **chute** discrète (une case entière, au rythme d'un pas configurable) si le bloc n'est plus
    soutenu par la case du dessous.
- Intégration dans `hmi::GameScreen` : la position courante des blocs complète la grille de
  collision déjà résolue par `MechanismController`, et une entité-tuile par bloc est repositionnée
  chaque pas pour le rendu (même principe que le retour visuel des portes).

### Exclus (hors périmètre de ce lot)
- **Bloc sur plaque de pression** : un bloc posé sur une `PressurePlate` ne l'active pas.
  `MechanismController` ne connaît que la boîte et la masse du **personnage** ; lui faire
  interroger aussi les positions de blocs est une évolution distincte, non nécessaire pour livrer
  le bloc poussable lui-même.
- **Clé / porte verrouillée** (`EX-GP-023`, optionnelle au MVP) — mécanique de collecte sans
  rapport avec un bloc poussable, laissée à un lot séparé si elle est un jour priorisée.
- **Poussée verticale ou diagonale** — `EX-GP-022` ne demande qu'un déplacement horizontal ; la
  chute couvre déjà l'axe vertical.
- **Chute continue (position flottante, animation lissée)** — décision de cadrage ci-dessous.

## Décisions de cadrage
- **Un bloc occupe toujours exactement une case, jamais une position intermédiaire.** Cohérent
  avec le reste des mécanismes de ce moteur (une porte bascule d'un état à l'autre sans étape
  intermédiaire) : la chute est **discrète** (une case entière au bout d'un nombre de pas fixes
  configurable, `BlockController::FALL_INTERVAL_STEPS`), pas une intégration continue façon
  `CharacterPhysicsSystem`. Plus simple à raisonner, à tester, et à garder déterministe — un bloc
  qui tomberait en position flottante continue exigerait de le faire participer au balayage
  `sweepAabb` comme un second acteur physique, une complexité non justifiée par le besoin.
- **La poussée est résolue AVANT la physique du personnage, avec la boîte du pas précédent.**
  `GameScreen::update` appelle `BlockController::update` avant `CharacterPhysicsSystem::update`,
  en lui passant la boîte du personnage **telle que laissée par le pas précédent** : ainsi, un bloc
  qui vient de se dégager ne bloque jamais le personnage sur ce même pas (la case est déjà libre
  quand le balayage de collision s'exécute juste après).
- **`collisionMap(base)` efface d'abord la position d'origine de chaque bloc.** `base` (la grille
  résolue par `MechanismController`) porte encore la tuile `Block` à sa position de **départ**,
  jamais mise à jour par le chargeur : sans effacement préalable, une case quittée par un bloc
  resterait perçue comme un mur, indéfiniment (mur fantôme — bug réellement rencontré en écrivant
  le contrôleur, corrigé avant livraison).

## Exigences couvertes
- `EX-GP-022` — implémentée.

## Découpage

> État : ✅ fait · 🔄 en cours · ⬜ non commencé.

| Tâche | Intitulé | Emplacement | État |
|-------|----------|-------------|:----:|
| [TACHE-01](tache-01-controleur-blocs.md) | Modèle et contrôleur (`Core`) | `Core/Levels`, `Core/Gameplay` | ✅ |
| [TACHE-02](tache-02-integration-editeur-jeu.md) | Intégration éditeur et jeu (`HMI`) | `HMI/Editor`, `HMI/Graphics`, `HMI/Interface` | ✅ |
| [TACHE-03](tache-03-documentation-verification.md) | Documentation et vérification | `Documentation` | ✅ |

## Critères d'acceptation du lot
1. Un bloc placé dans l'éditeur (palette) est chargé, sauvegardé et rechargé sans perte
   (round-trip JSON).
2. En jeu, pousser le personnage contre un bloc le déplace d'une case si la voie est libre ; un
   mur ou un autre bloc l'arrête.
3. Un bloc sans support tombe d'une case à la fois jusqu'à trouver un appui.
4. Aucune régression sur les mécanismes existants (interrupteur, plaque de pression, portes).
5. Logique nouvelle **couverte par des tests** (`ctest` vert), déterministe, sans GPU. Build
   `/W4 /WX` sans avertissement, Doxygen et lint des exigences verts.

## Dépendances
- Étend `TileType`/`TileMap` (LOT-03/07) et reprend le patron de `core::MechanismController`
  (LOT-12/LOT-19) sans le modifier. S'intègre à `hmi::GameScreen` (LOT-08 et suivants) et à la
  palette de l'éditeur (LOT-14/LOT-15).

## Navigation des tâches
- @subpage lot-21-tache-01-controleur-blocs
- @subpage lot-21-tache-02-integration-editeur-jeu
- @subpage lot-21-tache-03-documentation-verification
