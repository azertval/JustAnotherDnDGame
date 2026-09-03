# TACHE-01 — Socle de style : style Qt, palette applicative et jetons de design {#lot-56-tache-01-socle-style}

**Lot :** [LOT-56](epic.md) · **Emplacement :** `Source/HMI/Interface` · **Statut :** fait

## Contexte
`Source/HMI/main.cpp` charge une feuille de style et rien d'autre : l'application ne choisit jamais son
style Qt, et s'exécute donc sur le style **natif** de la plate-forme. Ce style dessine la plupart des
contrôles via l'API du système et ignore une large part de la feuille de style — c'est la raison pour
laquelle le thème existant a dû être restreint par `objectName` au menu et aux options.

Aucune `QPalette` n'est définie nulle part dans `Source/HMI`. Les couleurs qui ne passent pas par la
feuille de style — fonds de vues, texte désactivé, couleur de sélection, infobulles, boîtes de dialogue
standard — viennent donc directement du système d'exploitation.

Les grandeurs d'habillage sont par ailleurs éparpillées : taille de vignette dans *AssetThumbnailView*,
autre taille de vignette dans *PalettePanel*, taille d'icône de ligne dans *TexturePanel*, et deux
widgets de remappage jumeaux (*KeybindingsWidget* et *GamepadBindingsWidget*) dont les largeurs
minimales diffèrent sans raison. La couleur d'effacement du viewport, définie dans *GameViewport*, est
**proche mais différente** du fond de la feuille de style : la jointure entre le canevas Direct3D 11 et
les widgets qui l'entourent est visible.

Cette tâche est le socle : les cinq suivantes en dépendent.

## Travail à réaliser
- **Jetons de design** dans un en-tête unique : couleurs nommées **par rôle** (fond, surface, surface
  alternée, bordure, texte, texte atténué, accent, accent au survol, erreur), échelle d'espacement,
  échelle typographique, tailles d'icônes et de vignettes, largeurs de contrôles usuelles.
  Nommer par rôle et non par teinte — un jeton `accent` survit à un changement de couleur, un jeton
  `ambre` non, et la tâche 6 introduira un second jeu de valeurs pour le thème clair.
- **Deux portées d'habillage**, distinguées dès les jetons :
  - l'**identité du jeu** — menu principal, écran Options, jeu — dont les couleurs sont **invariantes**
    et ne suivront jamais aucun réglage d'affichage ;
  - le **châssis d'édition** — panneaux, barre d'outils, barre d'état, barre de menus de l'éditeur,
    boîtes de dialogue ouvertes depuis lui — dont les couleurs sont **variables** et deviendront
    réglables en TACHE-06.

  Les deux portées partagent la même **structure** de rôles, les mêmes échelles d'espacement et de
  typographie, et les mêmes tailles : seules les valeurs de couleur diffèrent. C'est cette symétrie qui
  permettra d'ajouter un jeu clair sans toucher à la feuille de style.
- **Choix explicite du style Qt** au démarrage, avant toute création de widget.
- **Palette applicative** complète construite à partir des jetons, couvrant les trois groupes de
  Qt (actif, inactif, désactivé) — le groupe désactivé est celui qu'on oublie, et celui qui trahit le
  plus vite un thème incomplet. La palette de l'application porte la portée **variable** (le châssis
  d'édition, majoritaire en nombre de widgets) ; les écrans d'identité reçoivent explicitement la
  leur, construite depuis les jetons invariants.
- **Couleur d'effacement du viewport dérivée des jetons**, en remplacement de la valeur littérale
  actuelle de *GameViewport* : du jeton de fond **variable** en mode édition, du jeton de fond
  **invariant** en mode jeu et en essai. Le viewport est la seule surface qui appartient tour à tour
  aux deux portées, selon le mode de l'application.
- **Remplacement des constantes de style locales** des widgets par les jetons correspondants, en
  unifiant au passage les deux largeurs minimales divergentes des widgets de remappage.
- **Conversion pure** jeton → chaîne de couleur, exposée comme fonction testable sans instance
  d'application ni GPU (elle servira aussi à produire la feuille de style en TACHE-02).

## Fichiers impactés
- `Source/HMI/Interface/DesignTokens.h` (nouveau) — les jetons et leur conversion.
- `Source/HMI/Interface/ApplicationTheme.{h,cpp}` (nouveau) — construction de la palette, application
  du style.
- `Source/HMI/main.cpp` — choix du style et application de la palette avant la fenêtre principale.
- `Source/HMI/Game/GameViewport.cpp` — couleur d'effacement dérivée du jeton de fond.
- `Source/HMI/Editor/AssetThumbnailView.cpp`, `Source/HMI/Editor/PalettePanel.cpp`,
  `Source/HMI/Editor/TexturePanel.cpp`, `Source/HMI/Interface/KeybindingsWidget.cpp`,
  `Source/HMI/Interface/GamepadBindingsWidget.cpp` — constantes locales remplacées par les jetons.
- `Source/Test/Unit/HMI/Interface/test_design_tokens.cpp` (nouveau).

## Tests (obligatoires)
- **Conversion de couleur** : chaque jeton produit une chaîne de couleur valide et stable ; la
  conversion est **pure** (même entrée, même sortie), testée sans instance d'application.
- **Cohérence viewport / interface** : la couleur d'effacement du viewport et le jeton de fond de la
  portée correspondante dérivent de la même valeur, en mode édition **comme** en mode jeu — un test
  par mode, qui échoue si l'une des deux est modifiée seule. C'est le seul garde-fou possible contre
  la réapparition de la couture actuelle.
- **Symétrie des deux portées** : les jeux de jetons invariant et variable définissent exactement le
  même ensemble de rôles, d'échelles et de tailles — un test qui échoue si un rôle est ajouté à l'un
  sans l'autre. Sans lui, la TACHE-06 découvrirait la divergence trop tard.
- **Aucun doublon de grandeur** : les widgets de remappage exposent la même largeur minimale.

## Points d'attention
- **Le style doit être choisi avant la création du moindre widget** : appliqué après, il ne se propage
  pas aux widgets déjà construits.
- **Changer de style change l'apparence de tout**, y compris les boîtes de dialogue standard
  (message, fichier, saisie) et le dialogue de redimensionnement construit à la main dans
  *MainWindow*. Les repasser en revue fait partie de la tâche, pas d'un correctif ultérieur.
- **Les jetons ne remplacent pas les couleurs du monde rendu.** `hmi::TileAppearance` et les
  `core::Color` du pipeline Direct3D 11 décrivent le contenu du jeu, pas l'habillage de l'application :
  ils restent inchangés. Seule la couleur d'effacement du viewport, visuellement contiguë aux widgets,
  entre dans le périmètre.
- Ne pas nommer les jetons d'après leur teinte : la TACHE-06 ajoutera un jeu clair, et un jeton nommé
  d'après sa couleur y deviendra un contresens.
- **La frontière entre les deux portées se pose maintenant, pas en TACHE-06.** Un habillage conçu
  comme un jeu unique de couleurs ne se scinde pas après coup sans réécrire la feuille de style : la
  TACHE-06 doit n'avoir qu'à fournir un second jeu de valeurs.
- **Ne pas faire de l'identité un « thème sombre » parmi d'autres.** Le fond sombre et l'accent ambre
  du menu appartiennent à l'apparence du jeu ; les exposer comme un thème inviterait tôt ou tard à les
  rendre réglables, ce que ce lot exclut.

## Définition de fait (DoD)
- L'application choisit explicitement son style et applique des palettes complètes dérivées d'un jeu
  unique de jetons, séparé en portée invariante et portée variable de structure identique ; la couleur
  d'effacement du viewport suit la portée du mode courant et des tests le garantissent ; aucune
  constante de style ne subsiste dans le code des widgets concernés ; les boîtes de dialogue standard
  ont été revues ; `/W4 /WX` propre.

## Exigences
`EX-IHM-050` (style maîtrisé, deux portées d'habillage), `EX-IHM-051` (source unique des grandeurs
d'habillage) ; prépare `EX-IHM-054` (thème clair/sombre de l'éditeur) ; réutilise `EX-IHM-001`
(interface hors-jeu en Qt), `EX-NFR-010` (`Core` indépendant de la présentation).
