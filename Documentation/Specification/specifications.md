# Spécifications {#specifications}

Les spécifications décrivent *quoi* faire et *pourquoi*, indépendamment de
l'implémentation. Elles servent de référence pour découper les [lots de
travail](@ref lots) et sont tracées par des identifiants d'exigences `EX-…`.

> **Deux programmes de lots, deux notations.** Ce dépôt est dérivé de `ProjectGaming`, un jeu de
> plateforme livré après 74 lots. Les deux numérotations repartent de `LOT-01` et **se recouvrent
> entièrement** : un renvoi nu serait ambigu sans être cassé, invisible au lint comme à Doxygen.
> D'où la convention, posée par le `LOT-78` et **vérifiée en CI** par `scripts/lint_lots.py` :
>
> - **`LOT-NN`** désigne un lot **de ce programme** — il doit exister, livré ou inscrit à la
>   [feuille de route](@ref roadmap-010) ;
> - **`LOT-H-NN`** désigne un lot **hérité**, archivé en lecture seule sous
>   `Documentation/Heritage/Lot/` et hors périmètre Doxygen.
>
> La plupart des renvois `LOT-H-NN` de ces pages accompagnent un « Concrétisé en… » : ils datent
> une décision du jeu d'origine, ils ne promettent rien pour le RPG.

## Documents

L'**ordre de lecture ci-dessous est la seule source d'ordre** : il est porté par
la liste `@subpage` de cette page, et non par les noms de fichiers (les
spécifications n'ont pas de préfixe numérique). Insérer une spécification =
créer son fichier avec une ancre `{#spec-<nom>}` et ajouter une ligne
`@subpage` à la position voulue ci-dessous — sans rien renuméroter.

- @subpage spec-vision
- @subpage spec-gameplay
- @subpage spec-regles-dnd
- @subpage spec-rpg
- @subpage spec-combat
- @subpage spec-inventaire
- @subpage spec-contenu
- @subpage spec-controles
- @subpage spec-rendu-technique
- @subpage spec-niveaux
- @subpage spec-exploration
- @subpage spec-exigences
- @subpage spec-editeur
- @subpage spec-interface-ihm
- @subpage spec-ia
- @subpage spec-architecture
- @subpage spec-decors
- @subpage spec-conventions
