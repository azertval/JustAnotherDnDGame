# Système de design et architecture de l'information {#guide-design-ihm}

> Statut : **livré** (`LOT-56`, `EX-IHM-050` → `EX-IHM-055` ; `LOT-57`, `EX-IHM-060` →
> `EX-IHM-062`). Cette page décrit **comment l'IHM Qt se présente et se répartit** : les jetons de
> design et leur application, puis la distribution de l'information dans l'éditeur. Le socle
> applicatif (fenêtre, viewport, boucle) est en @ref guide-ihm-qt.

## Pourquoi un système de design

De `LOT-34` à `LOT-52`, l'interface a gagné des panneaux, des menus, des grilles et des
inspecteurs — sans qu'aucun lot ne traite jamais son **apparence**. La cause racine était unique et
invisible : l'application n'appelait jamais `setStyle` et tournait donc sur le style **natif** de
la plate-forme, qui ignore une grande partie de toute feuille de style posée par-dessus. C'est
pourquoi `theme.qss` avait dû être restreint par `objectName` au menu principal et à la page
Options, au lieu d'être étendu : ailleurs, il n'avait tout simplement pas d'effet fiable.

Ni `QPalette`, ni `QToolBar`, ni `QActionGroup`, ni `QFontDatabase`, ni `QStyleHints` n'étaient
utilisés ; `devicePixelRatio` n'était pris en compte que par le viewport Direct3D 11, si bien que
les vignettes de la palette et des grilles d'assets étaient floues dès que l'affichage était mis à
l'échelle.

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

- `identityTokens()` — l'**identité** du jeu : menu principal, page Options, jeu. Invariante.
- `editorDarkTokens()` / `editorLightTokens()` — le **châssis d'édition** : panneaux, barres
  d'outils, barre d'état, boîtes de dialogue ouvertes depuis l'éditeur. Variable.

Réutiliser la même structure pour les deux rend leur symétrie garantie par le système de types
plutôt que par convention : un rôle ajouté à l'une existe nécessairement dans l'autre. Un test
verrouille par ailleurs leur **étanchéité** — l'identité du jeu doit rester rigoureusement
inchangée quel que soit le thème actif du châssis, y compris après une bascule à chaud.

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
   aucune couleur littérale dans la feuille de style. La portée **identité** est produite à part par
   \ref hmi::identityStyleSheet "identityStyleSheet", et posée par `MainWindow` sur la **pile
   d'écrans** — voir « Une feuille par portée » ci-dessous.

Le focus clavier est rendu visible partout — condition de la navigation à la manette
(`EX-IHM-040`), qui n'a pas de pointeur pour dire où elle en est.

> **Invariant à ne pas casser.** Ne jamais poser `titlebar-close-icon` ou
> `titlebar-normal-icon: none` sur un `QDockWidget` dans `theme-editor.qss` : ces propriétés ont provoqué
> un plantage intermittent à la fermeture depuis l'éditeur pendant le `LOT-56`. Masquer un bouton
> de barre de titre se fait par les `features` du dock, pas par la feuille de style.

## Typographie : une police embarquée, un repli qui n'invente rien

La police **Inter** est embarquée (`Assets/Fonts/Inter-{Regular,Bold}.ttf`, licence SIL OFL 1.1).
\ref hmi::resolveFontFamily "resolveFontFamily" décide de la famille effective : la police
enregistrée si elle l'est, sinon une famille **générique** — jamais un second nom de police codé en
dur, qui ne serait qu'un pari sur ce qui est installé chez l'utilisateur. Les tailles et marges
figées dans les fichiers `.ui` ont été retirées au profit des jetons.

## Les commandes : une définition, trois surfaces

\ref hmi::editorActionCatalog "editorActionCatalog" (`HMI/Interface/ActionCatalog.h`) est une
**table pure** décrivant les outils et les commandes de l'éditeur (`hmi::EditorActionSpec`, groupés
par `hmi::EditorActionGroup`). `hmi::EditorActions` construit les `QAction`
depuis cette table, et ces mêmes objets alimentent simultanément le menu, la barre d'outils et le
raccourci clavier (`EX-IHM-055`). C'est ce qui remplace les boutons radio empilés de l'ancien
panneau Outils — et, surtout, ce qui supprime la double définition : avant, une commande présente
au menu et dans une barre existait deux fois, et pouvait diverger.

Les icônes sont **dessinées par code** : \ref hmi::iconGeometry "iconGeometry" décide *quoi*
dessiner (géométrie pure, testable), `hmi::themeIcon` décide *comment* le peindre avec `QPainter`,
recoloré depuis les jetons. Même découpage que `hmi::gameHudLines` (`LOT-52`). Aucun fichier
d'icône à livrer, et un changement de thème recolore tout sans réexporter d'assets.

\ref hmi::EditorActions::applyShortcuts "EditorActions::applyShortcuts" synchronise le raccourci
**effectif** de chaque action depuis les touches remappées, y compris après un remappage à chaud.
C'est ce qui a permis, au `LOT-57`, de rendre réellement remappables dix actions d'éditeur dont
neuf n'étaient jamais lues : leurs raccourcis étaient interceptés en dur, et Copier/Coller au
clavier contournait même complètement `EditorKeyBindings`.

## Netteté à toute échelle d'affichage

\ref hmi::thumbnailPixelSize "thumbnailPixelSize" (fonction pure) donne la taille en pixels
**réels** d'une vignette à partir de sa taille logique et du facteur d'échelle de l'écran. Les
vignettes de la palette, des grilles d'assets et des lignes du panneau Textures sont rendues à
cette taille, et régénérées lors d'un changement d'écran : nettes à 100 %, 125 % et 150 %. Le
canevas de l'atelier de dessin d'assets (@ref guide-atelier-pixel-art) réutilise cette même fonction plutôt
que de redéfinir sa propre règle.

## Thème clair/sombre

\ref hmi::resolveEffectiveEditorTheme "resolveEffectiveEditorTheme" traduit un **réglage**
(`EditorThemeSetting` : Système, Clair, Sombre) et l'indication du système d'exploitation en un
**mode** effectif (`EditorThemeMode` : Clair ou Sombre). La séparation compte : le réglage est ce
que l'utilisateur a choisi et qu'on persiste, le mode est ce qu'on applique. « Système » n'est pas
une troisième apparence, c'est une délégation.

Le thème s'applique à chaud — palette, feuille de style, icônes — sans redémarrage. Le contraste
texte/fond est vérifié par test pour les deux thèmes (seuils WCAG ⧉).

## Architecture de l'information : ce qui informe reste, ce qui commande est unique

Le `LOT-57` part d'un constat de terrain : l'éditeur affichait son aide dans une **ligne unique**
de barre d'état, figée à l'entrée en mode éditeur et définitivement effacée par le premier message
transitoire. Passé la première minute, la barre d'état ne disait plus rien.

### Une barre d'état structurée

\ref hmi::editorStatusLines "editorStatusLines" (`HMI/Editor/EditorStatus.h`) est une **fonction
pure** — même patron que `hmi::gameHudLines` (`LOT-52`) — qui décide du contenu de zones
**permanentes** : niveau ouvert, modifications non enregistrées, outil actif, case survolée, zoom,
et une sixième zone (couleur courante) quand le contexte est l'atelier de dessin d'assets
(`PixelEditStatusInfo`). Ces zones sont ajoutées par `addPermanentWidget` : un message transitoire
ne peut donc plus les recouvrir. L'aide contextuelle à l'outil actif se restaure automatiquement à
l'expiration du message (`MainWindow::refreshStatusHelp`, minuteur unique).

Le viewport expose pour cela `hoveredCell()` et `zoom()`, avec des signaux émis **seulement sur
changement réel** — sinon la barre d'état se reconstruirait à chaque mouvement de souris.

### Des panneaux groupés, et qui suivent l'outil

Les panneaux sont regroupés en onglets par défaut (`tabifyDockWidget`), chacun restant
individuellement déplaçable, détachable et refermable. \ref hmi::panelForTool "panelForTool" est
une table pure — même patron que le catalogue d'actions — qui dit quel panneau mettre en avant pour
un outil donné ; `panelForPixelTool` fait de même pour les outils du canevas.

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
conséquences concrètes : la bascule Physique/Texture est devenue une entrée unique du menu
Affichage ; l'onglet Calques a quitté le panneau Textures pour ce même menu, une entrée par calque
dans l'ordre de dessin ; et Annuler/Refaire/Copier/Coller dispatchent via
\ref hmi::EditContextTarget "EditContextTarget", interface qu'implémentent `hmi::GameViewport`
puis, au `LOT-54`, `hmi::PixelCanvas`. C'est ce seuil de dispatch qui a permis à l'atelier pixel
art d'avoir son propre historique sans réécrire une seule ligne du dispatch existant.

Tout doublon apparent n'en est pas un : les deux sélecteurs de couche de décor avaient été
**conservés** au `LOT-57`, parce qu'ils ciblaient des états distincts — la couche du prochain décor
posé, et celle du décor sélectionné existant — puis renommés et rassemblés dans le panneau Décors,
l'ambiguïté se levant à l'écran plutôt que dans la documentation. Le `LOT-69` a retiré les deux
avec le système de décors : un plan pictural n'est pas posé, et sa profondeur est un réglage de la
liste, pas un mode de pose.

## Une feuille par portée, et pourquoi (LOT-73)

Les deux portées ci-dessus étaient deux **sections d'un même fichier**, appliqué à l'application
entière. Rien ne l'interdisait tant que l'habillage ne changeait pas en cours d'exécution.

Le `LOT-68` a changé cela : les grandeurs de la portée identité sont multipliées par un facteur qui
suit la hauteur de la fenêtre. Or reposer la feuille de style de l'**application** repolit *tous*
ses widgets — 862 sur celle-ci — et la re-polish leur recalcule métriques, tailles et dispositions.
Mesure en configuration Debug : **cinq secondes par appel**, pour une préoccupation qui ne touche
que les quelques dizaines de widgets des écrans du jeu.

On avait d'abord **regroupé** ces rejeux derrière un minuteur. Le coût redevenait supportable, mais
ne disparaissait pas — et il atterrissait désormais *après* que la fenêtre eut été placée et peinte,
d'où un recalage visible au relâchement de la souris. Un regroupement traite la fréquence d'un coût,
jamais son ampleur.

La séparation la traite (`EX-IHM-082`) :

| Feuille | Portée | Posée sur | Change quand |
|---|---|---|---|
| `theme-identity.qss` | Écrans du jeu (`objectName`) | La **pile d'écrans** | Le facteur d'agrandissement change |
| `theme-editor.qss` | Châssis d'édition | L'**application** | Le thème clair/sombre change |

Le coût d'un changement redevient proportionnel à ce qui change réellement, et il n'y a plus rien à
différer : le facteur s'applique **dans** le redimensionnement, là où l'utilisateur l'attend.

Les deux feuilles sont **disjointes**, et un test le vérifie
(`ApplicationThemeTest.LesDeuxPorteesSontDansDeuxFichiersDisjoints`) : une seule règle d'identité
replacée dans la feuille du châssis ramènerait le rejeu applicatif complet, sans que rien ne le
signale.

### Le Mode IA : enveloppe d'identité, contenu d'outil

`#AiModeScreen` appartient à la portée identité, et c'est le seul de ses écrans à n'être pas un écran
de **joueur** : vingt-six lignes de formulaire, une table à huit colonnes, un graphique. Habillé
comme un menu, il héritait de la police de titrage et du facteur d'agrandissement — d'où une hauteur
minimale de plus de deux mille pixels, et des données denses rendues dans une police conçue pour
sept mots à l'écran.

Il garde donc son **enveloppe** — fond, titre, cadre à bordure franche, bouton de retour — et son
**contenu** passe à la densité d'un outil. Le mécanisme tient en une omission : la règle de l'écran
ne déclare ni `font-family` ni `font-size`, ce qui laisse la police par défaut de l'application
s'appliquer aux descendants ; et les rembourrages du contenu viennent de `tokens.spacing.*`, jamais
multipliés. Les **couleurs**, elles, restent celles du jeu : les emprunter au châssis ferait basculer
le contenu en clair au milieu d'un écran sombre.

## Deux identités, deux règles d'échelle (LOT-66)

Le `LOT-56` avait donné aux deux portées la **même** échelle typographique. C'était cohérent tant
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

### Ce qu'une feuille de style ne sait pas faire

Deux éléments de la charte échappent à `theme.qss`, et c'est pourquoi ils sont **peints** :

- l'**encadrement à cabochons d'angle** (\ref hmi::ParchmentPanel "ParchmentPanel") : une bordure
  QSS ne peut pas poser un pavé par-dessus son propre trait, et ce sont ces cabochons — pas
  l'épaisseur du trait — qui distinguent l'encadrement d'une feuille de personnage d'un rectangle
  tracé ;
- la **marque de focus** (\ref hmi::MenuEntryButton "MenuEntryButton", \ref hmi::FocusMarker
  "FocusMarker") : une feuille de style change une teinte, elle n'ajoute pas de contenu. Or la
  teinte seule ne dit pas où l'on en est à la manette, faute de pointeur, et ne dit rien du tout à
  qui distingue mal les couleurs (`EX-IHM-071`). Le fleuron est tracé **une seule fois**
  (\ref hmi::paintFocusFleuron "paintFocusFleuron") et appelé des deux côtés : deux tracés séparés
  dériveraient l'un de l'autre à la première retouche, et le joueur verrait deux marques
  différentes là où l'exigence en demande une.

Les deux suivent le patron des icônes du `LOT-56` : une géométrie **pure et testable** décide *quoi*
dessiner, un peintre Qt décide *comment*. Aucun fichier d'image n'est livré — un cadre en PNG
figerait ses couleurs hors des jetons et devrait être réexporté à chaque changement de palette.

## Hiérarchie des surfaces, et espaces de travail (LOT-68)

Le `LOT-57` avait supprimé les **doubles définitions** ; il restait la **saturation**, qui est un
problème distinct. Une commande peut n'exister qu'une fois en code et rester exposée à trois endroits
à l'écran : `ToggleRenderMode` figurait en barre d'outils, au menu Niveau **et** au menu Affichage.

Deux arbitrages en découlent.

**La barre d'outils n'est pas un menu.** \ref hmi::ActionSurface "ActionSurface", porté par le
catalogue d'actions, dit de chaque commande si elle mérite une place permanente à l'écran. La barre
ne garde que la sélection d'outil et ce qui se déclenche au fil du geste ; le reste vit au menu, avec
son raccourci. Un test plafonne le nombre de commandes admises par barre : un plafond qu'on relève
sans y penser ne protège de rien.

**Des espaces, jamais superposés.** L'éditeur affichait ses neuf docks et ses deux barres d'outils
simultanément — une trentaine de contrôles permanents — alors que l'édition de niveau et l'atelier
pixel art ne se pratiquent jamais ensemble. \ref hmi::dressingForWorkspace "dressingForWorkspace" et
\ref hmi::workspacesForPanel "workspacesForPanel" répartissent panneaux, barres et menus entre
espaces exclusifs, chacun persistant **sa** disposition. Ils étaient deux au `LOT-68` ; le
`LOT-69` en ajoute un **troisième**, le mode création, et c'est ce qui a fait passer la seconde
d'une valeur unique à un **masque** — le canevas, l'historique et la palette servent aux deux
espaces de peinture, et dupliquer les docks aurait donné deux canevas à tenir synchronisés.

> À ne pas confondre avec la mise en avant d'`EX-IHM-061`, qui reste une suggestion et ne masque
> jamais rien. Changer d'espace est un acte explicite de l'utilisateur : c'est ce qui rend le
> masquage légitime ici, et lui seul.

Sélectionner un outil bascule sur son espace (`workspaceForTool`). Sans cela, l'outil devient actif
dans un espace qui ne montre ni son canevas ni ses panneaux, et rien à l'écran ne dit pourquoi il ne
répond pas.

## Voir aussi
- @ref guide-ihm-qt — le socle applicatif Qt, le viewport Direct3D 11, la boucle et les entrées.
- @ref guide-atelier-pixel-art — l'atelier qui hérite de ces jetons, actions et zones d'état.
- @ref guide-editeur — l'éditeur de niveau lui-même (brouillon, outils, essai immédiat).
- [Spécification IHM](@ref spec-interface-ihm) — le *quoi/pourquoi* (`EX-IHM-050` → `EX-IHM-062`).
