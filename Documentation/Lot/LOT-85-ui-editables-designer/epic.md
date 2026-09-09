# LOT-85 — Les planches s'éditent dans Qt Designer {#lot-85}

> Statut : **fait** (vérification automatisée : `scripts/check_ui_designer.py` et
> `scripts/sync_ui_theme.py --check` verts, `scripts/lint_exigences.py` et `scripts/lint_lots.py`
> verts, 1083 tests `ctest` au vert).
> Prérequis : `LOT-38` (la planche de la fiche), `LOT-68` (le châssis des écrans du RPG),
> `LOT-73` (les feuilles de style par portée).

## Objectif

Rendre vraie la promesse que la table portait déjà. `RpgScreens.h` déclare
`RpgRendering::DesignerPlate` — « une **planche** décrite en Qt Designer, pour l'écran qui a une
maquette gravée » — et `ParchmentPanel.h` annonçait « **Promu dans les `.ui`** (Qt Designer) ».

Rien ne l'outillait. Ouvrir `RpgCharacterSheetPlate.ui` dans Designer donnait des rectangles gris.

## Ce qui empêchait de voir, et ce qui empêchait de modifier

Trois causes distinctes, qu'il fallait traiter séparément :

| Cause | Effet dans Designer | Traité par |
|---|---|---|
| Aucun plugin : Designer ne compile pas le C++ et ne connaît des widgets promus que leur classe de base | Rectangles gris à la place des peintres | Étape 2 |
| `theme-identity.qss` est un gabarit à marqueurs, posé à l'exécution sur la pile d'écrans | Ni typographie ni couleurs de texte | Étape 3 |
| La mise en page vivait pour les deux tiers dans le constructeur (28 widgets déclarés, ~60 construits) | Le geste d'édition n'avait presque pas de portée | Étape 4 |

## Ce qui a été fait

**1. `IdentityWidgetsLib`.** Les sept peintres promus étaient compilés *dans l'exécutable* ; une
bibliothèque partagée ne pouvait pas s'y lier. Ils passent dans une bibliothèque statique, avec
`ApplicationTheme`. La coupure tient sans effort : ces fichiers ne connaissent que les jetons de
design, la géométrie pure de `HmiLib` et Qt Widgets — rien du jeu, de la fenêtre ni de la
localisation. Déplacement de sources, aucun changement de comportement.

**2. Le plugin `JadgDesignerPlugin`.** Une `QDesignerCustomWidgetCollectionInterface` exposant
**onze** widgets, groupe « JADG — Identité ». Il peint exactement ce que le jeu peint, et ce n'est
pas une coïncidence : `hmi::identityTokens()` est une table **pure**, sans état d'application.
`initialize()` pose `setIdentityScale(2)`, le facteur des maquettes à 1280 × 720, pour que les
proportions soient celles du jeu.

`SheetGauge` n'était pas constructible par Designer (son `Tone` était obligatoire) : le paramètre
reçoit un défaut, et le ton devient une `Q_PROPERTY` — l'encre d'une jauge se choisit désormais dans
l'inspecteur de propriétés.

`scripts/designer.ps1` bâtit et lance l'ensemble. Il existe pour deux pièges qui échouent en
**silence** : `designer.exe` est une binaire Release et rejette une DLL Debug sans un mot ; et
`QT_PLUGIN_PATH` désigne le dossier de build, jamais l'installation Qt — une copie dans
`plugins/designer/` marcherait une fois, puis survivrait périmée à la reconstruction suivante.

**3. Le thème, visible sans déformer le jeu.** `Source/Tools/IdentityQss` résout le gabarit au
facteur ×2 et écrit `theme-identity.resolved.qss`, versionné. Un outil C++ et non un script :
réécrire la substitution des marqueurs en Python en aurait fait une seconde vérité. Le fichier porte
en tête les empreintes du gabarit et de `DesignTokens.cpp`, ce qui rend sa péremption détectable
**sans compilateur** — le runner de CI n'a pas Qt.

`scripts/sync_ui_theme.py` en dépose dans chaque formulaire la tranche qui le cadre. Une
transformation, et une seule : les règles du RPG sont cadrées par `#RpgScreenHost`, le conteneur qui
accueille l'écran dans le jeu ; dans Designer la planche **est** la racine, et le préfixe est réécrit
vers son nom.

Chaque écran fait ensuite `setStyleSheet(QString())` après `setupUi`. Ce n'est pas un détail : la
feuille est figée à ×2, et une feuille posée sur le widget **prime** sur celle de la pile d'écrans
(`EX-IHM-082`). La laisser en place casserait la mise à l'échelle, et le défaut ne se verrait qu'en
redimensionnant la fenêtre.

**4. Le `.ui` devient l'autorité — colonne gauche de la page des caractéristiques.** Elle est
entièrement statique : aucune de ses lignes n'est engendrée par une donnée. Ses quinze widgets
passent dans le `.ui`, rôles compris (`rpgRole` en propriété dynamique, ce qui fait que Designer les
style comme le jeu). `buildLeftColumn` tombe de 65 à 38 lignes et ne garde que ce qu'un `.ui` ne peut
pas porter : les grandeurs — calculées depuis les jetons **multipliées par le facteur
d'agrandissement**, elles se figeraient à ×2 dans le fichier —, le rattachement des libellés au
catalogue de traduction, et le ton des jauges.

**5. La justification quitte les `.ui`.** Vingt-neuf commentaires XML relogés dans les en-têtes C++
des cinq écrans, sous une rubrique « Ce que la mise en page porte ». Designer les efface à
l'enregistrement : les laisser là, c'était les perdre au premier geste d'édition.

**6. La règle, et ce qui la tient.** `EX-IHM-006` (spécification IHM, § 11), deux contrôles dans le
job `lint-exigences`, et une entrée dans `CONTRIBUTING.md`.

**7. Une zone défilante par encart, et non une pour tout l'écran.** Les trois colonnes de la page
des caractéristiques partageaient une seule zone défilante. Une seule manquait de hauteur — les
dix-huit compétences — et elle imposait donc le défilement **aux trois**, alors que les deux autres
avaient plusieurs centaines de pixels de vide. Le châssis partait avec : on perdait le bandeau et
les onglets dès qu'on descendait.

Chaque encart a désormais la sienne — les quatorze de la fiche, et ceux du châssis
`RpgScreenFrame` dont héritent les huit autres écrans. Le retrait intérieur du parchemin reste
**dehors** : à l'intérieur, il défilerait avec le contenu et le texte viendrait toucher le cadre.

Deux réglages s'y sont révélés nécessaires, et le second seulement à l'écran :

- `Ignored` **en hauteur seulement**. Sur les deux axes, les colonnes perdaient toute revendication
  de largeur, la roue — seule `Expanding` — prenait la place, et la colonne de combat rognait
  « Vitesse » et « Perception passive ».
- La largeur des trois colonnes est désormais **écrite** (`stretch` 1,1,1 sur `bodyRow`) plutôt que
  laissée à un bras de fer entre politiques de taille.

**8. Les huit autres écrans du RPG ont enfin une planche.** Ils n'en avaient aucune : bâtis à
l'exécution par `RpgScreenFrame` depuis la table, ils n'offraient **rien à ouvrir** dans Designer.
`Source/Tools/RpgScreenUi` les engendre depuis cette même table — un outil C++, pour la raison déjà
dite : la table vit en C++, et la relire en Python en aurait fait une seconde vérité. Il s'exécute
**une fois** et refuse d'écraser sans `--force` : les fichiers produits appartiennent ensuite à qui
les édite.

Côté code, une seule classe pour les huit : `hmi::RpgUiScreen` porte le comportement commun, et le
patron `hmi::RpgUiPlate<Ui::…>` n'apporte que le `Ui::` de l'écran. Huit classes jumelles auraient
été huit copies d'un même corps.

La planche porte la structure et trois propriétés dynamiques — `rpgRole` (que le thème cible),
`rpgKey` (la traduction) et `rpgValue` (l'identifiant de la table, mot pour mot). Le code garde
**toutes les grandeurs**, appliquées par rôle : les figer dans le fichier les arrêterait au facteur
d'agrandissement du jour où il a été écrit.

`uic` et non `QUiLoader` : charger les planches à l'exécution aurait évité une recompilation après
chaque retouche, mais au prix d'un module Qt (`qttools`) de plus dans le jeu livré, dans les quatre
jobs de CI et dans le paquet de release — pour un dépôt qui n'installe aujourd'hui que
`qtmultimedia qtshadertools qtcanvaspainter qtsvg`. Le compte n'y était pas.

`EX-IHM-090` est **amendée** en conséquence : la table reste la source des identifiants, des
libellés et du cycle ; la disposition est passée aux planches.

## Ce que les contrôles rendent impossible

`check_ui_designer.py` et `sync_ui_theme.py --check` verrouillent quatre liens qui lâchent **sans
rien casser de visible** — le build reste vert, le jeu s'affiche bien, seule l'édition est morte :

- un widget promu que le plugin n'expose pas — Designer le rend en gris ;
- une entrée du catalogue que plus aucune fabrique ne construit, ou dont l'en-tête a bougé — le
  plugin ne se charge plus, sans message ;
- un écran qui garde le thème figé — la mise à l'échelle cesse ;
- un commentaire XML réintroduit — effacé au premier enregistrement.

Le contrôle refuse aussi de passer au vert **par vacuité** : un relevé vide est une erreur, pas un
succès. C'est la panne du `LOT-78`.

## Ce qui reste à faire

L'étape 4 n'est appliquée qu'à la **colonne gauche** de la page des caractéristiques. Restent
construits en code :

- la **colonne droite** de la même page — son écu et ses quatre médaillons sont statiques et
  devraient descendre dans le `.ui`, mais ses dix-huit lignes de compétences viennent de la table.
  Les deux s'intercalent dans un même `QVBoxLayout`, et les séparer proprement demande de donner à
  la région engendrée son propre conteneur nommé ;
- les **quatre autres onglets** (équipement, traits, sorts, compagnie), majoritairement engendrés
  par la table — la part statique y est plus mince, et le partage moins évident.

Rien n'y est bloqué : le procédé est éprouvé de bout en bout sur la colonne gauche, et le motif du
conteneur nommé est la seule pièce de conception qui manque. Tant que ces régions restent en code,
`check_ui_designer.py` ne peut pas les surveiller — c'est la limite connue de ce lot.

## Vérification

```bash
powershell -ExecutionPolicy Bypass -File scripts/designer.ps1 `
    -Path Source/Elements/UI/RpgCharacterSheetPlate.ui
```

La planche doit apparaître peinte, thème compris. *Aide → À propos des plugins* doit lister la
collection ; en cas d'absence silencieuse, relancer avec `QT_DEBUG_PLUGINS=1` (cause quasi
certaine : Debug/Release, ou ABI).

Puis, pour éprouver que le geste porte : y déplacer une jauge, enregistrer, reconstruire, et
constater le déplacement dans le jeu.
