# TACHE-04 — Intégration éditeur et habillage {#lot-63-tache-04-integration-editeur-habillage}

**Lot :** [LOT-63](epic.md) · **Emplacement :** `Source/HMI/Editor`, `Source/HMI/Graphics` ·
**Statut :** fait

## Contexte
Un mécanisme livré dans `Core` mais absent de la palette n'existe pas : le level designer ne peut ni
le poser, ni le lier, ni le voir. Le projet a un chemin d'intégration parfaitement établi — tous les
lots de mécanismes depuis le `LOT-12` l'ont emprunté — et cette tâche consiste à le suivre pour
trois nouveaux types, sans en inventer un quatrième.

## Travail à réaliser
- **Taxonomie et palette** : `hmi::TileTaxonomy` classe les trois nouveaux types dans les catégories
  existantes (mécanismes) ; ils apparaissent dans la palette en accordéon (`LOT-27`) avec leur
  vignette.
- **Libellés traduits** (`hmi::TaxonomyLabels`) dans les deux catalogues.
- **Liaison clé ↔ porte** dans l'outil Lien (`LOT-37`) : réutiliser `hmi::LinkGesture` et
  `hmi::LinkGeometry`, qui traitent déjà interrupteur ↔ porte et plaque ↔ porte. Aucune nouvelle
  machine à états de geste.
- **Édition des paramètres de plateforme** (points de parcours, vitesse, déphasage) : la plateforme
  est le seul des trois à porter des données propres. Réutiliser le patron du danger temporisé et
  du danger mobile (`LOT-31`), qui ont déjà posé ce problème.
- **Rendu de brouillon** (`hmi::DraftRenderer`) : les trois types sont visibles dans l'éditeur, avec
  le parcours de la plateforme matérialisé.
- **Habillage** : entrées dans `skins.json`, et fichiers `nom-asset.anim.json` pour les états —
  porte verrouillée fermée/ouverte, clé présente/ramassée — via `hmi::MechanismVisuals` (`LOT-47`),
  qui associe déjà état logique et clip.
- **Assets** générés par le script existant (`scripts/generate_mechanism_animations.py` et les
  `generate_test_*.py`), pour rester cohérent avec le reste du contenu.
- **Repli procédural** : les trois types doivent rester jouables et distinguables **sans aucun
  fichier d'image**, comme tout le reste depuis le `LOT-40`.

## Fichiers impactés
- `Source/HMI/Editor/TileTaxonomy.{h,cpp}`, `TaxonomyLabels.{h,cpp}`, `PaletteAppearance.cpp`,
  `LinkGesture.cpp`, `LinkPanel.cpp`.
- `Source/HMI/Graphics/TileVisuals.cpp`, `ProceduralAtlas.cpp`, `MechanismVisuals.cpp`,
  `DraftRenderer.cpp`.
- `Source/Elements/Assets/skins.json`, `Source/Elements/Assets/Skins/` (nouveaux fichiers).
- `Source/Elements/Localization/{fr,en}.lang`.
- Tests étendus : `test_tile_taxonomy.cpp`, `test_link_gesture.cpp`, `test_mechanism_visuals.cpp`,
  `test_procedural_atlas.cpp`, `test_palette_appearance.cpp`.

## Tests (obligatoires)
- Les trois types apparaissent dans la taxonomie, dans la bonne catégorie, avec un libellé traduit
  dans les **deux** catalogues.
- La liaison clé ↔ porte est acceptée par le geste de lien ; une liaison invalide (clé ↔ clé) est
  refusée.
- `hmi::MechanismVisuals` associe un clip à **chaque** état des nouveaux mécanismes — vérifié en
  parcourant l'énumération, pas par relecture.
- Le repli procédural produit une apparence **distincte** pour chacun des trois types : deux types
  indiscernables en mode repli est un défaut, pas un détail.
- La vignette de palette et le rendu du canevas appliquent la **même** priorité de résolution
  d'apparence (`hmi::PaletteAppearance` existe pour garantir qu'ils ne divergent pas).
- Round-trip d'édition : poser, lier, enregistrer, recharger — modèle identique.

## Points d'attention
- **Le piège de chemin d'asset** déjà rencontré avec `TextureCache` : `getAnimation`, `get` et
  `getMasked` doivent recevoir le **même** chemin, préfixe de sous-dossier compris, pour un même
  asset. Les tests « sans GPU » ne couvrent pas cette glu d'intégration — c'est un point à vérifier
  à l'essai.
- Les couleurs assourdies de `Switch`, `Door` et `PressurePlate` dans l'éditeur sont un défaut
  **préexistant**, pas une régression de ce lot : ne pas le corriger ici, ne pas s'en alarmer.
- Attention aux commentaires XML des fichiers `.ui` : un double tiret `--` casse `uic` avec une
  erreur « Expected '>' » qui ne mentionne pas le commentaire.
- Ne pas dupliquer la machine à états du geste de lien : elle est pure, testée, et prévue pour
  accepter de nouvelles paires.

## Réalisé (écarts par rapport au brouillon initial)
- **Grille procédurale agrandie de 5×5 à 6×6** (`hmi::TextureAtlas::TILES_PER_SIDE`) : la grille
  5×5 ne laissait que deux cases libres (déjà prises par `Key`/`LockedDoor`), aucune pour
  `MovingPlatform`. Agrandissement mécanique et sûr (chaque case existante garde ses coordonnées
  `(colonne, ligne)`, seule la formule d'indexation à plat change) — voir `ProceduralAtlas.cpp` et
  son test d'exhaustivité (`test_tile_visuals.cpp`, nouveau : vérifie que les trois mécanismes ont
  des régions procédurales deux à deux distinctes).
- **Édition des paramètres de plateforme** : ni le danger mobile ni le danger temporisé n'ont en
  réalité de panneau Qt dédié dans l'éditeur (vérifié : `setMoverConfig`/`setBlinkConfig` ne sont
  appelés que par les tests, jamais par un widget) — le « patron » à réutiliser est donc
  `core::LevelDraft::setPlatformConfig` (mutateur + lecture/écriture JSON), pas une UI qui n'existe
  pas encore pour les précédents non plus. `hmi::DraftRenderer` matérialise malgré tout le parcours
  (nouveau : `composeMovingPlatformPaths`, trait + flèche, même patron que les liens de mécanismes).
- **Habillage réel, sous licence libre** : à la demande explicite portée sur ce lot, `key.png`,
  `locked_door.png` et `platform.png` (jeu de skins `kenney`) sont de vraies illustrations
  retouchées à partir de sprites **Kenney** (CC0), et non des placeholders générés par script comme
  le reste du projet — voir `Source/Elements/Assets/CREDITS.md` et l'écran Crédits en jeu (section
  « Graphismes »). Le jeu par défaut (`test`) reste inchangé.
- **Bug corrigé en cours de lot : `Source/Elements/Assets/atlas.png` non régénéré.** L'agrandissement
  de la grille procédurale (5×5 → 6×6, ci-dessus) change la géométrie que `TextureAtlas::loadFromFile`
  attend, mais le fichier PNG **versionné** n'est pas recalculé automatiquement — seule
  `JustAnotherDnDGame.exe --export-atlas=<chemin>` le fait (`LOT-39`). L'avoir oublié ici a produit trois
  symptômes distincts en aval, tous constatés en essai manuel : personnage mal recadré en mouvement
  (lignes de la grille décalées), `Key`/`LockedDoor` lues dans une case hors bornes de l'ancien
  fichier (invisibles), et les trois nouveaux mécanismes en noir en mode Physique. Régénéré ; ajout
  d'un test d'intégration (`Source/Test/Integration/test_plateforme_composition.cpp`) qui rejoue le
  repérage d'entité-tuile et le rafraîchissement visuel **exactement** comme `hmi::GameSession`, seul
  point du projet qui exerçait cette composition avant ce lot (les tests système/intégration
  existants ne consomment `core::PlatformController` que via ses boîtes, jamais via une entité).

## Définition de fait (DoD)
- Les trois mécanismes se posent, se lient, se paramètrent, s'enregistrent et se rechargent depuis
  l'éditeur, sont habillés selon leur état, restent distinguables sans aucun asset, et sont couverts
  par des tests purs ; `/W4 /WX` propre.

## Exigences
Réutilise `EX-CTRL-022`, `EX-GP-023`, `EX-GP-026` (les trois mécanismes), `EX-EDIT-010`
(réutilisation du modèle de `Core`), `EX-REN-005` (animation par données), `EX-REN-006` (apparence
selon l'état), `EX-REN-007` (validation des assets), `EX-REN-033` (traduction), `EX-NFR-004`
(vérification sans GPU).
