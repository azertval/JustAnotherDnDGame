# TACHE-01 — Palette en accordéon à trois niveaux {#lot-27-tache-01-palette-accordeon}

**Lot :** [LOT-27](epic.md) · **Emplacement :** `HMI/Editor` · **Statut :** fait

## Contexte
Réorganise `hmi::TilePalette` d'une liste plate de 19 types en un accordéon à trois niveaux
(catégorie → sous-groupe → variante), validé au préalable sur un mockup HTML/CSS itéré avec le
demandeur (voir `epic.md`). La hauteur du panneau devenant variable, `hmi::ToolBar` et
`hmi::EditorLayout` sont ajustés pour rester cohérents.

## Travail à réaliser
- **`HMI/Editor/TilePalette.h`/`.cpp`** : remplace la table statique `PALETTE_TYPES` par un modèle
  d'arbre construit à la volée (`relayout()`) — deux entrées autonomes (Vide, Piège) et trois
  catégories (Tuile, Interactif, Jalon), Tuile/Interactif portant chacune des sous-groupes
  (Pente/Arrondi ; Bloc poussable). État de dépliage : `std::array<bool, 3>` par catégorie et par
  sous-groupe, tous `false` à la construction. `Entry` (forme publique inchangée :
  `type`/`x`/`y`/`width`/`height`/`label`) n'expose que les entrées **visibles** ; un tableau privé
  parallèle `_rows` retient l'action d'un clic sur chaque entrée (sélectionner un type, replier/
  déplier une catégorie, replier/déplier un sous-groupe). Nouvelle méthode `bottom()` (position `y`
  juste sous la dernière entrée visible).
- **`HMI/Editor/EditorLayout.h`** : suppression de `PALETTE_TYPE_COUNT` et `TOOLBAR_TOP` (la
  hauteur de la palette n'est plus un compte fixe) ; nouvelle constante `PALETTE_INDENT_STEP`
  (décalage horizontal d'un niveau d'imbrication).
- **`HMI/Editor/ToolBar.h`/`.cpp`** : nouvelle méthode `relayout(float top)` qui recalcule les
  rectangles des entrées sans toucher à la sélection ; le constructeur l'appelle avec `PALETTE_TOP`
  comme position provisoire.
- **`HMI/Interface/EditorScreen.cpp`** : le constructeur appelle
  `_toolBar.relayout(_palette.bottom() + PANEL_SECTION_GAP)` une première fois ; dans `update()`, le
  même appel est répété juste après `_palette.handleClick(...)` (avant le test de clic de
  `_toolBar`), pour qu'un dépliage/repliage se répercute sur la position de la barre d'outils
  **dans la même frame**. `renderPalette` reste inchangé (mêmes champs d'`Entry` qu'avant ce lot).
- Libellé « Danger » → « Piège » (affichage seulement ; `core::TileType::Danger` et son identifiant
  JSON inchangés).
- **Défilement** (gap découvert après la première revue, voir `epic.md`) : `relayout()` reconstruit
  désormais la liste complète des entrées dépliées puis la **découpe** sur une fenêtre visible
  (`visibleRowCount(viewportHeight)`, réserve la place de `ToolBar` sous la palette) déterminée par
  `_scrollOffset`/`_viewportHeight` ; `setViewportHeight(viewportHeight)` synchronise la hauteur
  connue (appelé chaque frame par `EditorScreen`) et `scroll(wheelDelta)` fait défiler la fenêtre
  (molette, sans toucher à la sélection ni à l'état de dépliage), sur le modèle de
  `LevelPicker::update`. Nouvelle méthode privée `followRow(absoluteIndex)` : après tout
  repliage/dépliage, ajuste `_scrollOffset` pour que l'en-tête qui vient d'être cliqué reste dans la
  fenêtre visible — sans quoi replier/déplier un en-tête proche du bord de la fenêtre pourrait le
  faire disparaître derrière elle, sans aucun moyen de revenir dessus autrement qu'en devinant qu'il
  faut faire défiler. `EditorScreen::update` route la molette vers `_palette.scroll` quand la souris
  survole le panneau latéral (`input.mouseX() < PANEL_WIDTH`), vers le zoom caméra sinon ;
  `renderPalette` dessine une barre de défilement (piste + curseur), même principe que
  `EditorScreen::renderPicker` (`LevelPicker`, LOT-15), uniquement si
  `totalRowCount() > visibleRowCount(viewportHeight)`.

## Fichiers impactés
- `Source/HMI/Editor/TilePalette.h`/`.cpp`, `EditorLayout.h`, `ToolBar.h`/`.cpp`.
- `Source/HMI/Interface/EditorScreen.cpp` (constructeur, `update()`, `renderPalette`).
- `Source/Test/Unit/HMI/Editor/test_tile_palette.cpp`, `test_tool_bar.cpp`.

## Tests (obligatoires)
- État initial : exactement 5 entrées visibles (`EtatInitialCinqEntreesVisibles`).
- Sélection par défaut `Solid` inchangée (`SelectionParDefautEstSolid`).
- Clic sur une entrée autonome (Piège) sélectionne son type (`ClicSurEntreeAutonomeSelectionneSonType`).
- Clic hors palette non consommé (`ClicHorsPaletteNonConsomme`).
- Chaque entrée visible porte un libellé non vide (`ChaqueEntreePorteUnLibelle`).
- Déplier/replier une catégorie (`ClicSurEnTeteDeCategorieLaDeplie`,
  `SecondClicSurEnTeteDeCategorieLaReplie`) sans changer la sélection.
- Sélectionner une feuille directe une fois la catégorie dépliée
  (`ClicSurFeuilleDirecteSelectionneSonType`).
- Déplier un sous-groupe nichée (Bloc poussable, Pente) et sélectionner l'une de ses variantes
  (`SousGroupeBlocPoussableDeplieExposeSesTailles`, `SousGroupePenteDeplieExposeSesOrientations`) —
  couvre le troisième niveau d'accordéon.
- `bottom()` suit l'état de dépliage courant (`BottomSuitLEtatDeDepliage`).
- `ToolBar::relayout` repositionne sans changer la sélection ni le nombre d'entrées
  (`RelayoutRepositionneSansChangerLaSelection`).
- Une fenêtre trop petite limite les entrées visibles sans jamais dépasser le total réel
  (`FenetreReduiteLimiteLesEntreesVisibles`).
- Le défilement change la fenêtre visible sans changer la sélection
  (`ScrollChangeLaFenetreSansChangerLaSelection`), et se borne aux deux extrémités
  (`ScrollBorneAuxExtremites`).
- Un en-tête déplié/replié reste visible même dans une fenêtre étroite
  (`EnTeteDeplieResteVisibleDansUneFenetreEtroite`) — couvre `followRow`.
- Build `/W4 /WX` sans avertissement ; suite complète (334 tests unitaires, 70 tests d'intégration,
  2 tests système) verte sans régression.

## Points d'attention
- **`Entry` garde exactement sa forme publique d'avant ce lot** : c'est ce qui permet à
  `EditorScreen::renderPalette`/`renderToolBar` de rester inchangés malgré la réorganisation
  complète de la palette — le risque de régression de rendu était donc nul par construction, pas
  vérifié a posteriori.
- **Le dépliage d'une catégorie peut déplacer la barre d'outils dans la même frame que le clic qui
  l'a causé** : sans le second appel à `ToolBar::relayout` juste après `_palette.handleClick`, la
  barre serait restée à son ancienne position jusqu'à la frame suivante (décalage visuel d'une
  frame après chaque clic sur un en-tête).
- **Le défilement seul ne suffisait pas** : ajouté seul (sans `followRow`), un dépliage cumulé
  dépassant la fenêtre visible aurait simplement déplacé le problème plutôt que de le résoudre —
  déplier une catégorie proche du bas de la liste pouvait faire disparaître **son propre en-tête**
  de la fenêtre (`_scrollOffset` restant inchangé alors que le total de lignes grandissait sous
  lui). `followRow` calcule l'indice **absolu** (indépendant du défilement) de la ligne cliquée
  avant `relayout()` — cet indice reste valide après le clic, seul ce qui suit la ligne dans la
  liste changeant — puis ajuste `_scrollOffset` pour qu'il reste dans la fenêtre.

## Définition de fait (DoD)
- Palette en accordéon à trois niveaux, panneau latéral jamais superposé au canevas, aucune
  régression sur la sélection de type ni sur la barre d'outils.

## Exigences
`EX-EDIT-018` (implémentation).
