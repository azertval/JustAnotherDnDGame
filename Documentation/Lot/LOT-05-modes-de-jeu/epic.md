# LOT-05 — Modes de jeu {#lot-05}

> Statut : **à faire**.
> Prérequis : [LOT-02](@ref lot-02), [LOT-04](@ref lot-04).

## Objectif

Sortir de `hmi::GameSession` l'ordre **codé en dur** des passes du pas fixe, derrière une interface
`hmi::IGameMode`.

## Le problème

`Source/HMI/Game/GameSession.cpp` mêle deux rôles : **orchestrateur** du pas fixe (monde ECS,
caméra, événements, HUD, `FixedTimestep`, interpolation de rendu) et **mode de jeu** (l'ordre des
passes lui-même). Tant qu'il n'y avait qu'un genre, la confusion ne coûtait rien.

Le RPG a besoin d'au moins **trois** ordres différents :

- **exploration** : déplacement, animation, caméra, mécanismes, issue ;
- **dialogue** : monde gelé, seul le runner de dialogue avance ;
- **combat** : initiative, tour actif, résolution d'action — l'exploration ne tourne plus du tout.

Sans cette séparation, les trois s'entasseraient en `if` dans une fonction déjà longue.

## Périmètre

- `hmi::IGameMode` : interface à cinq méthodes environ — `onLoad(World&, const Level&)`,
  `step(World&, const PlayerInput&, float) -> LevelOutcome`, `onUnload()`, plus de quoi décrire
  l'ordre des passes pour les diagnostics.
- `GameSession` conserve : monde ECS, caméra et cadrage, événements, HUD, particules, secousse
  d'écran, `FixedTimestep`, interpolation. Il **délègue** l'ordre des passes.
- `ExplorationMode` : premier mode, extrait du corps actuel de `GameSession::update`.

## Le critère qui compte

Ce lot est un refactoring **à comportement constant**, et c'est le plus risqué du programme : tout
ce qui suit en dépend. Il doit être validé par un **test de rejeu déterministe** — mêmes entrées,
mêmes positions au flottant près sur 600 pas, avant et après extraction.

**N'y ajouter aucune fonctionnalité.** Le mode dialogue et le mode combat arrivent aux `LOT-15` et
`LOT-18` ; les écrire ici mélangerait un refactoring et une nouveauté, et on ne saurait plus lequel
des deux a cassé quoi.

## Exigences couvertes

`EX-ARCH-*` : « l'ordre des passes du pas fixe est une donnée du mode de jeu, jamais de
l'orchestrateur ».

## Critères d'acceptation

- Extraction à comportement **identique**, prouvée par le test de rejeu déterministe.
- Les tests de `GameSession` passent **sans modification de leur corps**.
- Aucun `if (mode == …)` résiduel dans `GameSession` : la sélection se fait par polymorphisme.
