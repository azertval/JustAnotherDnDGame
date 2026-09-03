# TACHE-02 — Palette de tuiles `QTreeView` (taxonomie `LOT-27`) reliée à la sélection {#lot-35-tache-02-palette-arbre}

**Lot :** [LOT-35](epic.md) · **Emplacement :** `Source/Editor` · **Statut :** non commencé

## Contexte
La palette actuelle (`hmi::TilePalette`, `LOT-27`) est un **accordéon dessiné à la main** (catégories
→ sous-groupes → tuiles, défilement molette, barre de défilement — toute la géométrie en pixels).
Cette tâche la remplace par un **`QTreeView`** natif alimenté par la **même taxonomie**, dans le dock
« Palette ». La sélection d'une feuille (un `core::TileType`) devient le **type de tuile actif** de
l'éditeur.

## Travail à réaliser
- **Modèle de palette réutilisable** : extraire la taxonomie (catégories `Tuile/Interactif/Piège/
  Jalon`, sous-groupes `Pente/Arrondi/Concave/Bloc/Directionnel`, feuilles = `core::TileType`) dans
  une **structure de données pure** partagée (indépendante de Qt et du dessin) — factorisée depuis la
  logique de `TilePalette` qui décrit déjà `entries()`. Cette structure sert de source au
  `QAbstractItemModel`.
- **`QTreeView` + modèle** dans le dock « Palette » : arbre à 3 niveaux, catégories repliables,
  libellés localisés (`Localization`). Icône par tuile (couleur/atlas actuel ; vraies textures au
  [LOT-39](@ref lot-39)).
- **Sélection → type actif** : `selectionModel()->currentChanged` sur une feuille met à jour le
  `core::TileType` courant de l'éditeur (l'équivalent de `TilePalette::selected()`), consommé par
  l'outil de peinture (TACHE-03). Sélection clavier (flèches) et souris.
- **Recherche/filtre** optionnelle (champ au-dessus de l'arbre) si peu coûteuse.

## Fichiers impactés
- `Source/Editor/PalettePanel.{h,cpp}` (dock, vue, modèle Qt).
- `Source/Editor/TileTaxonomy.{h,cpp}` (ou réutilisation d'un modèle pur extrait de
  `HMI/Editor/TilePalette`) — structure de catégories testable.

## Tests (obligatoires)
- **Taxonomie pure testée** : chaque `core::TileType` « peignable » apparaît exactement une fois, sous
  la bonne catégorie/sous-groupe ; ordre déterministe. Réutiliser/adapter `test_tile_palette.cpp`
  (`Source/Test/Unit/HMI/Editor/`) vers la structure extraite.
- **Vérification manuelle** : l'arbre affiche toutes les tuiles ; sélectionner change le type actif
  (visible au survol/peinture).

## Points d'attention
- **Ne pas dupliquer la liste des types** : une seule source de vérité pour « quelles tuiles, dans
  quelle catégorie » — partagée entre le modèle Qt et (tant qu'il existe) l'accordéon legacy.
- **Parité de contenu** avec la palette `LOT-27` : mêmes tuiles, mêmes regroupements (non-régression
  fonctionnelle, seule la présentation change).
- **Localisation** des libellés de catégories/tuiles via les catalogues `.lang`.

## Définition de fait (DoD)
- Palette en `QTreeView` couvrant toute la taxonomie ; sélection pilotant le type actif ; taxonomie
  couverte par tests ; `/W4 /WX` propre ; vérification manuelle OK.

## Exigences
`EX-EDIT-018` (palette par catégories, présentation Qt), `EX-IHM-010` ; réutilise `EX-NFR-010`
(taxonomie testable sans GPU).
