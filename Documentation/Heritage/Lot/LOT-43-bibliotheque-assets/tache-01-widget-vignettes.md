# TACHE-01 — Widget de vignettes partagé {#lot-43-tache-01-widget-vignettes}

**Lot :** [LOT-43](epic.md) · **Emplacement :** `Source/HMI/Editor` · **Statut :** fait

## Contexte
Le panneau « Textures » livré en LOT-42 sélectionne les assets par **liste de noms de fichiers**.
C'était le minimum acceptable pour une seule section ; ce ne l'est plus dès qu'il y en a cinq
(skins, fond, objets, animations, décors) et que le level designer doit reconnaître une image parmi
une trentaine.

Le décodage nécessaire existe déjà : `hmi::decodeImageFile` (LOT-39) produit des pixels RGBA à
partir d'un fichier, indépendamment de Direct3D 11.

## Travail à réaliser
- **`AssetThumbnailView`** (`QWidget`) : grille de vignettes avec nom, sélection simple, filtrage
  par texte, et signal de sélection. Réutilisable tel quel par toutes les sections.
- **Source des vignettes** : `hmi::decodeImageFile` → `QImage` → `QPixmap`. **Aucune** texture
  Direct3D 11 : le widget ne doit pas dépendre du device graphique, ce qui le rendrait inutilisable
  quand aucun viewport n'est actif.
- **Mise à l'échelle en plus proche voisin** : les assets sont du pixel art, une vignette
  interpolée serait floue et trompeuse.
- **Cache de vignettes** par chemin de fichier, invalidé par le rechargement à chaud (TACHE-03).
- **Intégration en place** dans les sections existantes du panneau « Textures », **sans** changer
  leur modèle de données ni le câblage du dock.
- Vignette de repli pour un fichier illisible, cohérente avec le damier magenta du rendu.

## Fichiers impactés
- `Source/HMI/Editor/AssetThumbnailView.{h,cpp}` (nouveau).
- `Source/HMI/Editor/TexturePanel.{h,cpp}` (remplacement du sélecteur textuel).
- `Source/Test/Unit/HMI/Editor/test_asset_library.cpp` (nouveau, partie pure).

## Tests (obligatoires)
- Le **balayage et le filtrage** de la liste d'assets (extensions retenues, tri, filtre texte) sont
  une fonction pure, testée sans Qt.
- Le rendu du widget lui-même relève de la vérification visuelle, comme les autres panneaux Qt.

## Points d'attention
- **Ne pas introduire un second décodeur.** `decodeImageFile` est le point unique ; le *TextureCache*
  D3D11 (LOT-40) et ce widget en sont deux consommateurs indépendants.
- Décoder de façon **paresseuse** : ne pas charger trente images à l'ouverture du panneau, mais au
  fur et à mesure de leur affichage.
- Le widget ne doit **pas** connaître la sémantique des sections (skin, fond, objet) : il liste un
  dossier et émet une sélection. La sémantique reste dans les sections.

## Définition de fait (DoD)
- Les sections du panneau affichent des vignettes nettes au lieu de noms de fichiers, sans
  changement de comportement fonctionnel ; le widget est indépendant du device graphique ; le
  balayage est testé sans Qt ; `/W4 /WX` propre.

## Exigences
`EX-EDIT-026` (gestion des fichiers d'assets) ; réutilise `EX-REN-041` (décodage image),
`EX-EDIT-042` (sections concernées), `EX-ARCH-022` (*nearest*), `EX-NFR-040` (repli).
