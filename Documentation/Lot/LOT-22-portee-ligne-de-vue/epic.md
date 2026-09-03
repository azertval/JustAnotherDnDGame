# LOT-22 — Portée, ligne de vue et zones d'effet {#lot-22}

> Statut : **à faire**.
> Prérequis : [LOT-19](@ref lot-19).

## Objectif

Rendre la **géométrie** tactique signifiante : portée d'arme, ligne de vue bloquée, couverture,
gabarits d'effet de zone.

C'est ce lot qui fait qu'un combat se joue avec le terrain plutôt que sur une grille vide.

## Périmètre

- `Source/Core/Combat/LineOfSight.{h,cpp}` : tracé sur grille, obstacles issus de la **couche
  collision** du format v3 (`LOT-04`).
- **Couverture** partielle et totale, traduite en bonus de CA.
- `AreaOfEffect.{h,cpp}` : gabarits cercle, cône, ligne, carré — à l'échelle 1 case = 1,5 m figée
  au `LOT-12`.
- Portée d'arme et portée de sort, distinguées de la portée de déplacement.

## L'invariant à tester exhaustivement

**La ligne de vue doit être symétrique** : A voit B si et seulement si B voit A. C'est le défaut
classique des tracés sur grille — un algorithme qui part de A et s'arrête au premier obstacle ne
donne pas le même résultat en partant de B, et le joueur découvre qu'il peut tirer sur un ennemi
qui ne peut pas riposter (ou l'inverse, plus rageant encore).

À vérifier **exhaustivement sur des grilles générées**, pas sur trois cas choisis à la main.

## Exigences couvertes

`EX-CBT-*` (portée, ligne de vue, couverture, gabarits).

## Critères d'acceptation

- **Symétrie de la ligne de vue** vérifiée exhaustivement sur des grilles générées.
- Les gabarits couvrent exactement les cases attendues, figées par des cas de référence.
- La couverture modifie la CA du montant prévu, et ne s'applique jamais deux fois.
- Testable headless.
