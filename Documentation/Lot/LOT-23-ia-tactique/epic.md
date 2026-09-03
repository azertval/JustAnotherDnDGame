# LOT-23 — IA tactique ennemie {#lot-23}

> Statut : **à faire**.
> Prérequis : [LOT-20](@ref lot-20), [LOT-21](@ref lot-21), [LOT-22](@ref lot-22).

## Objectif

Donner aux ennemis un comportement de combat crédible, **déterministe** et testable sans GPU.

## La décision de conception

**Heuristiques pondérées, pas de réseau de neurones.**

Le dépôt d'origine embarquait 12 000 lignes de solveur RL (tenseurs, autodiff, réseaux,
optimiseurs, quatre familles d'algorithmes), retirées au `LOT-01`. La tentation serait de les
ressusciter ici : c'est le seul endroit du programme où elles auraient pu resservir.

Il ne faut pas. Une IA tactique par heuristiques est **plus prévisible, plus déboguable et bien
moins coûteuse** ; surtout, le déterminisme est ici un **critère d'acceptation**, pas une
préférence — sans lui, aucun test de combat ne tient. Un agent entraîné rendrait chaque
régression irreproductible.

## Périmètre

- `Source/Core/Combat/EnemyAi.{h,cpp}` : évaluation de cibles pondérée (distance, PV restants,
  menace, portée), choix de position, choix d'action.
- **Profils de comportement en JSON** : agressif, prudent, soutien, archer. Les poids sont des
  données, pas des constantes C++ (`EX-VIS-007`) — c'est ce qui permet d'ajuster un ennemi sans
  recompiler.

## Les deux défauts à prévenir

- **Le blocage.** Une IA qui ne trouve aucune action valide et passe son tour indéfiniment gèle le
  combat. Un test IA contre IA doit toujours **terminer**.
- **Le suicide.** Une IA qui ne pondère que la distance finit son tour au milieu de trois ennemis à
  bas PV. La menace subie doit peser dans le choix de position.

## Exigences couvertes

`EX-CBT-*` (choix de cible, de position et d'action).

## Critères d'acceptation

- Un combat IA contre IA **se termine toujours**, sur un échantillon de configurations générées.
- Rejeu à graine fixe strictement reproductible.
- L'IA ne finit pas son tour à portée de trois ennemis quand une position sûre existait.
- Tests **headless** intégralement automatisables — aucune vérification manuelle.
