# TACHE-03 — Capacites par tableau et charges de dash {#lot-67-tache-03-capacites-tableau}

**Lot :** [LOT-67](epic.md) · **Emplacement :** `Source/Core/{Physics,Ecs,Levels}`, `Source/HMI/Game` · **Statut :** fait

## Contexte
Un tableau pouvait deja **limiter** le nombre total de sauts et de dashs (`EX-GP-024`), mais pas
**moduler** les capacites du personnage. `PhysicsConfig::airJumps` etait global et code en dur, et
le dash n'avait qu'un booleen `Player::dashAvailable` recharge a l'atterrissage — donc exactement un
dash par saut, sans reglage possible.

`GameSession` construisait par ailleurs son systeme de physique par defaut : aucune `PhysicsConfig`
personnalisee n'etait passee nulle part en production. Il n'existait aucun point d'injection.

## Travail a realiser
- Ajouter `PhysicsConfig::dashCharges` (defaut `1`, soit le comportement historique) et remplacer
  `Player::dashAvailable` (booleen) par `Player::dashChargesRemaining` (compteur). Adapter la
  consommation dans `applyDash` et la recharge au contact du sol.
- Ajouter `CharacterPhysicsSystem::setConfig` : le systeme n'a **aucun** etat interne (tout l'etat
  de simulation vit dans les composants ECS), echanger sa configuration est donc sans effet de
  bord — et bien plus leger que le reconstruire en cours de vie de `GameSession`.
- Porter `airJumps` et `dashCharges` (`std::optional<int>`) a travers `Level`, `LevelDraft`
  (membres, snapshot, restauration, `fromLevel`), `LevelLoader` et `LevelWriter`, omis du JSON quand
  absents.
- Appliquer la configuration dans `GameSession::loadLevel`, **avant** l'apparition du personnage :
  sa recharge initiale en depend.
- **Corriger** `setJumpBudget`/`setDashBudget`, seules proprietes de niveau du brouillon a ne pas
  empiler de `pushUndo()` — donc les seules non annulables, contrairement au fond, au jeu de skins
  et au cadrage.

## Fichiers impactes
`Source/Core/Physics/PhysicsConfig.h`, `Source/Core/Ecs/Components/Player.h`,
`Source/Core/Ecs/Systems/CharacterPhysicsSystem.{h,cpp}`, `Source/Core/Levels/Level.h`,
`LevelLoader.cpp`, `LevelWriter.{h,cpp}`, `LevelDraft.{h,cpp}`, `Source/HMI/Game/GameSession.cpp`.

## Tests (obligatoires)
- `test_physique_personnage.cpp` : deux charges de dash autorisent deux ruees en l'air, la
  troisieme est refusee ; les tests existants du dash simple restent valides apres migration.
- `test_level_writer.cpp` : capacites conservees par un aller-retour, omises par defaut.
- `test_level_draft.cpp` : les quatre regles de tableau annulables ; capacites survivant a la
  conversion en niveau jouable.

## Points d'attention
Ne jamais confondre **budget** et **capacite** : le premier se consomme une fois pour toutes sur le
tableau, la seconde se recharge a chaque atterrissage. La documentation et le panneau les separent
explicitement pour cette raison.

Le renommage de `dashAvailable` change aussi son **type** : le compilateur recense les sites, mais
verifier qu'aucun affichage de diagnostic ne le lisait.

## Definition de fait (DoD)
Un niveau sans capacite declaree se joue a l'identique. Les quatre regles sont annulables.
`ctest` a 100 %.

## Exigences
`EX-GP-055`, `EX-GP-024`, `EX-GP-015`, `EX-GP-017`, `EX-EDIT-005`, `EX-LVL-008`.
