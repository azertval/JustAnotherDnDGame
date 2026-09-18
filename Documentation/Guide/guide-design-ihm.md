# Système de design et architecture de l'information {#guide-design-ihm}

> Statut : **livré** (`EX-IHM-050` → `EX-IHM-055`, `EX-IHM-060` → `EX-IHM-062`), **restreint à
> l'éditeur** au `LOT-86`. Cette page décrit **comment le châssis d'édition se présente et se
> répartit** : ses jetons de design et leur application, puis la distribution de l'information
> dans ses panneaux.
>
> **Les jetons du JEU n'y sont plus.** Ils vivent désormais dans `Source/Ui/Theme/Tokens.qml`,
> écrits à la main et possédés par la conception — voir @ref guide-conception-qds. L'étanchéité des
> deux portées, que cette page décrivait comme garantie par un test, est devenue **structurelle** :
> deux langages, deux binaires, aucun chemin entre eux. Le socle applicatif est en
> @ref guide-ihm-qt.

## Pourquoi un système de design

Une application Qt Widgets qui n'appelle jamais `setStyle` tourne sur le style **natif** de la
plate-forme, qui ignore une grande partie de toute feuille de style posée par-dessus : un thème n'y
a d'effet fiable que sur les quelques widgets qu'on a pris soin de cibler un par un. C'est la cause
racine, unique et invisible, que ce système traite — avec ses corollaires : ni `QPalette`, ni
`QToolBar`, ni `QActionGroup`, ni `QFontDatabase`, ni `QStyleHints` ne servaient, et
`devicePixelRatio` n'était pris en compte que par le viewport, si bien que les vignettes étaient
floues dès que l'affichage était mis à l'échelle.

## Les jetons : une seule source, deux portées étanches

\ref hmi::DesignTokens "DesignTokens" (`HMI/Interface/DesignTokens.h`) est la source unique des
couleurs, espacements, tailles et niveaux typographiques. Logique **pure**, sans Qt ni GPU
(`EX-NFR-010`), compilée à la fois dans l'application et directement dans `UnitTests`.

Deux traits méritent l'attention.

**Les couleurs sont nommées par rôle, pas par teinte.** \ref hmi::ColorTokens "ColorTokens" expose
`background`, `surface`, `border`, `text`, `textMuted`, `accent`, `error`… Un jeton `accent`
survit à un changement de couleur ; un jeton qui s'appellerait `ambre` deviendrait un mensonge le
jour où l'accent passe au bleu. Même principe pour la typographie
(\ref hmi::TypographyTokens "TypographyTokens") : `screenTitle`, `sectionTitle`, `body`,
`caption` — jamais une taille ponctuelle hors de cette échelle.

**Il y a deux portées, de structure identique.**

- `identityTokens()` — l'**identité** du jeu. Invariante. Les écrans du jeu ont désormais leurs
  propres jetons en QML ; côté C++, cette portée ne fixe plus que la police de base.
- `editorDarkTokens()` / `editorLightTokens()` — le **châssis d'édition** : panneaux, barres
  d'outils, barre d'état, boîtes de dialogue ouvertes depuis l'éditeur. Variable.

Réutiliser la même structure pour les deux rend leur symétrie garantie par le système de types
plutôt que par convention : un rôle ajouté à l'une existe nécessairement dans l'autre. Un test
verrouille par ailleurs qu'elles partagent les **mêmes échelles**
(`DesignTokensTest.LesDeuxPorteesPartagentLesMemesEchelles`).

> Piège de plate-forme consigné dans l'en-tête : `<Windows.h>` définit une macro `small`, qui
> casserait silencieusement `SpacingTokens::small`. Le fichier la neutralise — garde conservée
> après le portage QRhi, l'en-tête pouvant encore être tiré indirectement.

## De jetons purs à une application habillée

`HMI/Interface/ApplicationTheme.h` est la couche Qt au-dessus des jetons. Elle fait, dans cet
ordre :

1. \ref hmi::applyApplicationStyle "applyApplicationStyle" **choisit le style Fusion avant la
   création du moindre widget** (`EX-IHM-050`) — c'est ce choix, et lui seul, qui rend l'habillage
   prévisible et permet enfin au thème de couvrir toute l'application au lieu de deux écrans ;
2. \ref hmi::buildApplicationPalette "buildApplicationPalette" **construit la `QPalette` complète**,
   dans ses trois groupes (actif, inactif, désactivé) : une palette partielle laisse Qt combler les
   trous avec les couleurs du système, ce qui produit des incohérences uniquement visibles sur une
   fenêtre inactive ou un contrôle grisé ;
3. \ref hmi::applyStyleSheet "applyStyleSheet" **produit `theme-editor.qss`** par substitution de
   marqueurs depuis les jetons, via
   \ref hmi::substituteStyleSheetTemplate "substituteStyleSheetTemplate" (fonction pure). Plus
   aucune couleur littérale dans la feuille de style.

Le focus clavier est rendu visible partout — condition de la navigation à la manette
(`EX-IHM-040`), qui n'a pas de pointeur pour dire où elle en est.

> **Invariant à ne pas casser.** Ne jamais poser `titlebar-close-icon` ou
> `titlebar-normal-icon: none` sur un `QDockWidget` dans `theme-editor.qss` : ces propriétés ont
> provoqué un plantage intermittent à la fermeture de l'éditeur. Masquer un bouton de barre de titre se fait par les `features` du dock, pas par la feuille de style.

## Typographie : une police embarquée, un repli qui n'invente rien

La police **Inter** est embarquée (`Assets/Fonts/Inter-{Regular,Bold}.ttf`, licence SIL OFL 1.1).
\ref hmi::resolveFontFamily "resolveFontFamily" décide de la famille effective : la police
enregistrée si elle l'est, sinon une famille **générique** — jamais un second nom de police codé en
dur, qui ne serait qu'un pari sur ce qui est installé chez l'utilisateur. Les tailles et marges
figées dans les fichiers `.ui` ont été retirées au profit des jetons.

## Les commandes : une définition, trois surfaces

\ref hmi::editorActionCatalog "editorActionCatalog" (`HMI/Interface/ActionCatalog.h`) est une
**table pure** décrivant les outils et les commandes de l'éditeur (`hmi::EditorActionSpec`, groupés
par `hmi::EditorActionGroup`). `hmi::EditorActions` construit les `QAction` depuis cette table, et ces mêmes objets alimentent simultanément le menu, la barre d'outils et le
raccourci clavier (`EX-IHM-055`). C'est ce qui supprime la double définition : sans cette table, une
commande présente au menu et dans une barre existerait deux fois, et pourrait diverger.

Les icônes sont **dessinées par code** : \ref hmi::iconGeometry "iconGeometry" décide *quoi*
dessiner (géométrie pure, testable), `hmi::themeIcon` décide *comment* le peindre avec `QPainter`,
recoloré depuis les jetons. Aucun fichier d'icône à livrer, et un changement de thème recolore tout sans réexporter d'assets.

\ref hmi::EditorActions::applyShortcuts "EditorActions::applyShortcuts" synchronise le raccourci
**effectif** de chaque action depuis les touches de l'éditeur (`hmi::EditorKeyBindings`), et refait
les infobulles « libellé + raccourci » : un raccourci intercepté en dur à côté du catalogue
contournerait ces touches sans que rien ne le signale.

## Netteté à toute échelle d'affichage

\ref hmi::thumbnailPixelSize "thumbnailPixelSize" (fonction pure) donne la taille en pixels
**réels** d'une vignette à partir de sa taille logique et du facteur d'échelle de l'écran. Les
vignettes de la palette sont rendues à cette taille, et régénérées lors d'un changement d'écran :
nettes à 100 %, 125 % et 150 %.

## Thème clair/sombre

\ref hmi::resolveEffectiveEditorTheme "resolveEffectiveEditorTheme" traduit un **réglage**
(`EditorThemeSetting` : Système, Clair, Sombre) et l'indication du système d'exploitation en un
**mode** effectif (`EditorThemeMode` : Clair ou Sombre). La séparation compte : le réglage est ce
que l'utilisateur a choisi et qu'on persiste, le mode est ce qu'on applique. « Système » n'est pas
une troisième apparence, c'est une délégation.

Le thème s'applique à chaud — palette, feuille de style, icônes — sans redémarrage. Le contraste
texte/fond est vérifié par test pour les deux thèmes (seuils WCAG ⧉).

## Architecture de l'information : ce qui informe reste, ce qui commande est unique

Le constat de départ : une aide affichée dans une **ligne unique** de barre d'état, figée à
l'entrée en édition, est définitivement effacée par le premier message transitoire. Passé la
première minute, la barre d'état ne dit plus rien.

### Une barre d'état structurée

\ref hmi::editorStatusLines "editorStatusLines" (`HMI/Editor/EditorStatus.h`) est une **fonction
pure** qui décide du contenu de cinq zones **permanentes** : carte ouverte, modifications non
enregistrées, outil actif, case survolée, zoom. Ces zones sont ajoutées par `addPermanentWidget` : un message transitoire
ne peut donc plus les recouvrir. L'aide contextuelle à l'outil actif se restaure automatiquement à
l'expiration du message (`MainWindow::refreshStatusHelp`, minuteur unique).

Le viewport expose pour cela `hoveredCell()` et `zoom()`, avec des signaux émis **seulement sur
changement réel** — sinon la barre d'état se reconstruirait à chaque mouvement de souris.

### Des panneaux groupés, et qui suivent l'outil

Les panneaux sont regroupés en onglets par défaut (`tabifyDockWidget`), chacun restant
individuellement déplaçable, détachable et refermable. \ref hmi::panelForTool "panelForTool" est
une table pure — même patron que le catalogue d'actions — qui dit quel panneau mettre en avant pour
un outil donné.

La règle de mise en avant a deux garde-fous : elle **n'est jamais un masquage** (on met en avant,
on ne cache rien), et elle **cède dès que l'utilisateur a imposé son choix** (onglet sélectionné à
la main, panneau déplacé). Une interface qui réorganise les panneaux malgré l'utilisateur devient
vite un adversaire.

> La disposition des panneaux est persistée et **versionnée** (`LAYOUT_VERSION`). Chaque
> redistribution incrémente la version, ce qui invalide les dispositions antérieures : sans cela,
> une disposition enregistrée avant le changement rouvrirait des panneaux dans un agencement qui
> n'a plus de sens.

### Un état, un contrôle

`EX-IHM-062` interdit qu'un même état ou une même commande soit exposé à deux endroits. Les
conséquences concrètes : le menu Affichage porte en tête les seules commandes de **vue**
(recadrer, grille), et chaque panneau n'y a qu'une entrée, sa bascule de visibilité ; et
Annuler/Refaire/Copier/Coller dispatchent via \ref hmi::EditContextTarget "EditContextTarget",
interface qu'implémente `hmi::EditorViewport`. C'est ce seuil de dispatch qui permettrait à un
second contexte d'édition d'avoir son propre historique sans réécrire une ligne du dispatch.

## Deux identités, deux règles d'échelle (LOT-66)

Les deux portées avaient d'abord la **même** échelle typographique. C'était cohérent tant
qu'aucune des deux ne cherchait à être autre chose qu'un habillage correct ; ça ne l'est plus dès
que les écrans du jeu revendiquent l'identité du **parchemin de Tanares** (`LOT-66`).

La règle tient en une phrase : **le châssis d'édition suit les réglages du système, les écrans du
jeu sont une image agrandie d'un facteur entier.**

\ref hmi::identityScaleFor "identityScaleFor" décide de ce facteur depuis la hauteur **logique** de la
fenêtre — jamais la hauteur réelle : Qt applique la mise à l'échelle du système par-dessus, et
multiplier une seconde fois donnerait une interface deux fois trop grande sur un écran réglé à
200 %. La division est entière et non arrondie : une fenêtre de 700 px passerait sinon à l'échelle 2,
pour laquelle il manque 20 px, et la dernière entrée du menu disparaîtrait sous le bord.

\ref hmi::IdentityBaseScale "IdentityBaseScale" porte les grandeurs de la portée identité **en
pixels à l'échelle 1**, que la feuille de style reçoit déjà multipliées. En pixels et non en points :
un point vaut une fraction variable de pixel selon l'écran, et le facteur entier n'aurait alors plus
rien d'entier.

> Piège évité : multiplier les jetons **partagés** paraissait plus simple. Ils valent déjà 32 pt pour
> un titre — les doubler aurait donné 64 pt à 720p, soit l'excès inverse du problème de départ.

**Et le facteur se borne à l'écran, pas à la fenêtre** (`EX-IHM-081`,
\ref hmi::identityScaleForDisplay "identityScaleForDisplay"). Dériver le facteur de la seule
hauteur de fenêtre en faisait une boucle sans point fixe : le facteur grossit les grandeurs
d'habillage, qui grossissent la taille minimale des écrans, qui grossit la fenêtre — laquelle
relance le calcul un cran plus haut, sans que rien ne redescende jamais, une fenêtre ne pouvant
pas passer sous son propre minimum. La zone d'affichage disponible, elle, ne dépend d'aucune
décision de l'application : c'est ce qui ferme la boucle.

### Pourquoi le facteur reste entier après la sortie du pixel art

Le `LOT-68` l'exigeait parce que le filtrage au plus proche voisin ne sait pas rendre une bordure
d'un pixel et demi. Cette raison a disparu avec la charte parchemin, qui peint anticrénelé. Le
facteur reste pourtant entier, pour une **seconde** raison, elle intacte : les longueurs de la
feuille de style sont des **entiers de pixels**. À 1,5×, le trait d'une unité et le filet d'une
unité s'arrondissent tous deux à 2 px — la réserve de parchemin qui les sépare disparaît, et
l'encadrement se lit comme une bordure épaisse. Une échelle fractionnaire ne serait donc pas
*floue* : elle serait **fausse**, et silencieuse.

## La barre d'outils n'est pas un menu

Supprimer les **doubles définitions** ne suffit pas : reste la **saturation**, qui est un problème
distinct. Une commande peut n'exister qu'une fois en code et rester exposée à trois endroits à
l'écran.

\ref hmi::ActionSurface "ActionSurface", porté par le catalogue d'actions, dit de chaque commande si
elle mérite une place permanente à l'écran. La barre ne garde que la sélection d'outil et ce qui se
déclenche au fil du geste ; le reste vit au menu, avec son raccourci. Un test plafonne le nombre de
commandes admises par barre : un plafond qu'on relève sans y penser ne protège de rien.

## Voir aussi
- @ref guide-ihm-qt — le socle applicatif Qt, les surfaces de rendu QRhi, la boucle et les entrées.
- @ref guide-editeur — l'éditeur de niveau lui-même (brouillon, outils, essai immédiat).
- [Spécification IHM](@ref spec-interface-ihm) — le *quoi/pourquoi* (`EX-IHM-050` → `EX-IHM-062`).
