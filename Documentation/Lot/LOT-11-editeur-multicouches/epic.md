# LOT-11 — Éditeur multi-couches et placement d'entités {#lot-11}

> Statut : **à faire**.
> Prérequis : [LOT-04](@ref lot-04), [LOT-08](@ref lot-08), [LOT-10](@ref lot-10).

## Objectif

Rendre l'éditeur hérité capable d'éditer les **trois couches** et de poser des **entités** avec
leurs propriétés — c'est-à-dire de produire le contenu du RPG sans écrire de JSON à la main.

## Ce qui est déjà là

L'éditeur du dépôt d'origine est un poste de travail complet, et il est **entièrement conservé** :
peinture à la souris, outils rectangle et sélection, liaison de mécanismes, undo/redo, essai
immédiat, bibliothèque d'assets à vignettes avec rechargement à chaud et détection des références
avant suppression, atelier pixel art intégré. Ce lot **retarge**, il ne reconstruit pas.

Le `LOT-01` a retiré l'outil « Parcours » et le panneau « Propriétés », qui ne pilotaient que des
réglages de plateforme.

## Périmètre

- **Sélecteur de couche active** ; visibilité et opacité par couche (voir le sol sous le décor).
- Outil **« poser entité »** et panneau de propriétés d'entité (nouveau panneau, remplaçant celui
  retiré au `LOT-01`).
- Pose et édition des **portails** (carte cible, point d'arrivée) et des **points d'apparition**.
- Undo/redo étendu à tout cela — l'éditeur n'a jamais eu de mutation non annulable, cette règle
  tient.
- Vue **graphe du monde** dans le navigateur de cartes.

## La contrainte à rendre visible ici

Le combat se déroulant **sur la carte d'exploration** (décision de cadrage, cf.
`Documentation/Specification/vision.md`), *toute carte doit être un terrain tactique valide*.
L'éditeur doit **avertir** quand une zone est trop étroite ou trop encombrée pour y poser une
rencontre. Découvrir la contrainte au `LOT-18`, une fois vingt cartes dessinées, coûterait leur
reprise.

## Exigences couvertes

`EX-EDIT-*`.

## Critères d'acceptation

- Édition des trois couches avec undo/redo complet.
- Pose d'un PNJ, d'un coffre et d'un portail, avec leurs propriétés, puis essai immédiat.
- Aucune régression sur l'édition existante (peinture, sélection, liaisons, atelier pixel art).
- Avertissement visible quand une zone de rencontre n'est pas un terrain tactique valide.
