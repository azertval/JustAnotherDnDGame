# LOT-87 — Charte v2 et intégration des maquettes {#lot-87}

> Statut : **en cours**.
> Prérequis : `LOT-86`, `LOT-39`.
>
> Exigences refondues : [`EX-IHM-070`](@ref EX-IHM-070) (charte v2, facteur réel),
> [`EX-IHM-075`](@ref EX-IHM-075) (ornements produits en images, sous trois conditions),
> [`EX-IHM-076`](@ref EX-IHM-076) (illustrations extraites ou produites, jamais sans provenance).
> Exigence précisée : [`EX-IHM-081`](@ref EX-IHM-081) (les deux facteurs).

## Objectif

Adopter les dix maquettes du pack UI comme charte visuelle v2, et transcrire les écrans du jeu sur
cette base : panneaux sombres et parchemin, `Cinzel` en titres, `IM Fell English` en corps,
illustrations peintes. Les assets (cadres 9-patch, plaques, boutons, fonds) sont produits à part, à
1080p, depuis un cahier des assets — jamais découpés des maquettes elles-mêmes, qui restent des
références de cotes et de composition.

## Pourquoi les maquettes remplacent la charte parchemin pixel

Les maquettes du pack contredisent la direction retenue par les `LOT-66` et `LOT-76` (parchemin de
Tanares, sans illustration peinte). Elles sont malgré tout la matière la plus riche dont dispose le
projet pour les dix écrans qu'elles couvrent : la décision, prise le 11 septembre 2026, est de les
adopter comme charte v2 plutôt que de continuer à les ignorer. Le prix assumé : deux directions
graphiques auront coexisté dans l'historique du dépôt, la seconde remplaçant entièrement la
première.

## Ce que Claude ne produit pas

Aucun cadre doré, aucun fond peint n'est dessiné par un modèle Claude. La phase 2 sépare deux
métiers : Claude écrit le cahier des assets (liste, dimensions, marges 9-patch, description,
prompt de génération par pièce) et vérifie/enregistre ce qui revient ; la production des images
passe par un générateur d'images ou un illustrateur, à partir de ce cahier.

## Programme, en six phases

1. **Socle vert** — dépôt propre (branches mortes archivées et retirées, débris de travail retirés,
   maquettes déplacées dans ce dossier), lot inscrit à la feuille de route.
2. **Le module de conception** — Qt Design Studio 4.8.2 ouvre chaque `*Form.ui.qml` en mode
   conception, le jeu démarre : trois modules (`Jadg.Ui`, `Jadg.App`, `Jadg.Runtime`), des
   doublures QML pour les types C++, un garde-fou de compatibilité réécrit en contrat plutôt qu'en
   structure CMake.
3. **Charte v2 et matière première** — la charte écrite dans cet epic, les jetons de `Tokens.qml`
   étendus, les polices `Cinzel` et `IM Fell English` déposées, le cahier des assets, le manifeste
   étendu aux images produites, et les briques QML (`PanelFrame`, `OrnateButton`, `StatMedallion`,
   etc.) qui les consomment.
4. **Les écrans, un par Pull Request** — menu principal, options, crédits, fiche de personnage,
   inventaire et équipement, carte du monde, équipe de mercenaires, compétences et sorts ; pause,
   dialogue, marchand et journal restylés sans redessin.
5. **Le cadre du HUD** — `CombatHudForm` et `GameViewForm` réécrits avec le cadre de la maquette,
   branchement différé aux lots qui fourniront les données réelles.
6. **Retrait** — le dossier `JustAnotherDnDGame_UI_ASSET_PACK/` et l'ancienne charte (`LOT-86`)
   supprimés une fois qu'aucun écran ne les cite plus.

## Phase 1 — le module de conception

### T1.1 — Le diagnostic, avant d'écrire

L'hypothèse du plan — *le mode conception est grisé parce que `Jadg.Ui` mêle QML et C++* — a été
mise à l'épreuve le 12 septembre 2026 en ouvrant `Source/Ui/JadgUi.qmlproject` de l'état
`main` (phase 0) dans Qt Design Studio **4.8.3** (Qt 6.8.7 embarqué, `qmlpuppet-4.8.3.exe`), puis
en résolvant chaque fichier avec le `qmllint` de ce même Qt. **Elle est fausse dans sa forme, et
juste dans son fond.**

Ce qui a été observé :

- Le mode conception **n'est pas grisé** : les formulaires s'ouvrent, le panneau Propriétés
  fonctionne, trois marionnettistes tournent (kit « Desktop Qt 6.8.7 », style Basic).
- La bibliothèque de composants liste **chaque dossier « (vide) »** — `Controls`, `Screens`,
  `Theme`, `Logic` : l'artiste ne peut poser aucune brique du projet depuis ce panneau. Première
  hypothèse : le `qmldir` engendré ne porte pas `designersupported`. **Infirmée** après coup (voir
  ci-dessous) : le mot est posé, le panneau reste vide.
- Le panneau Problèmes signale « Nom pour le type `CharacterSheetModel` manquant » et
  « `InventoryModel` manquant » (`CharacterSheet.qml`, `Inventory.qml`) : les jumeaux qui
  instancient un type C++ restent irrésolus, et `GameViewForm.ui.qml` — un *formulaire* —
  nommait `GameViewport`, lui aussi C++. Le marionnettiste ne charge aucun plugin C++ du projet ;
  tout type C++ mêlé au module des formulaires y est irrésolvable.
- Avec le `qmllint` 6.8.7 de l'atelier et `-I Source/Ui` : les 13 formulaires purs se résolvent ;
  `GameViewForm.ui.qml`, `Main.qml`, `ScreenStack.qml` et tous les jumeaux échouent sur les types
  C++ (« unqualified access » sur `OptionsModel`, `ScreenRouter`, `PendingData`).

**Cause nommée** : un module unique (`Jadg.Ui`, porté par l'exécutable) qui contient à la fois les
formulaires, le câblage et les types C++. **Critère qui la fait disparaître** : chaque formulaire
et chaque jumeau se résout avec le `qmllint` du Qt embarqué de l'atelier, sans plugin C++, et
l'atelier les ouvre sans un problème signalé.

**Ce qui reste ouvert : le panneau Composants.** Une fois la phase 1 faite (0 problème, 0
avertissement dans l'atelier), le panneau liste toujours chaque dossier « (vide) », y compris
`Mocks`. Une expérience à part — un projet jetable avec deux modules `designersupported`, l'un à
fichiers plats dans le répertoire du `qmldir`, l'autre en sous-dossiers — donne le même résultat
pour les deux : ni la disposition ni le mot ne suffisent. Ce panneau n'est pas un critère de la
phase 1 ; l'artiste pose une brique depuis la galerie `DesignStudio/Main.ui.qml` ou par le code, et
la phase 2 (T2.7, les briques de la charte v2) instruira ce qu'il exige de plus — un `.qmltypes`
ou un `designer/*.metainfo`, probablement.

La branche abandonnée (125 commits) visait le bon découpage, mais l'a fait en laissant les
fichiers en place et en réécrivant leurs chemins de ressource un par un, avec des cibles qui
vérifiaient des fichiers engendrés. C'est cette plomberie qui a dérivé, pas l'idée.

### T1.2 à T1.4 — Ce qui a été fait

Trois modules QML, **chacun déclaré dans le répertoire de ses fichiers**, sans un seul alias de
ressource :

| Module | Répertoire | Contenu | Cible |
|---|---|---|---|
| `Jadg.Ui` | `Source/Ui` | formulaires, contrôles, jetons, galerie ; QML pur, `designersupported` | `JadgUi` (statique) |
| `Jadg.Runtime` | `Source/HMI/Runtime` | les neuf types C++ exposés au QML, dont la surface de rendu | `JadgRuntime` (statique) |
| `Jadg.App` | `Source/App` (fichiers sous `Game/Qml/`) | `Main.qml`, `Logic/`, les quatorze jumeaux | `JustAnotherDnDGame` |

- La découverte de Qt, `QT_VERSION_MINIMUM`, les politiques `QTP0001`/`QTP0004` et
  `QT_QML_OUTPUT_DIRECTORY` (un seul répertoire de sortie pour les `qmldir` engendrés, celui que
  `all_qmllint` prend pour chemin d'import) vivent dans `Source/CMakeLists.txt` : trois
  répertoires frères en dépendent, et les cibles importées d'un `find_package` ne sont visibles
  que sous le répertoire qui l'a appelé.
- Les deux bibliothèques statiques sont liées avec leurs plugins (`JadgUiplugin`,
  `JadgRuntimeplugin`) : le `qmldir` embarqué les désigne (`optional plugin`, `linktarget`), et
  Qt les importe à l'édition de liens. Aucune macro dans le C++.
- `world-map.jpg` est désigné par les formulaires par un chemin **relatif à leur source**
  (`../../Elements/Assets/UI/`), et embarqué par `qt_add_resources(... BASE ...)` à l'endroit
  exact où ce chemin le cherche dans la ressource : le même chemin sert à l'atelier, aux sources
  (`JADG_QML_FROM_SOURCE=1`) et au binaire.
- La surface de rendu C++ quitte `GameViewForm.ui.qml` pour son jumeau : le formulaire réserve un
  hôte (`viewportHost`), le jumeau y pose `GameViewport`.
- `Source/Ui/Mocks/Jadg/Runtime/` : six doublures QML (`OptionsModel`, `ScreenRouter`,
  `PendingData`, `CharacterSheetModel`, `InventoryModel`, `GameViewport`) aux mêmes noms et
  propriétés que le C++ ; le `.qmlproject` les place dans ses `importPaths` et déclare les jumeaux
  dans ses `QmlFiles`. CMake ne connaît pas ce dossier.
- `scripts/check_qml_designer_compat.py` réécrit : un **contrat** en neuf règles (imports et
  motifs des formulaires, aucun type C++ dans un formulaire, doublures complètes — chaque
  `Q_PROPERTY` et `Q_INVOKABLE` a son pendant —, jumeaux appareillés, chaque fichier QML listé
  par son CMake, aucun alias ni désactivation de `qmldir` engendré, `qmldir` de conception
  `designersupported`, aucune dépendance circulaire). `check_ui_layers.py` suit la nouvelle
  arborescence (règle 1 étendue à `Runtime/`, règle 3 sur `Source/App/CMakeLists.txt`, règle 6
  étendue au câblage). `check_qt_version_pin.py` lit `Source/CMakeLists.txt`. Le pin garde son
  objet — la CI installe cette version — donc le script reste.

### T1.5 — Le test de l'artiste, passé le 12 septembre 2026

Trois changements, faits dans `Source/Ui` seulement, puis le jeu relancé avec
`JADG_QML_FROM_SOURCE=1` — **sans qu'aucun compilateur C++ n'ait tourné** :

1. **déplacer un bloc** — la colonne du menu passe de deux à six espacements extra-larges
   (`MainMenuForm.ui.qml`) ;
2. **changer un jeton** — l'accent (`Tokens.accent`) passe de l'or au bleu-vert ;
3. **remplacer un ornement** — les quatre cabochons d'angle du cadre de parchemin deviennent des
   fleurons (`ParchmentFrame.ui.qml`).

| Avant | Après |
|---|---|
| ![menu avant](captures/phase1-avant-menu.jpg) | ![menu après](captures/phase1-apres-menu.jpg) |
| ![fiche avant](captures/phase1-avant-fiche.png) | ![fiche après](captures/phase1-apres-fiche.png) |

Le `git diff` du test touche trois fichiers, tous sous `Source/Ui/**` ; aucun `.cpp`, aucun
`.h`, aucun fichier engendré. Les deux journaux de session ne contiennent aucun avertissement QML.

### Où en est la phase 1

- Le jeu démarre sur les quatorze écrans (`--screen=<Nom> --screenshot=`), aucun avertissement QML
  dans le journal ; `JADG_QML_FROM_SOURCE=1` relit `Source/Ui` (« Interface lue depuis les
  sources »).
- `ctest` : 1033/1033.
- Avec le `qmllint` 6.8.7 de l'atelier et les doublures : 44 fichiers sur 46 se résolvent — les
  quatorze formulaires, les quatorze contrôles, la galerie, les quatorze jumeaux. Les deux
  restants, `Main.qml` et `ScreenStack.qml`, importent `Jadg.App` lui-même : fichiers de câblage,
  rien ne s'y dessine.
- `check_qml_designer_compat.py` (196 éléments, 9 règles), `check_ui_layers.py` (278 éléments),
  `check_qt_version_pin.py`, `lint_lots.py`, `lint_exigences.py` : verts.

## Phase 2 — charte v2 et matière première

### T2.1 — La charte v2

Cette section **remplace** la direction du parchemin pixel (`LOT-66`, `LOT-76`, `LOT-86`) pour les
écrans du jeu. Elle ne réécrit pas ces lots : ils décrivent ce qui a été livré, et la v1 reste à
l'écran jusqu'à ce que la phase 3 ait transcrit chaque écran.

#### Ce qui est gardé

- **Le parchemin de Tanares**, avec ses rôles et leurs valeurs : `background`, `surface`,
  `surfaceAlt`, `text`, `textMuted`, relevés sur les feuilles de personnage du corpus. Les maquettes
  ne les contredisent pas : le champ de parchemin de l'inventaire (maquette 04) mesure `#e4d4ac`,
  contre `#e0d0b0` pour `surface` — un pas de quantification d'écart. On garde la mesure du corpus :
  c'est la source, la maquette n'en est qu'une interprétation.
- **L'or et le grenat** : `accent`, `border`, `frameOrnament`, `gem`, `gemShadow`. Les maquettes en
  ajoutent les faces éclairées (`goldLight`, `gemLight`) sans remplacer les valeurs existantes.
- **Le focus signalé par une marque**, pas par une teinte (`EX-IHM-071`) ; l'étanchéité des deux
  portées, jeu et éditeur (`EX-IHM-050`) ; aucune couleur, famille ni taille hors de `Tokens.qml`
  (`EX-IHM-105`).

#### Ce qui change

- **Deux matières, et chaque écran en choisit une.** Les **panneaux sombres** cerclés d'un filet
  d'or portent le menu principal, les options, les crédits, la carte et le HUD — des écrans posés sur
  une scène peinte, qu'ils ne doivent pas masquer. Le **parchemin** porte la fiche de personnage,
  l'inventaire, l'équipe de mercenaires et les compétences — des documents du personnage, qui se
  lisent comme la feuille du corpus. Pause et journal passent sur panneau sombre, dialogue et
  marchand sur parchemin (T3.9).
- **La typographie.** `Cinzel` — capitales romaines — pour les titres, plaques et bandeaux ;
  `IM Fell English` pour le corps ; son italique pour les citations et textes d'ambiance (rôle
  `loreFamily`). Une échelle de cinq tailles, en pixels à 1080p : 58 (titre du jeu), 36 (titre
  d'écran), 24 (section), 18 (corps), 14 (légende). Ces tailles sont celles du plan du lot, lues sur
  les maquettes ; elles se vérifient écran par écran, captures à côté de la maquette, en phase 3.
- **Les illustrations peintes** : fonds de scène, portraits, carte, et les ornements eux-mêmes —
  cadres, plaques, boutons, médaillons — livrés en **images 9-patch produites à 1080p** depuis le
  cahier des assets (T2.4), et non plus tracés (`EX-IHM-075` refondue).
- **Le facteur d'agrandissement devient réel** : `uiScale`, la fenêtre rapportée à 1920 × 1080.

#### Ce qui est écarté, et pourquoi

- **Les polices pixel** (`Pixelify Sans`, `Press Start 2P`). Une police bitmap et une illustration
  peinte ne cohabitent pas — le `LOT-66` l'avait écrit contre le pixel art, et les maquettes le
  rendent visible. Leurs fichiers partent après la phase 3, pas avant : les écrans v1 les emploient.
- **Le facteur entier hors du viewport.** Il existait pour une seule raison : à 1,5×, le trait et le
  filet d'un cadre **tracé** s'arrondissaient à la même épaisseur et la réserve de parchemin
  disparaissait. La v2 ne trace plus de filet d'un pixel ; une image 9-patch s'échantillonne à tout
  facteur. Le viewport garde l'entier : ses tuiles, elles, sont encore des pixels.
- **Le tracé des ornements.** Les filigranes d'or en relief et les plaques à grain des maquettes ne se
  tracent pas de façon crédible en `Shape` : l'essayer aurait produit une troisième direction
  graphique, ni la v1 ni les maquettes. Les trois raisons de l'ancienne `EX-IHM-075` restent, comme
  conditions imposées à chaque image.
- **Le découpage des maquettes.** Elles sont sorties d'un générateur, à une définition qui n'est pas
  celle du jeu (1536 à 1672 px de large), avec du texte incrusté : on y relève des cotes et des
  couleurs, on n'y prélève aucun pixel.
- **La scène isométrique peinte du HUD** (maquette 01). Le moteur rend un viewport en pixel art ; la
  maquette en fixe le cadre, pas la scène (T4.1).

#### La palette relevée

Chaque rôle **nouveau** est mesuré par `scripts/measure_mockup_palette.py` sur une zone nommée d'une
maquette ; la valeur écrite dans `Tokens.qml` est celle que le script imprime, et
`--check` échoue si l'une s'en écarte. `--annotate <dossier>` dessine chaque zone sur sa maquette —
c'est ainsi que les zones ont été vérifiées une à une, et corrigées quand elles mordaient sur un
libellé ou un ornement.

Trois mesures, parce qu'une maquette porte trois sortes de matière :

- **surface** — un fond sans texte ni ornement : le mode de l'histogramme quantifié de la zone ;
- **trait** — un libellé, un filet, une icône : le fond est d'abord relevé, puis seuls les pixels
  dont l'écart de luminance à ce fond atteint 60 % de l'écart le plus fort sont gardés. Un glyphe
  anticrénelé a plus de pixels de bord que de cœur : sans ce tri, le mode serait une couleur
  intermédiaire qui n'existe nulle part ;
- **lumière** — la face éclairée d'une matière en relief : le mode du quart le plus lumineux. Le mode
  de la plaque entière donne sa face ombrée — pour le grenat, c'est `#6c0404`, déjà tenu par `gem`.

Quantification à 8 niveaux par canal, et non 16 comme au `LOT-66` : les panneaux sombres tiennent
tous entre `#000000` et `#181818`, qu'un pas de 16 confond en un seul noir. Coordonnées en pixels
de la maquette elle-même.

| Rôle | Valeur | Maquette | Zone (x0, y0, x1, y1) | Mesure | Ce que la zone montre |
|---|---|---|---|---|---|
| `panel` | `#0c0c0c` | 07 crédits | (420, 470, 800, 480) | surface | fond du grand panneau, entre deux sections |
| `panelRaised` | `#141414` | 01 HUD | (1560, 288, 1640, 310) | surface | panneau des quêtes posé sur la scène, à droite du titre |
| `panelEdge` | `#e4a43c` | 05 options | (1417, 300, 1424, 800) | trait | filet d'or du bord droit du panneau |
| `goldLight` | `#fcd444` | 05 options | (466, 160, 500, 200) | trait | losange d'or de l'intertitre « Langue » |
| `gemLight` | `#8c0404` | 06 menu | (392, 372, 500, 404) | lumière | plaque grenat de l'entrée active « Continuer », après le libellé |
| `textOnPanel` | `#fcfcfc` | 06 menu | (265, 455, 420, 480) | trait | libellé « Nouvelle partie » |
| `textOnPanelMuted` | `#74747c` | 05 options | (1010, 533, 1085, 550) | trait | libellé désactivé « Multijoueur » |
| `success` | `#0c2c0c` | 05 options | (1368, 884, 1388, 902) | lumière | plaque verte d'« Appliquer », après le libellé |
| `danger` | `#540c0c` | 05 options | (1172, 884, 1196, 902) | lumière | plaque rouge d'« Annuler », après le libellé |
| `info` | `#0c141c` | 05 options | (530, 884, 552, 902) | lumière | plaque bleu nuit de « Par défaut », entre l'icône et le libellé |

Deux constats de la mesure, qui contredisent ce que l'œil proposait :

- **`success`, `danger` et `info` sont des matières de plaque, pas des couleurs de texte.** Sur les
  maquettes, la coche d'« Appliquer » est crème et la croix d'« Annuler » est dorée : ce qui dit
  « valider » ou « annuler », c'est la plaque. Aucune des trois n'est lisible en texte sur `panel`.
  Le texte posé dessus est `textOnPanel`.
- **Le filet des panneaux sombres est de l'or vif, pas du bronze.** `panelEdge` (`#e4a43c`) est plus
  proche de `goldLight` que d'`accent` (`#c0a060`, l'or mat du parchemin) : un panneau sombre se
  cercle d'or éclairé, et c'est ce qui le détache de la scène.

#### Exigences refondues

`EX-IHM-070` décrit la charte v2 et son facteur réel ; `EX-IHM-075` passe du tracé à l'image
produite, en gardant ses trois raisons comme conditions ; `EX-IHM-076` admet une seconde provenance,
la production depuis le cahier ; `EX-IHM-081` borne les deux facteurs. Chaque refonte garde le texte
ou le motif de la version précédente, et nomme ce lot. Le guide de conception suit (facteur,
échelle, ornements).

#### Laissé ouvert, et à qui

- **`EX-IHM-072` contre le T3.2.** L'exigence retire tout contrôle grisé non branché ; le plan des
  options dessine les sections de la maquette « désactivées, non disponible ». Les deux ne tiennent
  pas ensemble : le T3.2 tranche — retirer ces sections, ou refondre `EX-IHM-072` — avant d'écrire le
  formulaire.
- **`EX-IHM-053` contre le T2.4.** L'exigence veut des icônes vectorielles ; le cahier prévoit des
  icônes 64 × 64. Le T2.4 fixe le format (SVG produit, ou PNG avec la limite d'échelle d'`EX-IHM-075`)
  et refond l'exigence s'il le faut.
- **Les couleurs de texte du HUD** — le bleu du tour du joueur, le rouge du tour ennemi, l'or de la
  quête active (maquette 01) — ne sont pas des rôles de cette phase. Le T4.1 les relève avec le même
  script s'il en a besoin.
- **Les noms de famille des polices** (`Cinzel`, `IM Fell English`) sont ceux de leur distribution ;
  le T2.3 les confronte à la table `name` des fichiers déposés, et corrige `Tokens.qml` s'ils
  diffèrent.

### T2.2 — Les jetons

`Source/Ui/Theme/Tokens.qml` gagne, sans rien retirer :

- les **dix rôles** relevés ci-dessus, dans une section « charte v2 » qui renvoie au script ;
- **`uiScale`**, réel, 1 par défaut — la valeur de la conception, où Design Studio dessine à 1080p ;
- les **familles** : `titleFamily` passe à `Cinzel`, `bodyFamily` à `IM Fell English`, et
  `loreFamily` (même famille, en italique) s'ajoute ;
- l'**échelle typographique** v2 : `fontDisplay`, `fontScreenTitle`, `fontSectionTitle`, `fontBody`,
  `fontCaption`.

Deux écarts au plan, assumés :

- **Les tailles portent le préfixe `font`.** Le plan les nommait `display`, `screenTitle`,
  `sectionTitle`, `body`, `caption` ; quatre de ces noms sont encore ceux de la v1, que le même plan
  garde jusqu'au T5.2. Deux propriétés ne peuvent pas porter le même nom.
- **Les tailles sont multipliées par `uiScale` dans `Tokens.qml`**, et arrondies. Le plan les voulait
  « non multipliées » — non multipliées par l'**entier**, ce qu'elles sont. Les multiplier par le
  réel au même endroit garde la règle qui a évité à la v1 de diverger : un formulaire n'écrit jamais
  le facteur, donc ne peut pas l'oublier. À la conception, `uiScale` vaut 1 et le formulaire lit
  exactement 58, 36, 24, 18 et 14.

`Source/App/Game/Qml/Main.qml` lie `uiScale` à la fenêtre :
`max(0,5 ; min(largeur / 1920 ; hauteur / 1080))`. Le plus petit des deux rapports, pour que l'écran
de conception tienne entier quel que soit le format ; une liaison et non deux gestionnaires, parce
que le facteur dépend des deux dimensions ; plancher à 0,5, sous lequel un corps de 18 px ne se lit
plus. Aucune boucle (`EX-IHM-080`, `EX-IHM-081`) : aucun écran ne contraint la fenêtre.

Les grandeurs v1 (`scale`, `screenTitle`… `frameThickness`) sont **marquées obsolètes** dans le
fichier et restent lues par les quatorze écrans. **Conséquence visible, attendue** : les écrans v1
lisent `bodyFamily` et `titleFamily`, qui nomment désormais des polices que le jeu n'enregistre pas
encore. Jusqu'au T2.3, ils s'affichent dans la famille générique de repli (`EX-IHM-052`) — tailles et
mise en page inchangées. Le journal ne le signale pas : il ne dit qu'un fichier absent de la liste de
`registerIdentityFonts()`, où ces polices n'entrent qu'au T2.3.

### Où en est la phase 2 — vérifié le 13 septembre 2026

- `scripts/build.ps1 -Preset ninja -Test` : construction propre, `ctest` 1033/1033.
- Les quatorze écrans se chargent (`--screen=<Nom> --screenshot=`) : quatorze captures écrites, code
  de sortie 0, aucune ligne d'avertissement QML ni d'erreur dans le journal. Mise en page v1
  inchangée, en famille de repli.
- `qmllint` du Qt 6.8.7 de l'atelier, avec les doublures : muet sur `Tokens.qml`,
  `MainMenuForm.ui.qml` et `OptionsForm.ui.qml`.
- `measure_mockup_palette.py --check` (10 rôles, 0 divergence), `check_ui_layers.py`,
  `check_qml_designer_compat.py`, `lint_exigences.py` (352/352), `lint_lots.py` : verts. Doxygen sans
  erreur.

## Exigences couvertes

- [`EX-IHM-070`](@ref EX-IHM-070), [`EX-IHM-075`](@ref EX-IHM-075), [`EX-IHM-076`](@ref EX-IHM-076),
  [`EX-IHM-081`](@ref EX-IHM-081) : refondues ou précisées par la phase 2 (T2.1), pour décrire la
  charte v2.

## Où en est le lot

**Phases 0 et 1 livrées** (PR #25 et #26) ; **phase 2 en cours** — T2.1 et T2.2 faits
(branche `lot/LOT-87-phase-2-charte-jetons`), T2.3 à T2.7 à venir.

Rappel de la phase 0 :

- Branches mortes (dix-neuf) archivées sous des tags `archive/<nom>` et retirées localement ; les
  branches distantes correspondantes restent à retirer (nécessite une confirmation explicite, hors
  de portée de l'automatisation).
- `main` avancé sur `origin/main`, les trois worktrees obsolètes purgés.
- `lot/LOT-87-charte-v2` ouverte depuis `main` ; `scripts/check_qml_designer_compat.py` et
  `Source/Ui/DesignStudio/Main.ui.qml` récupérés depuis `archive/fix/qt-designer-6.8.7-compatibility`
  pour la phase 1.
- Les dix maquettes déplacées dans `references/`.
- Ce dossier créé : comme tout lot démarré, le `LOT-87` quitte `roadmap-0.1.0.md` pour cet epic,
  et `lots.md` porte désormais son `@subpage lot-87`.

## Critères d'acceptation de la phase 0

- `git branch -a` ne liste plus que `main`, `gh-pages` et `lot/LOT-87-charte-v2`, hors branches
  distantes en attente de suppression manuelle.
- La branche construit et démarre sur les quatorze écrans via `scripts/build.ps1 -Test` (`ctest` :
  1033/1033 le 12 septembre 2026).
- `check_ui_assets.py`, `check_ui_layers.py`, `lint_lots.py`, `lint_exigences.py` verts.
- Doxygen sans erreur.
