# TACHE-02 — Intégration jeu (contrôleurs, résolution de fin de niveau) {#lot-31-tache-02-integration-jeu}

**Lot :** [LOT-31](epic.md) · **Emplacement :** `Core/Gameplay`, `Core/Levels` · **Statut :** ✅

## Contexte
Fait vivre les quatre variantes chaque pas fixe et branche leur résultat sur
`core::evaluateOutcome` (`LevelOutcome.cpp`), qui ne testait jusqu'ici qu'un recouvrement statique
avec `TileType::Danger`. Dépend du modèle posé en TACHE-01.

**Deux écarts de cadrage tranchés pendant l'implémentation** (l'un des deux était déjà envisagé
comme option dans la version initiale de cette tâche) :
- **Danger commuté résolu par `MechanismController`, pas par `DangerController`.** Consulter
  l'état d'un déclencheur nécessite la même détection front/continu (interrupteur vs plaque de
  pression) déjà écrite dans `MechanismController::update`. Plutôt que de dupliquer cette logique
  dans `DangerController`, `MechanismController` a été étendu : il porte aussi `_dangerLinks`
  (résolus depuis `Level::dangerLinks()`) et expose `isDangerActive(GridPosition)`, avec exactement
  le même calcul front/continu que pour une porte — seule différence, aucun effet sur la grille de
  collision (jamais solide). `DangerController` (nouveau, `Core/Gameplay/DangerController.h`/`.cpp`)
  ne porte donc que le **mobile** (`DangerMover` — position par aller-retour triangulaire
  déterministe basée sur un compteur de pas fixes, vitesse de conception `2 cases/s`) et le
  **temporisé** (`DangerBlink` — actif selon `(pasFixe - phase) mod period < activeDuration`).
  Le **directionnel** reste géométrie pure (`core::dangerHitbox`), sans contrôleur.
- **`core::evaluateOutcome` reste dans `Core/Levels`, sans dépendance vers `Core/Gameplay`.**
  Passer `DangerController`/`MechanismController` en paramètre aurait inversé la direction de
  dépendance actuelle (`Core/Gameplay` dépend de `Core/Levels`, jamais l'inverse —
  `MechanismController.h` inclut déjà `Level.h`/`TileMap.h`). À la place, `evaluateOutcome` gagne un
  paramètre `const std::vector<Aabb>& extraDangerBoxes = {}` : les boîtes **actuellement mortelles**
  des dangers à état (mobile/commuté/temporisé), assemblées par l'**appelant** (qui possède déjà les
  deux contrôleurs) — `HMI::GameScreen::collectActiveDangerBoxes` en jeu, un helper équivalent dans
  les tests d'intégration. Le danger directionnel (géométrie pure, sans état) reste résolu en
  interne par un balayage de la grille statique via `core::dangerHitbox`, comme `Danger` classique.

## Travail à réaliser
- **`core::MechanismController`** (étendu, pas nouveau) : `_dangerLinks`/`_dangerActive`/
  `_dangerContinuous`/`_playerOnDangerTriggerPrev` en plus des membres existants ; `isDangerActive
  (GridPosition)` (scan linéaire de `_dangerLinks`, `false` si aucune liaison ne correspond).
- **`core::DangerController`** (nouveau, `Core/Gameplay/DangerController.h`/`.cpp`) : `update()` par
  pas fixe (incrémente un compteur `_stepCount`), `moverBox(index)` (position triangulaire
  déterministe, `Aabb` en unités de grille), `isBlinkActive(GridPosition)`.
- **`core::evaluateOutcome`** (`LevelOutcome.cpp`) : le balayage de grille couvre désormais `Danger`
  et les quatre directionnels via `core::dangerHitbox` (au lieu du recouvrement de case entière) ;
  un nouveau paramètre `extraDangerBoxes` est testé après le balayage, avant le succès (`DangerMover`/
  `DangerSwitched`/`DangerBlink` sont explicitement exclus du balayage de grille — leur mortalité
  n'est jamais à leur position **statique** de la carte).
- **Câblage** (`HMI::GameScreen`) : `_dangers` (`DangerController`) instancié/mis à jour comme
  `_mechanisms`/`_blocks` ; nouvelle méthode privée `collectActiveDangerBoxes()` assemble les boîtes
  mobiles + temporisées actives + commutées actives, passée à `evaluateOutcome`.

## Fichiers impactés
- `Source/Core/Gameplay/DangerController.h`/`.cpp` (nouveau).
- `Source/Core/Gameplay/MechanismController.h`/`.cpp` (étendu : `_dangerLinks`, `isDangerActive`).
- `Source/Core/Levels/LevelOutcome.h`/`.cpp` (signature + logique étendues).
- `Source/HMI/Interface/GameScreen.h`/`.cpp` (câblage du nouveau contrôleur, `collectActiveDangerBoxes`,
  essai immédiat depuis l'éditeur inclus — même écran, aucun code dédié).
- `Source/Core/CMakeLists.txt`, `Source/Test/CMakeLists.txt` (nouveau fichier source/test).
- Tests : `Source/Test/Unit/Core/Gameplay/test_danger_controller.cpp` (nouveau, `DangerController`
  isolé), `Source/Test/Unit/Core/Levels/test_level_outcome.cpp` (nouveaux cas, `evaluateOutcome`
  isolé avec un `DangerController` factice), `Source/Test/Integration/test_danger_avance.cpp`
  (nouveau — `DangerController` **assemblé** avec `LevelOutcome` et `CharacterPhysicsSystem` sur
  plusieurs pas fixes, un scénario par variante : personnage qui longe un danger directionnel côté
  sûr puis le traverse par le bord mortel, qui se fait rattraper par un danger mobile, qui déclenche
  un interrupteur lié à un danger commuté, qui traverse un danger temporisé aux deux phases de son
  cycle).

## Tests (obligatoires)
- Directionnel : recouvrement du bord mortel → `Lost` ; recouvrement du reste de la case sans le
  bord → pas d'échec.
- Mobile : position courante suit exactement la formule déterministe attendue à un pas donné ;
  contact à la position **courante** tue, contact avec la position de **départ** une fois le
  mobile parti n'y tue plus.
- Commuté : mortel seulement quand le déclencheur lié est actif ; comportement de la porte liée au
  même déclencheur (s'il y en a une) inchangé.
- Temporisé : séquence active/inactive déterministe et reproductible sur plusieurs relances avec le
  même niveau ; deux tuiles de phases différentes désynchronisées comme attendu.
- Aucune régression sur `TileType::Danger` classique ni sur les mécanismes interrupteur↔porte
  existants (tests existants de `test_level_outcome.cpp`/`test_mechanism_controller.cpp` toujours
  verts sans modification de leurs assertions).

- `test_danger_avance.cpp` (intégration) : chaque scénario assemblé reproduit fidèlement le résultat
  attendu (`Lost` au bon moment, jamais avant/après) sur plusieurs pas fixes consécutifs, pas
  seulement à un instant isolé.

## Définition de fait (DoD)
- Les quatre variantes sont fonctionnelles **en jeu** (câblées dans `GameScreen`) avec des niveaux
  de test ad hoc (pas encore peignables depuis l'éditeur — palette en TACHE-03), logique couverte
  par des tests **aux deux niveaux** Unit (`test_danger_controller.cpp`, nouveaux cas de
  `test_mechanism_controller.cpp`/`test_level_outcome.cpp`) et Integration
  (`test_danger_avance.cpp`, quatre scénarios) — le niveau Système suit en TACHE-03, une fois les
  variantes peignables et intégrées à un niveau réel. Build `/W4 /WX` et suite complète
  (451 Unit + 81 Integration + 2 Système) vertes, sans régression.

## Exigences
`EX-GP-050`, `EX-GP-051`, `EX-GP-052`, `EX-GP-053`, `EX-GP-031`, `EX-GP-032`, `EX-NFR-002`.
