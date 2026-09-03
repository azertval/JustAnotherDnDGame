# TACHE-01 — Trois types de tuile, bout en bout {#lot-74-tache-01-types-de-tuile}

**Lot :** [LOT-74](epic.md) · **Emplacement :** `Source/Core/Levels`, `Source/HMI/Editor`,
`Source/HMI/Graphics`, `Source/Elements/Localization` · **Statut :** fait

## Contexte
Les trois blocs du lot (`EX-GP-027`/`EX-GP-028`/`EX-GP-029`) commencent tous par la même chose : une
valeur d'énumération et la longue chaîne de fichiers qui la rend chargeable, éditable, dessinable.
Cette tâche livre cette chaîne **sans aucun comportement** — les trois tuiles se posent, se
sauvegardent, se rechargent et se voient, mais ne font encore rien. Les contrôleurs viennent en
TACHE-03 à TACHE-05.

Ce découpage est délibéré : il isole la partie mécanique et guidée par le compilateur (les `switch`
du projet sont **exhaustifs, sans `default`** — en ajouter une valeur casse la compilation à chaque
endroit qui doit être mis à jour) de la partie où se joue la conception.

| Enum | Nom JSON | Libellé palette | Clé de localisation |
|---|---|---|---|
| `TileType::SinkingBlock` | `sinkingBlock` | Bloc descendant | `palette.tile.sinking_block` |
| `TileType::FragileBlock` | `fragileBlock` | Bloc fragile | `palette.tile.fragile_block` |
| `TileType::VanishingBlock` | `vanishingBlock` | Bloc éphémère | `palette.tile.vanishing_block` |

Les trois valeurs sont ajoutées **après** `MovingPlatform` (décision de cadrage de l'épic, voir
TACHE-02 pour les bornes que cela déplace).

## Travail à réaliser
- `Core/Levels/TileType.h` : les trois valeurs en fin d'énumération, leur description dans le grand
  commentaire Doxygen du type (sur le modèle des paragraphes existants — dire ce que la tuile est,
  qui résout son comportement, et ce que ce modèle ne représente que comme état **de départ**), et
  `isSolid` : `FragileBlock` et `VanishingBlock` sont **solides** (ils bloquent tant qu'ils
  existent) ; `SinkingBlock` ne l'est **jamais**, comme `MovingPlatform` — sa position est continue
  et sa collision est résolue par `core::sweepAabbVsAabb`, pas par ce test statique.
- `Core/Levels/TileTypeName.cpp` : les trois `case` de `tileTypeName`. `parseTileType` s'en déduit
  seul (table construite en itérant l'énumération) — mais sa borne d'itération est traitée en
  TACHE-02.
- **Aucune donnée par instance, décision révisée à l'implémentation.** Le cadrage prévoyait un
  `SinkingBlockConfig` portant la vitesse de descente, sur le patron de `MovingPlatformConfig`. Le
  constructeur de `core::Level` atteint déjà **19 paramètres**, et son propre commentaire acte cette
  surface comme maximale (« un agrégat `LevelData` devient nécessaire ») : lui en ajouter un
  vingtième pour un seul réglage aurait aggravé une dette reconnue en échange de peu. La vitesse est
  donc une **constante du moteur**
  (`SinkingBlockController::SINK_SPEED_CELLS_PER_SECOND`), exactement comme le délai du bloc
  éphémère l'était déjà au cadrage — et l'ouvrir plus tard n'invalidera aucun fichier, un champ
  optionnel absent valant le défaut. `EX-GP-027` a été amendée en conséquence.
- **Conséquence directe** : les trois tuiles n'ont **aucun** champ JSON, donc `LevelLoader`,
  `LevelWriter` et `LevelDraft` n'ont eu besoin d'aucune modification — elles se chargent, se
  sauvegardent et s'annulent par le chemin générique des tuiles simples, comme `solid` ou `danger`.
- `HMI/Editor/TileTaxonomy.cpp` : les trois tuiles, en **sous-groupe** « Bloc volatil » de la
  catégorie « Tuile » — à côté du sous-groupe « Bloc poussable » existant, dont elles sont le
  pendant. Puis `HMI/Editor/TaxonomyLabels.cpp` (libellé français → clé) et les deux fichiers
  `Source/Elements/Localization/{fr,en}.lang`, bloc « Palette de tuiles ».
- `HMI/Graphics/TileVisuals.cpp` (région d'atlas), `HMI/Graphics/ProceduralAtlas.cpp` (trois
  couleurs) et `HMI/Editor/PaletteAppearance.cpp` (vignette de palette).

## Fichiers impactés
- `Source/Core/Levels/` : `TileType.h`, `TileTypeName.cpp`. **Ni** `Level.h`, **ni**
  `LevelLoader.cpp`, **ni** `LevelWriter.cpp`, **ni** `LevelDraft` : voir la décision ci-dessus.
- `Source/HMI/Editor/` : `TileTaxonomy.cpp`, `TaxonomyLabels.cpp`, `PaletteAppearance.cpp`.
- `Source/HMI/Graphics/` : `TileVisuals.cpp`, `ProceduralAtlas.cpp`.
- `Source/Elements/Localization/fr.lang`, `en.lang`.

## Tests (obligatoires)
- `Test/Unit/Core/Levels/test_tile_type_name.cpp` : aller-retour nom ↔ type pour les trois.
- Chargement, écriture et brouillon : couverts par les tests **génériques** existants des tuiles
  sans champ, aucun test nouveau n'étant justifié dès lors que ces trois types n'empruntent aucun
  chemin de parsing spécifique.
- `Test/Unit/HMI/Editor/test_tile_taxonomy.cpp` : l'invariant existant — **chaque `TileType` figure
  exactement une fois** dans la palette — doit rester vrai sans modification du test.
- `Test/Unit/HMI/Graphics/test_tile_visuals.cpp` : les trois régions sont distinctes entre elles et
  de toutes les régions existantes.

## Points d'attention
- **Le piège des libellés.** `TileTaxonomy.cpp` porte les libellés **français en dur**, et
  `TaxonomyLabels.cpp` mappe *la chaîne française elle-même* vers la clé de localisation. Une
  divergence d'un caractère ne casse **pas** la compilation : elle retombe silencieusement sur le
  français, en anglais comme en français. Écrire les deux côtés dans le même passage et vérifier
  l'anglais à l'écran.
- **L'invariant de l'atlas.** `ProceduralAtlas.cpp` documente que la couleur des cinq premières
  colonnes ne doit **jamais** dépendre de la largeur de la grille — agrandir l'atlas ne doit pas
  redécaler les tuiles déjà posées dans les niveaux livrés. Prendre trois cases **libres** de la
  colonne 5 (aujourd'hui noires), sans toucher aux colonnes 0 à 4 ni à la dernière case, réservée au
  damier de transparence.
- Trois couleurs qui **se distinguent entre elles** et du violet des blocs poussables : ces trois
  tuiles seront lues côte à côte dans les tableaux de démonstration.
- Ne poser **aucune** de ces tuiles dans un niveau à cette étape : le garde-fou de couverture
  (`test_couverture_mecaniques`) exige trois occurrences **atteignables**, ce qui est le travail de
  TACHE-08. La tâche laisse donc volontairement ce test rouge jusque-là — le noter dans le commit
  plutôt que de le contourner.

## Définition de fait (DoD)
- Les trois tuiles se posent dans l'éditeur, se sauvegardent, se rechargent et s'affichent, en
  français comme en anglais ; build `/W4 /WX` ; tous les tests ci-dessus verts.

## Exigences
`EX-GP-027`, `EX-GP-028`, `EX-GP-029`, `EX-LVL-004`, `EX-EDIT-005`, `EX-NFR-040`, `EX-ARCH-011`.
