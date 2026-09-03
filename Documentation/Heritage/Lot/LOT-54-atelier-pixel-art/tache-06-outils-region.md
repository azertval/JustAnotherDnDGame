# TACHE-06 — Outils de région {#lot-54-tache-06-outils-region}

**Lot :** [LOT-54](epic.md) · **Emplacement :** `Source/HMI/Editor` · **Statut :** fait

## Contexte
Le cadrage initial du lot excluait la sélection, le déplacement de région et les transformations, au
motif qu'un premier jet devait rester minimal. Cette exclusion coûtait cher pour ce qu'elle
économisait : dessiner une tuile de bord droit quand la tuile de bord gauche existe déjà se fait en
une symétrie, et une planche à raccords de seize cases est massivement redondante par construction.
Sans ces outils, l'auteur repeint à la main ce qu'il a déjà dessiné.

Deux choses ont changé et rendent la réintégration peu coûteuse. D'une part, ces opérations sont des
**fonctions sur un tampon** — le même patron pur et testable que la TACHE-02, sans Qt ni GPU.
D'autre part, leur interface se réduit à des **actions** ([LOT-56](@ref lot-56)) : une entrée de
barre d'outils, un raccourci porté par l'action, rien à construire. Copier et coller ont même déjà
leur action remappable dans `hmi::EditorKeyBindings`, branchée par la TACHE-04 de
[LOT-57](@ref lot-57).

## Travail à réaliser
- **Sélection rectangulaire** : outil de sélection dans le groupe exclusif du canevas (TACHE-04),
  avec une région courante éventuellement vide, alignée sur la grille de pixels.
- **Déplacement de la région** sélectionnée, en laissant derrière elle des pixels transparents.
- **Symétrie** sur l'axe horizontal et sur l'axe vertical, appliquée à la région sélectionnée ou, à
  défaut de sélection, à l'image entière.
- **Rotation par quart de tour**, dans les deux sens ; sur une région non carrée, la rotation opère
  dans le rectangle englobant et le débordement est tronqué au cadre de l'image plutôt que de
  l'agrandir.
- **Copier et coller** : le contenu copié vit dans un tampon propre à l'atelier ; coller le dépose à
  la position courante, tronqué au cadre.
- **Opérations pures** : toutes ces transformations sont des fonctions sur un tampon et une région,
  ajoutées à `PixelOperations`, et chacune produit **une** entrée nommée dans l'historique
  (TACHE-02).
- **Traduction** des libellés, infobulles et noms d'opérations, dans les deux catalogues.

## Fichiers impactés
- `Source/HMI/Editor/PixelOperations.{h,cpp}` — transformations de région.
- `Source/HMI/Editor/PixelCanvas.{h,cpp}` — outil de sélection, tracé de la région, gestes.
- `Source/HMI/Interface/EditorActions.{h,cpp}` (créé en `LOT-56`) — actions de région.
- `Source/HMI/Interface/ThemeIcons.{h,cpp}` (créé en `LOT-56`) — icônes correspondantes.
- `Source/Elements/Localization/fr.lang`, `en.lang`.
- `Source/Test/Unit/HMI/Editor/test_pixel_operations.cpp` (complété).

## Tests (obligatoires)
- **Involution des symétries** : deux symétries successives sur le même axe restituent exactement
  l'image d'origine, avec et sans sélection active.
- **Cycle des rotations** : quatre rotations d'un quart de tour dans le même sens restituent
  l'image d'origine sur une région carrée.
- **Rotation non carrée** : le débordement est tronqué au cadre, jamais écrit hors du tampon.
- **Coller débordant** : coller près d'un bord tronque au cadre sans écriture hors limites.
- **Sélection vide** : une transformation sans sélection s'applique à l'image entière ; une sélection
  de largeur ou de hauteur nulle est sans effet et ne produit pas d'entrée d'historique.
- **Déplacement** : la zone quittée devient transparente ; un déplacement entièrement hors cadre
  laisse une image vide plutôt qu'un tampon corrompu.
- **Une entrée d'historique par transformation**, portant son nom, et annulable.
- Opérations **pures**, testées sans Qt ni GPU.

## Points d'attention
- **La troncature au cadre est la règle, pas l'agrandissement.** Un asset a des dimensions imposées
  par le contrat de sa famille (`EX-REN-007`) ; une rotation qui redimensionnerait l'image
  produirait un fichier que le chargement refuserait ensuite.
- Les bornes de région sont la source classique des écritures hors limites : les tests de
  débordement ne sont pas facultatifs, et un `/W4 /WX` propre ne les remplace pas.
- La région sélectionnée doit rester **visible** pendant qu'on la déplace, avec un tracé qui ne
  masque pas les pixels du bord — et dessiné depuis les jetons, comme le reste du canevas
  (TACHE-03).
- Ne pas confondre le presse-papiers de l'atelier avec la copie de zone de l'éditeur de niveaux :
  ce sont deux tampons distincts, et les actions de copier/coller les visent selon le contexte
  d'édition actif, exactement comme Annuler (TACHE-04).

## Définition de fait (DoD)
- Une région peut être sélectionnée, déplacée, retournée sur les deux axes, pivotée par quart de
  tour, copiée et collée ; chaque transformation est une fonction pure testée, produit une entrée
  nommée dans l'historique et n'écrit jamais hors du cadre ; chaînes traduites ; `/W4 /WX` propre.

## Exigences
`EX-EDIT-045` (outil de dessin pixel art) ; réutilise `EX-REN-007` (contrat d'asset),
`EX-IHM-055` (commandes exposées comme actions), `EX-IHM-062` (un état ou une commande à un seul
endroit), `EX-CTRL-012` (raccourcis d'éditeur remappables), `EX-REN-033` (traduction),
`EX-NFR-010` (testable sans GPU).
