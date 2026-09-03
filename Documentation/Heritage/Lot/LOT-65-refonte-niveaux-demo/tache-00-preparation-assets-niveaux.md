# TACHE-00 — Préparation : banque d'assets renforcée et remise à zéro des niveaux {#lot-65-tache-00-preparation-assets-niveaux}

**Lot :** [LOT-65](epic.md) · **Emplacement :** `Source/Elements/Assets`, `scripts`,
`Source/Elements/Levels` · **Statut :** non commencé

## Contexte
La `TACHE-01` pose le garde-fou de couverture, et les `TACHE-02`/`TACHE-03` s'appuient dessus pour
refondre le contenu. Deux constats, faits en préparant le lot, changent l'ordre de départ prévu :

1. **La banque d'assets est trop pauvre pour habiller quoi que ce soit.** Un seul jeu de skins
   réellement utilisable au-delà du schématique (`kenney`, trois types couverts sur la trentaine que
   compte `core::TileType`), un seul fond (`test_sky.png`), deux décors, deux textures d'objet,
   aucune spritesheet de personnage. L'architecture des seize lots d'habillage (`LOT-40` →
   `LOT-57`) est livrée ; son contenu ne l'est pas.
2. **Les dix-sept tableaux hérités du `LOT-25` biaiseraient une refonte pensée par couverture.**
   Réviser tableau par tableau (l'approche initialement décrite en `TACHE-02`) ancre chaque décision
   dans la géométrie existante plutôt que dans la liste de ce que le garde-fou de `TACHE-01` exige.
   Repartir d'une séquence vide force chaque tableau à se justifier par une mécanique à couvrir, pas
   par un héritage du MVP.

Cette tâche précède donc `TACHE-01` : elle prépare le terrain (assets, séquence vide) sur lequel le
garde-fou, puis le contenu, seront construits. Elle ne mesure aucune couverture — c'est le rôle de
`TACHE-01` — et ne recrée aucun tableau — c'est celui de `TACHE-02`/`TACHE-03`.

> Note au passage : l'epic et plusieurs documents antérieurs au lot mentionnent encore « quinze
> tableaux ». Le `LOT-63`, fusionné entre-temps, en a porté le nombre à dix-sept
> (`demo-cle.json`, `demo-plateforme.json`). Cette tâche corrige la mention dans les documents du
> `LOT-65` ; elle ne touche pas aux documents des lots déjà clos, qui décrivent un état passé.

## Travail à réaliser

### Banque d'assets
- **Étendre les scripts de génération procédurale** (`scripts/generate_test_*.py`) plutôt que d'en
  écrire de nouveaux à la main : plus de variantes de fond (`generate_test_backgrounds.py`), de
  décors (`generate_test_decors.py`), de textures d'objet (`generate_test_objects.py`) — toujours
  schématiques, reproductibles, sans dépendance externe, dans le même esprit que l'existant.
- **Livrer une spritesheet de personnage de test** (`Player/player.png` + `.anim.json`) : le
  personnage retombe aujourd'hui sur la silhouette procédurale historique faute d'un seul fichier
  livré dans `Player/`, alors que le contrat (`LOT-48`) est complet. Générée par script, comme le
  reste des assets de test.
- **Étoffer le jeu `kenney`** (`skins.json`) au-delà des trois types couverts par le `LOT-63` :
  importer d'autres sprites CC0 des mêmes packs Kenney (ou de packs compatibles), suivre exactement
  le protocole déjà documenté dans `Skins/README.md`/`CREDITS.md` (détourage, mise à l'échelle
  `LANCZOS` vers 16×16 ou 64×64 `bitmask16`, entrée `CREDITS.md` par fichier, source du pack).
  Priorité aux types les plus visibles dans les tableaux à venir : sol (`solid`, `bitmask16`),
  danger (`danger`), porte (`door`), interrupteur (`switch`), plaque (`pressurePlate`), fond.
  Réalisé pour `danger`/`door`/`switch`/fond ; **`solid` (`bitmask16`) et `pressurePlate` restent
  non couverts** — le premier faute de source Kenney offrant les seize combinaisons de voisinage
  exactes du contrat (`Skins/README.md`, section `bitmask16`) sans recomposition manuelle dont la
  correction ne peut se vérifier qu'en la voyant tourner dans le jeu — un essai manuel hors du
  moment prévu par le lot (`TACHE-04`), donc écarté ici —, le second faute de sprite au bon sens
  dans les packs déjà importés. Deux voies
  possibles si un lot ultérieur veut les couvrir : composer `bitmask16` à la main à partir de pièces
  Kenney distinctes puis le vérifier au moment du parcours manuel déjà prévu en `TACHE-04`, ou
  rester procédural (comme `stone.png` du jeu `test`) en reprenant la palette Kenney plutôt que ses
  sprites.
- **Étoffer aussi `Decors/`/`Objects/`** avec des sprites Kenney des mêmes packs (clôture, torche,
  chaîne, échelle, pièce, gemme) : même protocole, dimensions libres pour `Decors/` (calées sur la
  largeur ou la hauteur pour rester proches de la densité de la grille de 16 px), carré 16×16
  imposé pour `Objects/`.
- **Aucun art dessiné à la main dans ce lot** : génération par script ou retouche mécanique d'assets
  CC0 existants, jamais de création originale — même exclusion que celle déjà écrite dans l'epic
  pour `TACHE-02`/`TACHE-03`, étendue à cette tâche.

### Remise à zéro des niveaux
- **Supprimer les dix-sept tableaux actuels** (`Source/Elements/Levels/demo-*.json`).
- **Vider la séquence** (`sequence-demo.json`) plutôt que la supprimer : le format reste celui du
  `LOT-59`, prêt à recevoir les tableaux des `TACHE-02`/`TACHE-03`.
- **Ne pas tenter de garder `ctest`/la CI verts pendant cette tâche.** Le test système
  (`test_parcours_complet.cpp`) et `scripts/check_demo_sequence.py` échoueront tant qu'aucun tableau
  n'existe — c'est attendu, documenté ici, et accepté pour la durée de la préparation. Cette tâche
  se fait sur la branche dédiée du lot, jamais fusionnée en l'état : la CI ne redevient un critère
  qu'à la fin de `TACHE-03`.
  - **Liste rouge exacte, constatée en exécutant `ctest --preset ninja` après la suppression**
    (26 échecs sur 1114, tous et seulement liés au contenu retiré — aucune régression sur la banque
    d'assets) : `CameraFramingTest.NiveauxLivresReproduisentLeurComportementActuel`,
    `LevelLoaderTest.NiveauDeDemoLivreValide`/`LesDixSeptNiveauxDeDemoSeChargentSansErreur`,
    `LevelSequenceLoaderTest.SequenceDeDemoLivreeValide`,
    `LevelWriterTest.NiveauPuzzleLivreSurvieAuRoundTrip`, les quatre cas de `RenderBudgetTest`
    portant sur les niveaux livrés, `NiveauEcsIntegration.FichierDemoVersMonde`, les quatorze cas
    `PhysiquePersonnageIntegration.Niveau*Franchissable*`,
    `PlateformeCompositionTest.EntiteTuileSuitLaPositionSimulee`, et
    `ParcoursCompletSysteme.FranchitTouteLaSequence`. Cette liste **rétrécit** au fil de
    `TACHE-02`/`TACHE-03` à mesure que les tableaux sont recréés ; elle doit revenir à zéro à la fin
    de `TACHE-03` (critère d'acceptation du lot).

## Fichiers impactés
- `scripts/generate_test_backgrounds.py`, `generate_test_decors.py`, `generate_test_objects.py`
  (étendus), nouveau `scripts/generate_test_player.py`.
- `Source/Elements/Assets/Backgrounds/`, `Decors/`, `Objects/`, `Player/`, `Skins/` (nouveaux
  fichiers générés ou retouchés), `Source/Elements/Assets/skins.json`, `CREDITS.md`.
- `Source/Elements/Assets/*/README.md` (inventaire mis à jour, même patron que l'existant).
- `Source/Elements/Levels/demo-*.json` (supprimés), `sequence-demo.json` (vidé).
- `Documentation/Lot/LOT-65-refonte-niveaux-demo/epic.md` (compte corrigé, décision de cadrage
  ajoutée, découpage mis à jour).

## Tests (obligatoires)
- Chaque script de génération s'exécute sans erreur et produit des fichiers respectant le contrat
  de dimensions déjà validé au chargement (`EX-REN-007`) — vérifié en relançant l'éditeur ou par un
  test existant s'il couvre ces chemins.
- `skins.json` reste un JSON valide, chaque nouvelle entrée du jeu `kenney` pointe vers un fichier
  réellement présent dans `Skins/`.
- `CREDITS.md` documente chaque nouveau fichier retouché : sprite d'origine, pack, auteur, licence —
  même table que l'existant, une ligne par fichier.
- **Pas de critère de franchissabilité ni de couverture ici** : ces deux garanties reviennent avec
  `TACHE-01` (garde-fou) et `TACHE-02`/`TACHE-03` (tableaux). Le seul critère de cette tâche est que
  les assets se chargent et que les niveaux soient bien absents.

## Points d'attention
- **Le piège de chemin déjà rencontré avec `TextureCache`** s'applique aussi ici : un asset ajouté
  dans un sous-dossier doit être désigné par le même chemin, préfixe compris, partout où il est
  référencé (`skins.json` compris).
- **CC0 n'exige aucune attribution, mais ce dépôt crédite quand même** (même principe que
  `Source/Elements/Audio/CREDITS.md`, `LOT-60`) : ne pas sauter l'entrée `CREDITS.md` sous prétexte
  que la licence ne l'impose pas.
- **Ne pas céder à la tentation d'écrire déjà des tableaux ici.** Cette tâche prépare des
  ingrédients ; les combiner en niveaux, avec cadrage et progression, est le travail de
  `TACHE-02`/`TACHE-03`, qui s'appuient sur le garde-fou de `TACHE-01`.
- **Ce lot n'ajoute toujours aucune mécanique** : la banque d'assets s'étoffe, le moteur ne change
  pas. Un besoin de nouveau champ ou de nouveau type de tuile ressort comme un défaut à consigner,
  pas comme une extension de cette tâche.
- Volume : les images restées schématiques (générées par script) sont légères ; les sprites Kenney
  retouchés le sont aussi (16×16 ou 64×64), mais chaque ajout reste une image versionnée —
  réutiliser avant de multiplier.

## Définition de fait (DoD)
- La banque d'assets couvre sensiblement plus de types et de familles qu'au début de la tâche (fond,
  décors, objets, personnage, jeu `kenney` étoffé), chaque ajout crédité s'il provient d'une source
  externe ; les dix-sept tableaux existants et leur séquence sont retirés ; l'epic du lot reflète le
  compte corrigé et la nouvelle décision de cadrage ; rien de tout cela ne prétend rendre la CI
  verte, ce n'est pas son critère.

## Exigences
Réutilise `EX-REN-007` (contrat d'asset validé au chargement), `EX-REN-042` (assets hors code),
`EX-REN-044` (fond de niveau), `EX-DEC-002` (couches de décor), `EX-EDIT-043` (textures par
instance). N'introduit aucune exigence nouvelle : c'est de la préparation de contenu, pas une
fonctionnalité.
