# TACHE-02 — Collision boîte-contre-boîte {#lot-24-tache-02-collision-boite-boite}

**Lot :** [LOT-24](epic.md) · **Emplacement :** `Core/Physics` · **Statut :** fait

## Contexte
Un bloc réduit occupe sa case **partiellement** : `sweepAabb` (solide/vide par case entière, cf.
`Core/Physics/SweptCollision.cpp`) ne peut pas l'exprimer. Cette tâche ajoute une résolution
**boîte contre boîte**, plus simple qu'un suivi de surface (`LOT-22`/`LOT-23`) mais nouvelle dans
ce moteur (jusqu'ici, toute collision passe par la grille).

## Travail à réaliser
- **Nouvelle fonction pure**, par exemple `core::sweepAabbVsAabb(box, delta, obstacle)`
  (`Core/Physics/SweptCollision.h`/`.cpp`, ou nouveau fichier dédié si le mélange grille/boîte
  nuit à la lisibilité) : balayage continu d'une boîte mobile contre une boîte **fixe** (l'obstacle
  résolu par l'axe de pénétration minimal, même esprit — clamp direct, pas d'interpolation — que
  `sweepX`/`sweepY` existants, pour éviter la même dérive flottante déjà écartée dans
  `SweptCollision.cpp`).
- **`Core/Ecs/Systems/CharacterPhysicsSystem.cpp`** (ou l'orchestration côté `HMI/GameScreen`, à
  trancher en implémentant selon où vivent déjà les positions de blocs) : après la résolution sur
  grille, pour chaque bloc réduit dont la boîte (centrée, taille `facteur × 1`) chevauche le
  chemin du personnage, appliquer `sweepAabbVsAabb` et composer le résultat avec celui du balayage
  sur grille (le plus restrictif des deux l'emporte).

## Fichiers impactés
- `Source/Core/Physics/SweptCollision.h`/`.cpp` (ou nouveau fichier dédié).
- `Source/Core/Ecs/Systems/CharacterPhysicsSystem.cpp` et/ou `Source/HMI/Interface/GameScreen.cpp`
  (selon l'endroit retenu pour composer grille + blocs réduits).
- Tests : nouveau `Source/Test/Unit/Core/Physics/test_sweep_aabb_vs_aabb.cpp` + nouveaux cas dans
  `test_physique_personnage.cpp` (approche, contact, glissement le long d'un bloc réduit).

## Tests (obligatoires)
- Le personnage est arrêté par un bloc réduit exactement à son bord (pas de pénétration, pas de
  saut de position).
- Le personnage peut passer dans l'espace **autour** d'un bloc réduit (le vide laissé par la
  différence entre la case et la boîte centrée) sans être bloqué par la case elle-même.
- Glissement le long d'un bloc réduit, comme contre un mur classique (cohérence de ressenti).
- **Régression** : tous les tests existants (grille, blocs pleins de `LOT-21`) restent verts sans
  modification.

## Points d'attention
- **Composer, ne pas remplacer.** Le résultat final doit être le plus restrictif entre le balayage
  sur grille (murs, blocs pleins, sol) et le balayage boîte-boîte (blocs réduits) — jamais l'un
  au détriment de l'autre. Un personnage ne doit jamais pouvoir traverser un mur classique sous
  prétexte qu'un bloc réduit était présent ailleurs sur le chemin.
- **Où vit la composition** (`Core` vs `HMI`) mérite une vraie décision, pas un choix par défaut :
  `CharacterPhysicsSystem` ne connaît aujourd'hui que la grille (`TileMap`), pas les positions de
  blocs (portées par `BlockController`, côté `HMI/GameScreen` comme `LOT-21` l'a établi) — il
  faudra soit lui faire accepter une liste de boîtes en plus de la grille, soit garder la
  composition côté `GameScreen` après son appel. Documenter le choix retenu ici une fois tranché.

### Décision retenue : composition côté `GameScreen`, `CharacterPhysicsSystem` inchangé

`HMI::GameScreen::update` compose les deux passes **après** `_physics.update(...)` (grille) et
**avant** `_animation.update(...)` (l'animation lit `Player::grounded`/`Velocity`, donc doit voir
l'état **final**) : le déplacement **réel** obtenu par la grille
(`transform.position - previousBox.min`) est retesté contre la boîte réelle de chaque bloc réduit
(`BlockController::scales()`/`boxAt`) via `core::sweepAabbVsAabb`. Par construction, cette seconde
passe ne peut que **réduire** le déplacement (jamais l'étendre) : elle part du résultat déjà
restreint par la grille, donc « le plus restrictif des deux l'emporte » est garanti sans calcul
supplémentaire. Si **plusieurs** blocs réduits se trouvent sur le chemin (cas rare, hors du
périmètre normal de ce lot), le résultat le plus proche de la position de départ l'emporte **par
axe**, pour rester sûr même dans ce cas non prioritaire.

Alternative écartée : faire accepter à `CharacterPhysicsSystem` une liste de boîtes en plus de la
grille. Rejetée pour ne **pas** faire dépendre le système ECS générique de `BlockController`
(couplage `Core/Ecs` → `Core/Gameplay` inexistant ailleurs dans le moteur) et pour rester cohérent
avec `LOT-21` : `BlockController` est déjà orchestré côté `GameScreen`, pas injecté dans la
physique.

## Définition de fait (DoD)
- Collision boîte-contre-boîte fonctionnelle et testée, composée correctement avec le balayage sur
  grille ; **zéro régression** ; build `/W4 /WX` sans avertissement. Vérifiée par 6 tests unitaires
  (`sweepAabbVsAabb` seul) et 2 tests d'intégration rejouant l'orchestration exacte de
  `GameScreen::update` (arrêt au bord réel, franchissement de l'espace autour).

## Exigences
`EX-GP-005` (implémentation complète).
