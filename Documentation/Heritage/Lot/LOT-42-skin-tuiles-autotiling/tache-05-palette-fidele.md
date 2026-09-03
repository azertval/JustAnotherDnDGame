# TACHE-05 — Palette de l'éditeur fidèle au mode de rendu {#lot-42-tache-05-palette-fidele}

**Lot :** [LOT-42](epic.md) · **Emplacement :** `Source/HMI/Editor` · **Statut :** fait

## Contexte
`hmi::PalettePanel` affiche chaque type de tuile par sa **couleur plate**, obtenue via
`hmi::regionForTile` (`TileVisuals`). C'était fidèle tant que le rendu n'avait qu'un mode.

Dès ce lot, ce n'est plus vrai : en mode Texture, le canevas affiche des textures et la palette
continuerait d'afficher des pastilles de couleur. Le level designer choisirait « la tuile violette »
sans voir ce qu'il pose. C'est une régression d'usage introduite par le lot lui-même — d'où une
tâche dédiée plutôt qu'une correction ultérieure.

## Travail à réaliser
- **La palette suit le mode courant** (`RenderMode`, LOT-41) : en mode Texture, chaque entrée montre
  la texture réellement assignée au type dans le jeu de skins courant ; en mode Physique, la couleur
  plate d'aujourd'hui.
- **Type non skinné** : afficher le damier magenta, comme le canevas — la palette doit signaler ce
  qui reste à habiller, pas le masquer.
- **Type en mode `bitmask16`** : afficher une case **représentative** de la planche (l'intérieur
  plein), pas la planche entière ni la case zéro qui serait arbitraire.
- **Type à silhouette** (pente, arrondi) : afficher la variante détourée (TACHE-03), cohérente avec
  le canevas.
- **Rafraîchissement** : la palette se met à jour quand le jeu de skins change, quand une assignation
  change, et quand le mode bascule.
- Décodage CPU (`hmi::decodeImageFile` → `QPixmap`), **jamais** de texture Direct3D 11 : un widget
  Qt ne doit pas dépendre du device graphique.

## Fichiers impactés
- `Source/HMI/Editor/PalettePanel.{h,cpp}`.
- `Source/HMI/Editor/TexturePanel.{h,cpp}` (signal d'assignation modifiée).

## Tests (obligatoires)
- Choix de l'apparence à afficher pour un type donné, selon le mode, le jeu courant et le mode de
  skin : fonction **pure**, testée sans Qt (mode Physique → région d'atlas ; Texture + skin → asset ;
  Texture sans skin → repli ; `bitmask16` → case représentative).

## Points d'attention
- Ne pas dupliquer la logique de résolution : la palette doit interroger **le même** résolveur que
  le canevas (LOT-41, TACHE-01), sinon les deux divergeront au premier cas particulier.
- Attention au coût : la palette peut afficher trente entrées. Décoder les images une fois et les
  mettre en cache côté widget ; ne rien décoder pendant le défilement.
- Le rendu des vignettes doit rester **net** (pixel art) : mise à l'échelle en plus proche voisin,
  pas l'interpolation lisse par défaut de Qt.

## Définition de fait (DoD)
- La palette montre la texture réelle en mode Texture et la couleur plate en mode Physique, se met à
  jour aux trois événements listés, réutilise le résolveur du canevas, et sa logique de choix est
  testée sans Qt ; `/W4 /WX` propre.

## Exigences
`EX-EDIT-027` (palette fidèle au rendu) ; réutilise `EX-EDIT-018` (palette organisée par
catégories), `EX-EDIT-042` (association type → texture), `EX-REN-046` (bascule), `EX-ARCH-022`
(*nearest*).
