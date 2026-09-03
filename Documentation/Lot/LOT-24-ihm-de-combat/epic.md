# LOT-24 — IHM de combat {#lot-24}

> Statut : **à faire**.
> Prérequis : [LOT-21](@ref lot-21), [LOT-23](@ref lot-23).

## Objectif

Rendre le combat **lisible et jouable** : bandeau d'initiative, portées surlignées, curseur de
ciblage, prévisualisations, journal.

C'est le lot qui transforme une machine à états correcte en un combat qu'on a envie de jouer.

## Périmètre

- `Source/HMI/Game/CombatHud.{h,cpp}` : bandeau d'ordre d'initiative, PV, conditions actives.
- **Surbrillance de grille** via `SpriteBatch` / `ComposedScene` — le calque `EditorOverlay` hérité
  se réemploie tel quel pour les cases atteignables et les portées.
- **Curseur de ciblage** clavier et manette (pas seulement souris : cf. critère ci-dessous).
- **Prévisualisations** avant validation : chemin emprunté, cases atteintes par un gabarit,
  probabilité de toucher.
- **Journal de combat** défilant, alimenté par les entrées auditables du `LOT-21`.
- Animations d'attaque via `AnimationCatalog`.

## Le critère qui est souvent oublié

Un combat tactique doit se jouer **entièrement au clavier et entièrement à la manette**, pas
seulement à la souris. Le projet hérite d'un système de remappage complet (clavier, manette XInput)
et d'une navigation à la manette dans tous les écrans : il serait incohérent que le combat, cœur du
jeu, soit le seul écran à exiger une souris.

## Exigences couvertes

`EX-IHM-*`, `EX-REN-*`.

## Critères d'acceptation

- Un combat complet se joue **entièrement au clavier** et **entièrement à la manette**.
- Chaque jet affiché est traçable au journal.
- `check_design_tokens.py` vert (la palette d'identité reste cohérente entre maquettes et code).
- Traduction fr/en complète.
- Vérification IHM manuelle par l'utilisateur, comme pour tout lot de rendu.
