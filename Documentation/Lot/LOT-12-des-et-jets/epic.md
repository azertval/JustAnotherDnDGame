# LOT-12 — Dés, caractéristiques, jets {#lot-12}

> Statut : **à faire**.
> Prérequis : **aucun**. Pur `Core`, zéro dépendance — **parallélisable dès le `LOT-01`**.

## Objectif

Implémenter le cœur chiffré du système : dés déterministes, six caractéristiques, modificateurs,
jet d20 avec avantage et désavantage contre un degré de difficulté ou une classe d'armure.

## Pourquoi ce lot peut démarrer tout de suite

Il ne dépend ni du format de carte, ni du mode de jeu, ni du rendu. C'est du **calcul pur**, dans
`Core`, testable unitairement sans fenêtre ni GPU. C'est le meilleur candidat à une exécution en
parallèle du socle technique.

## Périmètre

- `Source/Core/Rpg/Dice.{h,cpp}` : notation `2d6+3` (parseur et évaluation).
- `Ability.h` : les six caractéristiques, modificateur `(score - 10) / 2` **arrondi vers le bas**
  (attention aux scores impairs inférieurs à 10 : `7 → -2`, pas `-1`).
- `Check.{h,cpp}` : jet d20 contre difficulté, **avantage** (max de deux d20), **désavantage** (min
  de deux d20), détection des 1 et 20 naturels.
- Fige l'**échelle 1 case = 1,5 m** (décision de cadrage) comme constante nommée : seule source de
  vérité des portées et vitesses, consommée par les `LOT-19` et `LOT-22`.

## Le socle déterministe

`Source/Core/Math/DeterministicRandom.h` fournit déjà splitMix64 et
`deriveSeed(baseSeed, step, entityId)`. **Il lui manque un `nextInt(min, max)`** — à ajouter ici,
sans biais modulo.

Le déterminisme n'est pas un confort : sans lui, **aucun test de combat n'est écrivable**. C'est
`EX-NFR-002`, qui prend avec ce lot une importance qu'il n'avait pas dans le platformer.

## Exigences couvertes

Catégorie `EX-DND-*`, déclarée par ce lot dans `Documentation/Specification/regles-dnd.md`.

## Critères d'acceptation

- Distribution vérifiée statistiquement sur 100 000 tirages à graine fixe (uniformité, bornes).
- **Rejouabilité stricte** : même graine → même séquence, y compris après sérialisation de l'état.
- Avantage = max de deux d20, désavantage = min ; les deux ensemble s'annulent.
- 1 et 20 naturels détectés et distingués d'un total de 1 ou 20.
- Aucune dépendance ECS, Qt ou rendu : `Core/Rpg/` compile seul.
